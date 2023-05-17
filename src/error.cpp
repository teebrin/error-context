#include "include/y/error.h"

#include "include/y/demangle.h"

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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-noreturn"
extern "C" void __cxa_throw(void* ex, std::type_info* tinfo, void (*dest)(void*)) {
    Context *errorContext = Context::current;
    if (errorContext) {
        errorContext->captureBackTrace();
        errorContext->clearDetails();
    }
    using RethrowType = void (*)(void*, std::type_info*, void(*)(void*));
    static auto rethrow = (RethrowType)dlsym(RTLD_NEXT, "__cxa_throw");
    rethrow(ex, tinfo, dest);
}

#pragma clang diagnostic pop