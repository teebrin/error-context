#include "y/error.h"

namespace y::error {

std::ostream& print_exception_and_context(std::ostream& os, const std::exception_ptr& eptr, const Context& context) {
    return print_exception(os, eptr) << context << '\n';
}

std::ostream& print_exception(std::ostream& os, const std::exception_ptr& eptr) {
    try {
        std::rethrow_exception(eptr);
    }
    catch (const std::exception& e) {
        return os << e << '\n';
    }
    catch (...) {
        return os << "Unknown exception" << '\n';
    }
}

std::function<void(const Context&, std::exception_ptr)> make_print_and_rethrow_error_handler(std::ostream& os) {
    return [&os](const Context& context, const std::exception_ptr& eptr) {
        print_exception_and_context(os, eptr, context);
        std::rethrow_exception(eptr);
    };
}
} // namespace y::error
