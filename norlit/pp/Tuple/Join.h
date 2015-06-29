#ifndef NORLIT_PP_TUPLE_JOIN_H
#define NORLIT_PP_TUPLE_JOIN_H

#include "Get.h"
#include "Size.h"
#include "../Math/Dec.h"
#include "../Math/Inc.h"
#include "../Iterate/Repeat.h"
#include "../Control/If.h"

#define NORLIT_PP_JOIN_CALLBACK(x, arg) \
	NORLIT_PP_TUPLE_GET(arg,2) NORLIT_PP_TUPLE_GET(arg,1) (NORLIT_PP_TUPLE_GET(NORLIT_PP_TUPLE_GET(arg,3),NORLIT_PP_INC(x)), NORLIT_PP_TUPLE_GET(arg,4))

#define NORLIT_PP_JOIN(callback,delim,tuple,extra) \
	NORLIT_PP_IF(NORLIT_PP_TUPLE_ISEMPTY(tuple),,\
		callback(NORLIT_PP_TUPLE_GET(tuple, 1))\
		NORLIT_PP_REPEAT(NORLIT_PP_JOIN_CALLBACK, NORLIT_PP_DEC(NORLIT_PP_TUPLE_SIZE(tuple)), (callback, delim, tuple, extra))\
	)

#endif