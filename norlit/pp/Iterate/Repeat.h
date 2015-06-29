#ifndef NORLIT_PP_REPEAT_H
#define NORLIT_PP_REPEAT_H

#include "../Basic.h"

#define NORLIT_PP_REPEAT(callback,bound,arg) NORLIT_PP_CONCAT_2(NORLIT_PP_REPEAT_,bound)(callback,arg)

#define NORLIT_PP_REPEAT_0(callback,arg)
#define NORLIT_PP_REPEAT_1(callback,arg) NORLIT_PP_REPEAT_0(callback,arg) callback(1,arg)
#define NORLIT_PP_REPEAT_2(callback,arg) NORLIT_PP_REPEAT_1(callback,arg) callback(2,arg)
#define NORLIT_PP_REPEAT_3(callback,arg) NORLIT_PP_REPEAT_2(callback,arg) callback(3,arg)
#define NORLIT_PP_REPEAT_4(callback,arg) NORLIT_PP_REPEAT_3(callback,arg) callback(4,arg)
#define NORLIT_PP_REPEAT_5(callback,arg) NORLIT_PP_REPEAT_4(callback,arg) callback(5,arg)
#define NORLIT_PP_REPEAT_6(callback,arg) NORLIT_PP_REPEAT_5(callback,arg) callback(6,arg)
#define NORLIT_PP_REPEAT_7(callback,arg) NORLIT_PP_REPEAT_6(callback,arg) callback(7,arg)
#define NORLIT_PP_REPEAT_8(callback,arg) NORLIT_PP_REPEAT_7(callback,arg) callback(8,arg)
#define NORLIT_PP_REPEAT_9(callback,arg) NORLIT_PP_REPEAT_8(callback,arg) callback(9,arg)
#define NORLIT_PP_REPEAT_10(callback,arg) NORLIT_PP_REPEAT_9(callback,arg) callback(10,arg)
#define NORLIT_PP_REPEAT_11(callback,arg) NORLIT_PP_REPEAT_10(callback,arg) callback(11,arg)
#define NORLIT_PP_REPEAT_12(callback,arg) NORLIT_PP_REPEAT_11(callback,arg) callback(12,arg)
#define NORLIT_PP_REPEAT_13(callback,arg) NORLIT_PP_REPEAT_12(callback,arg) callback(13,arg)
#define NORLIT_PP_REPEAT_14(callback,arg) NORLIT_PP_REPEAT_13(callback,arg) callback(14,arg)
#define NORLIT_PP_REPEAT_15(callback,arg) NORLIT_PP_REPEAT_14(callback,arg) callback(15,arg)
#define NORLIT_PP_REPEAT_16(callback,arg) NORLIT_PP_REPEAT_15(callback,arg) callback(16,arg)
#define NORLIT_PP_REPEAT_17(callback,arg) NORLIT_PP_REPEAT_16(callback,arg) callback(17,arg)
#define NORLIT_PP_REPEAT_18(callback,arg) NORLIT_PP_REPEAT_17(callback,arg) callback(18,arg)
#define NORLIT_PP_REPEAT_19(callback,arg) NORLIT_PP_REPEAT_18(callback,arg) callback(19,arg)
#define NORLIT_PP_REPEAT_20(callback,arg) NORLIT_PP_REPEAT_19(callback,arg) callback(20,arg)
#define NORLIT_PP_REPEAT_21(callback,arg) NORLIT_PP_REPEAT_20(callback,arg) callback(21,arg)
#define NORLIT_PP_REPEAT_22(callback,arg) NORLIT_PP_REPEAT_21(callback,arg) callback(22,arg)
#define NORLIT_PP_REPEAT_23(callback,arg) NORLIT_PP_REPEAT_22(callback,arg) callback(23,arg)
#define NORLIT_PP_REPEAT_24(callback,arg) NORLIT_PP_REPEAT_23(callback,arg) callback(24,arg)
#define NORLIT_PP_REPEAT_25(callback,arg) NORLIT_PP_REPEAT_24(callback,arg) callback(25,arg)
#define NORLIT_PP_REPEAT_26(callback,arg) NORLIT_PP_REPEAT_25(callback,arg) callback(26,arg)
#define NORLIT_PP_REPEAT_27(callback,arg) NORLIT_PP_REPEAT_26(callback,arg) callback(27,arg)
#define NORLIT_PP_REPEAT_28(callback,arg) NORLIT_PP_REPEAT_27(callback,arg) callback(28,arg)
#define NORLIT_PP_REPEAT_29(callback,arg) NORLIT_PP_REPEAT_28(callback,arg) callback(29,arg)
#define NORLIT_PP_REPEAT_30(callback,arg) NORLIT_PP_REPEAT_29(callback,arg) callback(30,arg)
#define NORLIT_PP_REPEAT_31(callback,arg) NORLIT_PP_REPEAT_30(callback,arg) callback(31,arg)
#define NORLIT_PP_REPEAT_32(callback,arg) NORLIT_PP_REPEAT_31(callback,arg) callback(32,arg)

#endif