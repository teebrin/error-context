#ifndef Y_UNIQUE_C_PTR_H
#define Y_UNIQUE_C_PTR_H

#include <memory>

namespace y {
template<typename T>
class UniqueCPtr {
public:
    explicit UniqueCPtr(T* p = nullptr) : p_(p) {
    }

    UniqueCPtr(UniqueCPtr&& p) noexcept : p_(p.release()) {
    }

    ~UniqueCPtr() { std::free(const_cast<std::remove_cv_t<T>*>(p_)); }

    T* get() const { return p_; }
    T* release() { return std::exchange(p_, nullptr); }

    void reset(T* p = nullptr) {
        std::free(const_cast<std::remove_cv_t<T>*>(std::exchange(p_, p)));
    }

    UniqueCPtr& operator=(UniqueCPtr&& p) noexcept {
        if (&p != this) {
            reset(p.release());
        }
        return *this;
    }

    operator bool() const { return static_cast<bool>(get()); } // NOLINT(*-explicit-constructor)
    const T& operator*() const { return *get(); }
    T& operator*() { return *get(); }

private:
    T* p_;
};
} // namespace y

#endif // Y_UNIQUE_C_PTR_H
