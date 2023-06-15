#ifndef ERROR_CONTEXT_DETAILS_H
#define ERROR_CONTEXT_DETAILS_H

#include "throwRuntimeError.h"
#include <y/error.h>
#include <string>

struct SomeDetail { using ValueType = std::string; };

template<typename TDetail>
void throwWithDetail(typename TDetail::ValueType detail)
try
{
    throwRuntimeError();
}
catch(...)
{
    y::error::Context::addDetail<TDetail>(detail);
    throw;
}

#endif //ERROR_CONTEXT_DETAILS_H
