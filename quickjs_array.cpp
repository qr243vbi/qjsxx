#include "quickjs.hpp"
using namespace qjs;

 Runtime Array::getRuntime() {
    return getContext().getRuntime();
}

 Context Array::getContext() {
    Context ctx(this->context_ptr);
    return ctx;
}

Value Array::get(size_t index) const{
    auto ret = this->array_ptr->get(index);
    exceptions::ContextException::throw_unless(ret.isSameContext(((Array*)this)->getContext()), exceptions::ContextException::Type::NotSame);
    return ret;
}

void Array::set(size_t index, const Value & value){
    exceptions::ContextException::throw_unless(value.isSameContext(((Array*)this)->getContext()), exceptions::ContextException::Type::NotSame);
    this->array_ptr->set(index, value);
}

size_t Array::size() const{
    return this->array_ptr->size();
}

Array Array::newArray(Context context, int argc, JSValueConst *argv){
    Array array;
    array.context_ptr = context.context_ptr;
    auto holder = std::make_shared<qjs_private::PointerArrayHolder>();
    holder->context_ptr = context.context_ptr;
    holder->argc = argc;
    holder->argv = argv;
    array.array_ptr = holder;
    return array;
};
