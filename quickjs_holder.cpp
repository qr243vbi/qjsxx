#include "quickjs.hpp"
using namespace qjs_private;

RuntimeHolder::RuntimeHolder(){
    this->runtime = JS_NewRuntime();
}

RuntimeHolder::~RuntimeHolder(){
    JS_FreeRuntime(this->runtime);
    this->runtime = nullptr;
}

ContextHolder::ContextHolder(std::shared_ptr<RuntimeHolder> holder){
    this->runtime_ptr = holder;
    this->context = JS_NewContext(holder->runtime);
}

ContextHolder::~ContextHolder(){
    JS_FreeContext(this->context);
    this->runtime_ptr = nullptr;
    this->context = nullptr;
}

ValueHolder::ValueHolder(std::shared_ptr<ContextHolder> context, JSValue value){
    this->context_ptr = context;
    this->value = value;
}

ValueHolder::~ValueHolder(){
    JS_FreeValue(this->context_ptr->context, value);
}
