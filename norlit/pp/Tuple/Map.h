#ifndef NORLIT_PP_TUPLE_MAP_H
#define NORLIT_PP_TUPLE_MAP_H

#include "Get.h"
#include "Size.h"
#include "../Math/Dec.h"
#include "../Math/Inc.h"
#include "../Iterate/Repeat.h"
#include "../Control/If.h"

#define NORLIT_PP_MAP_LEAVE(...) __VA_ARGS__
#define NORLIT_PP_MAP_EAT(...)

#define NORLIT_PP_MAP_CALLBACK(x, arg) \
	NORLIT_PP_IF(NORLIT_PP_TOBOOL(NORLIT_PP_DEC(x)),NORLIT_PP_MAP_LEAVE,NORLIT_PP_MAP_EAT)(,) NORLIT_PP_TUPLE_GET(arg,1) (NORLIT_PP_TUPLE_GET(NORLIT_PP_TUPLE_GET(arg,2),x), NORLIT_PP_TUPLE_GET(arg,3))

#define NORLIT_PP_MAP(callback,tuple,extra) \
	NORLIT_PP_REPEAT(NORLIT_PP_MAP_CALLBACK, NORLIT_PP_TUPLE_SIZE(tuple), (callback, tuple, extra))\


#endif
