#include "../include/y/demangle.h"

#include <cxxabi.h>
#include <string.h>

namespace y {

UniqueCPtr<const char> demangle(const char* mangledName) {
    int status;
    const char* demangledName = abi::__cxa_demangle(mangledName, 0, 0, &status);
    return UniqueCPtr<const char>(demangledName ? demangledName : strdup(mangledName));
}

}