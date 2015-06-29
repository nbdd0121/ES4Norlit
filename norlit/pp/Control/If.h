#ifndef NORLIT_PP_IF_H
#define NORLIT_PP_IF_H

#include "../Basic.h"

#define NORLIT_PP_IF(cond,t,f) NORLIT_PP_CONCAT_2(NORLIT_PP_IF_,cond)(t,f)
#define NORLIT_PP_IF_0(t,f) f
#define NORLIT_PP_IF_1(t,f) t

#endif