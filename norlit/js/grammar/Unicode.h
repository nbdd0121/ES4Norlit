#ifndef NORLIT_JS_GRAMMAR_UNICODE_H
#define NORLIT_JS_GRAMMAR_UNICODE_H

#include <cwchar>

namespace norlit {
namespace js {
namespace grammar {

class Unicode {
    static const unsigned char bmpTable[];
    static unsigned char getAttribute(char16_t codepoint);
  public:
    static bool IsIdStart(char16_t codepoint);
    static bool IsIdContinue(char16_t codepoint);
    static bool IsSpace(char16_t codepoint);
};

}
}
}

#endif
