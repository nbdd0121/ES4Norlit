#include "Node.h"

#include "../JSString.h"
#include "../JSNumber.h"
#include "../JSBoolean.h"
#include <cstdio>

#include "../../pp/Tuple/ForEach.h"
#include "Token.h"

using namespace norlit::gc;
using namespace norlit::js::grammar;

static const char* typeToString(uint16_t type) {
    switch (type) {
        case Token::kMulAssign:
            return "*=";
        case Token::kDivAssign:
            return "/=";
        case Token::kModAssign:
            return "%=";
        case Token::kAddAssign:
            return "+=";
        case Token::kSubAssign:
            return "-=";
        case Token::kLShiftAssign:
            return "<<=";
        case Token::kRShiftAssign:
            return ">>=";
        case Token::kURShiftAssign:
            return ">>>=";
        case Token::kAndAssign:
            return "&=";
        case Token::kXorAssign:
            return "^=";
        case Token::kOrAssign:
            return "|=";
        default:
            return nullptr;
    }
}

#define IDENT(x) printf("\n%*s", static_cast<int>(ident + x), "")

#define CREATE_ITERATOR_ITEM(x,discard) \
	iter(&NORLIT_PP_CONCAT_2(x,_));

#define CREATE_ITERATOR(clazz, ...) \
	void clazz::IterateField(const FieldIterator& iter) {\
		NORLIT_PP_FOREACH(CREATE_ITERATOR_ITEM, (__VA_ARGS__),)\
	}

#define CREATE_DUMP_ITEM(x,discard) \
	if(thisPtr->NORLIT_PP_CONCAT_2(x,_)) {\
		IDENT(2);\
		thisPtr->NORLIT_PP_CONCAT_2(x,_)->Dump(ident + 2);\
	}

#define CREATE_DUMP(clazz, ...) \
	void clazz::Dump(size_t ident) {\
		Handle<clazz> thisPtr = this;\
		printf(#clazz);\
		NORLIT_PP_FOREACH(CREATE_DUMP_ITEM, (__VA_ARGS__),)\
	}

CREATE_ITERATOR(Identifier, name);
CREATE_ITERATOR(CoveredFormals, items);
CREATE_ITERATOR(ArrayLiteral, items);
CREATE_ITERATOR(ObjectLiteral, items);
CREATE_ITERATOR(Property, key, value);
CREATE_ITERATOR(Literal, literal);
CREATE_ITERATOR(RegexpLiteral, regexp, flags);
CREATE_ITERATOR(TemplateLiteral, cooked, raw, subst);
CREATE_ITERATOR(PropertyExpression, base, member);
CREATE_ITERATOR(SuperPropertyExpression, member);
CREATE_ITERATOR(PostfixExpression, expr);
CREATE_ITERATOR(UnaryExpression, expr);
CREATE_ITERATOR(BinaryExpression, left, right);
CREATE_ITERATOR(ConditionalExpression, cond, then, otherwise);
CREATE_ITERATOR(SpreadExpression, expr);
CREATE_ITERATOR(CallExpression, callee, args);
CREATE_ITERATOR(SuperCallExpression, args);
CREATE_ITERATOR(NewExpression, ctor, args);
CREATE_ITERATOR(TaggedTemplateExpression, callee, arg);
CREATE_ITERATOR(YieldExpression, expr);
CREATE_ITERATOR(YieldAllExpression, expr);

CREATE_ITERATOR(BlockStatement, items);
CREATE_ITERATOR(VariableDeclaration, decl);
CREATE_ITERATOR(ExpressionStatement, expr);
CREATE_ITERATOR(IfStatement, cond, then, otherwise);
CREATE_ITERATOR(DoStatement, body, cond);
CREATE_ITERATOR(WhileStatement, cond, body);
CREATE_ITERATOR(ContinueStatement, label);
CREATE_ITERATOR(BreakStatement, label);
CREATE_ITERATOR(ReturnStatement, expr);
CREATE_ITERATOR(WithStatement, base, body);
CREATE_ITERATOR(SwitchClause, cond, body);
CREATE_ITERATOR(SwitchStatement, expr, clauses);
CREATE_ITERATOR(LabelledStatement, label, body);
CREATE_ITERATOR(ThrowStatement, expr);
CREATE_ITERATOR(TryStatement, body, param, error, finally);

CREATE_ITERATOR(DirectiveStatement, value);
CREATE_ITERATOR(FunctionExpression, name, param, body);
CREATE_ITERATOR(FunctionStatement, func);

CREATE_ITERATOR(Script, body);

void Identifier::Dump(size_t ident) {
    printf("Identifier %s", &name_->ToCString()->At(0));
}

void CoveredFormals::Dump(size_t ident) {
    Handle<CoveredFormals> thisPtr = this;
    printf("CoveredFormals");
    for (size_t i = 0, len = thisPtr->items_->Length(); i < len; i++) {
        IDENT(2);
        thisPtr->items_->Get(i)->Dump(ident+2);
    }
}

void ArrayLiteral::Dump(size_t ident) {
    Handle<ArrayLiteral> thisPtr = this;
    Handle<Expression> item;
    printf("ArrayLiteral");
    for (size_t i = 0, len = thisPtr->items_->Length(); i < len; i++) {
        IDENT(2);
        item = thisPtr->items_->Get(i);
        if (item)
            item->Dump(ident + 2);
        else {
            printf("Elision");
        }
    }
}

void ObjectLiteral::Dump(size_t ident) {
    Handle<ObjectLiteral> thisPtr = this;
    Handle<Property> item;
    printf("ObjectLiteral");
    for (size_t i = 0, len = thisPtr->items_->Length(); i < len; i++) {
        IDENT(2);
        item = thisPtr->items_->Get(i);
        item->Dump(ident + 2);
    }
}

void Property::Dump(size_t ident) {
    Handle<Property> thisPtr = this;
    switch (type_) {
        case Type::kMethod:
            printf("Method");
        case Type::kGetter:
            printf("Get");
        case Type::kSetter:
            printf("Set");
        default:
            printf("Property");
            break;
    }
    IDENT(2);
    key_->Dump(ident+2);
    IDENT(2);
    thisPtr->value_->Dump(ident+2);
}

void Literal::Dump(size_t ident) {
    switch (literal_->GetType()) {
        case JSValue::Type::kString: {
            Handle<JSString> str = (JSString*)literal_;
            printf("StringLiteral %s", &str->ToCString()->At(0));
            break;
        }
        case JSValue::Type::kNumber: {
            Handle<JSNumber> val = (JSNumber*)literal_;
            printf("NumberLiteral %lf", val->Value());
            break;
        }
        case JSValue::Type::kBoolean: {
            Handle<JSBoolean> val = (JSBoolean*)literal_;
            printf("BooleanLiteral %s", val->Value()?"true":"false");
            break;
        }
        case JSValue::Type::kNull: {
            printf("NullLiteral");
            break;
        }
        default:
            throw "UNK";
    }
}

void RegexpLiteral::Dump(size_t ident) {
    Handle<RegexpLiteral> thisPtr = this;
    Handle<ValueArray<char>> rstr = this->regexp_->ToCString();
    Handle<ValueArray<char>> fstr = thisPtr->flags_->ToCString();

    printf("RegexpLiteral /%s/%s", &rstr->At(0), &fstr->At(0));

}

void TemplateLiteral::Dump(size_t ident) {
    Handle<TemplateLiteral> thisPtr = this;
    printf("TemplateLiteral");
    IDENT(2);
    printf("Cooked %s, ", &thisPtr->cooked_->Get(0)->ToCString()->At(0));
    printf("Raw %s", &thisPtr->raw_->Get(0)->ToCString()->At(0));
    if (thisPtr->subst_) {
        for (size_t i = 0, len = thisPtr->subst_->Length(); i < len; i++) {
            IDENT(2);
            thisPtr->subst_->Get(i)->Dump(ident + 2);
            IDENT(2);
            printf("Cooked %s, ", &thisPtr->cooked_->Get(i+1)->ToCString()->At(0));
            printf("Raw %s", &thisPtr->raw_->Get(i+1)->ToCString()->At(0));
        }
    }
}

CREATE_DUMP(NewTargetExpression);
CREATE_DUMP(ThisExpression);
CREATE_DUMP(PropertyExpression, base, member);
CREATE_DUMP(SuperPropertyExpression, member);

void PostfixExpression::Dump(size_t ident) {
    printf("PostfixExpression %s", inc?"++":"--");
    IDENT(2);
    expr_->Dump(ident + 2);
}

void UnaryExpression::Dump(size_t ident) {
    printf("UnaryExpression ");
    switch (op_) {
        case Token::kDelete:
            printf("delete");
            break;
        case Token::kVoid:
            printf("void");
            break;
        case Token::kTypeof:
            printf("typeof");
            break;
        case Token::kInc:
            printf("++");
            break;
        case Token::kDec:
            printf("--");
            break;
        default:
            printf("%c", op_);
            break;
    }
    IDENT(2);
    expr_->Dump(ident + 2);
}

void BinaryExpression::Dump(size_t ident) {
    printf("BinaryExpression ");
    switch (op_) {
        case Token::kLShift:
            printf("<<");
            break;
        case Token::kRShift:
            printf(">>");
            break;
        case Token::kURShift:
            printf(">>>");
            break;
        case Token::kIn:
            printf("in");
            break;
        case Token::kLteq:
            printf("<=");
            break;
        case Token::kGteq:
            printf(">=");
            break;
        case Token::kInstanceof:
            printf("instanceof");
            break;
        case Token::kEq:
            printf("==");
            break;
        case Token::kIneq:
            printf("!=");
            break;
        case Token::kStrictEq:
            printf("===");
            break;
        case Token::kStrictIneq:
            printf("!==");
            break;
        case Token::kLAnd:
            printf("&&");
            break;
        case Token::kLOr:
            printf("||");
            break;
        default:
            if (typeToString(op_)) {
                printf("%s", typeToString(op_));
            } else {
                printf("%c", op_);
            }
            break;
    }
    // left_->Dump() may trigger GC
    Handle<Expression> right = right_;
    IDENT(2);
    left_->Dump(ident + 2);
    IDENT(2);
    right->Dump(ident + 2);
}

CREATE_DUMP(ConditionalExpression, cond, then, otherwise);
CREATE_DUMP(SpreadExpression, expr);

void CallExpression::Dump(size_t ident) {
    Handle<CallExpression> thisPtr = this;
    printf("CallExpression");
    IDENT(2);
    callee_->Dump(ident + 2);
    for (size_t i = 0, len=thisPtr->args_->Length(); i < len; i++) {
        IDENT(2);
        thisPtr->args_->Get(i)->Dump(ident + 2);
    }
}

void SuperCallExpression::Dump(size_t ident) {
    Handle<SuperCallExpression> thisPtr = this;
    printf("SuperCallExpression");
    for (size_t i = 0, len = thisPtr->args_->Length(); i < len; i++) {
        IDENT(2);
        thisPtr->args_->Get(i)->Dump(ident + 2);
    }
}

void NewExpression::Dump(size_t ident) {
    Handle<NewExpression> thisPtr = this;
    printf("NewExpression");
    IDENT(2);
    ctor_->Dump(ident + 2);
    if (thisPtr->args_) {
        for (size_t i = 0, len = thisPtr->args_->Length(); i < len; i++) {
            IDENT(2);
            thisPtr->args_->Get(i)->Dump(ident + 2);
        }
    }
}

CREATE_DUMP(TaggedTemplateExpression, callee, arg);
CREATE_DUMP(YieldExpression, expr);
CREATE_DUMP(YieldAllExpression, expr);


void BlockStatement::Dump(size_t ident) {
    Handle<BlockStatement> thisPtr = this;
    printf("BlockStatement");
    for (size_t i = 0, len = thisPtr->items_->Length(); i < len; i++) {
        IDENT(2);
        thisPtr->items_->Get(i)->Dump(ident + 2);
    }
}

void VariableDeclaration::Dump(size_t ident) {
    Handle<VariableDeclaration> thisPtr = this;
    printf("VariableDeclaration %s", type_==Type::kConst?"const":(type_==Type::kLet?"let":"var"));
    for (size_t i = 0, len = thisPtr->decl_->Length(); i < len; i++) {
        IDENT(2);
        thisPtr->decl_->Get(i)->Dump(ident + 2);
    }
}


CREATE_DUMP(EmptyStatement);
CREATE_DUMP(ExpressionStatement, expr);
CREATE_DUMP(IfStatement, cond, then, otherwise);
CREATE_DUMP(DoStatement, body, cond);
CREATE_DUMP(WhileStatement, cond, body);

void ContinueStatement::Dump(size_t ident) {
    printf("ContinueStatement");
    if(label_)printf(" %s", &label_->ToCString()->At(0));
}

void BreakStatement::Dump(size_t ident) {
    printf("BreakStatement");
    if (label_)printf(" %s", &label_->ToCString()->At(0));
}

CREATE_DUMP(ReturnStatement, expr);
CREATE_DUMP(WithStatement, base, body);

void SwitchClause::Dump(size_t ident) {
    Handle<SwitchClause> thisPtr = this;
    printf("SwitchClause");
    IDENT(2);
    if (cond_) {
        cond_->Dump(ident+2);
    } else {
        printf("default");
    }
    for (size_t i = 0, len = thisPtr->body_->Length(); i < len; i++) {
        IDENT(2);
        thisPtr->body_->Get(i)->Dump(ident + 2);
    }
}

void SwitchStatement::Dump(size_t ident) {
    Handle<SwitchStatement> thisPtr = this;
    printf("SwitchStatement");
    IDENT(2);
    expr_->Dump(ident + 2);
    for (size_t i = 0, len = thisPtr->clauses_->Length(); i < len; i++) {
        IDENT(2);
        thisPtr->clauses_->Get(i)->Dump(ident + 2);
    }
}

void LabelledStatement::Dump(size_t ident) {
    Handle<LabelledStatement> thisPtr = this;
    printf("LabelledStatement %s", &label_->ToCString()->At(0));
    IDENT(2);
    thisPtr->body_->Dump(ident + 2);
}

CREATE_DUMP(ThrowStatement, expr);
CREATE_DUMP(TryStatement, body, param, error, finally);
CREATE_DUMP(DebuggerStatement);

void DirectiveStatement::Dump(size_t ident) {
    printf("DirectiveStatement '%s'", &value_->ToCString()->At(0));
}

void FunctionExpression::Dump(size_t ident) {
    Handle<FunctionExpression> thisPtr = this;
    printf("FunctionExpression%s", generator?"*":"");
    if (name_) {
        printf(" %s", &this->name_->ToCString()->At(0));
    }
    IDENT(2);
    printf("Parameters");
    for (size_t i = 0, len = thisPtr->param_->Length(); i < len; i++) {
        IDENT(4);
        thisPtr->param_->Get(i)->Dump(ident + 4);
    }
    IDENT(2);
    printf("Body");
    for (size_t i = 0, len = thisPtr->body_->Length(); i < len; i++) {
        IDENT(4);
        thisPtr->body_->Get(i)->Dump(ident + 4);
    }
}

CREATE_DUMP(FunctionStatement, func);

void Script::Dump(size_t ident) {
    Handle<Script> thisPtr = this;
    printf("Script");
    for (size_t i = 0, len = thisPtr->body_->Length(); i < len; i++) {
        IDENT(2);
        thisPtr->body_->Get(i)->Dump(ident + 2);
    }
}