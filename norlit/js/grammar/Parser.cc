#include "../all.h"

#include "Parser.h"
#include "Scanner.h"
#include "Token.h"
#include "Node.h"

#include <typeinfo>

#include "../JSBoolean.h"
#include "../../util/ArrayList.h"

using namespace norlit::gc;
using namespace norlit::js::grammar;
using namespace norlit::util;

Parser::Parser(Scanner& s):scanner(s) {
    Fetch_();
}

void Parser::Fetch_() {
    if (t1_) {
        t0_ = t1_;
        t1_ = nullptr;
    } else {
        t0_ = scanner.NextToken();
        if (t0_->type == Token::kIdentifier) {
            scanner.TranslateIdentifierName(t0_);
        }
    }
}

void Parser::FetchLookahead_() {
    if (t1_) {
        return;
    }
    t1_ = scanner.NextToken();
    if (t1_->type == Token::kIdentifier) {
        scanner.TranslateIdentifierName(t1_);
    }
}

void Parser::Advance_() {
    Fetch_();
}

void Parser::AdvanceAndFetchIdentifierName_() {
    assert(!t1_);
    t0_ = scanner.NextToken();
}

bool Parser::ConsumeIf_(uint16_t type) {
    if (t0_->type == type) {
        Advance_();
        return true;
    }
    return false;
}

void Parser::ConsumeSemicolon_() {
    if (t0_->type == ';') {
        Advance_();
        return;
    }
    if (t0_->type=='}' || (t0_->flags&Token::kLineBefore)) {
        return;
    } else {
        Exceptions::ThrowSyntaxError("Expected semicolon after statement");
    }
}

Handle<Expression> Parser::ParsePrimaryExpr() {
    Handle<Expression> returnVal;
    switch (t0_->type) {
        /* 12.2.1 The this Keyword */
        case Token::kThis:
            returnVal = new ThisExpression();
            Advance_();
            break;
        /* Inline: 12.1 Identifier */
        case Token::kYield:
            //return this._yieldAsIdentifier();
            throw "TODO: yield";
        case Token::kIdentifier:
            returnVal = new Identifier((JSString*)t0_->value);
            Advance_();
            break;
        /* 12.2.3 Literal */
        case Token::kNull:
            returnVal = new Literal(JSNull::New());
            Advance_();
            break;
        case Token::kTrue:
            returnVal = new Literal(JSBoolean::New(true));
            Advance_();
            break;
        case Token::kFalse:
            returnVal = new Literal(JSBoolean::New(false));
            Advance_();
            break;
        case Token::kNumber:
        case Token::kString:
            returnVal =new Literal(t0_->value);
            Advance_();
            break;
        case '[':
            return ParseArrayLiteral();
        case '{':
            return ParseObjectLiteral();
        case Token::kFunction:
            return ParseFunctionOrGeneratorExpression();
        case Token::kClass:
            throw "return this.parseClass();";
        /* 12.2.7 Regular Expression Literals */
        case '/':
        case Token::kDivAssign:
            t0_ = scanner.NextRegexp(t0_);
            returnVal = new RegexpLiteral((JSString*)t0_->value, (JSString*)t0_->additional);
            Advance_();
            break;
        /* 12.2.8 Template Literals */
        case Token::kNoSubTemplate:
        case Token::kTemplateHead:
            return ParseTemplate();
        /* 12.2.9 The Grouping Operator */
        case '(': {
            Handle<Array<Expression>> items = ParseCoveredFormals();
            return new CoveredFormals(items);
        }
        default:
            Exceptions::ThrowSyntaxError("Unexpected token when parsing expression");
    }
    return returnVal;
}

Handle<TemplateLiteral> Parser::ParseTemplate() {
    Handle<Array<JSString>> cooked;
    Handle<Array<JSString>> raw;
    Handle<Array<Expression>> subst;
    if (t0_->type == Token::kNoSubTemplate) {
        cooked = Array<JSString>::New(1);
        raw = Array<JSString>::New(1);
        cooked->Put(0, (JSString*)t0_->value);
        raw->Put(0, (JSString*)t0_->additional);
    } else {
        assert(t0_->type = Token::kTemplateHead);
        ArrayList<JSString> cookedList;
        ArrayList<JSString> rawList;
        ArrayList<Expression> substList;

        cookedList.Add((JSString*)t0_->value);
        rawList.Add((JSString*)t0_->additional);

        do {
            Advance_();
            substList.Add(ParseExpression());
            if (t0_->type != '}') {
                Exceptions::ThrowSyntaxError("Expected } in template literal");
            }
            assert(!t1_);
            t0_ = scanner.NextTemplatePart();
            cookedList.Add((JSString*)t0_->value);
            rawList.Add((JSString*)t0_->additional);
            if (t0_->type == Token::kTemplateTail) {
                break;
            }
        } while (true);
        cooked = cookedList.ToArray();
        raw = rawList.ToArray();
        subst = substList.ToArray();
    }
    Advance_();
    return new TemplateLiteral(cooked, raw, subst);
}

Handle<Array<Expression>> Parser::ParseCoveredFormals() {
    if (!ConsumeIf_('(')) {
        // This only occur in function parsing.
        Exceptions::ThrowSyntaxError("Expected ( to start a parameter list");
    }
    if (t0_->type == ')') {
        Advance_();
        return Array<Expression>::New(0);
    } else {
        ArrayList<Expression> args;
        Handle<Expression> expr;
        while (true) {
            if (t0_->type == Token::kEllipse) {
                Advance_();
                // TODO YIELD!!
                if (t0_->type != Token::kIdentifier) {
                    Exceptions::ThrowSyntaxError("Illegal rest binding pattern");
                }
                // TODO Directly use identifier
                expr = ParsePrimaryExpr();
                args.Add(new SpreadExpression(expr));
                if (t0_->type != ')') {
                    Exceptions::ThrowSyntaxError("Rest binding pattern should be the last in the list");
                }
                break;
            } else {
                expr = ParseAssignmentExpr();
                args.Add(expr);
            }
            if (t0_->type == ')') {
                break;
            } else if (t0_->type != ',') {
                Exceptions::ThrowSyntaxError("Parenthesis is not enclosed");
            }
            Advance_();
        }
        Advance_();
        return args.ToArray();
    }
}

Handle<Expression> Parser::ParseArrayLiteral() {
    // Consume [
    Advance_();
    ArrayList<Expression> elements;
    Handle<Expression> expr;
    do {
        switch (t0_->type) {
            case Token::kEllipse:
                Advance_();
                expr = ParseAssignmentExpr();
                elements.Add(new SpreadExpression(expr));
                break;
            case ',':
                elements.Add(nullptr);
                break;
            case ']':
                goto finished;
            default:
                elements.Add(ParseAssignmentExpr());
                break;
        }
    } while (ConsumeIf_(','));
finished:
    // Consume ]
    Advance_();
    Handle<Array<Expression>> arr = elements.ToArray();
    return new ArrayLiteral(arr);
}

Handle<Expression> Parser::RefineArrayAssignmentPattern(const Handle<ArrayLiteral>& pattern) {
    Handle<Array<Expression>> items = pattern->items();
    size_t size = items->Length();
    // Zero-sized array literal is a valid pattern
    if (size == 0) {
        return pattern;
    }
    Handle<Expression> expr;

    // Check the last element. Spread expression is fine
    expr = items->Get(size - 1);
    if (typeid(*expr) == typeid(SpreadExpression)) {
        size--;
        // TODO Hey, don't forget to refine
    }

    for (size_t i = 0; i < size; i++) {
        expr = items->Get(size);
        // Elision is allowed
        if (!expr)
            continue;
        if (typeid(*expr) == typeid(BinaryExpression)) {
            Handle<BinaryExpression> binExpr = expr.CastTo<BinaryExpression>();
            if (binExpr->op() == '=') {
//TODO
            }
        }
    }
    return pattern;
}

Handle<Expression> Parser::ParseObjectLiteral() {
    // Consume {
    ArrayList<Property> elements;
    Handle<Expression> key, value;
    Property::Type type;
    do {
        AdvanceAndFetchIdentifierName_();

        switch (t0_->type) {
            case '}':
                goto finish;
            case '*':
                throw "* PropertyName ( FormalParameters ) { FunctionBody }";
                //elements.push(this.parseMethodDefinition());
                break;
            case '[': {
                Advance_();
                key = ParseAssignmentExpr();
                if (t0_->type != ']') {
                    Exceptions::ThrowSyntaxError("Bracket mismatch when processing computed property");
                }
                Advance_();
                if (t0_->type == '(') {
                    throw "MethodDefinition : PropertyName ( FormalParameters ) { FunctionBody }";
                }
                Advance_();
                value = ParseAssignmentExpr();
                type = Property::Type::kNormal;
                break;
            }
            case Token::kString:
            case Token::kNumber: {
                key = new Literal(t0_->value);
                Advance_();
                if (t0_->type == '(') {
                    throw "MethodDefinition : PropertyName ( FormalParameters ) { FunctionBody }";
                } else if (t0_->type == ':') {
                    Advance_();
                    value = ParseAssignmentExpr();
                    type = Property::Type::kNormal;
                    break;
                } else {
                    Exceptions::ThrowSyntaxError("Expected : or ( in object literal");
                }
            }
            case Token::kIdentifier: {
                Handle<Token> name = t0_;
                AdvanceAndFetchIdentifierName_();
                switch (t0_->type) {
                    case '=': {
                        throw "PropertyDefinition : Identifier = AssignmentExpression";
                    }
                    case ':': {
                        Advance_();
                        key = new Literal(name->value);
                        value = ParseAssignmentExpr();
                        type = Property::Type::kNormal;
                        break;
                    }
                    case '(':
                        throw "MethodDefinition : PropertyName(FormalParameters) { FunctionBody }";
                    case '[':
                    case Token::kIdentifier:
                    case Token::kString:
                    case Token::kNumber:
                        throw "id = get\n"
                        "    MethodDefinition : get PropertyName() { FunctionBody }\n"
                        "id = set\n"
                        "    MethodDefinition : set PropertyName(FormalParameter) { FunctionBody }\n"
                        "Else\n"
                        "    ERROR";
                    default:
                        key = new Literal(name->value);
                        value = new Identifier((JSString*)name->value);
                        type = Property::Type::kNormal;
                        break;
                }
                break;
            }
            default:
                Exceptions::ThrowSyntaxError("Expected property definition in object literal");
        }
        elements.Add(new Property(key, value, type));
        // Do not use ConsumeIf so it is consumed by AdvanceAndFetchIdentifierName_
    } while (t0_->type == ',');
finish:
    Advance_();
    Handle<Array<Property>> arr = elements.ToArray();
    return new ObjectLiteral(arr);
}

Handle<Expression> Parser::ParseLeftHandSideExpr(bool noCall) {
    Handle<Expression> returnVal;
    switch (t0_->type) {
        case Token::kNew: {
            Advance_();
            if (ConsumeIf_('.')) {
                static Handle<JSString> target = JSString::New("target");
                if (t0_->type != Token::kIdentifier || t0_->value != target) {
                    Exceptions::ThrowSyntaxError("Expected new.target");
                }
                Advance_();
                returnVal = new NewTargetExpression();
                break;
            }
            Handle<Expression> ctor = ParseLeftHandSideExpr(true);
            if (t0_->type != '(') {
                return new NewExpression(ctor, nullptr);
            }
            Advance_();
            Handle<Array<Expression>> args = ParseArguments();
            returnVal = new NewExpression(ctor, args);
            break;
        }
        case Token::kSuper:
            Advance_();
            switch (t0_->type) {
                case '.': {
                    AdvanceAndFetchIdentifierName_();
                    if (t0_->type != Token::kIdentifier) {
                        Exceptions::ThrowSyntaxError("Expected identifier after in member expression");
                    }
                    Handle<Expression> convertedLiteral = new Literal(t0_->value);
                    Advance_();
                    returnVal = new SuperPropertyExpression(convertedLiteral);
                    break;
                }
                case '[': {
                    Advance_();
                    Handle<Expression> member = ParseExpression();
                    if (t0_->type != ']') {
                        Exceptions::ThrowSyntaxError("Bracket mismatch in member expression");
                    }
                    Advance_();
                    returnVal = new SuperPropertyExpression(member);
                    break;
                }
                case '(': {
                    Advance_();
                    Handle<Array<Expression>> args = ParseArguments();
                    returnVal = new SuperCallExpression(args);
                    break;
                }
                default:
                    Exceptions::ThrowSyntaxError("'super' should be used only for property access or constructor call");
            }
            break;
        default:
            returnVal = ParsePrimaryExpr();
            break;
    }
    while (true) {
        switch (t0_->type) {
            case '.': {
                AdvanceAndFetchIdentifierName_();
                if (t0_->type != Token::kIdentifier) {
                    Exceptions::ThrowSyntaxError("Expected identifier after in member expression");
                }
                Handle<Expression> convertedLiteral = new Literal(t0_->value);
                Advance_();
                returnVal = new PropertyExpression(returnVal, convertedLiteral);
                break;
            }
            case '[': {
                Advance_();
                Handle<Expression> member = ParseExpression();
                if (t0_->type != ']') {
                    Exceptions::ThrowSyntaxError("Bracket mismatch in member expression");
                }
                Advance_();
                returnVal = new PropertyExpression(returnVal, member);
                break;
            }
            case '(': {
                if (noCall) {
                    return returnVal;
                }
                Advance_();
                Handle<Array<Expression>> args = ParseArguments();
                returnVal = new CallExpression(returnVal, args);
                break;
            }
            case Token::kNoSubTemplate:
            case Token::kTemplateHead: {
                Handle<TemplateLiteral> arg = ParseTemplate();
                returnVal = new TaggedTemplateExpression(returnVal, arg);
                break;
            }
            default:
                return returnVal;
        }
    }
}

Handle<Array<Expression>> Parser::ParseArguments() {
    if (t0_->type == ')') {
        Advance_();
        return Array<Expression>::New(0);
    }
    ArrayList<Expression> args;
    Handle<Expression> expr;
    while (true) {
        if (t0_->type==Token::kEllipse) {
            Advance_();
            expr = ParseAssignmentExpr();
            args.Add(new SpreadExpression(expr));
        } else {
            expr = ParseAssignmentExpr();
            args.Add(expr);
        }
        if (t0_->type == ')') {
            Advance_();
            return args.ToArray();
        } else if (t0_->type != ',') {
            Exceptions::ThrowSyntaxError("Expected expression in argument list");
        }
        Advance_();
    }
}

Handle<Expression> Parser::ParsePostfixExpr() {
    Handle<Expression> expr = ParseLeftHandSideExpr();
    if (!(t0_->flags&Token::kLineBefore) && (t0_->type == Token::kInc || t0_->type == Token::kDec)) {
        uint16_t type = t0_->type;
        Advance_();
        return new PostfixExpression(expr, type == Token::kInc);
    }
    return expr;
}

Handle<Expression> Parser::ParseUnaryExpr() {
    switch (t0_->type) {
        case Token::kDelete:
        case Token::kVoid:
        case Token::kTypeof:
        case Token::kInc:
        case Token::kDec:
        case '+':
        case '-':
        case '~':
        case '!': {
            uint16_t op = t0_->type;
            Advance_();
            Handle<Expression> expr = ParseUnaryExpr();
            return new UnaryExpression(expr, op);
        }
        default:
            return ParsePostfixExpr();
    }
}

#define GENERATE_BINARY_EXPR_PARSER_ITEM(type,_) case type:
#define GENERATE_BINARY_EXPR_PARSER(name, prev, ...)\
	Handle<Expression> Parser::name() {\
		Handle<Expression> returnVal = prev();\
		Handle<Expression> right;\
		while (true) {\
			switch (t0_->type) {\
				NORLIT_PP_FOREACH(GENERATE_BINARY_EXPR_PARSER_ITEM, (__VA_ARGS__),) {\
					uint16_t type = t0_->type;\
					Advance_();\
					right = prev();\
					returnVal = new BinaryExpression(returnVal, right, type);\
					break;\
				}\
			default:\
				return returnVal;\
			}\
		}\
	}

GENERATE_BINARY_EXPR_PARSER(ParseMulExpr, ParseUnaryExpr, '*', '/', '%');
GENERATE_BINARY_EXPR_PARSER(ParseAddExpr, ParseMulExpr, '+', '-');
GENERATE_BINARY_EXPR_PARSER(ParseShiftExpr, ParseAddExpr, Token::kLShift, Token::kRShift, Token::kURShift);

Handle<Expression> Parser::ParseRelationalExpr(bool noIn) {
    Handle<Expression> returnVal = ParseShiftExpr();
    Handle<Expression> right;
    while (true) {
        switch (t0_->type) {
            case Token::kIn:
                if (noIn) {
                    return returnVal;
                }
            case '<':
            case '>':
            case Token::kLteq:
            case Token::kGteq:
            case Token::kInstanceof: {
                uint16_t type = t0_->type;
                Advance_();
                right = ParseShiftExpr();
                returnVal = new BinaryExpression(returnVal, right, type);
                break;
            }
            default:
                return returnVal;
        }
    }
}

#define GENERATE_BINARY_EXPR_PARSER_NO_IN(name, prev, ...)\
	Handle<Expression> Parser::name(bool noIn) {\
		Handle<Expression> returnVal = prev(noIn);\
		Handle<Expression> right;\
		while (true) {\
			switch (t0_->type) {\
				NORLIT_PP_FOREACH(GENERATE_BINARY_EXPR_PARSER_ITEM, (__VA_ARGS__),) {\
					uint16_t type = t0_->type;\
					Advance_();\
					right = prev(noIn);\
					returnVal = new BinaryExpression(returnVal, right, type);\
					break;\
				}\
			default:\
				return returnVal;\
			}\
		}\
	}

GENERATE_BINARY_EXPR_PARSER_NO_IN(ParseEqualityExpr, ParseRelationalExpr, Token::kEq, Token::kIneq, Token::kStrictEq, Token::kStrictIneq);
GENERATE_BINARY_EXPR_PARSER_NO_IN(ParseAndExpr, ParseEqualityExpr, '&');
GENERATE_BINARY_EXPR_PARSER_NO_IN(ParseXorExpr, ParseAndExpr, '^');
GENERATE_BINARY_EXPR_PARSER_NO_IN(ParseOrExpr, ParseXorExpr, '|');
GENERATE_BINARY_EXPR_PARSER_NO_IN(ParseLAndExpr, ParseOrExpr, Token::kLAnd);
GENERATE_BINARY_EXPR_PARSER_NO_IN(ParseLOrExpr, ParseLAndExpr, Token::kLOr);

Handle<Expression> Parser::ParseConditionalExpr(bool noIn) {
    Handle<Expression> cond = ParseLOrExpr(noIn);
    if (t0_->type!='?') {
        return cond;
    }
    Advance_();
    Handle<Expression> trueExpr = ParseAssignmentExpr(noIn);
    if (t0_->type != ':') {
        Exceptions::ThrowSyntaxError("Expected : in conditional expression");
    }
    Advance_();
    Handle<Expression> falseExpr = ParseAssignmentExpr(noIn);
    cond = new ConditionalExpression(cond, trueExpr, falseExpr);
    return cond;
}

Handle<Expression> Parser::ParseAssignmentExpr(bool noIn) {
    if (t0_->type == Token::kYield) {
        Advance_();
        if (t0_->flags&Token::kLineBefore) {
            return new YieldExpression(nullptr);
        }
        if (t0_->type == '*') {
            throw "yield*";
        } else {
            Handle<Expression> expr = ParseAssignmentExpr();
            return new YieldExpression(expr);
        }
    }
    // TODO SeekForYield
    Handle<Expression> returnVal = ParseConditionalExpr(noIn);
    switch (t0_->type) {
        case Token::kLambda:
            if (typeid(*returnVal) == typeid(CoveredFormals) || typeid(*returnVal) == typeid(Identifier)) {
                throw "TODO: Lambda Expression";
            } else {
                Exceptions::ThrowSyntaxError("Unexpected => encountered");
            }
        case '=': {
            // Need refine
            Advance_();
            Handle<Expression> right = ParseAssignmentExpr(noIn);
            return new BinaryExpression(returnVal, right, '=');
        }
        case Token::kMulAssign:
        case Token::kDivAssign:
        case Token::kModAssign:
        case Token::kAddAssign:
        case Token::kSubAssign:
        case Token::kLShiftAssign:
        case Token::kRShiftAssign:
        case Token::kURShiftAssign:
        case Token::kAndAssign:
        case Token::kXorAssign:
        case Token::kOrAssign: {
            uint16_t op = t0_->type;
            Advance_();
            Handle<Expression> right = ParseAssignmentExpr(noIn);
            return new BinaryExpression(returnVal, right, op);
        }
        default:
            return returnVal;
    }
}

GENERATE_BINARY_EXPR_PARSER_NO_IN(ParseExpression, ParseAssignmentExpr, ',');

Handle<Statement> Parser::ParseStatement() {
    switch (t0_->type) {
        case '{':
            return ParseBlock();
        case Token::kVar:
            return ParseVariable();
        case ';':
            /* 13.4 Empty Statement */
            Advance_();
            return new EmptyStatement();
        case Token::kIf:
            return ParseIf();
        /* 13.7 Iteration Statements */
        case Token::kDo:
            return ParseDo();
        case Token::kWhile:
            return ParseWhile();
        case Token::kFor:
            return ParseFor();
        case Token::kSwitch:
            return ParseSwitch();
        case Token::kContinue:
            return ParseContinue();
        case Token::kBreak:
            return ParseBreak();
        case Token::kReturn:
            return ParseReturn();
        case Token::kWith:
            return ParseWith();
        case Token::kThrow:
            return ParseThrow();
        case Token::kTry:
            return ParseTry();
        case Token::kDebugger:
            /* Inline: 13.16 Debugger Statement */
            Advance_();
            ConsumeSemicolon_();
            return new DebuggerStatement();
        case Token::kIdentifier:
        case Token::kYield:
            FetchLookahead_();
            if (t1_->type == ':') {
                return ParseLabelled();
            }
        default:
            return ParseExpressionStmt();
    }
}

Handle<Statement> Parser::ParseDeclaration() {
    switch (t0_->type) {
        case Token::kFunction:
            return ParseFunctionOrGeneratorStatement();
        case Token::kClass:
            throw "TODO: ParseClass()";
        case Token::kConst:
        case Token::kIdentifier /* let */:
            return ParseVariable();
        default:
            throw "Assertion failure";
    }
}

/* 13.2 Block */
Handle<BlockStatement> Parser::ParseBlock() {
    // Called by ParseTry()
    if (t0_->type != '{') {
        Exceptions::ThrowSyntaxError("Expected { to start a block");
    }
    Advance_();
    Handle<Array<Statement>> stmts = ParseStatementList();
    if (t0_->type != '}') {
        Exceptions::ThrowSyntaxError("Expected } to close up a block");
    }
    Advance_();
    return new BlockStatement(stmts);
}

Handle<Array<Statement>> Parser::ParseStatementList(bool useDirective) {
    ArrayList<Statement> list;
    Handle<Statement> stmt;
    while (true) {
        switch (t0_->type) {
            case '}':
            case Token::kEOF:
            /* Those two are for statement list within switch statement */
            case Token::kCase:
            case Token::kDefault:
                return list.ToArray();
        }
        stmt = ParseStatementListItem();
        if (useDirective) {
            if (typeid(*stmt) == typeid(ExpressionStatement)) {
                Handle<Expression> expr = stmt.CastTo<ExpressionStatement>()->expr();
                if (typeid(*expr) == typeid(Literal)) {
                    Handle<JSValue> lit = expr.CastTo<Literal>()->literal();
                    if (lit->GetType() == JSValue::Type::kString) {
                        stmt = new DirectiveStatement(lit.CastTo<JSString>());
                        goto add;
                    }
                }
            }
            useDirective = false;
        }
add:
        list.Add(stmt);
    }
}

Handle<Statement> Parser::ParseStatementListItem() {
    switch (t0_->type) {
        case Token::kImport:
            throw "TODO: import";
        case Token::kExport:
            throw "TODO: export";
        case Token::kConst:
        case Token::kFunction:
        case Token::kClass:
            return ParseDeclaration();
        case Token::kIdentifier: {
            static Handle<JSString> let = JSString::New("let");
            if (t0_->value == let) {
                FetchLookahead_();
                switch(t1_->type) {
                    case '{':
                    case '[':
                    case Token::kIdentifier:
                    case Token::kYield:
                        return ParseDeclaration();
                }
            }
        }
        default:
            return ParseStatement();
    }
}


Handle<Statement> Parser::ParseVariable() {
    VariableDeclaration::Type type;
    ArrayList<Expression> decl;
    switch (t0_->type) {
        case Token::kConst:
            type = VariableDeclaration::Type::kConst;
            break;
        case Token::kVar:
            type = VariableDeclaration::Type::kVar;
            break;
        case Token::kIdentifier:
            type = VariableDeclaration::Type::kLet;
            break;
        default:
            assert(0);
    }
    Advance_();
    do {
        decl.Add(ParseAssignmentExpr());
    } while (ConsumeIf_(','));
    ConsumeSemicolon_();
    Handle<Array<Expression>> exprs = decl.ToArray();
    return new VariableDeclaration(exprs, type);
}

/* 13.5 Expression Statement */
Handle<Statement> Parser::ParseExpressionStmt() {
    Handle<Expression> expr = ParseExpression();
    ConsumeSemicolon_();
    return new ExpressionStatement(expr);
}

Handle<Statement> Parser::ParseIf() {
    // Consume If
    Advance_();
    if (t0_->type != '(') {
        Exceptions::ThrowSyntaxError("Expected ( after if");
    }
    Advance_();
    Handle<Expression> cond = ParseExpression();
    if (t0_->type != ')') {
        Exceptions::ThrowSyntaxError("Parenthesis is not enclosed");
    }
    Advance_();
    Handle<Statement> then = ParseStatement();
    Handle<Statement> otherwise;
    if (t0_->type == Token::kElse) {
        Advance_();
        otherwise = ParseStatement();
    }
    return new IfStatement(cond, then, otherwise);
}

Handle<Statement> Parser::ParseDo() {
    // Consume do
    Advance_();
    Handle<Statement> body = ParseStatement();
    if (t0_->type != Token::kWhile) {
        Exceptions::ThrowSyntaxError("Expected while in do while loop");
    }
    Advance_();
    if (t0_->type != '(') {
        Exceptions::ThrowSyntaxError("Expected ( after while");
    }
    Advance_();
    Handle<Expression> cond = ParseExpression();
    if (t0_->type != ')') {
        Exceptions::ThrowSyntaxError("Parenthesis is not enclosed");
    }
    Advance_();
    // See Automatic Semicolon Insertion. Do-while loop can end without semicolon
    ConsumeIf_(';');
    return new DoStatement(body, cond);
}

Handle<Statement> Parser::ParseWhile() {
    // Consume while
    Advance_();
    if (t0_->type != '(') {
        Exceptions::ThrowSyntaxError("Expected ( after while");
    }
    Advance_();
    Handle<Expression> cond = ParseExpression();
    if (t0_->type != ')') {
        Exceptions::ThrowSyntaxError("Parenthesis is not enclosed");
    }
    Advance_();
    Handle<Statement> body = ParseStatement();
    ConsumeSemicolon_();
    return new WhileStatement(cond, body);
}

Handle<Statement> Parser::ParseFor() {
    // Consume for
    Advance_();
    throw "TODO: For not supported";
}

Handle<Statement> Parser::ParseContinue() {
    // Consume continue
    Advance_();
    if (t0_->type != Token::kIdentifier || (t0_->flags&Token::kLineBefore)) {
        ConsumeSemicolon_();
        return new ContinueStatement(nullptr);
    }
    Handle<JSString> name = (JSString*)t0_->value;
    Advance_();
    ConsumeSemicolon_();
    return new ContinueStatement(name);
}

Handle<Statement> Parser::ParseBreak() {
    // Consume break
    Advance_();
    if (t0_->type != Token::kIdentifier || (t0_->flags&Token::kLineBefore)) {
        ConsumeSemicolon_();
        return new BreakStatement(nullptr);
    }
    Handle<JSString> name = (JSString*)t0_->value;
    Advance_();
    ConsumeSemicolon_();
    return new BreakStatement(name);
}

Handle<Statement> Parser::ParseReturn() {
    // Consume return
    Advance_();
    Handle<Expression> expr;
    if (t0_->type == ';') {
        Advance_();
    } else if(t0_->type != '}' && !(t0_->flags&Token::kLineBefore)) {
        expr = ParseExpression();
        ConsumeSemicolon_();
    }
    return new ReturnStatement(expr);
}

Handle<Statement> Parser::ParseWith() {
    // Consume with
    Advance_();
    if (t0_->type != '(') {
        Exceptions::ThrowSyntaxError("Expected ( after with");
    }
    Advance_();
    Handle<Expression> cond = ParseExpression();
    if (t0_->type != ')') {
        Exceptions::ThrowSyntaxError("Parenthesis is not enclosed");
    }
    Advance_();
    Handle<Statement> body = ParseStatement();
    ConsumeSemicolon_();
    return new WithStatement(cond, body);
}

Handle<Statement> Parser::ParseSwitch() {
    Advance_();
    if (!ConsumeIf_('(')) {
        Exceptions::ThrowSyntaxError("Expected ( after switch");
    }
    Handle<Expression> expr = ParseExpression();
    if (!ConsumeIf_(')')) {
        Exceptions::ThrowSyntaxError("Parenthesis mismatch");
    }
    if (!ConsumeIf_('{')) {
        Exceptions::ThrowSyntaxError("Expected { after switch");
    }
    ArrayList<SwitchClause> clauses;
    Handle<Expression> cond;
    Handle<Array<Statement>> stmt;
    bool defDefined = false;
    while (true) {
        switch (t0_->type) {
            case Token::kCase:
                Advance_();
                cond = ParseExpression();
                break;
            case Token::kDefault:
                if (defDefined) {
                    Exceptions::ThrowSyntaxError("Duplicate default clause");
                }
                defDefined = true;
                Advance_();
                break;
            default:
                goto finish;
        }
        if (!ConsumeIf_(':')) {
            Exceptions::ThrowSyntaxError("Expected : after case clause");
        }
        stmt = ParseStatementList();
        clauses.Add(new SwitchClause(cond, stmt));
    }
finish:
    if (!ConsumeIf_('}')) {
        Exceptions::ThrowSyntaxError("Expected } in switch statement");
    }
    Handle<Array<SwitchClause>> clauseArray = clauses.ToArray();
    return new SwitchStatement(expr, clauseArray);
}

Handle<Statement> Parser::ParseLabelled() {
    Handle<JSString> name = (JSString*)t0_->value;
    // Consume Identifier
    Advance_();
    // Consume :
    Advance_();
    Handle<Statement> stmt;
    if (t0_->type == Token::kFunction) {
        stmt = ParseFunctionOrGeneratorStatement();
    } else {
        stmt = ParseStatement();
    }
    return new LabelledStatement(name, stmt);
}

Handle<Statement> Parser::ParseThrow() {
    // Consume throw
    Advance_();
    if (t0_->flags&Token::kLineBefore) {
        Exceptions::ThrowSyntaxError("No line terminator allowed after throw");
    }
    Handle<Expression> expr = ParseExpression();
    ConsumeSemicolon_();
    return new ThrowStatement(expr);
}

Handle<Statement> Parser::ParseTry() {
    // Consume try
    Advance_();
    Handle<BlockStatement> body = ParseBlock();
    Handle<Expression> param;
    Handle<BlockStatement> error;
    if (ConsumeIf_(Token::kCatch)) {
        if (!ConsumeIf_('(')) {
            Exceptions::ThrowSyntaxError("Expected ( after catch");
        }
        param = ParseAssignmentExpr();
        if (!ConsumeIf_(')')) {
            Exceptions::ThrowSyntaxError("Parenthesis mismatch");
        }
        error = ParseBlock();
    }
    Handle<BlockStatement> finally;
    if (ConsumeIf_(Token::kFinally)) {
        finally = ParseBlock();
    }

    if (!param && !finally) {
        Exceptions::ThrowSyntaxError("catch and finally cannot both be missing");
    }

    return new TryStatement(body, param, error, finally);
}

Handle<FunctionExpression> Parser::ParseFunctionOrGeneratorExpression() {
    // Consume function
    Advance_();
    bool generator = ConsumeIf_('*');
    Handle<JSString> name;
    if (t0_->type == Token::kIdentifier/*yield*/) {
        name = (JSString*)t0_->value;
        Advance_();
    }

    Handle<Array<Expression>> param = ParseCoveredFormals();

    if (!ConsumeIf_('{')) {
        Exceptions::ThrowSyntaxError("Expected { in function definition");
    }

    // TODO generator and yield
    Handle<Array<Statement>> body = ParseStatementList(true);

    if (!ConsumeIf_('}')) {
        Exceptions::ThrowSyntaxError("Brace mismatch in function definition");
    }

    return new FunctionExpression(generator, name, param, body);
}

Handle<Statement> Parser::ParseFunctionOrGeneratorStatement() {
    Handle<FunctionExpression> func = ParseFunctionOrGeneratorExpression();
    return new FunctionStatement(func);
}

Handle<Script> Parser::ParseScript() {
    Handle<Array<Statement>> stmts = ParseStatementList(true);
    if (t0_->type != Token::kEOF) {
        Exceptions::ThrowSyntaxError("Excessive token");
    }
    return new Script(stmts);
}
