#ifndef NORLIT_JS_GRAMMAR_NODE_H
#define NORLIT_JS_GRAMMAR_NODE_H

#include "../JSString.h"
#include "../../gc/Handle.h"
#include "../../gc/Array.h"

#include "../../pp/Tuple/ForEach.h"
#include "../../pp/Tuple/Map.h"

namespace norlit {
namespace js {
namespace bytecode {
class Emitter;
}

namespace grammar {

#define NORLIT_PP_EXPAND(...) __VA_ARGS__
#define NORLIT_PP_TUPLE_CALLLER(tuple, callback) NORLIT_PP_EXPAND(callback tuple)
#define NORLIT_PP_FOREACH_TUPLE(callback, tuple) NORLIT_PP_FOREACH(NORLIT_PP_TUPLE_CALLLER, tuple, callback)
#define NORLIT_PP_MAP_TUPLE(callback, tuple) NORLIT_PP_MAP(NORLIT_PP_TUPLE_CALLLER, tuple, callback)

#define DECLARE_NODE_FIELD(type, name) type* name##_ = nullptr;
#define DECLARE_NODE_ACCESSOR(type, name) gc::Handle<type> name(){return name##_;}
#define DECLARE_NODE_CTOR_ARG(type,name) const gc::Handle<type>& name
#define DECLARE_NODE_WRITE_BARRIER(type,name) WriteBarrier(&this->name##_, name);
#define CODEGEN virtual void Codegen(bytecode::Emitter&);
#define DECLGEN virtual void VarDeclGen(bytecode::Emitter&);

#define DECLARE_FIELDS(...) \
	private: NORLIT_PP_FOREACH_TUPLE(DECLARE_NODE_FIELD, (__VA_ARGS__))\
	public: NORLIT_PP_FOREACH_TUPLE(DECLARE_NODE_ACCESSOR, (__VA_ARGS__))

#define NORLIT_AST_CLASS(name, base, fields, extra) class name : public base {\
	DECLARE_FIELDS(NORLIT_PP_EXPAND fields)\
public:\
	NORLIT_PP_IF(NORLIT_PP_TUPLE_ISEMPTY(fields),,\
		name(NORLIT_PP_MAP_TUPLE(DECLARE_NODE_CTOR_ARG, fields)) {\
			NORLIT_PP_FOREACH_TUPLE(DECLARE_NODE_WRITE_BARRIER, fields)\
		}\
		virtual void IterateField(const gc::FieldIterator& iter) override final;\
	)\
	virtual void Dump(size_t ident) override final;\
	extra\
}

#define DECLARE_NODE(name, base, ...) NORLIT_AST_CLASS(name, base, (__VA_ARGS__), )

#define NORLIT_AST_CLASS_C(name, base, ...) NORLIT_AST_CLASS(name, base, (__VA_ARGS__), CODEGEN)
#define NORLIT_AST_CLASS_D(name, base, ...) NORLIT_AST_CLASS(name, base, (__VA_ARGS__), CODEGEN DECLGEN)

class Node : public gc::Object {
  public:
    virtual void Dump(size_t ident) = 0;
};

class Expression: public Node {
  public:
    virtual void Codegen(bytecode::Emitter&) = 0;
};

NORLIT_AST_CLASS_C(
    Identifier, Expression,
    (JSString, name)
);

NORLIT_AST_CLASS_C(
    Literal, Expression,
    (JSValue, literal)
);

NORLIT_AST_CLASS_C(
    RegexpLiteral, Expression,
    (JSString, regexp),
    (JSString, flags)
);

NORLIT_AST_CLASS_C(
    TemplateLiteral, Expression,
    (gc::Array<JSString>, cooked),
    (gc::Array<JSString>, raw),
    (gc::Array<Expression>, subst)
);

NORLIT_AST_CLASS_C(
    CoveredFormals, Expression,
    (gc::Array<Expression>, items)
);

NORLIT_AST_CLASS_C(
    ArrayLiteral, Expression,
    (gc::Array<Expression>, items)
);

class Property final : public Node {
    DECLARE_FIELDS(
        (Expression, key),
        (Expression, value)
    )
  public:
    enum class Type {
        kNormal,
        kMethod,
        kSetter,
        kGetter
    };
  private:
    Type type_;
  public:
    Type type() const {
        return type_;
    }
    Property(const gc::Handle<Expression>& key, const gc::Handle<Expression>& value, Type type) :type_(type) {
        DECLARE_NODE_WRITE_BARRIER(, key);
        DECLARE_NODE_WRITE_BARRIER(, value);
    }
    virtual void IterateField(const gc::FieldIterator& iter) override final;
    virtual void Dump(size_t ident) override final;
    CODEGEN
};

NORLIT_AST_CLASS_C(
    ObjectLiteral, Expression,
    (gc::Array<Property>, items)
);

NORLIT_AST_CLASS_C(ThisExpression, Expression);

NORLIT_AST_CLASS_C(NewTargetExpression, Expression);

NORLIT_AST_CLASS_C(
    PropertyExpression, Expression,
    (Expression, base),
    (Expression, member)
);

NORLIT_AST_CLASS_C(
    SuperPropertyExpression, Expression,
    (Expression, member)
);

class PostfixExpression: public Expression {
    DECLARE_NODE_FIELD(Expression, expr)
    bool inc;
  public:
    DECLARE_NODE_ACCESSOR(Expression, expr)
    bool increment() const {
        return inc;
    }
    bool decrement() const {
        return !inc;
    }
    PostfixExpression(const gc::Handle<Expression>& expr, bool inc):inc(inc) {
        DECLARE_NODE_WRITE_BARRIER(, expr);
    }
    virtual void IterateField(const gc::FieldIterator& iter) override final;
    virtual void Dump(size_t ident) override final;
    CODEGEN
};

class UnaryExpression : public Expression {
    DECLARE_NODE_FIELD(Expression, expr)
    uint16_t op_;
  public:
    DECLARE_NODE_ACCESSOR(Expression, expr)
    uint16_t op() const {
        return op_;
    }
    UnaryExpression(const gc::Handle<Expression>& expr, uint16_t op) :op_(op) {
        DECLARE_NODE_WRITE_BARRIER(,expr);
    }
    virtual void IterateField(const gc::FieldIterator& iter) override final;
    virtual void Dump(size_t ident) override final;
    CODEGEN
};

class BinaryExpression : public Expression {
    DECLARE_FIELDS(
        (Expression, left),
        (Expression, right)
    )
  private:
    uint16_t op_;
  public:
    uint16_t op() const {
        return op_;
    }
    BinaryExpression(const gc::Handle<Expression>& left, const gc::Handle<Expression>& right, uint16_t op) :op_(op) {
        DECLARE_NODE_WRITE_BARRIER(, left);
        DECLARE_NODE_WRITE_BARRIER(, right);
    }
    virtual void IterateField(const gc::FieldIterator& iter) override final;
    virtual void Dump(size_t ident) override final;
    CODEGEN
};

NORLIT_AST_CLASS_C(
    ConditionalExpression, Expression,
    (Expression, cond),
    (Expression, then),
    (Expression, otherwise)
);

NORLIT_AST_CLASS_C(
    SpreadExpression, Expression,
    (Expression, expr)
);

NORLIT_AST_CLASS_C(
    CallExpression, Expression,
    (Expression, callee),
    (gc::Array<Expression>, args)
);

NORLIT_AST_CLASS_C(
    SuperCallExpression, Expression,
    (gc::Array<Expression>, args)
);

NORLIT_AST_CLASS_C(
    NewExpression, Expression,
    (Expression, ctor),
    (gc::Array<Expression>, args)
);

NORLIT_AST_CLASS_C(
    TaggedTemplateExpression, Expression,
    (Expression, callee),
    (TemplateLiteral, arg)
);

NORLIT_AST_CLASS_C(
    YieldExpression, Expression,
    (Expression, expr)
);

NORLIT_AST_CLASS_C(
    YieldAllExpression, Expression,
    (Expression, expr)
);

/* Statements and Declarations */
class Statement : public gc::Object {
  public:
    virtual void Dump(size_t ident) = 0;
    virtual void Codegen(bytecode::Emitter&);
    virtual void VarDeclGen(bytecode::Emitter&);
    virtual void LexDeclGen(bytecode::Emitter&);
};

NORLIT_AST_CLASS_D (
    BlockStatement, Statement,
    (gc::Array<Statement>, items)
);

class VariableDeclaration : public Statement {
    DECLARE_FIELDS(
        (gc::Array<Expression>, decl)
    )
  public:
    enum class Type {
        kVar,
        kConst,
        kLet,
    };
  private:
    Type type_;
  public:
    Type type() const {
        return type_;
    }
    VariableDeclaration(const gc::Handle<gc::Array<Expression>>& decl, Type type) :type_(type) {
        DECLARE_NODE_WRITE_BARRIER(, decl);
    }
    virtual void IterateField(const gc::FieldIterator& iter) override final;
    virtual void Dump(size_t ident) override final;
    CODEGEN
    DECLGEN
    virtual void LexDeclGen(bytecode::Emitter&);
};

NORLIT_AST_CLASS_D(
    EmptyStatement, Statement
);

NORLIT_AST_CLASS_D (
    ExpressionStatement, Statement,
    (Expression, expr)
);

NORLIT_AST_CLASS_D(
    IfStatement, Statement,
    (Expression, cond),
    (Statement, then),
    (Statement, otherwise)
);

NORLIT_AST_CLASS_D(
    DoStatement, Statement,
    (Statement, body),
    (Expression, cond)
);

NORLIT_AST_CLASS_D(
    WhileStatement, Statement,
    (Expression, cond),
    (Statement, body)
);

DECLARE_NODE(
    ContinueStatement, Statement,
    (JSString, label)
);

DECLARE_NODE(
    BreakStatement, Statement,
    (JSString, label)
);

NORLIT_AST_CLASS_D (
    ReturnStatement, Statement,
    (Expression, expr)
);

DECLARE_NODE(
    WithStatement, Statement,
    (Expression, base),
    (Statement, body)
);

DECLARE_NODE(
    SwitchClause, Node,
    (Expression, cond),
    (gc::Array<Statement>, body)
);

DECLARE_NODE(
    SwitchStatement, Statement,
    (Expression, expr),
    (gc::Array<SwitchClause>, clauses)
);

// Switch

DECLARE_NODE(
    LabelledStatement, Statement,
    (JSString, label),
    (Statement, body)
);

NORLIT_AST_CLASS_D(
    ThrowStatement, Statement,
    (Expression, expr)
);

NORLIT_AST_CLASS_D(
    TryStatement, Statement,
    (BlockStatement, body),
    (Expression, param),
    (BlockStatement, error),
    (BlockStatement, finally)
);

NORLIT_AST_CLASS_D(DebuggerStatement, Statement);

// Not in ECMAScript specification. This is a special type of ExpressionStatement
NORLIT_AST_CLASS_D(
    DirectiveStatement, Statement,
    (JSString, value)
);


class FunctionExpression : public Expression {
    DECLARE_FIELDS(
        (JSString, name),
        (gc::Array<Expression>, param),
        (gc::Array<Statement>, body)
    )
  private:
    bool generator;
  public:
    uint16_t isGenerator() const {
        return generator;
    }
    FunctionExpression(
        bool generator,
        const gc::Handle<JSString>& name,
        const gc::Handle<gc::Array<Expression>>& param,
        const gc::Handle<gc::Array<Statement>>& body
    ) :generator(generator) {
        DECLARE_NODE_WRITE_BARRIER(, name);
        DECLARE_NODE_WRITE_BARRIER(, param);
        DECLARE_NODE_WRITE_BARRIER(, body);
    }
    virtual void IterateField(const gc::FieldIterator& iter) override final;
    virtual void Dump(size_t ident) override final;
    CODEGEN

    friend class FunctionStatement;
};

NORLIT_AST_CLASS_D (
    FunctionStatement, Statement,
    (FunctionExpression, func)
);

NORLIT_AST_CLASS(
    Script, Node,
    (
        (gc::Array<Statement>, body)
    ),
    CODEGEN
);

}
}
}

#undef DECLARE_NODE_FIELD
#undef DECLARE_NODE_ACCESSOR
#undef DECLARE_NODE_CTOR_ARG
#undef DECLARE_NODE_WRITE_BARRIER
#undef DECLARE_FIELDS
#undef DECLARE_NODE

#endif
