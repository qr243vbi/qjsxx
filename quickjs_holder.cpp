#include "quickjs.hpp"
#include <quickjs.h>
using namespace qjs_private;

RuntimeHolder::RuntimeHolder(){
    JSRuntime * runtime = this->runtime = JS_NewRuntime();
}

void RuntimeHolder::setOpaque(std::shared_ptr<void> pointer){
    this->pointer = pointer;
    JS_SetRuntimeOpaque(this->runtime, pointer.get());
};

RuntimeHolder::~RuntimeHolder(){
    setOpaque(nullptr);
    JS_FreeRuntime( runtime);
}

ContextHolder::ContextHolder(std::shared_ptr<RuntimeHolder> holder){
    this->runtime_ptr = holder;
    JSContext* context = this->context = JS_NewContext(holder->runtime);
}

void ContextHolder::setOpaque(std::shared_ptr<void> pointer){
    this->pointer = pointer;
    JS_SetContextOpaque(this->context, pointer.get());
};

ContextHolder::~ContextHolder(){
    setOpaque(nullptr);
    JS_FreeContext(context);
}

ValueHolder::ValueHolder(std::shared_ptr<ContextHolder> context, JSValue value){
    this->context_ptr = context;
    this->value = value;
}

ValueHolder::~ValueHolder(){
    JS_FreeValue(this->context_ptr->context, value);
}

std::shared_ptr<void> OpaqueHolder::getOpaque(){
    return this->pointer;
};

qjs::Value PointerArrayHolder::get(size_t index) const {
    if (index >= this->argc){
        throw qjs::exceptions::IndexOutOfBoundException();    
    }
    return qjs::Value(this->context_ptr, this->argv[index]);
};

void PointerArrayHolder::set(size_t index, qjs::Value value){
    if (index >= this->argc){
        throw qjs::exceptions::IndexOutOfBoundException();    
    }
    this->argv[index] = value.value_ptr->value;
};

size_t PointerArrayHolder::size() const {
    return this->argc;
};