#ifndef NORLIT_JS_COMMON_H
#define NORLIT_JS_COMMON_H


#define NORLIT_DEFINE_FIELD_POD(type, name) \
  private:\
	type name##_;\
  public:\
	type name() const {return name##_;} \
	void name(type name) {name##_ = name;}

#define NORLIT_DEFINE_FIELD(type, name) \
  private:\
	type* name##_ = nullptr;\
  public:\
	gc::Handle<type> name() const {return name##_;} \
	void name(const gc::Handle<type>& name) {this->WriteBarrier(&this->name##_, name);}

namespace norlit {
namespace js {

class JSSymbol;

namespace vm {
class BytecodeContext;
}
}
}

#endif