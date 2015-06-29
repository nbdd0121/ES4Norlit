#include "../all.h"

#include "Scanner.h"
#include "Unicode.h"
#include "Token.h"

#include "../JSNumber.h"
#include "../../util/HashMap.h"
#include "../../util/StaticBlock.h"
#include "../../util/TaggedInteger.h"
#include "../../util/Double2String.h"

#include <string>
#include <cstdio>

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::grammar;
using namespace norlit::util;

void Scanner::Dump(const Handle<Token>& t) {
    if (t->type < 0x80) {
        printf("[Punctuator %c]\n", t->type);
        return;
    }
    if (t->type & 0x80) {
        printf("[Punctuator %c=]\n", t->type - 0x80);
        return;
    }
    if (t->type & 0x100) {
        printf("[Punctuator %c%c]\n", t->type - 0x100, t->type - 0x100);
        return;
    }
    switch (t->type) {
        case Token::kEllipse:
            printf("[Punctuator ...]\n");
            break;
        case Token::kStrictEq:
            printf("[Punctuator ===]\n");
            break;
        case Token::kStrictIneq:
            printf("[Punctuator !==]\n");
            break;
        case Token::kURShift:
            printf("[Punctuator >>>]\n");
            break;
        case Token::kLShiftAssign:
            printf("[Punctuator <<=]\n");
            break;
        case Token::kRShiftAssign:
            printf("[Punctuator >>=]\n");
            break;
        case Token::kURShiftAssign:
            printf("[Punctuator >>>=]\n");
            break;
        case Token::kLambda:
            printf("[Punctuator =>]\n");
            break;
        case Token::kEOF:
            printf("[EOF]\n");
            break;
        case Token::kString: {
            Handle<JSString> str = (JSString*)t->value;
            printf("[StringLiteral %s]\n", &str->ToCString()->At(0));
            break;
        }
        case Token::kIdentifier: {
            Handle<JSString> str = (JSString*)t->value;
            printf("[Identifier %s]\n", &str->ToCString()->At(0));
            break;
        }
        case Token::kTemplateHead: {
            Handle<ValueArray<char>> cstr = ((JSString*)t->value)->ToCString();
            Handle<ValueArray<char>> rstr = ((JSString*)t->additional)->ToCString();
            printf("[TemplateHead %s; Raw: %s]\n", &cstr->At(0), &rstr->At(0));
            break;
        }
        case Token::kNoSubTemplate: {
            Handle<ValueArray<char>> cstr = ((JSString*)t->value)->ToCString();
            Handle<ValueArray<char>> rstr = ((JSString*)t->additional)->ToCString();
            printf("[NoSubstitutionTemplate %s; Raw: %s]\n", &cstr->At(0), &rstr->At(0));
            break;
        }
        case Token::kNumber: {
            Handle<JSNumber> val = (JSNumber*)t->value;
            printf("[NumberLiteral %lf]\n", val->Value());
            break;
        }
        case Token::kRegexp: {
            Handle<ValueArray<char>> rstr = ((JSString*)t->value)->ToCString();
            Handle<ValueArray<char>> fstr = ((JSString*)t->additional)->ToCString();
            printf("[Regexp /%s/%s]\n", &rstr->At(0), &fstr->At(0));
            break;
        }
        case Token::kBreak:
        case Token::kCase:
        case Token::kCatch:
        case Token::kClass:
        case Token::kConst:
        case Token::kContinue:
        case Token::kDebugger:
        case Token::kDefault:
        case Token::kDelete:
        case Token::kDo:
        case Token::kElse:
        case Token::kExport:
        case Token::kExtends:
        case Token::kFinally:
        case Token::kFor:
        case Token::kFunction:
        case Token::kIf:
        case Token::kImport:
        case Token::kIn:
        case Token::kInstanceof:
        case Token::kNew:
        case Token::kReturn:
        case Token::kSuper:
        case Token::kSwitch:
        case Token::kThis:
        case Token::kThrow:
        case Token::kTry:
        case Token::kTypeof:
        case Token::kVar:
        case Token::kVoid:
        case Token::kWhile:
        case Token::kWith:
        case Token::kYield:
        case Token::kNull:
        case Token::kTrue:
        case Token::kFalse: {
            Handle<JSString> str = (JSString*)t->value;
            printf("[%s]\n", &str->ToCString()->At(0));
            break;
        }
        default:
            Exceptions::ThrowSyntaxError("UNK");
    }
}

void Scanner::Fetch_() {
    if ((unsigned)ptr >= content->Length()) {
        c0_ = -1;
    } else {
        c0_ = content->At(ptr);
    }
}

void Scanner::Advance_(int number) {
    ptr += number;
    Fetch_();
}

int Scanner::Lookahead_(int distance) {
    if ((unsigned)ptr + distance >= content->Length()) {
        return -1;
    } else {
        return content->At(ptr + distance);
    }
}

void Scanner::Pushback_(int number) {
    ptr -= number;
    Fetch_();
}

bool Scanner::LookaheadMatches_(const char* str) {
    for (int dist = 0; *str; str++, dist++) {
        if (Lookahead_(dist) != *str) {
            return false;
        }
    }
    return true;
}

void Scanner::Expect_(const char* str) {
    for (; *str; str++) {
        if (c0_ != *str) {
            Exceptions::ThrowSyntaxError("Assertion failure");
        }
        Advance_();
    }
}

Handle<Token> Scanner::Wrap(Handle<Token>&& tok) {
    if (lineBefore_) {
        tok->flags |= Token::kLineBefore;
        lineBefore_ = false;
    }
    return tok;
}

/* 11.4 Comments */
void Scanner::NextLineComment() {
    Expect_("//");
    while (c0_ != -1 && !IsLineTerminator(c0_)) {
        Advance_();
    }
}

/* 11.4 Comments */
void Scanner::NextBlockComment() {
    Expect_("/*");
    while (true) {
        if (c0_ == '*') {
            Advance_();
            if (c0_ == '/') {
                Advance_();
                break;
            }
        } else if (c0_ == -1) {
            Exceptions::ThrowSyntaxError("Block comment is not enclosed");
        } else if (IsLineTerminator(c0_)) {
            lineBefore_ = true;
            Advance_();
        } else {
            Advance_();
        }
    }
}

/* 11.4 Comments */
void Scanner::NextHTMLOpenComment() {
    Expect_("<!--");
    while (c0_ != -1 && !IsLineTerminator(c0_)) {
        Advance_();
    }
}

/* 11.4 Comments */
void Scanner::NextHTMLCloseComment() {
    Expect_("-->");
    while (c0_ != -1 && !IsLineTerminator(c0_)) {
        Advance_();
    }
}

/* 11.2, 11.3, 11.4 combined */
void Scanner::SkipWhitespace() {
    while (true) {
        int ch = c0_;
        Advance_();
        if (ch == -1) {
            return;
        } else if (IsWhitespace(ch)) {
            continue;
        } else if (IsLineTerminator(ch)) {
            lineBefore_ = true;
        } else if (ch == '/') {
            int llh = c0_;
            Pushback_();
            if (llh == '/') {
                NextLineComment();
            } else if (llh == '*') {
                NextBlockComment();
            } else {
                return;
            }
        } else if (htmlLikeComments_ && ch == '<') {
            Pushback_();
            if (LookaheadMatches_("<!--")) {
                NextHTMLOpenComment();
            } else {
                return;
            }
        } else if (htmlLikeComments_ && lineBefore_ && ch == '-') {
            Pushback_();
            if (LookaheadMatches_("-->")) {
                NextHTMLCloseComment();
            } else {
                return;
            }
        } else {
            Pushback_();
            return;
        }
    }
}

/* ES6 11.6 */
Handle<Token> Scanner::NextIdentifierName() {
    std::wstring buf;
    int id = c0_;
    bool escaped = false;
    Advance_();
    if (id == '\\') {
        id = NextUnicodeEscapeSequence();
        if (!IsIdentifierStart(id)) {
            Exceptions::ThrowSyntaxError("Unicode escape sequence should be proper identifier start");
        }
        escaped = true;
    } else if (!IsIdentifierStart(id)) {
        Exceptions::ThrowSyntaxError("Expected identifier start");
    }
    buf += id;
    while (true) {
        int nxt = c0_;
        Advance_();
        if (nxt == '\\') {
            int esc = NextUnicodeEscapeSequence();
            if (!IsIdentifierPart(esc)) {
                Exceptions::ThrowSyntaxError("Unicode escape sequence should be proper identifier part");
            }
            escaped = true;
            buf += esc;
        } else if (IsIdentifierPart(nxt)) {
            buf += nxt;
        } else {
            Pushback_();
            break;
        }
    }
    Handle<JSString> str = JSString::New(buf.c_str());
    return Wrap(new Token(Token::kIdentifier, escaped ? Token::kEscaped:0, str));
}

void Scanner::TranslateIdentifierName(const Handle<Token>& tok) {
    static HashMap<JSString, TaggedInteger> map;
    NORLIT_STATIC_BLOCK {
        map.Put(JSString::New("break"), TaggedInteger::New(Token::kBreak));
        map.Put(JSString::New("case" ), TaggedInteger::New(Token::kCase));
        map.Put(JSString::New("catch"), TaggedInteger::New(Token::kCatch));
        map.Put(JSString::New("class"), TaggedInteger::New(Token::kClass));
        map.Put(JSString::New("const"), TaggedInteger::New(Token::kConst));
        map.Put(JSString::New("continue"), TaggedInteger::New(Token::kContinue));
        map.Put(JSString::New("debugger"), TaggedInteger::New(Token::kDebugger));
        map.Put(JSString::New("default"), TaggedInteger::New(Token::kDefault));
        map.Put(JSString::New("delete"), TaggedInteger::New(Token::kDelete));
        map.Put(JSString::New("do"), TaggedInteger::New(Token::kDo));
        map.Put(JSString::New("else"), TaggedInteger::New(Token::kElse));
        map.Put(JSString::New("export"), TaggedInteger::New(Token::kExport));
        map.Put(JSString::New("extends"), TaggedInteger::New(Token::kExtends));
        map.Put(JSString::New("finally"), TaggedInteger::New(Token::kFinally));
        map.Put(JSString::New("for"), TaggedInteger::New(Token::kFor));
        map.Put(JSString::New("function"), TaggedInteger::New(Token::kFunction));
        map.Put(JSString::New("if"), TaggedInteger::New(Token::kIf));
        map.Put(JSString::New("import"), TaggedInteger::New(Token::kImport));
        map.Put(JSString::New("in"), TaggedInteger::New(Token::kIn));
        map.Put(JSString::New("instanceof"), TaggedInteger::New(Token::kInstanceof));
        map.Put(JSString::New("new"), TaggedInteger::New(Token::kNew));
        map.Put(JSString::New("return"), TaggedInteger::New(Token::kReturn));
        map.Put(JSString::New("super"), TaggedInteger::New(Token::kSuper));
        map.Put(JSString::New("switch"), TaggedInteger::New(Token::kSwitch));
        map.Put(JSString::New("this"), TaggedInteger::New(Token::kThis));
        map.Put(JSString::New("throw"), TaggedInteger::New(Token::kThrow));
        map.Put(JSString::New("try"), TaggedInteger::New(Token::kTry));
        map.Put(JSString::New("typeof"), TaggedInteger::New(Token::kTypeof));
        map.Put(JSString::New("var"), TaggedInteger::New(Token::kVar));
        map.Put(JSString::New("void"), TaggedInteger::New(Token::kVoid));
        map.Put(JSString::New("while"), TaggedInteger::New(Token::kWhile));
        map.Put(JSString::New("with"), TaggedInteger::New(Token::kWith));
        map.Put(JSString::New("yield"), TaggedInteger::New(Token::kYield));
        map.Put(JSString::New("null"), TaggedInteger::New(Token::kNull));
        map.Put(JSString::New("true"), TaggedInteger::New(Token::kTrue));
        map.Put(JSString::New("false"), TaggedInteger::New(Token::kFalse));

        map.Put(JSString::New("enum"), TaggedInteger::New(Token::kReserved));
        map.Put(JSString::New("await"), TaggedInteger::New(Token::kReserved));
    };

    // Escaped identifiers are not keywords
    if (tok->flags&Token::kEscaped) {
        return;
    }

    Handle<TaggedInteger> value = map.Get((JSString*)tok->value);
    if (!value)
        return;
    uint16_t type = value->Value();
    if (type == Token::kReserved) {
        Exceptions::ThrowSyntaxError("Future reserved keywords cannot be used as identifiers");
    }
    tok->type = type;

}

int Scanner::NextUnicodeEscapeSequence() {
    if (c0_ != 'u') {
        Exceptions::ThrowSyntaxError("Expected unicode escape sequence");
    }
    Advance_();

    if (c0_ == '{') {
        Advance_();
        int val = 0;
        while (true) {
            int d = c0_;
            Advance_();
            if (d == '}') {
                return val;
            } else if (IsHexDigit(d)) {
                val *= 16;
                val += GetDigit(d);
            } else {
                Exceptions::ThrowSyntaxError("Expected hex digits in unicode escape sequence");
            }
        }
    } else {
        int val = 0;
        for (int i = 0; i < 4; i++) {
            int d = c0_;
            Advance_();
            if (IsHexDigit(d)) {
                val *= 16;
                val += GetDigit(d);
            } else {
                Exceptions::ThrowSyntaxError("Expected hex digits in unicode escape sequence");
            }
        }
        return val;
    }
}

template<uint8_t base>
void Scanner::ScanDecimal_(uint64_t& ret, int16_t& length, int16_t* overflow) {
    bool overflowFlag = false;
    while (true) {
        if (IsHexDigit(c0_)) {
            int val = GetDigit(c0_);
            if (val >= base) {
                return;
            } else if (!overflowFlag) {
                if (ret < (0x7FFFFFFFFFFFFFFFULL / base)) {
                    ret = ret * base + val;
                    length++;
                } else {
                    overflowFlag = true;
                    if (val >= base / 2) {
                        ret++;
                    }
                    if (overflow) {
                        (*overflow)++;
                    }
                }
            } else if (overflow) {
                (*overflow)++;
            }
            Advance_();
        } else {
            return;
        }
    }
}

int16_t Scanner::ScanExp_() {
    int16_t expValue = 0;
    bool negative = false;

    if (c0_ == '+') {
        Advance_();
    } else if(c0_ == '-') {
        negative = true;
        Advance_();
    }

    size_t expPartLength = 0;
    while (true) {
        if (c0_ >= '0' && c0_ <= '9') {
            // If expValue > 3275, then a overflow is possible to happen
            // Since such a large expValue will cause the result to become infinity or 0
            // It is okay to fix it as this value and not modify it
            if (expValue < 3275) {
                expValue = expValue * 10 + (c0_ - '0');
                expPartLength++;
            }
            Advance_();
        } else {
            break;
        }
    }

    if (negative) expValue = -expValue;

    if (!expPartLength) {
        Exceptions::ThrowSyntaxError("Expected +, - or digits after the exponential mark");
    }

    return expValue;
}

/* Numeric Literals 11.8.3 */
Handle<Token> Scanner::NextDecimal() {
    Start_();

    uint64_t s = 0;
    int16_t n = 0;
    int16_t k = 0;

    if (c0_ == '.') {
        Advance_();
        ScanDecimal_<10>(s, k, nullptr);
        assert(k);
    } else {
        ScanDecimal_<10>(s, k, &n);
        n += k;
        if (c0_ == '.') {
            Advance_();
            ScanDecimal_<10>(s, k, nullptr);
        }
    }
    /* Exponential part */{
        if (c0_ == 'e' || c0_ == 'E') {
            Advance_();
            n += ScanExp_();
        }
    }

    double value = AssembleDouble(s, n, static_cast<uint8_t>(k));

    if (c0_ == '\\' || IsIdentifierStart(c0_)) {
        Exceptions::ThrowSyntaxError("Unexpected character after number literal");
    }

    Handle<JSNumber> obj = JSNumber::New(value);
    return Wrap(new Token(Token::kNumber, 0, std::move(obj)));
}

Handle<Token> Scanner::NextNumber() {
    if (c0_ != '0') {
        return NextDecimal();
    }
    Advance_();

    int base;
    uint64_t result = 0;
    int16_t length = 0;
    int16_t overflow = 0;
    bool legacy = false;

    if (c0_ == 'b' || c0_ == 'B') {
        base = 2;
        Advance_();
        ScanDecimal_<2>(result, length, &overflow);
    } else if (c0_ == 'o' || c0_ == 'O') {
        base = 8;
        Advance_();
        ScanDecimal_<8>(result, length, &overflow);
    } else if (c0_ == 'x' || c0_ == 'X') {
        base = 16;
        Advance_();
        ScanDecimal_<16>(result, length, &overflow);
    } else if (c0_ >= '0' && c0_ <= '7') {
        base = 8;
        legacy = true;
        ScanDecimal_<8>(result, length, &overflow);
    } else if (c0_ == '8' || c0_ == '9') {
        Handle<Token> tk = NextDecimal();
        tk->flags |= Token::kLegacy;
        return tk;
    } else {
        Pushback_();
        return NextDecimal();
    }

    if ((c0_ >= '0'&&c0_ <= '9') || c0_ == '\\' || IsIdentifierStart(c0_)) {
        Exceptions::ThrowSyntaxError("Unexpected character after number literal");
    }

    Handle<JSNumber> val;
    if (overflow) {
        double value = static_cast<double>(result);
        if (overflow)
            value *= pow(static_cast<double>(base), static_cast<double>(overflow));
        val = JSNumber::New(value);
    } else {
        val = JSNumber::New(static_cast<int64_t>(result));
    }

    return Wrap(new Token(Token::kNumber, legacy ? Token::kLegacy : 0, std::move(val)));
}

Handle<Token> Scanner::NextString() {
    int quote = c0_;
    assert(quote == '\'' || quote == '"');
    Advance_();

    std::wstring value;
    uint8_t flags = 0;

    while (true) {
        switch (c0_) {
            case '"':
            case '\'': {
                if (quote != c0_) {
                    value += c0_;
                    break;
                }
                goto finish;
            }
            case '\\': {
                flags |= Token::kEscaped;
                Advance_();
                switch (c0_) {
                    case '\r':
                        Advance_();
                        // \r\n is considered as one line terminator
                        if (c0_ != '\n') {
                            continue;
                        }
                        break;
                    case '\n':
                    case 0x2028:
                    case 0x2029:
                        break;
                    case '\'':
                        value += '\'';
                        break;
                    case '"':
                        value += '"';
                        break;
                    case '\\':
                        value += '\\';
                        break;
                    case 'b':
                        value += '\b';
                        break;
                    case 'f':
                        value += '\f';
                        break;
                    case 'n':
                        value += '\n';
                        break;
                    case 'r':
                        value += '\r';
                        break;
                    case 't':
                        value += '\t';
                        break;
                    case 'v':
                        value += '\v';
                        break;
                    case '0': {
                        Advance_();
                        if (c0_ < '0' || c0_ > '7') {
                            value += (wchar_t)0;
                            continue;
                        }
                        Pushback_();
                    }
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7': {
                        char16_t val = c0_ - '0';
                        flags |= Token::kLegacy;
                        Advance_();

                        if (c0_ < '0' || c0_ > '7') {
                            value += val;
                            break;
                        }
                        val = val * 8 + (c0_ - '0');
                        Advance_();

                        if (c0_ < '0' || c0_ > '7' || val > 040) {
                            value += val;
                        } else {
                            val = val * 8 + (c0_ - '0');
                            Advance_();
                            value += val;
                        }
                        continue;
                    }
                    case 'x': {
                        char16_t val = 0;
                        Advance_();
                        if (IsHexDigit(c0_)) {
                            val = GetDigit(c0_);
                            Advance_();
                        } else {
                            Exceptions::ThrowSyntaxError("Expected hex digits in hexical escape sequence");
                        }
                        if (IsHexDigit(c0_)) {
                            val = val * 16 + GetDigit(c0_);
                            Advance_();
                        } else {
                            Exceptions::ThrowSyntaxError("Expected hex digits in hexical escape sequence");
                        }
                        value += val;
                        continue;
                    }
                    case 'u':
                        value += NextUnicodeEscapeSequence();
                        continue;
                    default:
                        value += c0_;
                        break;
                }
                break;
            }
            case -1:
            case '\r':
            case '\n':
            case 0x2028:
            case 0x2029:
                Exceptions::ThrowSyntaxError("String literal is not enclosed");
            default:
                value += c0_;
                break;
        }
        Advance_();
    }
finish:
    Advance_();
    Handle<JSString> str = JSString::New(value.c_str());
    return Wrap(new Token(Token::kString, flags, str));
}

/* Template Literal Lexical Components 11.8.6 */
Handle<Token> Scanner::NextTemplate() {
    Expect_("`");
    Handle<Token> tok = ScanTemplateCharacters_();
    if (c0_ == '`') {
        Advance_();
    } else {
        Expect_("${");
        tok->type = Token::kTemplateHead;
    }
    return tok;
}

Handle<Token> Scanner::NextTemplatePart() {
    Handle<Token> tok = ScanTemplateCharacters_();
    if (c0_ == '`') {
        Advance_();
        tok->type = Token::kTemplateTail;
    } else {
        Expect_("${");
        tok->type = Token::kTemplateMiddle;
    }
    return tok;
}

Handle<Token> Scanner::ScanTemplateCharacters_() {
    std::wstring cooked;
    std::wstring raw;
    while (true) {
        switch (c0_) {
            case -1:
                Exceptions::ThrowSyntaxError("Template Literal is not enclosed");
                break;
            case '`':
                goto finish;
            case '$':
                Advance_();
                if (c0_ == '{') {
                    Pushback_();
                    goto finish;
                } else {
                    cooked += '$';
                    raw += '$';
                    continue;
                }
            case '\\': {
                Advance_();
                switch (c0_) {
                    case '\r':
                        Advance_();
                        raw += L"\\\n";
                        if (c0_ != '\n') {
                            continue;
                        }
                        break;
                    case '\n':
                    case 0x2028:
                    case 0x2029:
                        raw += '\\';
                        raw += c0_;
                        break;
                    case '\'':
                    case '"':
                    case '\\':
                        cooked += c0_;
                        raw += '\\';
                        raw += c0_;
                        break;
                    case 'b':
                        cooked += '\b';
                        raw += L"\\b";
                        break;
                    case 'f':
                        cooked += '\f';
                        raw += L"\\f";
                        break;
                    case 'n':
                        cooked += '\n';
                        raw +=L"\\n";
                        break;
                    case 'r':
                        cooked += '\r';
                        raw +=L"\\r";
                        break;
                    case 't':
                        cooked += '\t';
                        raw +=L"\\t";
                        break;
                    case 'v':
                        cooked += '\v';
                        raw +=L"\\v";
                        break;
                    case '0': {
                        Advance_();
                        if (c0_ >= '0' && c0_ <= '7') {
                            Exceptions::ThrowSyntaxError("Octal escape sequences are not allowed in template literal");
                        }
                        cooked += (wchar_t)0;
                        raw +=L"\\0";
                        continue;
                    }
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                        Exceptions::ThrowSyntaxError("Octal escape sequences are not allowed in template literal");
                    case 'x': {
                        char16_t val = 0;
                        Advance_();

                        raw += L"\\x";
                        if (IsHexDigit(c0_)) {
                            raw += c0_;
                            val = GetDigit(c0_);
                            Advance_();
                        } else {
                            Exceptions::ThrowSyntaxError("Expected hex digits in hexical escape sequence");
                        }
                        if (IsHexDigit(c0_)) {
                            raw += c0_;
                            val = val * 16 + GetDigit(c0_);
                            Advance_();
                        } else {
                            Exceptions::ThrowSyntaxError("Expected hex digits in hexical escape sequence");
                        }
                        cooked += val;
                        continue;
                    }
                    case 'u': {
                        int start = ptr;
                        cooked += NextUnicodeEscapeSequence();
                        raw += '\\';
                        for (int i = start; i < ptr; i++) {
                            raw += content->At(i);
                        }
                        continue;
                    }
                    default:
                        cooked += c0_;
                        raw += '\\';
                        raw += c0_;
                        break;
                }
                break;
            }
            case '\r':
                Advance_();
                cooked += '\n';
                raw += '\n';
                if (c0_ != '\n') {
                    continue;
                }
                break;
            default:
                cooked += c0_;
                raw += c0_;
                break;
        }
        Advance_();
    }
finish:
    Handle<JSString> cstr = JSString::New(cooked.c_str());
    Handle<JSString> rstr = JSString::New(raw.c_str());
    return Wrap(new Token(Token::kNoSubTemplate, 0, cstr, rstr));
}

Handle<Token> Scanner::NextRegexp(const Handle<Token>& div) {
    size_t startPos = ptr;
    if (div->type == Token::kDivAssign) {
        ptr--;
    }
    bool inClass = false;
    while (true) {
        switch (c0_) {
            case '/':
                Advance_();
                if (!inClass) {
                    goto scanFlags;
                }
                break;
            case '\\':
                Advance_();
                switch (c0_) {
                    case -1:
                    case '\r':
                    case '\n':
                    case 0x2028:
                    case 0x2029:
                        Exceptions::ThrowSyntaxError("Regexp literal is not enclosed");
                }
                Advance_();
                break;
            case '[':
                Advance_();
                inClass = true;
                break;
            case ']':
                Advance_();
                inClass = false;
                break;
            case -1:
            case '\r':
            case '\n':
            case 0x2028:
            case 0x2029:
                Exceptions::ThrowSyntaxError("Regexp literal is not enclosed");
            default:
                Advance_();
                break;
        }
    }
scanFlags:
    size_t flagStart = ptr;
    while (true) {
        if (c0_ == '\\') {
            Advance_();
            if (c0_ != 'u') {
                Exceptions::ThrowSyntaxError("Expected unicode escape sequence");
                continue;
            }
            int esc = NextUnicodeEscapeSequence();
            if (!IsIdentifierPart(esc)) {
                Exceptions::ThrowSyntaxError("Illegal identifier part in regexp flags");
            }
        } else if (IsIdentifierPart(c0_)) {
            Advance_();
        } else {
            break;
        }
    }
    Handle<JSString> rstr = content->Substring(startPos, flagStart - 1);
    Handle<JSString> fstr = content->Substring(flagStart, ptr);
    return new Token(Token::kRegexp, 0, rstr, fstr);
}

Handle<Token> Scanner::NextToken() {
    SkipWhitespace();
    switch (c0_) {
        case -1: {
            return new Token(Token::kEOF, Token::kLineBefore);
        }
        case '}': // Dealing with right brace as template tail is in parser
        case '{':
        case '(':
        case ')':
        case '[':
        case ']':
        case ';':
        case ',':
        case '~':
        case '?':
        case ':': {
            uint8_t ch = c0_;
            Advance_();
            return Wrap(new Token(ch));
        }
        case '.': {
            Advance_();
            if (c0_ >= '0' && c0_ <= '9') {
                Pushback_();
                return NextDecimal();
            }
            if (LookaheadMatches_("..")) {
                Advance_(2);
                return Wrap(new Token(Token::kEllipse));
            }
            return Wrap(new Token('.'));
        }
        case '<': {
            Advance_();
            if (c0_ == '<') {
                Advance_();
                if (c0_ == '=') {
                    Advance_();
                    return Wrap(new Token(Token::kLShiftAssign));
                } else {
                    return Wrap(new Token(Token::kLShift));
                }
            } else if (c0_ == '=') {
                Advance_();
                return Wrap(new Token(Token::kLteq));
            } else {
                return Wrap(new Token('<'));
            }
        }
        case '>': {
            Advance_();
            if (c0_ == '>') {
                Advance_();
                if (c0_ == '>') {
                    Advance_();
                    if (c0_ == '=') {
                        Advance_();
                        return Wrap(new Token(Token::kURShiftAssign));
                    } else {
                        return Wrap(new Token(Token::kURShift));
                    }
                } else if (c0_ == '=') {
                    Advance_();
                    return Wrap(new Token(Token::kRShiftAssign));
                } else {
                    return Wrap(new Token(Token::kRShift));
                }
            } else if (c0_ == '=') {
                Advance_();
                return Wrap(new Token(Token::kGteq));
            } else {
                return Wrap(new Token('>'));
            }
        }
        case '=':
            Advance_();
            if (c0_ == '>') {
                Advance_();
                return Wrap(new Token(Token::kLambda));
            }
            Pushback_();
        case '!': {
            int firstChar = c0_;
            Advance_();
            if (c0_ == '=') {
                Advance_();
                if (c0_ == '=') {
                    Advance_();
                    return Wrap(new Token(firstChar == '=' ? Token::kStrictEq : Token::kStrictIneq));
                } else {
                    return Wrap(new Token(firstChar + 0x80));
                }
            } else {
                return Wrap(new Token(firstChar));
            }
        }
        case '+':
        case '-':
        case '&':
        case '|': {
            int firstChar = c0_;
            Advance_();
            if (c0_ == firstChar) {
                Advance_();
                return Wrap(new Token(firstChar + 0x100));
            } else if (c0_ == '=') {
                Advance_();
                return Wrap(new Token(firstChar + 0x80));
            } else {
                return Wrap(new Token(firstChar));
            }
        }
        case '*':
        case '%':
        case '^':
        case '/': {
            int firstChar = c0_;
            Advance_();
            if (c0_ == '=') {
                Advance_();
                return Wrap(new Token(firstChar + 0x80));
            } else {
                return Wrap(new Token(firstChar));
            }
        }
        case '0':
            return NextNumber();
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return NextDecimal();
        case '`':
            return NextTemplate();
        case '"':
        case '\'':
            return NextString();
        case '\\':
            return NextIdentifierName();
        default:
            if (IsIdentifierStart(c0_)) {
                return NextIdentifierName();
            }
            Exceptions::ThrowTypeError((std::string("Unexpected character ") + (char)c0_ + " in source text").c_str());
    }
}

bool Scanner::IsWhitespace(int ch) {
    switch (ch) {
        case -1:
            return false;
        case '\t':
        case '\v':
        case '\f':
        case ' ':
        case 0xA0:
        case 0xFEFF:
            return true;
        default:
            return Unicode::IsSpace((char16_t)ch);
    }
}

bool Scanner::IsLineTerminator(int ch) {
    switch (ch) {
        case '\r':
        case '\n':
        case 0x2028:
        case 0x2029:
            return true;
        default:
            return false;
    }
}

bool Scanner::IsIdentifierStart(int ch) {
    switch (ch) {
        case -1:
            return false;
        case '$':
        case '_':
            return true;
    }
    return Unicode::IsIdStart(ch);
}

bool Scanner::IsIdentifierPart(int ch) {
    switch (ch) {
        case -1:
            return false;
        case '$':
        case '_':
        case 0x200C:
        case 0x200D:
            return true;
    }
    return Unicode::IsIdContinue(ch);
}

bool Scanner::IsHexDigit(int ch) {
    return
        (ch >= '0'&&ch <= '9') ||
        (ch >= 'a'&&ch <= 'f') ||
        (ch >= 'A'&&ch <= 'F');
}

int Scanner::GetDigit(int ch) {
    if (ch >= '0'&&ch <= '9') {
        return ch - '0';
    } else if (ch >= 'a'&&ch <= 'z') {
        return ch - 'a' + 10;
    } else if (ch >= 'A'&&ch <= 'Z') {
        return ch - 'A' + 10;
    } else {
        return -1;
    }
}

Scanner::Scanner(const Handle<JSString>& cnt) :content(cnt) {
    Fetch_();
}