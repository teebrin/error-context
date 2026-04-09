#ifndef Y_ERROR_H
#define Y_ERROR_H

#include "stacktrace.h"
#include <cxxabi.h>
#include <functional>
#include <iostream>
#include <typeindex>
#include <unordered_map>

namespace y::error {
struct DetailBase {
    virtual ~DetailBase() = default;
    virtual std::ostream& stream(std::ostream& s) const = 0;
};

template<typename T>
struct Detail final : DetailBase {
    using ValueType = T::ValueType;
    const ValueType value;

    explicit Detail(ValueType value) : value(std::move(value)) {
    }

    std::ostream& stream(std::ostream& s) const override { return s << value; }
};

class Context {
    using ValueType = std::unique_ptr<DetailBase>;
    using DetailsByType = std::unordered_map<std::type_index, ValueType>;

public:
    struct Details {
        const Context& context;
    };

    Context() : previous_context_(current_) { current_ = this; }

    ~Context() {
        if (std::uncaught_exceptions() && previous_context_) {
            previous_context_->details_by_type_.merge(details_by_type_);
        }
        current_ = previous_context_;
    }

    void capture_stack_trace() { stack_trace_ = StackTrace::current(); }
    void clear_details() { details_by_type_.clear(); }

    [[nodiscard]] const DetailsByType& get_details_by_type() const {
        return details_by_type_;
    }

    [[nodiscard]] const StackTrace& get_stack_trace() const {
        return stack_trace_;
    }

    Details get_details() const { return Details{*this}; }

    template<typename T>
    bool has_detail() {
        return details_by_type_.find(typeid(T)) != details_by_type_.end();
    }

    template<typename T>
    const T::ValueType& get_detail() {
        return static_cast<Detail<T>&>(*details_by_type_[typeid(T)]).value;
    }

    template<typename T>
    static void add_detail(T::ValueType detail) {
        if (auto context = current_) {
            context->add_detail(
                typeid(T),
                std::make_unique<Detail<T>>(std::move(detail)));
        }
    }

    static Context* current() { return current_; }

private:
    void add_detail(const std::type_info& typeinfo, ValueType detail);

    StackTrace stack_trace_;
    DetailsByType details_by_type_;
    Context* previous_context_;

    static thread_local Context* current_;
};

template<std::invocable<> F, std::invocable<Context&, std::exception_ptr> E>
std::invoke_result_t<F> run_with_context(F f, E error_handler) {
    // ReSharper disable once CppTooWideScope
    Context context;
    try {
        return f();
    }
#if !__clang__
  catch (const abi::__forced_unwind&){
        throw;


    }
#endif
    catch (...) {
        return error_handler(context, std::current_exception());
    }
}


std::ostream& print_exception_and_context(std::ostream& os, const std::exception_ptr& eptr, const Context& context);
std::ostream& print_exception(std::ostream& os, const std::exception_ptr& eptr);
std::function<void(const Context&, std::exception_ptr)>
make_print_and_rethrow_error_handler(std::ostream& os = std::cerr);

template<typename T>
auto make_print_and_return_error_handler(
    std::ostream& os = std::cerr,
    T failure_return_value = EXIT_FAILURE) {
    return [&os, failure_return_value](const Context& context, const std::exception_ptr& eptr) {
        print_exception_and_context(os, eptr, context);
        return failure_return_value;
    };
}
} // namespace y::error

std::ostream& operator<<(std::ostream& stream, const std::exception& exception);
std::ostream& operator<<(
    std::ostream& stream,
    const y::error::Context& errorContext);
std::ostream& operator<<(
    std::ostream& stream,
    const y::error::Context::Details& details);

#endif // Y_ERROR_H
