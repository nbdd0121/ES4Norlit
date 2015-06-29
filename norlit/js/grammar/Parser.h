#ifndef NORLIT_JS_GRAMMAR_PARSER_H
#define NORLIT_JS_GRAMMAR_PARSER_H

#include "../JSString.h"
#include "../../gc/Handle.h"
#include "../../gc/Array.h"

namespace norlit {
namespace js {
namespace grammar {

class Scanner;
class Token;
class Expression;
class TemplateLiteral;
class CoveredFormals;
class ArrayLiteral;
class Statement;
class BlockStatement;
class Script;
class FunctionExpression;

class Parser {
  private:
    Scanner& scanner;
    gc::Handle<Token> t0_;
    gc::Handle<Token> t1_;

  private:
    void Fetch_();
    void FetchLookahead_();
    void Advance_();
    void AdvanceAndFetchIdentifierName_();

    bool ConsumeIf_(uint16_t type);
    void ConsumeSemicolon_();

  public:
    gc::Handle<Expression> ParsePrimaryExpr();
    gc::Handle<gc::Array<Expression>> ParseCoveredFormals();
    gc::Handle<Expression> ParseArrayLiteral();
    gc::Handle<Expression> RefineArrayAssignmentPattern(const gc::Handle<ArrayLiteral>&);
    gc::Handle<Expression> ParseObjectLiteral();

    gc::Handle<Expression> ParseLeftHandSideExpr(bool = false);

    gc::Handle<gc::Array<Expression>> ParseArguments();
    gc::Handle<TemplateLiteral> ParseTemplate();

    gc::Handle<Expression> ParsePostfixExpr();
    gc::Handle<Expression> ParseUnaryExpr();
    gc::Handle<Expression> ParseMulExpr();
    gc::Handle<Expression> ParseAddExpr();
    gc::Handle<Expression> ParseShiftExpr();
    gc::Handle<Expression> ParseRelationalExpr(bool);
    gc::Handle<Expression> ParseEqualityExpr(bool);
    gc::Handle<Expression> ParseAndExpr(bool);
    gc::Handle<Expression> ParseXorExpr(bool);
    gc::Handle<Expression> ParseOrExpr(bool);
    gc::Handle<Expression> ParseLAndExpr(bool);
    gc::Handle<Expression> ParseLOrExpr(bool);
    gc::Handle<Expression> ParseConditionalExpr(bool);
    gc::Handle<Expression> ParseAssignmentExpr(bool = false);
    gc::Handle<Expression> ParseExpression(bool = false);

    gc::Handle<Statement> ParseStatement();
    gc::Handle<Statement> ParseDeclaration();
    gc::Handle<BlockStatement> ParseBlock();
    gc::Handle<gc::Array<Statement>> ParseStatementList(bool = false);
    gc::Handle<Statement> ParseStatementListItem();

    gc::Handle<Statement> ParseVariable();
    gc::Handle<Statement> ParseExpressionStmt();
    gc::Handle<Statement> ParseIf();
    gc::Handle<Statement> ParseDo();
    gc::Handle<Statement> ParseWhile();
    gc::Handle<Statement> ParseFor();
    gc::Handle<Statement> ParseContinue();
    gc::Handle<Statement> ParseBreak();
    gc::Handle<Statement> ParseReturn();
    gc::Handle<Statement> ParseWith();
    gc::Handle<Statement> ParseSwitch();
    gc::Handle<Statement> ParseLabelled();
    gc::Handle<Statement> ParseThrow();
    gc::Handle<Statement> ParseTry();

    gc::Handle<FunctionExpression> ParseFunctionOrGeneratorExpression();
    gc::Handle<Statement> ParseFunctionOrGeneratorStatement();

    gc::Handle<Script> ParseScript();

    Parser(Scanner& s);
};

}
}
}

#endif
