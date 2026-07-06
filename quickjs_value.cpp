#include "quickjs.hpp"
using namespace qjs;
using namespace qjs_private;

bool Value::isBoolean() const {
    return JS_IsBool(this->value_ptr->value);
}

bool Value::isNumber() const {
    return JS_IsNumber(this->value_ptr->value);
}

bool Value::isFunction() const {
    return JS_IsFunction(this->value_ptr->context_ptr->context, this->value_ptr->value);
}

bool Value::isSameContext(const Context & context) const{
    return this->value_ptr->context_ptr == context.context_ptr;
}

double Value::asNumber(bool safe) const{
    qjs::exceptions::CastException::throw_unless(safe || isNumber(), qjs::exceptions::CastException::Type::Number);
    double value;
    int ret = JS_ToFloat64(this->value_ptr->context_ptr->context, &value, this->value_ptr->value);
    if (ret < 0) {
        return 0;
    }
    return value;
}

std::string Value::asString(bool safe) const {
    qjs::exceptions::CastException::throw_unless(safe || isString(), qjs::exceptions::CastException::Type::String);
    const char * value = JS_ToCString(this->value_ptr->context_ptr->context, this->value_ptr->value);
    std::string ret = std::string(value);
    return ret;
}

bool Value::asBoolean(bool safe) const {
    qjs::exceptions::CastException::throw_unless(safe || isBoolean(), qjs::exceptions::CastException::Type::Boolean);
    return JS_ToBool(this->value_ptr->context_ptr->context, this->value_ptr->value);
}

bool Value::isString() const {
    return JS_IsString(this->value_ptr->value);
}

template <> long double Value::as<long double>(bool safe) const {
    return this->asNumber(safe);
}
    
template <> std::basic_string<char> Value::as<std::basic_string<char>>(bool safe) const {
    return this->asString(safe);
}

template <> bool Value::as<bool>(bool safe) const {
    return this->asBoolean(safe);
}

template <> bool Value::is<long double>() const {
    return this->isNumber();
}
    
template <> bool Value::is<std::basic_string<char>>() const {
    return this->isString();
}

template <> bool Value::is<bool>() const {
    return this->isBoolean();
}

Value::Value(const Value & value){
    this->value_ptr = value.value_ptr;
}

Value::Value(std::shared_ptr<ValueHolder> value){
    this->value_ptr = value;
}

Value::Value(std::shared_ptr<ContextHolder> context, JSValue value){
    this->value_ptr = std::make_shared<ValueHolder>(context, value);
}

Runtime Value::getRuntime() {
    return getContext().getRuntime();
};

Context Value::getContext() {
    Context ctx(this->value_ptr->context_ptr);
    return ctx;
};