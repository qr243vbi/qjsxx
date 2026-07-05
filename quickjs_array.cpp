#include "quickjs.hpp"
using namespace qjs;

Runtime Array::getRuntime(){
    return getContext().getRuntime();
}

Context Array::getContext(){
    Context ctx(this->context_ptr);
    return ctx;
}

Value Array::get(size_t index){
    auto ret = this->array_ptr->get(index);
    qjs::exceptions::NotSameContextException::throw_unless(ret.isSameContext(getContext()));
    return ret;
}

void Array::set(size_t index, const Value & value){
    qjs::exceptions::NotSameContextException::throw_unless(value.isSameContext(getContext()));
    this->array_ptr->set(index, value);
}

size_t Array::size(){
    return this->array_ptr->size();
}