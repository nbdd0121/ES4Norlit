#include "Emitter.h"
#include "Instruction.h"
#include "../grammar/Node.h"
#include "../grammar/Token.h"

#include "../JSNumber.h"

#include <typeinfo>
#include <cstdio>

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::grammar;
using namespace norlit::js::bytecode;
using namespace norlit::util;

namespace {

void ConvertTop2ToNum(Emitter& emitter) {
    emitter.Emit(Instruction::kXchg);
    emitter.Emit(Instruction::kNum);
    emitter.Emit(Instruction::kXchg);
    emitter.Emit(Instruction::kNum);
}

Handle<Expression> TryToLvalue(const Handle<Expression>& expr) {
    const std::type_info& targetType = typeid(*expr);
    if (targetType == typeid(PropertyExpression) ||
            targetType == typeid(Identifier) ||
            targetType == typeid(SuperPropertyExpression)) {
        return expr;
    } else if (targetType == typeid(CoveredFormals)) {
        Handle<Array<Expression>> items = expr.CastTo<CoveredFormals>()->items();
        if (items->Length() != 1) {
            return nullptr;
        }
        Handle<Expression> ret = items->Get(0);
        if (typeid(*ret) == typeid(SpreadExpression)) {
            return nullptr;
        }
        return TryToLvalue(ret);
    } else {
        return nullptr;
    }
}

Handle<Expression> ToSimpleAssignmentTarget(const Handle<Expression>& expr) {
    Handle<Expression> ret = TryToLvalue(expr);
    if (!ret) {
        throw "Invalid simple assignment target";
    }
    return ret;
}

}

/* Variable Declaration Code Generation */

static void GenerateDefinition(const Handle<Expression>& expr, Emitter& emitter, VariableDeclaration::Type type) {
    if (Handle<BinaryExpression> pair = expr.ExactCheckedCastTo<BinaryExpression>()) {
        assert(pair->op() == '=');
        GenerateDefinition(pair->left(), emitter, type);
        return;
    }
    if (Handle<Identifier> id = expr.ExactCheckedCastTo<Identifier>()) {
        size_t index = emitter.EmitConstant(id->name());
        switch (type) {
            case VariableDeclaration::Type::kVar:
                emitter.Emit(Instruction::kDefVar);
                break;
            case VariableDeclaration::Type::kLet:
                emitter.Emit(Instruction::kDefLet);
                break;
            case VariableDeclaration::Type::kConst:
                emitter.Emit(Instruction::kDefConst);
                break;
        }
        emitter.Emit16(index);
    } else {
        throw "TODO: var [a], {a};";
    }
}

// Bind the top of stack into a binding pattern
static void BindIntoPattern(const Handle<Expression>& lhs, Emitter& emitter) {
    if (Handle<Identifier> id = lhs.ExactCheckedCastTo<Identifier>()) {
        size_t index = emitter.EmitConstant(id->name());
        emitter.Emit(Instruction::kInitDef);
        emitter.Emit16(index);
    } else {
        throw "TODO: let [a] = xx;";
    }
}

// Bind the top of stack into a binding pattern
static void AssignIntoPattern(const Handle<Expression>& lhs, Emitter& emitter) {
    if (Handle<Identifier> id = lhs.ExactCheckedCastTo<Identifier>()) {
        size_t index = emitter.EmitConstant(id->name());
        emitter.Emit(Instruction::kPutName);
        emitter.Emit16(index);
        emitter.Emit(Instruction::kPop);
    } else {
        throw "TODO: var [a] = xx;";
    }
}

static void GenerateVarAssignment(const Handle<Expression>& expr, Emitter& emitter, VariableDeclaration::Type type) {
    if (Handle<BinaryExpression> pair = expr.ExactCheckedCastTo<BinaryExpression>()) {
        assert(pair->op() == '=');
        pair->right()->Codegen(emitter);
        if (type == VariableDeclaration::Type::kVar) {
            AssignIntoPattern(pair->left(), emitter);
        } else {
            BindIntoPattern(pair->left(), emitter);
        }
    } else {
        // Var is initialized at first
        if (type == VariableDeclaration::Type::kVar) return;

        emitter.Emit(Instruction::kUndef);
        BindIntoPattern(expr, emitter);
    }
}

void VariableDeclaration::Codegen(Emitter& emitter) {
    VariableDeclaration::Type type = this->type_;
    Handle<Array<Expression>> items = this->decl_;
    for (size_t i = 0, length = items->Length(); i < length; i++) {
        GenerateVarAssignment(items->Get(i), emitter, type);
    }
}

void VariableDeclaration::VarDeclGen(Emitter& emitter) {
    if (this->type_ != Type::kVar) {
        return;
    }
    Handle<Array<Expression>> items = decl_;
    for (size_t i = 0, length = items->Length(); i < length; i++) {
        GenerateDefinition(items->Get(i), emitter, VariableDeclaration::Type::kVar);
    }
}


void VariableDeclaration::LexDeclGen(Emitter& emitter) {
    if (this->type_ == Type::kVar) {
        return;
    }
    VariableDeclaration::Type type = this->type_;
    Handle<Array<Expression>> items = decl_;
    for (size_t i = 0, length = items->Length(); i < length; i++) {
        GenerateDefinition(items->Get(i), emitter, type);
    }
}

/* Normal Code Generation */

// the namespace is just for folding
namespace norlit {
namespace js {
namespace grammar {

void Literal::Codegen(Emitter& emitter) {
    size_t id = emitter.EmitConstant(literal_);
    emitter.Emit(Instruction::kLoad);
    emitter.Emit16(id);
}

void Identifier::Codegen(Emitter& emitter) {
    size_t id = emitter.EmitConstant(this->name_);
    emitter.Emit(Instruction::kGetName);
    emitter.Emit16(id);
}

void TemplateLiteral::Codegen(Emitter& emitter) {
    Handle<TemplateLiteral> self = this;
    size_t id = emitter.EmitConstant(self->cooked_->Get(0));
    emitter.Emit(Instruction::kLoad);
    emitter.Emit16(id);
    if (!self->subst_) {
        return;
    }
    for (size_t i = 0, size = self->subst_->Length(); i < size; i++) {
        self->subst_->Get(i)->Codegen(emitter);
        emitter.Emit(Instruction::kStr);
        emitter.Emit(Instruction::kConcat);

        id = emitter.EmitConstant(self->cooked_->Get(i + 1));
        emitter.Emit(Instruction::kLoad);
        emitter.Emit16(id);
        emitter.Emit(Instruction::kConcat);
    }
}

void ThisExpression::Codegen(Emitter& emitter) {
    emitter.Emit(Instruction::kThis);
}

void CoveredFormals::Codegen(Emitter& emitter) {
    Handle<Array<Expression>> items = items_;
    size_t length = items->Length();
    if (length == 0) {
        throw "Expression expected in parenthesis expression";
    }
    Handle<Expression> last = items->Get(length - 1);
    if (typeid(*last) == typeid(SpreadExpression)) {
        throw "Rest parameter should not appear in parenthesis expression";
    }
    for (size_t i = 0; i < length - 1; i++) {
        items->Get(i)->Codegen(emitter);
        emitter.Emit(Instruction::kPop);
    }
    last->Codegen(emitter);
}

void RegexpLiteral::Codegen(Emitter& emitter) {
    throw "TODO";
}

void ArrayLiteral::Codegen(Emitter& emitter) {
    Handle<Array<Expression>> items = this->items_;
    emitter.Emit(Instruction::kArrayStart);
    for (size_t i = 0, len = items->Length(); i < len; i++) {
        Handle<Expression> expr = items->Get(i);
        if (expr) {
            expr->Codegen(emitter);
        } else {
            emitter.Emit(Instruction::kArrayElision);
        }
    }
    emitter.Emit(Instruction::kArray);
}

void ObjectLiteral::Codegen(Emitter& emitter) {
    emitter.Emit(Instruction::kCreateObject);
    for (Handle<Property> prop: this->items_->GetIterable()) {
        prop->Codegen(emitter);
    }
}

void Property::Codegen(Emitter& emitter) {
    Handle<Property> self = this;
    switch (self->type_) {
        case Type::kNormal:
            self->key_->Codegen(emitter);
            self->value_->Codegen(emitter);
            emitter.Emit(Instruction::kCreateDataProperty);
            break;
        default:
            throw("TODO: Property\n");
    }
}

void CallExpression::Codegen(Emitter& emitter) {
    Handle<Expression> callee = this->callee_;
    Handle<Array<Expression>> args = this->args_;
    Handle<Expression> lvalCallee = TryToLvalue(callee);
    if (!lvalCallee) {
        callee->Codegen(emitter);
        emitter.Emit(Instruction::kThis);
    } else {
        const std::type_info& targetType = typeid(*lvalCallee);
        if (targetType == typeid(PropertyExpression)) {
            Handle<PropertyExpression> prop = lvalCallee.CastTo<PropertyExpression>();
            prop->base()->Codegen(emitter);
            emitter.Emit(Instruction::kDup);
            prop->member()->Codegen(emitter);
            emitter.Emit(Instruction::kGetProperty);
            emitter.Emit(Instruction::kXchg);
        } else if (targetType == typeid(Identifier)) {
            size_t id = emitter.EmitConstant(lvalCallee.CastTo<Identifier>()->name());
            emitter.Emit(Instruction::kGetName);
            emitter.Emit16(id);
            emitter.Emit(Instruction::kImplicitThis);
            emitter.Emit16(id);
        } else {
            throw "TODO";
        }
    }
    emitter.Emit(Instruction::kArrayStart);
    for (size_t i = 0, size = args->Length(); i < size; i++) {
        Handle<Expression> expr = args->Get(i);
        expr->Codegen(emitter);
    }
    emitter.Emit(Instruction::kCall);
}

void NewExpression::Codegen(Emitter& emitter) {
    Handle<Expression> ctor = this->ctor_;
    Handle<Array<Expression>> args = this->args_;

    ctor->Codegen(emitter);
    emitter.Emit(Instruction::kArrayStart);
    for (size_t i = 0, size = args->Length(); i < size; i++) {
        Handle<Expression> expr = args->Get(i);
        expr->Codegen(emitter);
    }
    emitter.Emit(Instruction::kNew);
}

void TaggedTemplateExpression::Codegen(Emitter& emitter) {
    throw "Tagged Template is not completed";
}

void NewTargetExpression::Codegen(Emitter& emitter) {
    throw "TODO";
}

void PropertyExpression::Codegen(Emitter& emitter) {
    Handle<PropertyExpression> self = this;
    self->base_->Codegen(emitter);
    self->member_->Codegen(emitter);
    emitter.Emit(Instruction::kGetProperty);
}

void SuperPropertyExpression::Codegen(Emitter& emitter) {
    throw "TODO";
}

void SuperCallExpression::Codegen(Emitter& emitter) {
    throw "TODO";
}

void SpreadExpression::Codegen(Emitter& emitter) {
    this->expr_->Codegen(emitter);
    emitter.Emit(Instruction::kSpread);
}

void PostfixExpression::Codegen(Emitter& emitter) {
    Instruction inc = this->increment()?Instruction::kAdd:Instruction::kSub;
    Handle<Expression> lvalue = ToSimpleAssignmentTarget(this->expr_);

    const std::type_info& targetType = typeid(*lvalue);
    if (targetType == typeid(PropertyExpression)) {
        Handle<PropertyExpression> prop = lvalue.CastTo<PropertyExpression>();
        prop->base()->Codegen(emitter);
        prop->member()->Codegen(emitter);
        emitter.Emit(Instruction::kGetPropertyNoPop);
        emitter.Emit(Instruction::kNum);
        emitter.Emit(Instruction::kDup);
        // Now it is [Base] [PropertyKey] [Value] [Value]
        emitter.Emit(Instruction::kRotate4);
        // Now it is [Value] [Base] [PropertyKey] [Value]
        emitter.Emit(Instruction::kOne);
        emitter.Emit(inc);
        emitter.Emit(Instruction::kSetProperty);
        // Now it is [Value] [Value + 1]
        emitter.Emit(Instruction::kPop);
    } else if (targetType == typeid(Identifier)) {
        size_t id = emitter.EmitConstant(lvalue.CastTo<Identifier>()->name());
        emitter.Emit(Instruction::kGetName);
        emitter.Emit16(id);
        emitter.Emit(Instruction::kNum);
        emitter.Emit(Instruction::kDup);
        emitter.Emit(Instruction::kOne);
        emitter.Emit(inc);
        emitter.Emit(Instruction::kPutName);
        emitter.Emit16(id);
        emitter.Emit(Instruction::kPop);
    } else {
        throw "TODO";
    }
}

void UnaryExpression::Codegen(Emitter& emitter) {
    Handle<Expression> operand = this->expr_;
    uint16_t op = this->op_;
    switch (op) {
        case Token::kDelete: {
            Handle<Expression> lvalue = TryToLvalue(operand);
            if (!lvalue) {
                operand->Codegen(emitter);
                emitter.Emit(Instruction::kPop);
                emitter.Emit(Instruction::kTrue);
                break;
            }
            const std::type_info& targetType = typeid(*lvalue);
            if (targetType == typeid(PropertyExpression)) {
                Handle<PropertyExpression> prop = lvalue.CastTo<PropertyExpression>();
                prop->base()->Codegen(emitter);
                prop->member()->Codegen(emitter);
                emitter.Emit(Instruction::kDeleteProperty);
            } else if (targetType == typeid(Identifier)) {
                size_t id = emitter.EmitConstant(lvalue.CastTo<Identifier>()->name());
                emitter.Emit(Instruction::kDeleteName);
                emitter.Emit16(id);
            } else {
                throw "TODO";
            }
            break;
        }
        case Token::kVoid:
            operand->Codegen(emitter);
            emitter.Emit(Instruction::kPop);
            emitter.Emit(Instruction::kUndef);
            break;
        case Token::kTypeof: {
            Handle<Expression> lvalue = TryToLvalue(operand);
            if (lvalue && typeid(*lvalue) == typeid(Identifier)) {
                emitter.Emit(Instruction::kGetNameOrUndef);
                emitter.Emit16(emitter.EmitConstant(lvalue.CastTo<Identifier>()->name()));
            } else {
                operand->Codegen(emitter);
            }
            emitter.Emit(Instruction::kTypeOf);
            break;
        }
        case Token::kInc: {
            Handle<Expression> lvalue = ToSimpleAssignmentTarget(operand);
            const std::type_info& targetType = typeid(*lvalue);
            if (targetType == typeid(PropertyExpression)) {
                Handle<PropertyExpression> prop = lvalue.CastTo<PropertyExpression>();
                prop->base()->Codegen(emitter);
                prop->member()->Codegen(emitter);
                emitter.Emit(Instruction::kGetPropertyNoPop);
                emitter.Emit(Instruction::kOne);
                emitter.Emit(Instruction::kAdd);
                emitter.Emit(Instruction::kSetProperty);
            } else if (targetType == typeid(Identifier)) {
                size_t id = emitter.EmitConstant(lvalue.CastTo<Identifier>()->name());
                emitter.Emit(Instruction::kGetName);
                emitter.Emit16(id);
                emitter.Emit(Instruction::kOne);
                emitter.Emit(Instruction::kAdd);
                emitter.Emit(Instruction::kPutName);
                emitter.Emit16(id);
            } else {
                throw "TODO";
            }
            break;
        }
        case Token::kDec: {
            Handle<Expression> lvalue = ToSimpleAssignmentTarget(operand);
            const std::type_info& targetType = typeid(*lvalue);
            if (targetType == typeid(PropertyExpression)) {
                Handle<PropertyExpression> prop = lvalue.CastTo<PropertyExpression>();
                prop->base()->Codegen(emitter);
                prop->member()->Codegen(emitter);
                emitter.Emit(Instruction::kGetPropertyNoPop);
                emitter.Emit(Instruction::kNum);
                emitter.Emit(Instruction::kOne);
                emitter.Emit(Instruction::kSub);
                emitter.Emit(Instruction::kSetProperty);
            } else if (targetType == typeid(Identifier)) {
                size_t id = emitter.EmitConstant(lvalue.CastTo<Identifier>()->name());
                emitter.Emit(Instruction::kGetName);
                emitter.Emit16(id);
                emitter.Emit(Instruction::kNum);
                emitter.Emit(Instruction::kOne);
                emitter.Emit(Instruction::kSub);
                emitter.Emit(Instruction::kPutName);
                emitter.Emit16(id);
            } else {
                throw "TODO";
            }
            break;
        }
        case '+':
            operand->Codegen(emitter);
            emitter.Emit(Instruction::kNum);
            break;
        case '-':
            operand->Codegen(emitter);
            emitter.Emit(Instruction::kNum);
            emitter.Emit(Instruction::kNeg);
            break;
        case '~':
            operand->Codegen(emitter);
            emitter.Emit(Instruction::kNum);
            emitter.Emit(Instruction::kBitwiseNot);
            break;
        case '!':
            operand->Codegen(emitter);
            emitter.Emit(Instruction::kBool);
            emitter.Emit(Instruction::kNot);
            break;
        default:
            throw "Assertion failure";
    }
}

namespace {

void OpAssign(Emitter& emitter, const Handle<Expression>& left, const Handle<Expression>& right, void(*op)(Emitter&)) {
    Handle<Expression> lval = ToSimpleAssignmentTarget(left);
    const std::type_info& targetType = typeid(*lval);
    if (targetType == typeid(PropertyExpression)) {
        Handle<PropertyExpression> prop = lval.CastTo<PropertyExpression>();
        prop->base()->Codegen(emitter);
        prop->member()->Codegen(emitter);
        emitter.Emit(Instruction::kGetPropertyNoPop);
        right->Codegen(emitter);
        op(emitter);
        emitter.Emit(Instruction::kSetProperty);
    } else if (targetType == typeid(Identifier)) {
        size_t index = emitter.EmitConstant(lval.CastTo<Identifier>()->name());
        emitter.Emit(Instruction::kGetName);
        emitter.Emit16(index);
        right->Codegen(emitter);
        op(emitter);
        emitter.Emit(Instruction::kPutName);
        emitter.Emit16(index);
    } else {
        throw "TODO";
    }
}

}

void BinaryExpression::Codegen(Emitter& emitter) {
    Handle<Expression> left = left_;
    Handle<Expression> right = right_;
    uint16_t op = op_;
    switch (op) {
        case '+':
            left->Codegen(emitter);
            right->Codegen(emitter);
            emitter.Emit(Instruction::kAddGeneric);
            break;
        case '-':
            left->Codegen(emitter);
            right->Codegen(emitter);
            ConvertTop2ToNum(emitter);
            emitter.Emit(Instruction::kSub);
            break;
        case '*':
            left->Codegen(emitter);
            right->Codegen(emitter);
            ConvertTop2ToNum(emitter);
            emitter.Emit(Instruction::kMul);
            break;
        case '/':
            left->Codegen(emitter);
            right->Codegen(emitter);
            ConvertTop2ToNum(emitter);
            emitter.Emit(Instruction::kDiv);
            break;
        case '%':
            left->Codegen(emitter);
            right->Codegen(emitter);
            ConvertTop2ToNum(emitter);
            emitter.Emit(Instruction::kMod);
            break;
        case Token::kLShift:
            left->Codegen(emitter);
            right->Codegen(emitter);
            ConvertTop2ToNum(emitter);
            emitter.Emit(Instruction::kShl);
            break;
        case Token::kRShift:
            left->Codegen(emitter);
            right->Codegen(emitter);
            ConvertTop2ToNum(emitter);
            emitter.Emit(Instruction::kShr);
            break;
        case Token::kURShift:
            left->Codegen(emitter);
            right->Codegen(emitter);
            ConvertTop2ToNum(emitter);
            emitter.Emit(Instruction::kUshr);
            break;
        case '<':
            left->Codegen(emitter);
            right->Codegen(emitter);
            emitter.Emit(Instruction::kXchg);
            emitter.Emit(Instruction::kPrim);
            emitter.Emit(Instruction::kXchg);
            emitter.Emit(Instruction::kPrim);
            emitter.Emit(Instruction::kLt);
            break;
        case '>':
            left->Codegen(emitter);
            right->Codegen(emitter);
            emitter.Emit(Instruction::kXchg);
            emitter.Emit(Instruction::kPrim);
            emitter.Emit(Instruction::kXchg);
            emitter.Emit(Instruction::kPrim);
            emitter.Emit(Instruction::kXchg);
            emitter.Emit(Instruction::kLt);
            break;
        case Token::kLteq:
            left->Codegen(emitter);
            right->Codegen(emitter);
            emitter.Emit(Instruction::kXchg);
            emitter.Emit(Instruction::kPrim);
            emitter.Emit(Instruction::kXchg);
            emitter.Emit(Instruction::kPrim);
            emitter.Emit(Instruction::kLteq);
            break;
        case Token::kGteq:
            left->Codegen(emitter);
            right->Codegen(emitter);
            emitter.Emit(Instruction::kXchg);
            emitter.Emit(Instruction::kPrim);
            emitter.Emit(Instruction::kXchg);
            emitter.Emit(Instruction::kPrim);
            emitter.Emit(Instruction::kXchg);
            emitter.Emit(Instruction::kLteq);
            break;
        case Token::kEq:
            left->Codegen(emitter);
            right->Codegen(emitter);
            emitter.Emit(Instruction::kEq);
            break;
        case Token::kIneq:
            left->Codegen(emitter);
            right->Codegen(emitter);
            emitter.Emit(Instruction::kEq);
            emitter.Emit(Instruction::kNot);
            break;
        case Token::kStrictEq:
            left->Codegen(emitter);
            right->Codegen(emitter);
            emitter.Emit(Instruction::kSeq);
            break;
        case Token::kStrictIneq:
            left->Codegen(emitter);
            right->Codegen(emitter);
            emitter.Emit(Instruction::kSeq);
            emitter.Emit(Instruction::kNot);
            break;
        case '&':
            left->Codegen(emitter);
            right->Codegen(emitter);
            ConvertTop2ToNum(emitter);
            emitter.Emit(Instruction::kAnd);
            break;
        case '^':
            left->Codegen(emitter);
            right->Codegen(emitter);
            ConvertTop2ToNum(emitter);
            emitter.Emit(Instruction::kXor);
            break;
        case '|':
            left->Codegen(emitter);
            right->Codegen(emitter);
            ConvertTop2ToNum(emitter);
            emitter.Emit(Instruction::kOr);
            break;
        case Token::kLAnd: {
            left->Codegen(emitter);
            emitter.Emit(Instruction::kDup);
            emitter.Emit(Instruction::kBool);
            emitter.Emit(Instruction::kNot);
            emitter.Emit(Instruction::kJumpIfTrue);
            Emitter::Placeholder place = emitter.EmitPlaceholder();
            emitter.Emit(Instruction::kPop);
            right->Codegen(emitter);
            emitter.PatchLabel(place, emitter.EmitLabel());
            break;
        }
        case Token::kLOr: {
            left->Codegen(emitter);
            emitter.Emit(Instruction::kDup);
            emitter.Emit(Instruction::kBool);
            emitter.Emit(Instruction::kJumpIfTrue);
            Emitter::Placeholder place = emitter.EmitPlaceholder();
            emitter.Emit(Instruction::kPop);
            right->Codegen(emitter);
            emitter.PatchLabel(place, emitter.EmitLabel());
            break;
        }
        case Token::kIn:
            throw "in & instanceof not done";
        case Token::kInstanceof:
            left->Codegen(emitter);
            right->Codegen(emitter);
            emitter.Emit(Instruction::kInstanceOf);
            break;
        case ',':
            left->Codegen(emitter);
            emitter.Emit(Instruction::kPop);
            right->Codegen(emitter);
            break;
        case '=': {
            Handle<Expression> lval = TryToLvalue(left);
            if (lval) {
                const std::type_info& targetType = typeid(*lval);
                if (targetType == typeid(PropertyExpression)) {
                    Handle<PropertyExpression> prop = lval.CastTo<PropertyExpression>();
                    prop->base()->Codegen(emitter);
                    prop->member()->Codegen(emitter);
                    right->Codegen(emitter);
                    emitter.Emit(Instruction::kSetProperty);
                } else if (targetType == typeid(Identifier)) {
                    right->Codegen(emitter);
                    emitter.Emit(Instruction::kPutName);
                    emitter.Emit16(emitter.EmitConstant(lval.CastTo<Identifier>()->name()));
                } else {
                    throw "TODO";
                }
            } else {
                right->Codegen(emitter);
                emitter.Emit(Instruction::kDup);
                AssignIntoPattern(left, emitter);
            }
            break;
        }
        case Token::kAddAssign: {
            OpAssign(emitter, left, right, [] (Emitter& e) {
                e.Emit(Instruction::kAddGeneric);
            });
            break;
        }
        case Token::kSubAssign: {
            OpAssign(emitter, left, right, [] (Emitter& e) {
                e.Emit(Instruction::kXchg);
                e.Emit(Instruction::kNum);
                e.Emit(Instruction::kXchg);
                e.Emit(Instruction::kNum);
                e.Emit(Instruction::kSub);
            });
            break;
        }
        case Token::kMulAssign: {
            OpAssign(emitter, left, right, [] (Emitter& e) {
                e.Emit(Instruction::kXchg);
                e.Emit(Instruction::kNum);
                e.Emit(Instruction::kXchg);
                e.Emit(Instruction::kNum);
                e.Emit(Instruction::kMul);
            });
            break;
        }
        case Token::kDivAssign: {
            OpAssign(emitter, left, right, [] (Emitter& e) {
                e.Emit(Instruction::kXchg);
                e.Emit(Instruction::kNum);
                e.Emit(Instruction::kXchg);
                e.Emit(Instruction::kNum);
                e.Emit(Instruction::kDiv);
            });
            break;
        }
        case Token::kModAssign: {
            OpAssign(emitter, left, right, [] (Emitter& e) {
                e.Emit(Instruction::kXchg);
                e.Emit(Instruction::kNum);
                e.Emit(Instruction::kXchg);
                e.Emit(Instruction::kNum);
                e.Emit(Instruction::kMod);
            });
            break;
        }
        default:
            throw "TODO: Assignment";
    }
}

void ConditionalExpression::Codegen(Emitter& emitter) {
    Handle<ConditionalExpression> self = this;
    self->cond_->Codegen(emitter);
    emitter.Emit(Instruction::kBool);
    emitter.Emit(Instruction::kJumpIfTrue);
    Emitter::Placeholder trueBranch = emitter.EmitPlaceholder();
    self->otherwise_->Codegen(emitter);
    emitter.Emit(Instruction::kJump);
    Emitter::Placeholder finishBranch = emitter.EmitPlaceholder();
    emitter.PatchLabel(trueBranch, emitter.EmitLabel());
    self->then_->Codegen(emitter);
    emitter.PatchLabel(finishBranch, emitter.EmitLabel());
}

}
}
}

void BlockStatement::Codegen(Emitter& emitter) {
    emitter.Emit(Instruction::kPushScope);
    Handle<Array<Statement>> items = items_;
    for (size_t i = 0, size = items_->Length(); i < size; i++) {
        items_->Get(i)->LexDeclGen(emitter);
    }
    for (size_t i = 0, length = items->Length(); i < length; i++) {
        items->Get(i)->Codegen(emitter);
    }
    emitter.Emit(Instruction::kPopScope);
}

void ExpressionStatement::Codegen(Emitter& emitter) {
    emitter.Emit(Instruction::kPop);
    expr_->Codegen(emitter);
}

void DirectiveStatement::Codegen(Emitter& emitter) {
    emitter.Emit(Instruction::kPop);
    size_t id = emitter.EmitConstant(this->value_);
    emitter.Emit(Instruction::kLoad);
    emitter.Emit16(id);
}

void EmptyStatement::Codegen(Emitter& emitter) {}

void IfStatement::Codegen(Emitter& emitter) {
    Handle<IfStatement> self = this;
    if (self->otherwise_) {
        self->cond_->Codegen(emitter);
        emitter.Emit(Instruction::kBool);
        emitter.Emit(Instruction::kJumpIfTrue);
        Emitter::Placeholder trueBranch = emitter.EmitPlaceholder();
        self->otherwise_->Codegen(emitter);
        emitter.Emit(Instruction::kJump);
        Emitter::Placeholder finishBranch = emitter.EmitPlaceholder();
        emitter.PatchLabel(trueBranch, emitter.EmitLabel());
        self->then_->Codegen(emitter);
        emitter.PatchLabel(finishBranch, emitter.EmitLabel());
    } else {
        self->cond_->Codegen(emitter);
        emitter.Emit(Instruction::kBool);
        emitter.Emit(Instruction::kNot);
        emitter.Emit(Instruction::kJumpIfTrue);
        Emitter::Placeholder finishBranch = emitter.EmitPlaceholder();
        self->then_->Codegen(emitter);
        emitter.PatchLabel(finishBranch, emitter.EmitLabel());
    }
}

void DoStatement::Codegen(Emitter& emitter) {
    Handle<DoStatement> self = this;
    Emitter::Label bodyLabel = emitter.EmitLabel();
    self->body_->Codegen(emitter);
    // Continue point
    self->cond_->Codegen(emitter);
    emitter.Emit(Instruction::kBool);
    emitter.Emit(Instruction::kJumpIfTrue);
    emitter.PatchLabel(emitter.EmitPlaceholder(), bodyLabel);
    // Break point
}

void WhileStatement::Codegen(Emitter& emitter) {
    Handle<WhileStatement> self = this;
    // Continue point
    Emitter::Label condLabel = emitter.EmitLabel();
    self->cond_->Codegen(emitter);
    emitter.Emit(Instruction::kBool);
    emitter.Emit(Instruction::kNot);
    emitter.Emit(Instruction::kJumpIfTrue);
    Emitter::Placeholder finishBranch = emitter.EmitPlaceholder();
    self->body_->Codegen(emitter);
    emitter.Emit(Instruction::kJump);
    emitter.PatchLabel(emitter.EmitPlaceholder(), condLabel);
    // Break point
    emitter.PatchLabel(finishBranch, emitter.EmitLabel());
}


void DebuggerStatement::Codegen(Emitter& emitter) {
    emitter.Emit(Instruction::kDebugger);
}

void Script::Codegen(Emitter& emitter) {
    Handle<Script> self = this;
    for (size_t i = 0, size = self->body_->Length(); i < size; i++) {
        self->body_->Get(i)->VarDeclGen(emitter);
        self->body_->Get(i)->LexDeclGen(emitter);
    }
    emitter.Emit(Instruction::kUndef);
    for (size_t i = 0, size = self->body_->Length(); i < size; i++) {
        self->body_->Get(i)->Codegen(emitter);
    }
    emitter.Emit(Instruction::kReturn);
}

void ThrowStatement::Codegen(Emitter& emitter) {
    this->expr_->Codegen(emitter);
    emitter.Emit(Instruction::kThrow);
}

void ReturnStatement::Codegen(Emitter& emitter) {
    if (this->expr_)
        this->expr_->Codegen(emitter);
    else
        emitter.Emit(Instruction::kUndef);
    emitter.Emit(Instruction::kReturn);
}

void TryStatement::Codegen(Emitter& emitter) {
    Handle<TryStatement> self = this;

    // General Step 1
    Emitter::Label bodyBlockStart = emitter.EmitLabel();
    self->body_->Codegen(emitter);
    Emitter::Label bodyBlockEnd = emitter.EmitLabel();

    // General Step 2
    emitter.Emit(Instruction::kJump);
    Emitter::Placeholder finishHolder = emitter.EmitPlaceholder();

    if (self->param_ && self->finally_) {
        // Try-Catch-Finally
        // 1  BodyBlock | OnError Goto Catch
        // 2  Goto Finish
        // Catch:
        // 3  CatchBlock | OnError Goto Finally
        // 4  Goto Finish
        // Finally:
        // 5  FinallyBlock
        // 6  Rethrow
        // Finish:
        // 7  FinallyBlock

        Emitter::Label catchLabel = emitter.EmitLabel();

        // Step 3
        // When exception landed here, the stack is [Exception]
        // TODO We need to store it to param instead of pop it out
        Emitter::Label catchBlockStart = emitter.EmitLabel();
        emitter.Emit(Instruction::kPushScope);
        GenerateDefinition(self->param_, emitter, VariableDeclaration::Type::kLet);
        BindIntoPattern(self->param_, emitter);
        emitter.Emit(Instruction::kUndef);
        self->error_->Codegen(emitter);
        emitter.Emit(Instruction::kPopScope);
        Emitter::Label catchBlockEnd = emitter.EmitLabel();

        // Step 4
        emitter.Emit(Instruction::kJump);
        Emitter::Placeholder finishHolder2 = emitter.EmitPlaceholder();

        Emitter::Label finallyLabel = emitter.EmitLabel();

        // Step 5
        // When exception landed here, the stack is [Exception]
        // Since statement will pop, we need to push undef
        emitter.Emit(Instruction::kUndef);
        self->finally_->Codegen(emitter);

        // Step 6
        // Pop the completion and rethrow
        emitter.Emit(Instruction::kPop);
        emitter.Emit(Instruction::kThrow);

        Emitter::Label finishLabel = emitter.EmitLabel();

        // Step 7
        self->finally_->Codegen(emitter);

        emitter.PatchLabel(finishHolder, finishLabel);
        emitter.PatchLabel(finishHolder2, finishLabel);
        emitter.NewExceptionTableEntry(bodyBlockStart, bodyBlockEnd, catchLabel);
        emitter.NewExceptionTableEntry(catchBlockStart, catchBlockEnd, finallyLabel);
    } else if (self->param_) {
        // Try-Catch
        // 1  BodyBlock | OnError Goto Catch
        // 2  Goto Finish
        // Catch:
        // 3  CatchBlock
        // Finish:

        Emitter::Label catchLabel = emitter.EmitLabel();

        // Step 3
        // When exception landed here, the stack is [Exception]
        emitter.Emit(Instruction::kPushScope);
        GenerateDefinition(self->param_, emitter, VariableDeclaration::Type::kLet);
        BindIntoPattern(self->param_, emitter);
        emitter.Emit(Instruction::kUndef);
        self->error_->Codegen(emitter);
        emitter.Emit(Instruction::kPopScope);

        Emitter::Label finishLabel = emitter.EmitLabel();

        emitter.PatchLabel(finishHolder, finishLabel);
        emitter.NewExceptionTableEntry(bodyBlockStart, bodyBlockEnd, catchLabel);
    } else {
        // Try-Finally
        // 1  BodyBlock | OnError Goto Finally
        // 2  Goto Finish
        // Finally:
        // 3  FinallyBlock
        // 4  Rethrow
        // Finish:
        // 5  FinallyBlock

        Emitter::Label finallyLabel = emitter.EmitLabel();

        // Step 3
        // When exception landed here, the stack is [Exception]
        // Since statement will pop, we need to push undef
        emitter.Emit(Instruction::kUndef);
        self->finally_->Codegen(emitter);

        // Step 4
        // Pop the completion and rethrow
        emitter.Emit(Instruction::kPop);
        emitter.Emit(Instruction::kThrow);

        Emitter::Label finishLabel = emitter.EmitLabel();

        // Step 5
        self->finally_->Codegen(emitter);

        emitter.PatchLabel(finishHolder, finishLabel);
        emitter.NewExceptionTableEntry(bodyBlockStart, bodyBlockEnd, finallyLabel);
    }
}

void Expression::Codegen(Emitter& emitter) {
    printf("%s does not implement codegen\n", typeid(*this).name());
    emitter.Emit(Instruction::kUndef);
}

void Statement::Codegen(Emitter& emitter) {
    printf("%s does not implement codegen\n", typeid(*this).name());
}

// Function Generation
void FunctionExpression::Codegen(Emitter& emitter) {
    Handle<FunctionExpression> self = this;
    Emitter innerEmitter;

    {
        size_t i = 0;
        for (Handle<Expression> param : self->param_->GetIterable()) {
            size_t index = innerEmitter.EmitConstant(JSNumber::New(static_cast<int64_t>(i)));
            innerEmitter.Emit(Instruction::kLoad);
            innerEmitter.Emit16(index);
            innerEmitter.Emit(Instruction::kGetPropertyNoPop);
            GenerateDefinition(param, innerEmitter, VariableDeclaration::Type::kLet);
            BindIntoPattern(param, innerEmitter);
            innerEmitter.Emit(Instruction::kPop);
            i++;
        }
    }

    for (Handle<Statement> stmt : self->body_->GetIterable()) {
        stmt->VarDeclGen(innerEmitter);
        stmt->LexDeclGen(innerEmitter);
    }

    innerEmitter.Emit(Instruction::kUndef);
    for (Handle<Statement> stmt : self->body_->GetIterable()) {
        stmt->Codegen(innerEmitter);
    }
    innerEmitter.Emit(Instruction::kUndef);
    innerEmitter.Emit(Instruction::kReturn);
    size_t codeIndex = emitter.EmitCode(innerEmitter.ToCode());

    size_t nameIndex;
    if (self->name_) {
        nameIndex = emitter.EmitConstant(self->name_);
        emitter.Emit(Instruction::kPushScope);
        emitter.Emit(Instruction::kDefConst);
        emitter.Emit16(nameIndex);
    }

    if (self->isGenerator())
        emitter.Emit(Instruction::kGenerator);
    else
        emitter.Emit(Instruction::kFunction);
    emitter.Emit16(codeIndex);

    if (self->name_) {
        emitter.Emit(Instruction::kDup);
        emitter.Emit(Instruction::kInitDef);
        emitter.Emit16(nameIndex);
        emitter.Emit(Instruction::kPopScope);
    }
}

void FunctionStatement::Codegen(Emitter& emitter) {}

void FunctionStatement::VarDeclGen(Emitter& emitter) {
    Handle<FunctionExpression> self = this->func_;
    Handle<JSString> name = self->name_;

    size_t nameIndex = emitter.EmitConstant(self->name_);
    emitter.Emit(Instruction::kDefLet);
    emitter.Emit16(nameIndex);

    self->WriteBarrier(&self->name_, nullptr);
    self->Codegen(emitter);
    self->WriteBarrier(&self->name_, name);

    emitter.Emit(Instruction::kInitDef);
    emitter.Emit16(nameIndex);
}

void YieldExpression::Codegen(Emitter& emitter) {
    if (this->expr_) {
        this->expr_->Codegen(emitter);
    } else {
        emitter.Emit(Instruction::kUndef);
    }
    emitter.Emit(Instruction::kYield);
}

void YieldAllExpression::Codegen(Emitter& emitter) {
    throw "TODO";
}

void Statement::VarDeclGen(Emitter& emitter) {
    printf("%s does not implement vardeclgen\n", typeid(*this).name());
}

void Statement::LexDeclGen(Emitter& emitter) {}

// VarDeclGen

void BlockStatement::VarDeclGen(Emitter& emitter) {
    Handle<Array<Statement>> items = items_;
    for (size_t i = 0, length = items->Length(); i < length; i++) {
        items->Get(i)->VarDeclGen(emitter);
    }
}

void ExpressionStatement::VarDeclGen(Emitter& emitter) {}
void EmptyStatement::VarDeclGen(Emitter& emitter) {}
void ThrowStatement::VarDeclGen(Emitter& emitter) {}
void DebuggerStatement::VarDeclGen(Emitter& emitter) {}
void DirectiveStatement::VarDeclGen(Emitter& emitter) {}
void ReturnStatement::VarDeclGen(Emitter& emitter) {}

#define CREATE_VARDECLGEN_ITEM(x,discard) \
	if(self->NORLIT_PP_CONCAT_2(x,_)) self->NORLIT_PP_CONCAT_2(x,_)->VarDeclGen(emitter);

#define CREATE_VARDECLGEN(clazz, ...) \
	void clazz::VarDeclGen(Emitter& emitter) {\
		Handle<clazz> self = this;\
		NORLIT_PP_FOREACH(CREATE_VARDECLGEN_ITEM, (__VA_ARGS__),)\
	}

CREATE_VARDECLGEN(IfStatement, then, otherwise);
CREATE_VARDECLGEN(DoStatement, body);
CREATE_VARDECLGEN(WhileStatement, body);
CREATE_VARDECLGEN(TryStatement, body, error, finally);
