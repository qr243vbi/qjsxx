#include "quickjs.hpp"
#include <quickjs.h>
using namespace qjs;
using namespace qjs_private;


Context::Context(std::shared_ptr<ContextHolder> holder){
    this->context_ptr = holder;
}

Context::Context(const Runtime & run){
    this->context_ptr = std::make_shared<ContextHolder>(run.runtime_ptr);
}

Context::Context(){
    Runtime run;
    this->context_ptr = std::make_shared<ContextHolder>(run.runtime_ptr);
}

Context::Context(const Context &context){
    this->context_ptr = context.context_ptr;
}

Value Context::newNumber(long double number){
    JSValue value = JS_NewNumber(this->context_ptr->context, number);
    return Value(this->context_ptr, value);
}

Value Context::newBoolean(bool flag){
    JSValue value = JS_NewBool(this->context_ptr->context, flag);
    return Value(this->context_ptr, value);
}

Value Context::newString(const std::string_view& str){
    JSValue value = JS_NewStringLen(this->context_ptr->context, str.data(), str.length());
    return Value(this->context_ptr, value);
}

Value Context::evalCode(const std::string_view & str, const std::string_view & context){
    JSValue value = JS_Eval(this->context_ptr->context, str.data(), str.length(), context.data(), JS_EVAL_TYPE_GLOBAL);
    return Value(this->context_ptr, value);
}

Runtime Context::getRuntime() const {
    Runtime ctx(this->context_ptr->runtime_ptr);
    return ctx;
}

template <> Value Context::newValue<long double>(const long double & value){
    return newNumber(value);
};

template <> Value Context::newValue<std::basic_string<char>>(const std::basic_string<char> & value){
    return newString(value);
};

template <> Value Context::newValue<bool>(const bool & value){
    return newBoolean(value);
};