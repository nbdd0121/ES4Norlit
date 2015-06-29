#ifndef NORLIT_PP_TUPLE_FOREACH_H
#define NORLIT_PP_TUPLE_FOREACH_H

#include "Get.h"
#include "Size.h"
#include "../Iterate/Repeat.h"

#define NORLIT_PP_FOREACH_CALLBACK(x, arg) \
	NORLIT_PP_TUPLE_GET(arg,1) (NORLIT_PP_TUPLE_GET(NORLIT_PP_TUPLE_GET(arg,2),x), NORLIT_PP_TUPLE_GET(arg,3))

#define NORLIT_PP_FOREACH(callback,tuple,extraArguments) \
	NORLIT_PP_REPEAT(NORLIT_PP_FOREACH_CALLBACK, NORLIT_PP_TUPLE_SIZE(tuple), (callback, tuple, extraArguments))\


#endif