// Get rid of MSVC's gmtime warning
#define _CRT_SECURE_NO_WARNINGS 1

#include "../all.h"
#include "../object/Exotics.h"
#include "Builtin.h"

#include <chrono>
#include <ctime>

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::js::builtin;

namespace {
const int64_t invalidDateValue = 0x4000000000000000LL;

// 20.3.1.2 Day Number and Time within Day
const int64_t msPerDay = 86400000;

int64_t Day(int64_t t) {
    return t / msPerDay;
}

int64_t TimeWithinDay(int64_t t) {
    return t % msPerDay;
}

// 20.3.1.3 Year Number
// DaysInYear is optimized out

int64_t DayFromYear(int64_t y) {
    return 365 * (y - 1970) + (y - 1969) / 4 - (y - 1901) / 100 + (y - 1601) / 400;
}

// TimeFromYear is optimized out

int64_t YearFromTime(int64_t t) {
    int64_t day = t / msPerDay;
    int64_t y = day / 365 + 1970;
    while (DayFromYear(y) > t) {
        y--;
    }
    return y;
}

bool IsLeapYear(int64_t y) {
    if (y % 400 == 0)return true;
    if (y % 100 == 0)return false;
    if (y % 4 == 0)return true;
    return false;
}

bool InLeapYear(int64_t t) {
    return IsLeapYear(YearFromTime(t));
}

// 20.3.1.4 Month Number
int64_t DayWithinYear(int64_t t) {
    return Day(t) - DayFromYear(YearFromTime(t));
}

int64_t MonthFromTime(int64_t t) {
    int64_t d = DayWithinYear(t);
    if (d < 31) return 0;
    d -= InLeapYear(t);
    if (d < 59) return 1;
    if (d < 90) return 2;
    if (d < 120) return 3;
    if (d < 151) return 4;
    if (d < 181) return 5;
    if (d < 212) return 6;
    if (d < 243) return 7;
    if (d < 273) return 8;
    if (d < 304) return 9;
    if (d < 334) return 10;
    return 11;
}

int64_t DayFromMonth(int64_t m, bool leap) {
    static const int table[] = { 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    if (m < 0 || m >= 12) {
        throw "internal error: invalid month";
    }
    if (m == 0)return 0;
    if (m == 1)return 32;
    return table[m - 2] + leap;
}

// 20.3.1.5 Date Number
int64_t DateFromTime(int64_t t) {
    return DayWithinYear(t) - DayFromMonth(MonthFromTime(t), InLeapYear(t)) + 1;
}

// 20.3.1.6 Week Day
int64_t WeekDay(int64_t t) {
    return (Day(t) + 4) % 7;
}

// 20.3.1.7 Local Time Zone Adjustment & 20.3.1.8 Daylight Saving Time Adjustment
int64_t GetTimezoneOffset(int64_t timestamp) {
    time_t time = timestamp / 1000;
    // gmtime convert the UTC timestamp to UTC time,
    // mktime regard it as localtime and convert back to timestamp.
    // The difference is the timezone offset
    return (mktime(gmtime(&time)) - time) * 1000;
}

// 20.3.1.9 LocalTime
int64_t LocalTime(int64_t timestamp) {
    return timestamp - GetTimezoneOffset(timestamp);
}

// 20.3.1.10 UTC
int64_t UTC(int64_t timestamp) {
    return timestamp + GetTimezoneOffset(timestamp);
}

// 20.3.1.11 Hours, Minutes, Second, and Milliseconds
const int64_t HoursPerDay = 24;
const int64_t MinutesPerHour = 60;
const int64_t SecondsPerMinute = 60;
const int64_t msPerSecond = 1000;
const int64_t msPerMinute = msPerSecond * SecondsPerMinute;
const int64_t msPerHour = msPerMinute * MinutesPerHour;

int64_t HourFromTime(int64_t t) {
    return (t / msPerHour) % HoursPerDay;
}

int64_t MinFromTime(int64_t t) {
    return (t / msPerMinute) % MinutesPerHour;
}

int64_t SecFromTime(int64_t t) {
    return (t / msPerSecond) % SecondsPerMinute;
}

int64_t msFromTime(int64_t t) {
    return t % msPerSecond;
}

// 20.3.1.12 MakeTime (hour, min, sec, ms)
int64_t MakeTime(int64_t h, int64_t m, int64_t s, int64_t milli) {
    return ((h * 60 + m) * 60 + s) * 1000 + milli;
}

// 20.3.1.13 MakeDay (year, month, date)
int64_t MakeDay(int64_t y, int64_t m, int64_t dt) {
    int64_t ym = y + (m / 12);
    int64_t mn = m % 12;
    return DayFromYear(ym) + DayFromMonth(mn, IsLeapYear(y)) + dt - 1;
}

// 20.3.1.14 MakeDate (day, time)
int64_t MakeDate(int64_t day, int64_t time) {
    return day * msPerDay + time;
}

// 20.3.1.15 TimeClip (time)
int64_t TimeClip(int64_t time) {
    return time;
}

int64_t TimeClip(double time) {
    if (!std::isfinite(time) || std::abs(time) > 8.64e15) {
        return invalidDateValue;
    }
    return static_cast<int64_t>(time);
}

static int64_t thisTimeValue(const Handle<JSValue>& val, const char* caller) {
    if (Testing::Is<JSObject>(val)) {
        if (Handle<DateObject> n = val.ExactCheckedCastTo<DateObject>()) {
            return n->dateValue();
        }
    }
    Exceptions::ThrowIncompatibleReceiverTypeError(caller);
}

static Handle<JSValue> setTimeValue(const Handle<JSValue>& val, int64_t t) {
    val.CastTo<DateObject>()->dateValue(t);
    return JSNumber::New(t);
}

int64_t currentTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

Handle<JSString> toDateString(int64_t date) {
    return JSString::New("This is some date string");
}





}

Handle<JSValue> Date::Call(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    return toDateString(currentTime());
}

Handle<JSObject> Date::Construct(const Handle<Array<JSValue>>& args, const Handle<JSObject>& target) {
    size_t len = args->Length();
    int64_t timeclip;
    if (len == 0) {
        timeclip = currentTime();
    } else if (len == 1) {
        Handle<JSValue> val = GetArg(args, 0);
        if (Testing::Is<JSObject>(val) && val.ExactInstanceOf<DateObject>()) {
            timeclip = val.CastTo<DateObject>()->dateValue();
        } else {
            val = Conversion::ToPrimitive(val);
            if (Testing::Is<JSString>(val)) {
                throw "TODO Date::Construct";
            } else {
                timeclip = TimeClip(Conversion::ToNumberValue(val));
            }
        }
    } else {
        double y = Conversion::ToNumberValue(GetArg(args, 0));
        double m = Conversion::ToNumberValue(GetArg(args, 1));
        double dt = len>2 ? Conversion::ToNumberValue(GetArg(args, 2)):1;
        double h = len>3 ? Conversion::ToNumberValue(GetArg(args, 3)) : 0;
        double min = len>4 ? Conversion::ToNumberValue(GetArg(args, 4)) : 0;
        double s = len>5 ? Conversion::ToNumberValue(GetArg(args, 5)) : 0;
        double milli = len>6 ? Conversion::ToNumberValue(GetArg(args, 6)) : 0;
        if (std::isfinite(y) && static_cast<int64_t>(y) >= 0 && static_cast<int64_t>(y)<=99) {
            y += 1900;
        }
        timeclip = MakeDate(MakeDay((int64_t)y, (int64_t)m, (int64_t)dt), MakeTime((int64_t)h, (int64_t)min, (int64_t)s, (int64_t)milli));
        timeclip = TimeClip(::UTC(timeclip));
    }
    Handle<DateObject> O = Objects::OrdinaryCreateFromConstructor<DateObject>(target, &Realm::DatePrototype);
    int64_t time = timeclip;
    O->dateValue(time);
    return O;
}

Handle<JSValue> Date::now(const Handle<JSValue>&, const Handle<Array<JSValue>>&) {
    int64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return JSNumber::New(time);
}

Handle<JSValue> Date::prototype::getDate(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getDate");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(DateFromTime(LocalTime(time)));
}

Handle<JSValue> Date::prototype::getDay(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getDay");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(WeekDay(LocalTime(time)));
}

Handle<JSValue> Date::prototype::getFullYear(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getFullYear");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(YearFromTime(LocalTime(time)));
}

Handle<JSValue> Date::prototype::getHours(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getHours");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(HourFromTime(LocalTime(time)));
}

Handle<JSValue> Date::prototype::getMilliseconds(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getMilliseconds");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(msFromTime(LocalTime(time)));
}

Handle<JSValue> Date::prototype::getMinutes(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getMinutes");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(MinFromTime(LocalTime(time)));
}

Handle<JSValue> Date::prototype::getMonth(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getMonth");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(MonthFromTime(LocalTime(time)));
}

Handle<JSValue> Date::prototype::getSeconds(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getMonth");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(SecFromTime(LocalTime(time)));
}

Handle<JSValue> Date::prototype::getTime(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getTime");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(time);
}

Handle<JSValue> Date::prototype::getTimezoneOffset(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getTimezoneOffset");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(GetTimezoneOffset(time) / (60*1000));
}

Handle<JSValue> Date::prototype::getUTCDate(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getUTCDate");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(DateFromTime(time));
}

Handle<JSValue> Date::prototype::getUTCDay(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getUTCDay");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(WeekDay(time));
}

Handle<JSValue> Date::prototype::getUTCFullYear(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getUTCFullYear");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(YearFromTime(time));
}

Handle<JSValue> Date::prototype::getUTCHours(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getUTCHours");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(HourFromTime(time));
}

Handle<JSValue> Date::prototype::getUTCMilliseconds(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getUTCMilliseconds");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(msFromTime(time));
}

Handle<JSValue> Date::prototype::getUTCMinutes(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getUTCMinutes");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(MinFromTime(time));
}

Handle<JSValue> Date::prototype::getUTCMonth(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getUTCMonth");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(MonthFromTime(time));
}

Handle<JSValue> Date::prototype::getUTCSeconds(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getUTCMonth");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(SecFromTime(time));
}

Handle<JSValue> Date::prototype::getYear(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    int64_t time = thisTimeValue(that, "Date.prototype.getYear");
    if (time == invalidDateValue)return JSNumber::NaN();
    return JSNumber::New(YearFromTime(LocalTime(time)) - 1900);
}

Handle<JSValue> Date::prototype::setDate(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {
    int64_t t = LocalTime(thisTimeValue(that, "Date.prototype.setDate"));
    int64_t dt = static_cast<int64_t>(Conversion::ToNumberValue(GetArg(args, 0)));
    int64_t newDate = MakeDate(MakeDay(YearFromTime(t), MonthFromTime(t), dt), TimeWithinDay(t));
    int64_t u = TimeClip(::UTC(newDate));
    return setTimeValue(that, u);
}

Handle<JSValue> Date::prototype::valueOf(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    return JSNumber::New(thisTimeValue(that, "Date.prototype.valueOf"));
}

Handle<JSValue> Date::prototype::Symbol_toPrimitive(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {
    if (!Testing::Is<JSObject>(that)) {
        Exceptions::ThrowIncompatibleReceiverTypeError("Date.prototype[Symbol.toPrimitive]");
    }
    Handle<JSString> hint = Testing::CastIf<JSString>(GetArg(args, 0));

    Handle<JSString> string = JSString::New("string");
    Handle<JSString> default_ = JSString::New("default");
    Handle<JSString> number = JSString::New("number");

    JSValue::Type tryFirst;
    if (hint == string || hint == default_) {
        tryFirst = JSValue::Type::kString;
    } else if(hint==number) {
        tryFirst = JSValue::Type::kNumber;
    } else {
        Exceptions::ThrowTypeError("hint of Date.prototype[Symbol.toPrimitive] should be either string, default or number");
    }
    return Conversion::OrdinaryToPrimitive(that.CastTo<JSObject>(), tryFirst);
}