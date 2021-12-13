/*
 * VecMem project, part of the ACTS project (R&D line)
 *
 * (c) 2021 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

#pragma once

#include <memory>
#include <type_traits>

#include "vecmem/memory/details/unique_alloc_deleter.hpp"
#include "vecmem/memory/details/unique_obj_deleter.hpp"
#include "vecmem/memory/memory_resource.hpp"
#include "vecmem/vecmem_core_export.hpp"

namespace vecmem {
/**
 * @brief A unique pointer type for non-trivial objects.
 *
 * This type alias serves as a unique pointer to some non-trivial object(s).
 * When the pointer goes out of scope, the pointee is automatically deallocated
 * and deleted.
 *
 * @tparam T The type of the object stored.
 *
 * @note This type supports non-array types and array types with unknown bound,
 * but array types with known bound are not supported.
 *
 * @warning This type should never be used with host-inaccessible memory! We
 * cannot check this at compile-time or at run-time, so it is the
 * responsibility of the user to ensure that this requirement is respected.
 */
template <typename T>
using unique_obj_ptr = std::unique_ptr<T, details::unique_obj_deleter<T>>;

/**
 * @brief A unique pointer type for trivial types.
 *
 * This type alias serves as a unique pointer to some trivial object(s). When
 * the pointer goes out of scope, the pointee is automatically deallocated, but
 * NOT deleted.
 *
 * @tparam T The type of the object stored.
 *
 * @note This type supports non-array types, as well as array types with
 * unknown bound, but not with known bound.
 *
 * @note This type can only be used with types that are both trivially
 * destructible and constructible.
 */
template <typename T>
using unique_alloc_ptr = std::unique_ptr<T, details::unique_alloc_deleter<T>>;

/**
 * @brief Create a unique object pointer to a newly constructed object.
 *
 * This is the vecmem analogue to std::make_unique<T>(Args...). That is to say,
 * it allocates memory for, and then constructs, a single object. The
 * construction uses the arguments passed to this function.
 *
 * @tparam T The type to allocate.
 * @tparam Args The argument types to pass to the constructor.
 *
 * @param m The memory resource to use.
 * @param a The arguments to pass to the constructor.
 *
 * @return A unique object pointer to the newly constructed object.
 *
 * @warning The memory resource used by this function must be host-accessible.
 */
template <typename T, typename... Args>
typename std::enable_if_t<!std::is_array_v<T>, unique_obj_ptr<T>>
make_unique_obj(memory_resource& m, Args&&... a) {
    /*
     * Calculate the size of the allocation and use the memory resource to
     * perform an allocation of the requested size.
     */
    std::size_t s = sizeof(T);
    T* p = static_cast<T*>(m.allocate(s));

    /*
     * Perform a placement-new with forwarded arguments to construct the
     * object.
     */
    new (p) T(std::forward<Args>(a)...);

    /*
     * Create a new unique_ptr, with its own deleter, and return it.
     */
    return unique_obj_ptr<T>(p, details::unique_obj_deleter<T>(m, s));
}

/**
 * @brief Create a unique object pointer to an array of default-constructed
 * objects.
 *
 * This is the vecmem analogue of std::make_unique<T>(std::size_t). That is to
 * say, it allocates an array of compile-time-unknown bounds, where the size is
 * given by the argument. The objects are all default-constructed.
 *
 * @tparam T The type to allocate.
 *
 * @param m The memory resource to use.
 * @param n The number of elements to allocate.
 *
 * @return A unique object pointer to the newly constructed array of objects.
 *
 * @warning The memory resource used by this function must be host-accessible.
 */
template <typename T>
typename std::enable_if_t<std::is_array_v<T> && std::extent_v<T> == 0,
                          unique_obj_ptr<T>>
make_unique_obj(memory_resource& m, std::size_t n) {
    using pointer_t = typename unique_obj_ptr<T>::deleter_type::pointer_t;

    /*
     * Calculate the size of the allocation and use the memory resource to
     * perform an allocation of the requested size.
     */
    std::size_t s = n * sizeof(std::remove_extent_t<T>);
    pointer_t p = static_cast<pointer_t>(m.allocate(s));

    /*
     * Use placement-new to default-construct each of our elements.
     */
    for (std::size_t i = 0; i < n; ++i) {
        new (&p[i]) std::remove_extent_t<T>();
    }

    /*
     * Create a new unique_ptr, with its own deleter, and return it.
     */
    return unique_obj_ptr<T>(p, details::unique_obj_deleter<T>(m, s, 0, n));
}

/**
 * @brief Create a unique allocation pointer to a type.
 *
 * This function creates a unique allocation pointer to an allocation, which
 * means that the memory is only deallocated, not deconstructed, when it goes
 * out of scope.
 *
 * @tparam T The type to allocate.
 *
 * @param m The memory resource to use.
 *
 * @return A unique allocation pointer to the newly allocated memory.
 *
 * @note This function can only be used with types that are trivially
 * constructible and destructible.
 */
template <typename T>
typename std::enable_if_t<!(std::is_array_v<T> && std::extent_v<T> == 0),
                          unique_alloc_ptr<T>>
make_unique_alloc(memory_resource& m) {
    using pointer_t =
        std::conditional_t<std::is_array_v<T>, std::decay_t<T>, T*>;

    /*
     * Calculate the size of the allocation and use the memory resource to
     * perform an allocation of the requested size.
     */
    std::size_t s = sizeof(T);
    pointer_t p = static_cast<pointer_t>(m.allocate(s));

    /*
     * Create a new unique_ptr, with its own deleter, and return it.
     */
    return unique_alloc_ptr<T>(p, details::unique_alloc_deleter<T>(m, s));
}

/**
 * @brief Create a unique allocation pointer to an array type.
 *
 * This function creates a unique allocation pointer to an allocation of an
 * array type of trivial objects, which is deallocated but not deleted when the
 * pointer goes out of scope.
 *
 * @tparam T The type to allocate.
 *
 * @param m The memory resource to use.
 * @param n The number of elements to allocate.
 *
 * @return A unique allocation pointer to a newly allocated array.
 */
template <typename T>
typename std::enable_if_t<std::is_array_v<T> && std::extent_v<T> == 0,
                          unique_alloc_ptr<T>>
make_unique_alloc(memory_resource& m, std::size_t n) {
    using pointer_t =
        std::conditional_t<std::is_array_v<T>, std::decay_t<T>, T*>;

    /*
     * Calculate the size of the allocation and use the memory resource to
     * perform an allocation of the requested size.
     */
    std::size_t s = n * sizeof(std::remove_extent_t<T>);
    pointer_t p = static_cast<pointer_t>(m.allocate(s));

    /*
     * Create a new unique_ptr, with its own deleter, and return it.
     */
    return unique_alloc_ptr<T>(p, details::unique_alloc_deleter<T>(m, s, 0));
}
}  // namespace vecmem