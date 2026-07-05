#pragma once
#include <memory>
#include <quickjs.h>
#include <type_traits>

#if __has_include(<QString>)
#include <QString>
#define QT_ENABLED_FOR_QJS 1
#else
#ifdef QT_ENABLED_FOR_QJS
#undef QT_ENABLED_FOR_QJS
#endif
#endif

namespace qjs {
constexpr bool SafeCast = false;

class Runtime;
class Context;
class Value;
}; // namespace qjs

namespace qjs_private {


class ContextHolder;

struct ArrayHolder {
  virtual qjs::Value get(size_t) = 0;
  virtual void set(size_t, qjs::Value) = 0;
  virtual size_t size() = 0;
};

class ValueHolder {
public:
  friend class qjs::Value;
  ValueHolder(std::shared_ptr<ContextHolder> context, JSValue value);
  ~ValueHolder();

private:
  std::shared_ptr<ContextHolder> context_ptr;
  JSValue value;
};

class OpaqueHolder {
  public:
    virtual void setOpaque(std::shared_ptr<void> pointer) = 0;
    std::shared_ptr<void> getOpaque();

  protected:
    std::shared_ptr<void> pointer;
};  

class RuntimeHolder: public OpaqueHolder {
  friend class qjs::Runtime;
  friend class ContextHolder;

public:
  RuntimeHolder();
  ~RuntimeHolder();
  virtual void setOpaque(std::shared_ptr<void> pointer) override;

private:
  JSRuntime *runtime;
};

class ContextHolder: public OpaqueHolder {
  friend class ValueHolder;
  friend class qjs::Value;
  friend class qjs::Context;
  friend class qjs::Runtime;

public:
  ContextHolder(std::shared_ptr<RuntimeHolder> runtime);
  ~ContextHolder();
  virtual void setOpaque(std::shared_ptr<void> pointer) override;

private:
  std::shared_ptr<RuntimeHolder> runtime_ptr;
  JSContext *context;
};

} // namespace qjs_private

namespace qjs {
namespace exceptions {

typedef std::string (*ExceptionMessage)(int message_);

class QJSException : public std::exception {
public:
  const char *what() const noexcept override;
  explicit QJSException(int message);
  virtual ExceptionMessage getExceptionFunction() const = 0;

protected:
  std::shared_ptr<void> pointer;
};
class CastException : public QJSException {
public:
  enum Type { Number = 1, Boolean = 2, String = 3 };

  virtual ExceptionMessage getExceptionFunction() const override;

  explicit CastException(Type message);

  static void throw_unless(bool condition, Type type);
};

class NotSameContextException : public QJSException {
public:
  virtual ExceptionMessage getExceptionFunction() const override;

  explicit NotSameContextException();

  static void throw_unless(bool condition);
};
} // namespace exceptions

template <typename T> using U = std::remove_cvref_t<T>;

template <typename T>
concept Number = (std::integral<std::remove_cvref_t<T>> ||
                  std::floating_point<std::remove_cvref_t<T>>) &&
                 !std::same_as<std::remove_cvref_t<T>, bool> &&
                 !std::same_as<std::remove_cvref_t<T>, long double>;

template <typename T>
concept NotNumber = !Number<T>;

class Runtime {
  friend class Context;
  friend class Value;

public:
  Runtime();
  Runtime(const Runtime &run);
  Context newContext() const;

private:
  Runtime( std::shared_ptr<qjs_private::RuntimeHolder> );
  std::shared_ptr<qjs_private::RuntimeHolder> runtime_ptr;
};

class Context {
  friend class Runtime;
  friend class Value;
  friend class Array;

public:
  static JSValue function_trampoline(JSContext *ctx,
                         JSValueConst this_val,
                         int argc,
                         JSValueConst *argv,
                         int magic,
                         JSValue *func_data);
                         
  Context();
  Context(const Runtime &run);
  Context(const Context &context);
  Value newNumber(long double number);
  Value newString(const std::string_view &str);
  Value newBoolean(bool value);
  Value evalCode(const std::string_view &str,
                 const std::string_view &context = "eval.js");

  Runtime getRuntime() const;

  template <NotNumber T> Value newValue(const T &value);

  template <Number T> inline Value newValue(const T &value);

private:
  Context(std::shared_ptr<qjs_private::ContextHolder>);
  std::shared_ptr<qjs_private::ContextHolder> context_ptr;
};

  template <> Value Context::newValue<long double>(const long double &);

  template <>
  Value Context::newValue<std::basic_string<char>>(const std::basic_string<char> &);

  template <> Value Context::newValue<bool>(const bool &);

class Value {
  friend class Runtime;
  friend class Context;

public:
  Value(const Value &value);
  bool isNumber() const ;
  bool isString() const ;
  bool isBoolean() const ;
  bool isFunction() const ;

  Runtime getRuntime();
  Context getContext();

  bool isSameContext(const Context & ctx) const;

  double asNumber(bool safe = SafeCast) const;
  std::string asString(bool safe = SafeCast) const;
  bool asBoolean(bool safe = SafeCast) const;

  template <NotNumber T> T as(bool safe = SafeCast) const;

  template <NotNumber T> bool is() const;

  template <Number T> inline T as(bool safe = SafeCast) const{
    return static_cast<T>(as<long double>(safe));
  };

  template <Number T> inline bool is() const { return is<long double>(); };

private:
  Value(std::shared_ptr<qjs_private::ValueHolder>);
  Value(std::shared_ptr<qjs_private::ContextHolder>, JSValue value);
  std::shared_ptr<qjs_private::ValueHolder> value_ptr;
};

template <Number T> Value Context::newValue(const T &value) {
  return newValue((long double)value);
};

  template <> long double Value::as<long double>(bool safe) const;

  template <> std::basic_string<char> Value::as<std::basic_string<char>>(bool safe) const;

  template <> bool Value::as<bool>(bool safe) const;

  template <> bool Value::is<long double>() const;

  template <> bool Value::is<std::basic_string<char>>() const;

  template <> bool Value::is<bool>() const;

#ifdef QT_ENABLED_FOR_QJS
  template <> inline QString Value::as<QString>(bool safe) {
    return QString::fromStdString(as<std::string>(safe));
  };

  template <> inline bool Value::is<QString>() { return is<std::string>(safe); };
#endif

class Array {
public:
  Value get(size_t index);
  void set(size_t index, const Value &value);
  size_t size();
  Context getContext();
  Runtime getRuntime();
private:
  std::shared_ptr<qjs_private::ContextHolder> context_ptr;
  std::shared_ptr<qjs_private::ArrayHolder> array_ptr; 
};

} // namespace qjs

void init();