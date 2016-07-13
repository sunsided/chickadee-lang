//
// Created by Markus on 13.07.2016.
//

#ifndef CHICKADEE_HELPER_H
#define CHICKADEE_HELPER_H

#include <memory>

namespace helper {
    // Cloning make_unique here until it's standard in C++14.
    // Using a namespace to avoid conflicting with MSVC's std::make_unique (which
    // ADL can sometimes find in unqualified calls).
    template <class T, class... Args>
    static
    typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
            make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
} // end namespace helper


#endif //CHICKADEE_HELPER_H
