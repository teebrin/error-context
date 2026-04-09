#include "y/stacktrace.h"

#include <y/demangle.h>

#include <string.h>

#include <dlfcn.h>
#include <execinfo.h>
#include <ios>
#include <sstream>

namespace y {
StackTrace StackTrace::current() {
    StackTrace stackTrace;
    stackTrace.trace_.resize(32);
    stackTrace.trace_.resize(
        backtrace(const_cast<void**>(stackTrace.trace_.data()), static_cast<int>(stackTrace.trace_.size())));
    return stackTrace;
}


UniqueCPtr<const char> StackTrace::Iterator::symbol(const void* addr) {
    Dl_info info;
    if (dladdr(addr, &info)) {
        return info.dli_sname
                   ? demangle(info.dli_sname)
                   : UniqueCPtr<const char>(strdup(info.dli_fname));
    }
    return UniqueCPtr<const char>(strdup((std::ostringstream() << std::hex << addr).str().c_str()));
}
}


std::ostream& operator<<(std::ostream& stream, const y::StackTrace& stack_trace) {
    for (const auto functionName : stack_trace) {
        stream << "  " << functionName << '\n';
    }
    return stream;
}
