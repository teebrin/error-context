#ifndef ERROR_CONTEXT_STACKTRACE_H
#define ERROR_CONTEXT_STACKTRACE_H

#include "unique-c-ptr.h"
#include <vector>

namespace y {
class StackTrace {
    using Trace = std::vector<const void*>;

public:
    static StackTrace current();

    class Iterator {
    public:
        explicit Iterator(const Trace::const_iterator it) : it_(it) {
        }

        Iterator& operator++() {
            symbol_.reset();
            ++it_;
            return *this;
        }

        const char* operator*() const {
            if (!symbol_) { symbol_ = symbol(*it_); }
            return symbol_.get();
        }

        bool operator!=(const Iterator& other) const { return it_ != other.it_; }

    private:
        static UniqueCPtr<const char> symbol(const void* addr);

        Trace::const_iterator it_;
        mutable UniqueCPtr<const char> symbol_;
    };

    [[nodiscard]] Iterator begin() const {
        return Iterator(trace_.begin() == trace_.end() ? trace_.begin() : trace_.begin() + 2);
    }

    [[nodiscard]] Iterator end() const { return Iterator(trace_.end()); }

private:
    Trace trace_;
};
}

std::ostream& operator<<(std::ostream& stream, const y::StackTrace& stack_trace);


#endif //ERROR_CONTEXT_STACKTRACE_H
