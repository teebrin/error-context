#ifndef ERROR_CONTEXT_DETAILS_H
#define ERROR_CONTEXT_DETAILS_H

#include "throw-runtime-error.h"
#include <y/error.h>
#include <string>

struct SomeDetail {
    using ValueType = std::string;
};

template<typename TDetail>
void throw_with_detail(typename TDetail::ValueType detail)
try {
    throwRuntimeError();
}
catch (...) {
    y::error::Context::add_detail<TDetail>(detail);
    throw;
}

#endif //ERROR_CONTEXT_DETAILS_H
