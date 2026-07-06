#include "quickjs.hpp"
using namespace qjs::exceptions;
#include <memory>

#ifdef QJS_USE_STACKTRACE
#include <sstream>
#include <stacktrace>
#endif

struct ExceptionPointer {
  int message_;
  std::string lazy_formatted_what_;
#ifdef QJS_USE_STACKTRACE
  std::stacktrace trace_;
#endif

  ExceptionPointer(int message, int skip) {
    message_ = message;
#ifdef QJS_USE_STACKTRACE
    this->trace_ = std::stacktrace::current(skip + 1);
#endif
  }

  const char *what(qjs::exceptions::ExceptionMessage cast_exception_message) {
    auto message_ = this->message_;
    if (this->lazy_formatted_what_.empty()) {
    return_lazy_formatted_what:
      return this->lazy_formatted_what_.c_str();
    }
#ifdef QJS_USE_STACKTRACE
    try {
      std::stringstream ss;
      ss << cast_exception_message(message_) << "\n\n--- Stacktrace ---\n"
         << std::to_string(this->trace_);
      this->lazy_formatted_what_ = ss.str();
    } catch (...) {
    }
#else
    this->lazy_formatted_what_ =
        cast_exception_message(message_);
#endif
    goto return_lazy_formatted_what;
  }
};

static std::string cast_exception_message(int message_) {
  typedef CastException::Type type;
  switch (message_) {
  case type::Number:
    return "Value is not number";
  case type::Boolean:
    return "Value is not boolean";
  case type::String:
    return "Value is not string";
  default:
    return "Wrong type";
  }
}

static std::string not_same_context_exception(int message_) {
  typedef ContextException::Type type;
  switch (message_){
  case type::NotSame:
    return "Not the same context";
  default:
    return "Context destroyed";
  }
}


static std::string out_of_bound_error(int) {
  return "Index out of bound";
}


static void set_shared_ptr(std::shared_ptr<void> &pointer, int message,
                           int skip) {
  auto point = new ExceptionPointer(message, skip + 1);
  pointer = std::shared_ptr<void>(point);
}

static const char *
get_what(const std::shared_ptr<void> &pointer,
         qjs::exceptions::ExceptionMessage cast_exception_message) {
  auto point = ((ExceptionPointer *)(pointer.get()));
  return point->what(cast_exception_message);
}

const char *QJSException::what() const noexcept {
  return get_what(this->pointer, this->getExceptionFunction());
}

void CastException::throw_unless(bool condition, CastException::Type type) {
  if (!condition) {
    throw CastException(type);
  }
};

void ContextException::throw_unless(bool condition, ContextException::Type type) {
  if (!condition) {
    throw ContextException(type);
  }
};

QJSException::QJSException(int message) {
  set_shared_ptr(this->pointer, message, 1);
}

qjs::exceptions::ExceptionMessage CastException::getExceptionFunction() const {
  return &cast_exception_message;
}

CastException::CastException(CastException::Type message) : QJSException((int)message) {}

ContextException::ContextException(ContextException::Type message) : QJSException((int)message) {}

qjs::exceptions::ExceptionMessage ContextException::getExceptionFunction() const {
  return &not_same_context_exception;
}

IndexOutOfBoundException::IndexOutOfBoundException() : QJSException(0) {}

qjs::exceptions::ExceptionMessage IndexOutOfBoundException::getExceptionFunction() const {
  return &not_same_context_exception;
}