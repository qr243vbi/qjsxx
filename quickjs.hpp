#pragma once
#include <memory>
#include <quickjs.h>
#include <type_traits>

namespace qjs {
constexpr bool SafeCast = false;

class Runtime;
class Context;
class Value;
}; // namespace qjs

namespace qjs_private {
class ContextHolder;

class ValueHolder {
public:
  friend class qjs::Value;
  ValueHolder(std::shared_ptr<ContextHolder> context, JSValue value);
  ~ValueHolder();

private:
  std::shared_ptr<ContextHolder> context_ptr;
  JSValue value;
};

class RuntimeHolder {
  friend class qjs::Runtime;
  friend class ContextHolder;

public:
  RuntimeHolder();
  ~RuntimeHolder();

private:
  JSRuntime *runtime;
};

class ContextHolder {
  friend class ValueHolder;
  friend class qjs::Value;
  friend class qjs::Context;
  friend class qjs::Runtime;

public:
  ContextHolder(std::shared_ptr<RuntimeHolder> runtime);
  ~ContextHolder();

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
  std::shared_ptr<qjs_private::RuntimeHolder> runtime_ptr;
};

class Context {
  friend class Runtime;
  friend class Value;

public:
  Context(const Runtime &run);
  Context(const Context &context);
  Value newNumber(long double number);
  Value newString(const std::string_view &str);
  Value newBoolean(bool value);
  Value evalCode(const std::string_view &str,
                 const std::string_view &context = "eval.js");

private:
  Context(std::shared_ptr<qjs_private::ContextHolder>);
  std::shared_ptr<qjs_private::ContextHolder> context_ptr;
};

class Value {
  friend class Runtime;
  friend class Context;

public:
  Value(const Value &value);
  bool isNumber();
  bool isString();
  bool isBoolean();

  double asNumber(bool safe = SafeCast);
  std::string asString(bool safe = SafeCast);
  bool asBoolean(bool safe = SafeCast);

  template <NotNumber T> T as(bool safe = SafeCast);

  template <NotNumber T> bool is();

  template <> long double as<long double>(bool safe);

  template <> std::basic_string<char> as<std::basic_string<char>>(bool safe);

  template <> bool as<bool>(bool safe);

  template <> bool is<long double>();

  template <> bool is<std::basic_string<char>>();

  template <> bool is<bool>();

  template <Number T> inline T as(bool safe = SafeCast) {
    return static_cast<T>(as<long double>(safe));
  };

  template <Number T> inline bool is() { return isNumber(); };

private:
  Value(std::shared_ptr<qjs_private::ValueHolder>);
  Value(std::shared_ptr<qjs_private::ContextHolder>, JSValue value);
  std::shared_ptr<qjs_private::ValueHolder> value_ptr;
};

} // namespace qjs

void init();