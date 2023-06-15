#include "throwRuntimeError.h"
#include <stdexcept>

void throwRuntimeError()
{
    throw std::runtime_error("Runtime error description");
}