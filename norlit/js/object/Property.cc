#include "Property.h"

using namespace norlit::gc;
using namespace norlit::js::object;

void DataProperty::IterateField(const FieldIterator& callback) {
    callback(&value);
}

void AccessorProperty::IterateField(const FieldIterator& callback) {
    callback(&get);
    callback(&set);
}