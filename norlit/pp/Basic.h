#ifndef NORLIT_PP_BASIC_H
#define NORLIT_PP_BASIC_H

#define NORLIT_PP_CONCAT_2_IMPL(a, b) a##b
#define NORLIT_PP_CONCAT_2(a, b) NORLIT_PP_CONCAT_2_IMPL(a, b)
#define NORLIT_PP_CONCAT_3(a, b, c) NORLIT_PP_CONCAT_2(NORLIT_PP_CONCAT_2(a, b),c)

#endif