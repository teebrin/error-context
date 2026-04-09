#include "y/error.h"

#include "y/demangle.h"

#include <dlfcn.h>
#include <sstream>

namespace y::error {

thread_local Context *Context::current_ = nullptr;

void Context::add_detail(const std::type_info &typeinfo, ValueType detail) {
  auto it = details_by_type_.find(typeinfo);
  if (it != details_by_type_.end()) {
    return;
  }
  details_by_type_[typeinfo] = std::move(detail);
}

} // namespace y::error

using namespace y;
using namespace y::error;

std::ostream &operator<<(std::ostream &stream,
                         const std::exception &exception) {
  return stream << demangle(typeid(exception).name()).get() << ": "
                << exception.what();
}

std::ostream &operator<<(std::ostream &stream, const Context &errorContext) {
  char threadName[16] = "";
  pthread_getname_np(pthread_self(), threadName, sizeof(threadName));

  return stream << "Contextual details:" << '\n'
                << errorContext.get_details() << "Call stack on '" << threadName
                << "' thread:" << '\n'
                << errorContext.get_stack_trace();
}

std::ostream &operator<<(std::ostream &stream,
                         const y::error::Context::Details &details) {
  for (auto &[typeIndex, detail] : details.context.get_details_by_type()) {
    detail->stream(stream << "  " << demangle(typeIndex.name()).get() << ": ")
        << '\n';
  }
  return stream;
}

#if __clang__
using TypeInfo = std::type_info;
#else
using TypeInfo = void;
#endif

extern "C" void __cxa_throw(void *ex, TypeInfo *tinfo, void (*dest)(void *)) {
  if (Context *context = Context::current()) {
    context->capture_stack_trace();
    context->clear_details();
  }
  using RethrowType = void
      __attribute__((__noreturn__)) (*)(void *, TypeInfo *, void (*)(void *));
  static auto rethrow =
      reinterpret_cast<RethrowType>(dlsym(RTLD_NEXT, "__cxa_throw"));
  rethrow(ex, tinfo, dest);
}