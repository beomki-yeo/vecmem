/**
 * VecMem project, part of the ACTS project (R&D line)
 *
 * (c) 2021 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

#pragma once

#include <cstddef>
#include <stdexcept>
#include <type_traits>

#include "vecmem/utils/types.hpp"

namespace vecmem {
template <typename T, std::size_t N>
VECMEM_HOST_AND_DEVICE static_array<T, N>::static_array(void) {
    /*
     * This function does quite literally nothing, and leaves the array's
     * contents uninitialized.
     */
}

template <typename T, std::size_t N>
VECMEM_HOST constexpr typename static_array<T, N>::reference
static_array<T, N>::at(size_type i) {
    /*
     * The at function is bounds-checking in the standard library, so we
     * do a boundary check in our code, too. This makes this method
     * incompatible with device code.
     */
    if (i >= N) {
        throw std::out_of_range("Index greater than size of static array.");
    }

    return operator[](i);
}

template <typename T, std::size_t N>
VECMEM_HOST constexpr typename static_array<T, N>::const_reference
static_array<T, N>::at(size_type i) const {
    /*
     * Same thing as with the other at function, we do a bounds check in
     * accordance with the standard library.
     */
    if (i >= N) {
        throw std::out_of_range("Index greater than size of static array.");
    }

    return operator[](i);
}

template <typename T, std::size_t N>
VECMEM_HOST_AND_DEVICE constexpr typename static_array<T, N>::reference
static_array<T, N>::operator[](size_type i) {
    /*
     * Non-bounds checking access, which could cause a segmentation
     * violation.
     */
    return v[i];
}

template <typename T, std::size_t N>
VECMEM_HOST_AND_DEVICE constexpr typename static_array<T, N>::const_reference
static_array<T, N>::operator[](size_type i) const {
    /*
     * Return an element as constant.
     */
    return v[i];
}

template <typename T, std::size_t N>
VECMEM_HOST_AND_DEVICE constexpr typename static_array<T, N>::reference
static_array<T, N>::front(void) {
    /*
     * Return the first element.
     */
    return v[0];
}

template <typename T, std::size_t N>
VECMEM_HOST_AND_DEVICE constexpr typename static_array<T, N>::const_reference
static_array<T, N>::front(void) const {
    /*
     * Return the first element, but it's const.
     */
    return v[0];
}

template <typename T, std::size_t N>
VECMEM_HOST_AND_DEVICE constexpr typename static_array<T, N>::reference
static_array<T, N>::back(void) {
    /*
     * Return the last element.
     */
    return v[N - 1];
}

template <typename T, std::size_t N>
VECMEM_HOST_AND_DEVICE constexpr typename static_array<T, N>::const_reference
static_array<T, N>::back(void) const {
    /*
     * Return the last element, but it's const.
     */
    return v[N - 1];
}

template <typename T, std::size_t N>
VECMEM_HOST_AND_DEVICE constexpr typename static_array<T, N>::pointer
static_array<T, N>::data(void) {
    /*
     * Return a pointer to the underlying data.
     */
    return v;
}

template <typename T, std::size_t N>
VECMEM_HOST_AND_DEVICE constexpr typename static_array<T, N>::const_pointer
static_array<T, N>::data(void) const {
    /*
     * Return a pointer to the underlying data, but the elements are const.
     */
    return v;
}

template <typename T, std::size_t N>
template <typename std::size_t... Is, typename... Tp>
VECMEM_HOST_AND_DEVICE static_array<T, N>::static_array(
    std::index_sequence<Is...>, Tp&&... a) {
    /*
     * Construct the array from an enumerated list of elements of arbitrary
     * size. This is a fold expression which iterates over the elements of
     * Is (the indices) and Tp (the values), and inserts them into the inner
     * array using a comma expression.
     */
    (static_cast<void>(v[Is] = a), ...);
}

template <typename T, std::size_t N>
VECMEM_HOST_AND_DEVICE bool operator==(const static_array<T, N>& lhs,
                                       const static_array<T, N>& rhs) {
    /*
     * Iterate over all elements in the arrays, if any of them are unequal
     * between them, the arrays are not equal.
     */
    for (typename static_array<T, N>::size_type i = 0; i < N; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }

    /*
     * If we have iterated over the entire array without finding a counter-
     * example to the equality, the arrays must be equal.
     */
    return true;
}

template <typename T, std::size_t N>
VECMEM_HOST_AND_DEVICE bool operator!=(const static_array<T, N>& lhs,
                                       const static_array<T, N>& rhs) {
    /*
     * Same thing as before, we check all element pairs, if any of them are
     * unequal, the entire array is unequal. We could also implement this as
     * return !(lhs == rhs).
     */
    for (typename static_array<T, N>::size_type i = 0; i < N; ++i) {
        if (lhs[i] != rhs[i]) {
            return true;
        }
    }

    /*
     * No counter example, so the arrays are not unequal.
     */
    return false;
}
}  // namespace vecmem
