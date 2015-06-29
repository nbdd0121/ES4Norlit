#include <iostream>
#include <fstream>

#include <cstdlib>
#include <cstring>
#include <string>

#include "norlit/gc/Heap.h"
#include "norlit/gc/Handle.h"
#include "norlit/gc/Array.h"

#include "norlit/meta/Math.h"

#include "norlit/js/JSNumber.h"
#include "norlit/js/JSBoolean.h"
#include "norlit/js/JSString.h"
#include "norlit/js/JSSymbol.h"
#include "norlit/js/Objects.h"
#include "norlit/js/Testing.h"
#include "norlit/js/Conversion.h"

#include "norlit/util/HashMap.h"
#include "norlit/util/ArrayList.h"

#include "norlit/js/grammar/Scanner.h"
#include "norlit/js/grammar/Parser.h"

#include "norlit/js/object/JSOrdinaryObject.h"
#include "norlit/js/object/JSFunction.h"

#include "norlit/js/bytecode/Emitter.h"
#include "norlit/js/bytecode/Code.h"

#include "norlit/js/vm/Realm.h"
#include "norlit/js/vm/Context.h"
#include "norlit/js/vm/Environment.h"

#include "norlit/js/Exception.h"

using namespace norlit::gc;
using namespace norlit::meta;
using namespace norlit::js;
using namespace norlit::js::object;
using namespace norlit::js::grammar;
using namespace norlit::js::bytecode;
using namespace norlit::js::vm;
using namespace norlit::util;

#include <unordered_map>
#include <typeinfo>
#include <cstdio>

#ifndef _MSC_VER
#include <cxxabi.h>
#endif

struct HeapDumper: public HeapIterator {
    mutable std::unordered_map<const std::type_info*, int> instance_counts;

    ~HeapDumper() {
        for (auto&& pair : instance_counts) {
#ifdef _MSC_VER
            printf("%7d %s\n", pair.second, pair.first->name());
#else
            int status;
            char* name = abi::__cxa_demangle(pair.first->name(), 0, 0, &status);
            printf("%7d %s\n", pair.second, name);
            free(name);
#endif
        }
    }

    virtual void operator()(Object* object) const override final {
        ++instance_counts[&typeid(*object)];
    }
};

struct ReferenceDumper : public HeapIterator, public FieldIterator {
    mutable Object* object;

    ReferenceDumper() {
        printf("digraph ref {\n  node [shape=box];\n");
    }

    ~ReferenceDumper() {
        printf("}\n");
    }

    virtual void operator()(Object** field) const override final {
        if (*field)
            printf("  \"%p\" -> \"%p\";\n", object, *field);
    }

    virtual void operator()(Object** field, decltype(weak)) const override final {
        if (*field)
            printf("  \"%p\" -> \"%p\" [style=dashed];\n", object, *field);
    }

    virtual void operator()(Object* object) const override final {
        printf("  \"%p\" [label=\"%s\"];\n", object, typeid(*object).name());
        this->object = object;
        object->IterateField(*this);
    }
};

#include "norlit/js/grammar/Token.h"
#include "norlit/js/grammar/Node.h"

int main(int argc, char* argv[]) {
    std::string text;
    Optional<std::ifstream> file;
    std::istream* target;
    if (argc >= 2) {
        file = std::ifstream{ argv[1] };
        target = &*file;
    } else {
        file = std::ifstream{ "test.js" };
        target = &*file;
        //target = &std::cin;
    }
    if (!*target) {
        printf("Cannot open file.\n");
        return 1;
    }


    while (!target->eof()) {
        std::string buffer;
        std::getline(*target, buffer);
        text += buffer;
        text += "\n";
    }

    Handle<Realm> realm = new Realm();

    try {
        Handle<Context> guard = new Context(realm, nullptr, nullptr);
        Context::PushContext(guard);

        Scanner lex(JSString::NewFromCString(text.c_str()));
        Parser gram{ lex };
        Handle<Script> expr = gram.ParseScript();

        //printf("---AST---\n");
        //expr->Dump(0);

        //printf("\n---Bytecode---\n");
        Emitter e;
        expr->Codegen(e);
        Handle<Code> c = e.ToCode();
        //c->Dump();
        //printf("\n");

        Handle<JSObject> global = realm->GetGlobalObject();
        {
            Handle<ESNativeFunction> func = new ESNativeFunction(realm->FunctionPrototype(), realm,
            [] (const Handle<JSValue>&, const Handle<Array<JSValue>>& args)->Handle<JSValue> {
                for (size_t i = 0, size = args->Length(); i < size; i++) {
                    Handle<JSValue> result = args->Get(i);
                    if (Testing::Is<JSSymbol>(result)) {
                        Handle<JSString> desc = result.CastTo<JSSymbol>()->Descriptor();
                        if (desc)
                            printf("Symbol(%s)\n", &desc->ToCString()->At(0));
                        else
                            printf("Symbol()\n");
                    } else {
                        printf("%s\n", &Conversion::ToString(result)->ToCString()->At(0));
                    }
                }
                return nullptr;
            }, nullptr);

            Handle<JSOrdinaryObject> console = new JSOrdinaryObject(realm->ObjectPrototype());
            Objects::Set(console, JSString::New("log"), func, true);
            Objects::CreateDataProperty(func, "name", JSString::New("console.log"));
            Objects::Set(global, JSString::New("console"), console, true);
        }
        Objects::Set(global, JSString::New("version"), JSString::New("NorlitJS Engine V0.0.1"), true);
        Handle<GlobalEnvironment> genv = realm->GetGlobalEnvironment();
        Handle<BytecodeContext> ctx = new BytecodeContext(genv, genv, realm, nullptr, c);
        Context::PopContext(); // Pop the guard
        Context::PushContext(ctx);
        ctx->Run();
        return 0;
    } catch (const char* error) {
        printf("%s\n", error);
    } catch (std::string& str) {
        std::cout << str << std::endl;
    } catch (ESException& e) {
        Handle<JSValue> exception = e.value();
        try {
            if (Handle<JSValue> stack = Objects::GetV(exception, "stack")) {
                exception = stack;
            }
            printf("Uncaught ECMAScript Exception\n%s\n", &Conversion::ToString(exception)->ToCString()->At(0));
        } catch (ESException&) {
            printf("Uncaught ECMAScript Exception\n%s\n", &Conversion::ToString(exception)->ToCString()->At(0));
        }
    }
    return 1;
}





