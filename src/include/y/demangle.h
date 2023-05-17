#ifndef Y_DEMANGLE_H
#define Y_DEMANGLE_H

#include "unique-c-ptr.h"

namespace y {

UniqueCPtr<const char> demangle(const char* mangledName);

}

#endif //Y_DEMANGLE_H
