#include "include/y/stacktrace.h"

#include <y/demangle.h>

#include <dlfcn.h>
#include <execinfo.h>
#include <ios>
#include <sstream>

namespace y {

StackTrace StackTrace::current() {
    StackTrace stackTrace;
    stackTrace.trace.resize(32);
    stackTrace.trace.resize(backtrace(const_cast<void**>(stackTrace.trace.data()), (int)stackTrace.trace.size()));
    return stackTrace;
}


UniqueCPtr<const char> StackTrace::Iterator::Symbol(const void* addr) {
    Dl_info info;
    if (dladdr(addr, &info)) {
        return info.dli_sname
               ? demangle(info.dli_sname)
               : UniqueCPtr<const char>(strdup(info.dli_fname));
    }
    else {
        return UniqueCPtr<const char>(strdup((std::ostringstream() << std::hex << addr).str().c_str()));
    }
}

}


std::ostream& operator<<(std::ostream& stream, const y::StackTrace& stackTrace)
{
    for (auto functionName : stackTrace) {
        stream << "  " << functionName << '\n';
    }
    return stream;
}