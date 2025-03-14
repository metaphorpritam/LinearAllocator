// src/LinearAllocator/LinearAllocator.hpp
#pragma once

#include <cstdint>
#include <cstddef>
#include <expected>

namespace alloc {

/**
 * @brief Error codes for the linear allocator
 */
enum class AllocError {
    OutOfMemory,      ///< Not enough memory in the allocator
    InvalidAlignment  ///< Alignment is not a power of 2
};

/**
 * @brief A simplified linear allocator
 *
 * This allocator manages a pre-allocated memory block and allocates
 * from it linearly, with support for alignment requirements.
 */
struct LinearAllocator {
    uint8_t* buffer;  ///< Pointer to the memory buffer
    size_t capacity;  ///< Total capacity of the allocator in bytes
    size_t used;      ///< Current number of bytes used

    /**
     * @brief Construct a new Linear Allocator
     *
     * @param buffer_ptr Pointer to the memory buffer
     * @param capacity_in_bytes Capacity of the allocator in bytes
     */
    LinearAllocator(uint8_t* buffer_ptr, size_t capacity_in_bytes)
        : buffer(buffer_ptr), capacity(capacity_in_bytes), used(0) {}

    /**
     * @brief Create a new Linear Allocator with a given capacity
     *
     * @param size_in_bytes The total size of the allocator in bytes
     * @return std::expected<LinearAllocator, AllocError> A new allocator or an error
     */
    static std::expected<LinearAllocator, AllocError> create(size_t size_in_bytes);

    /**
     * @brief Allocate memory with a specified alignment
     *
     * @tparam T The type to allocate for
     * @param count The number of elements to allocate
     * @param alignment The alignment required (must be a power of 2)
     * @return std::expected<T*, AllocError> Pointer to the allocated memory or an error
     */
    template <typename T>
    std::expected<T*, AllocError> allocate(size_t count = 1, size_t alignment = alignof(T));

    /**
     * @brief Allocate uninitialized memory with a specified alignment
     *
     * @param size_in_bytes The size to allocate in bytes
     * @param alignment The alignment required (must be a power of 2)
     * @return std::expected<void*, AllocError> Pointer to the allocated memory or an error
     */
    std::expected<void*, AllocError> allocate_bytes(size_t size_in_bytes, size_t alignment = alignof(std::max_align_t));

    /**
     * @brief Reset the allocator, effectively freeing all allocations
     *
     * This simply resets the used counter so the memory can be reused.
     */
    void reset() {
        used = 0;
    }
};

} // namespace alloc

// Include template implementation
#include "LinearAllocator.tpp"
