#include "quickjs.hpp"

using namespace qjs;
using namespace qjs_private;


Runtime::Runtime(){
    this->runtime_ptr = std::make_shared<RuntimeHolder>();
}

Runtime::Runtime(const Runtime & runtime){
    this->runtime_ptr = runtime.runtime_ptr;
}


Context Runtime::newContext() const {
    std::shared_ptr<ContextHolder> holder = std::make_shared<ContextHolder>(this->runtime_ptr);
    Context ctx(holder);
    return ctx;
}