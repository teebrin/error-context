#include "y/error.h"

#include "y/demangle.h"

#include <dlfcn.h>
#include <sstream>

namespace y::error {

thread_local Context* Context::current = nullptr;

void Context::addDetail(const std::type_info& typeinfo, ValueType detail) {
    auto it = detailsByType.find(typeinfo);
    if (it != detailsByType.end()) { return; }
    detailsByType[typeinfo] = std::move(detail);
}

}

using namespace y;
using namespace y::error;

std::ostream& operator<<(std::ostream& stream, const std::exception& exception) {
    return stream << demangle(typeid(exception).name()).get() << ": " << exception.what();
}

std::ostream& operator<<(std::ostream& stream, const Context& errorContext) {
    char threadName[16] = "";
    pthread_getname_np(pthread_self(), threadName, sizeof(threadName));

    return stream << "Contextual details:" << '\n'
                  << errorContext.getDetails()
                  << "Call stack on '" << threadName << "' thread:" << '\n'
                  << errorContext.getStackTrace();
}

std::ostream& operator<<(std::ostream& stream, const y::error::Context::Details& details)
{
    for (auto& [typeIndex, detail] : details.context.getDetailsByType()) {
        detail->stream(stream << "  " << demangle(typeIndex.name()).get() << ": ") << '\n';
    }
    return stream;
}

#if __clang__
using TypeInfo = std::type_info;
#else
using TypeInfo = void;
#endif

extern "C" void __cxa_throw(void* ex, TypeInfo* tinfo, void(*dest)(void*)) {
    if (Context *errorContext = Context::current) {
        errorContext->captureBackTrace();
        errorContext->clearDetails();
    }
    using RethrowType = void (* __attribute__((__noreturn__)))(void*, TypeInfo*, void(*)(void*));
    static auto rethrow =
        reinterpret_cast<RethrowType>(
            dlsym(RTLD_NEXT, "__cxa_throw"));
    rethrow(ex, tinfo, dest);
}