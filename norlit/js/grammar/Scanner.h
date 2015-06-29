#ifndef NORLIT_JS_GRAMMAR_LEXER_H
#define NORLIT_JS_GRAMMAR_LEXER_H

#include "../JSString.h"
#include "../../gc/Handle.h"

namespace norlit {
namespace js {
namespace grammar {

class Token;

class Scanner {
  private:
    gc::Handle<JSString> content;
    int ptr = 0;
    int tokenStart_;
    bool lineBefore_ = false;

    int c0_;

    void Fetch_();
    void Advance_(int number = 1);


    bool htmlLikeComments_ = true;

    int Lookahead_(int distance = 0);
    void Pushback_(int number = 1);

    bool LookaheadMatches_(const char*);
    void Expect_(const char* str);

    /* Token position marking family */
    void Start_() {
        tokenStart_ = ptr;
    }
    template<uint8_t base>
    void ScanDecimal_(uint64_t& result, int16_t& length, int16_t* overflow);
    int16_t ScanExp_();

    int NextUnicodeEscapeSequence();
    void NextLineComment();
    void NextBlockComment();
    void NextHTMLOpenComment();
    void NextHTMLCloseComment();
    void SkipWhitespace();
    gc::Handle<Token> Wrap(gc::Handle<Token>&& tok);
    gc::Handle<Token> ScanTemplateCharacters_();
    gc::Handle<Token> NextIdentifierName();
    gc::Handle<Token> NextDecimal();
    gc::Handle<Token> NextNumber();
    gc::Handle<Token> NextString();
    gc::Handle<Token> NextTemplate();

  public:
    void Dump(const gc::Handle<Token>&);

    void TranslateIdentifierName(const gc::Handle<Token>&);
    gc::Handle<Token> NextToken();
    gc::Handle<Token> NextRegexp(const gc::Handle<Token>&);
    gc::Handle<Token> NextTemplatePart();
  public:
    static bool IsWhitespace(int ch);
    static bool IsLineTerminator(int ch);
    static bool IsIdentifierStart(int ch);
    static bool IsIdentifierPart(int ch);
    static bool IsHexDigit(int ch);
    static int GetDigit(int ch);

    Scanner(const gc::Handle<JSString>&);
};

}
}
}

#endif
