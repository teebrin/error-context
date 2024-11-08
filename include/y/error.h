#ifndef Y_ERROR_H
#define Y_ERROR_H

#include "stacktrace.h"
#include <cxxabi.h>
#include <iostream>
#include <unordered_map>
#include <typeindex>


namespace y::error {

struct DetailBase {
    virtual ~DetailBase() = default;
    virtual std::ostream& stream(std::ostream& s) const = 0;
};

template<typename T>
struct Detail final : DetailBase {
    using ValueType = typename T::ValueType;
    const ValueType value;

    Detail(ValueType value) : value(std::move(value)) {}

    std::ostream& stream(std::ostream& s) const override { return s << value; }
};

class Context {
    using ValueType = std::unique_ptr<DetailBase>;

    StackTrace stackTrace;
    using DetailsByType = std::unordered_map<std::type_index, ValueType>;
    DetailsByType detailsByType;
    Context* previousContext;

public:
    static thread_local Context* current;
    struct Details { const Context& context; };

    Context() : previousContext(current) { current = this; }

    ~Context() {
        if (std::uncaught_exceptions() && previousContext) {
            previousContext->detailsByType.merge(detailsByType);
        }
        current = previousContext;
    }

    void captureBackTrace() { stackTrace = StackTrace::current(); }
    void clearDetails() { detailsByType.clear(); }

    [[nodiscard]] const DetailsByType& getDetailsByType() const { return detailsByType; }
    [[nodiscard]] const StackTrace& getStackTrace() const { return stackTrace; }

    Details getDetails() const { return Details{*this}; }

    template<typename T>
    bool hasDetail() {
        return detailsByType.find(typeid(T)) != detailsByType.end();
    }

    template<typename T>
    const typename T::ValueType& getDetail() {
        return static_cast<Detail<T>&>(*detailsByType[typeid(T)]).value;
    }

    template<typename T>
    inline static void addDetail(typename T::ValueType detail) {
        auto context = current;
        if (context) { context->addDetail(typeid(T), std::make_unique<Detail<T>>(std::move(detail))); }
    }

private:
    void addDetail(const std::type_info& typeinfo, ValueType detail);
};

template<typename F, typename E,
        std::enable_if_t<std::is_invocable<F>::value, bool> = true,
        std::enable_if_t<std::is_invocable<E, const Context&, std::exception_ptr>::value, bool> = true>
typename std::invoke_result<F>::type handleExceptionsWithContext(F f, E errorHandler) {
    Context errorContext;
    try {
        return f();
    }
    catch (...) {
        return errorHandler(errorContext, std::current_exception());
    }
}

std::function<int(const Context&, std::exception_ptr)> makeMainPrintErrorHandler(std::ostream& os = std::cerr, int exitCode = EXIT_FAILURE);
std::function<void(const Context&, std::exception_ptr)> makeThreadPrintErrorHandler(std::ostream& os = std::cerr);

}

std::ostream& operator<<(std::ostream& stream, const std::exception& exception);
std::ostream& operator<<(std::ostream& stream, const y::error::Context& errorContext);
std::ostream& operator<<(std::ostream& stream, const y::error::Context::Details& details);

#endif //Y_ERROR_H
