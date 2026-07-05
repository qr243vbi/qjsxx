#include <iostream>
#include "quickjs.hpp"
#include <exception>

#include <exception>
#include <string>
#include <utility>
#include <iostream>

#ifdef QJS_USE_STACKTRACE
#include <stacktrace>
#include <sstream>
#endif

using namespace qjs;
using namespace std;

class TracedException : public std::exception {
public:
    explicit TracedException(std::string message) 
        : message_(std::move(message))
#ifdef QJS_USE_STACKTRACE
        , trace_(std::stacktrace::current(1)) 
#endif
          {}
#ifdef QJS_USE_STACKTRACE
    // Expose the raw trace container if caller needs to iterate entries
    [[nodiscard]] const std::stacktrace& trace() const noexcept { 
        return trace_; 
    }
#endif

    // Override what() to combine the error string and formatted stacktrace
    [[nodiscard]] const char* what() const noexcept override {
#ifdef QJS_USE_STACKTRACE
        if (lazy_formatted_what_.empty()) {
            try {
                std::stringstream ss;
                ss << message_ << "\n\n--- Stacktrace ---\n" << std::to_string(trace_);
                lazy_formatted_what_ = ss.str();
            } catch (...) {
                // Return fallback string if memory allocation fails inside catch
#endif
                return message_.c_str(); 
#ifdef QJS_USE_STACKTRACE
            }
        }
        return lazy_formatted_what_.c_str();
#endif
    }

private:
    std::string message_;
#ifdef QJS_USE_STACKTRACE
    std::stacktrace trace_;
    mutable std::string lazy_formatted_what_; // Cached allocation block
#endif
};

void assert(bool assertFlag){
    if (!assertFlag){
        throw TracedException("assertion failed");
    }
}


void main_execute(){
    Runtime runtime;
    Context context(runtime);
    {
        auto value = context.newString("fine");
        assert(value.is<std::string>());
    } 
    cout << "Test 1 success" << endl;
    auto value2 = context.newValue(12);
    assert(value2.is<int>());
    cout << "Test 2 success" << endl;
    auto value3 = context.evalCode("\"234\"");
    assert(value3.is<std::string>());
    assert(value3.as<std::string>() == "234");
    cout << "Test 3 success" << endl;
    auto value4 = context.evalCode("234");
    assert(! value4.is<std::string>());
    assert(value4.as<std::string>(true) == "234");

    cout << "Test 4 success" << endl;
    bool unsafe_cast_raises_exception = false;
    try {
        auto _ = value4.as<std::string>(false);
    } catch (qjs::exceptions::CastException e){
        unsafe_cast_raises_exception = true;
    }
    assert(unsafe_cast_raises_exception);
    cout << "Test 5 success" << endl;
}

int main(){
    try {
        main_execute();
    } catch (const std::exception& E){
        std::cout << "Exception: " << E.what() << std::endl;
        return 1;
    }
}