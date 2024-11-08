#include "y/error.h"

namespace y::error {

std::function<int(const Context&, std::exception_ptr)> makeMainPrintErrorHandler(std::ostream& os, int exitCode)
{
    return [&os, exitCode](const Context& context, const std::exception_ptr& eptr){
        try {
            std::rethrow_exception(eptr);
        }
        catch (const std::exception& e) {
            os << e << '\n';
        }
        catch (...){
            os << "Unknown exception" << '\n';
        }

        os << context << '\n';
        return exitCode;
    };
}

std::function<void(const Context&, std::exception_ptr)> makeThreadPrintErrorHandler(std::ostream& os)
{
    return [&os](const Context& context, const std::exception_ptr& eptr){
        try {
            std::rethrow_exception(eptr);
        }
        catch (const std::exception& e) {
            os << e << '\n';
        }
        catch (...) {
            os << "Unknown exception" << '\n';
        }

        os << context << '\n';
        std::rethrow_exception(eptr);
    };
}

}