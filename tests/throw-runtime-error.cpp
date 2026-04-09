#include "throw-runtime-error.h"
#include <stdexcept>

void throwRuntimeError() {
    throw std::runtime_error("Runtime error description");
}
