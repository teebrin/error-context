#ifndef ERROR_CONTEXT_STACKTRACE_H
#define ERROR_CONTEXT_STACKTRACE_H

#include "unique-c-ptr.h"
#include <vector>

namespace y {

class StackTrace {
    using Trace = std::vector<const void*>;
    Trace trace;

public:
    static StackTrace current();

    class Iterator {
        Trace::const_iterator it;
        mutable UniqueCPtr<const char> symbol;
        static UniqueCPtr<const char> Symbol(const void* mangledSymbol);
    public:
        explicit Iterator(Trace::const_iterator it) : it(it) {}

        Iterator& operator++() {
            symbol.reset();
            ++it;
            return *this;
        }

        const char* operator*() const {
            if (!symbol) { symbol = Symbol(*it); }
            return symbol.get();
        }

        bool operator!=(const Iterator& other) const { return it != other.it; }
    };

    [[nodiscard]] Iterator begin() const { return Iterator(trace.begin() == trace.end() ? trace.begin() : trace.begin() + 2); }
    [[nodiscard]] Iterator end() const { return Iterator(trace.end()); }
};

}

std::ostream& operator<<(std::ostream& stream, const y::StackTrace& stackTrace);


#endif //ERROR_CONTEXT_STACKTRACE_H
