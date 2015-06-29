#ifndef NORLIT_UTIL_DOUBLE2STRING_H
#define NORLIT_UTIL_DOUBLE2STRING_H

#include <cstdint>

namespace norlit {
namespace util {
uint8_t CountDigits(uint64_t n);
void DesembleDouble(double v, uint64_t& s, int16_t& n, uint8_t& k);
double AssembleDouble(uint64_t s, int16_t n, uint8_t k);
}
}

#endif