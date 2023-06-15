#ifndef Y_UNIQUE_C_PTR_H
#define Y_UNIQUE_C_PTR_H

#include <type_traits>
#include <cstdlib>
#include <memory>

namespace y {

template<typename T>
class UniqueCPtr {
    T* p;
public:
    UniqueCPtr(T* p = nullptr): p(p) {}
    UniqueCPtr(UniqueCPtr<T>&& p): p(p.p) {}
    ~UniqueCPtr() { std::free((void*)p); }

    T* get() const { return p; }
    T* release() { auto p = this->p; this->p = nullptr; return p; }
    void reset(T* p = nullptr) { std::free((void*)this->p); this->p = p; }

    UniqueCPtr<T>& operator=(UniqueCPtr<T>&& p) { if (&p != this) { reset(p.release()); } return *this; }
    operator bool() { return (bool)get(); }
    const T& operator*() const { return *get(); }
    T& operator*() { return *get(); }
};

}


#endif //Y_UNIQUE_C_PTR_H
