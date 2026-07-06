#include "quickjs.hpp"
#include <quickjs.h>

using namespace qjs;
using namespace qjs_private;


Runtime::Runtime(){
    this->runtime_ptr = std::make_shared<RuntimeHolder>();
}

Runtime::Runtime(const Runtime & runtime){
    this->runtime_ptr = runtime.runtime_ptr;
}

Runtime::Runtime(std::shared_ptr<qjs_private::RuntimeHolder> runtime){
    this->runtime_ptr = runtime;
}

Context Runtime::newContext() const {
    std::shared_ptr<ContextHolder> holder = std::make_shared<ContextHolder>(this->runtime_ptr);
    Context ctx(holder);
    return ctx;
}

void Runtime::finalizeFunction(JSRuntime *rt, JSValue val){
    auto pointer = static_cast<RuntimeOpaque*>(JS_GetRuntimeOpaque(rt));
    if (pointer != nullptr){
        auto id = pointer->function_class_id;
        auto function = static_cast<Function*>(JS_GetOpaque(val, id));
        delete function;
    }
}

RuntimeOpaque::RuntimeOpaque(){
}

JSClassDef & Runtime::getFunctionClassDef(){
  static JSClassDef my_class = {
    "QJSFunction",
    Runtime::finalizeFunction,
  };
  return my_class;
}

std::shared_ptr<RuntimeOpaque> Runtime::getOpaque(){
    auto pointer = std::static_pointer_cast<RuntimeOpaque>(this->runtime_ptr->getOpaque());
    if (pointer == nullptr){
        pointer = std::make_shared<RuntimeOpaque>();
        pointer->runtime_ptr = this->runtime_ptr;
        runtime_ptr->setOpaque(pointer);
        JS_NewClassID(runtime_ptr->runtime, &pointer->function_class_id);
        JS_NewClass(runtime_ptr->runtime, pointer->function_class_id, &Runtime::getFunctionClassDef());
    }
    return pointer;
}
