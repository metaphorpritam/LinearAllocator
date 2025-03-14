// src/LinearAllocator/LinearAllocator.hpp
#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <memory>
#include <expected>
#include <string>

namespace alloc {

/**
 * @brief Error codes for the linear allocator
 */
enum class AllocError {
    OutOfMemory,      ///< Not enough memory in the allocator
    InvalidAlignment, ///< Alignment is not a power of 2
    NullPointer,      ///< Buffer pointer is null
    AllocationFailed  ///< General allocation failure
};

/**
 * @brief A simple linear allocator with alignment specification
 *
 * This allocator manages a contiguous block of memory and allocates
 * from it linearly, with support for alignment requirements.
 */
struct LinearAllocator {
    uint8_t* buffer;       ///< Pointer to the allocated memory
    size_t capacity;       ///< Total capacity of the allocator in bytes
    size_t used;           ///< Current number of bytes used
    bool owns_memory;      ///< Whether the allocator owns the memory (responsible for freeing)

    /**
     * @brief Construct a default Linear Allocator (no memory allocated)
     */
    LinearAllocator() : buffer(nullptr), capacity(0), used(0), owns_memory(false) {}

    /**
     * @brief Construct a new Linear Allocator
     *
     * @param buffer_ptr Pointer to the memory buffer
     * @param capacity_in_bytes Capacity of the allocator in bytes
     * @param owns_mem Whether the allocator owns the memory
     */
    LinearAllocator(uint8_t* buffer_ptr, size_t capacity_in_bytes, bool owns_mem)
        : buffer(buffer_ptr), capacity(capacity_in_bytes), used(0), owns_memory(owns_mem) {}

    /**
     * @brief Create a new Linear Allocator with a given capacity
     *
     * @param size_in_bytes The total size of the allocator in bytes
     * @return std::expected<LinearAllocator, AllocError> A new allocator or an error
     */
    static std::expected<LinearAllocator, AllocError> create(size_t size_in_bytes);

    /**
     * @brief Create a new Linear Allocator using an existing buffer
     *
     * @param existing_buffer Pointer to the existing buffer
     * @param size_in_bytes The size of the buffer in bytes
     * @return std::expected<LinearAllocator, AllocError> A new allocator or an error
     */
    static std::expected<LinearAllocator, AllocError> create_from_buffer(uint8_t* existing_buffer, size_t size_in_bytes);

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
     * This does not free the memory, it just resets the used counter so the
     * memory can be reused for new allocations.
     */
    void reset();

    /**
     * @brief Free the allocator and its memory if it owns it
     *
     * This deallocates the memory if the allocator owns it, and resets
     * the allocator state.
     */
    void free();

    /**
     * @brief Move constructor
     *
     * @param other The allocator to move from
     */
    LinearAllocator(LinearAllocator&& other) noexcept;

    /**
     * @brief Move assignment operator
     *
     * @param other The allocator to move from
     * @return LinearAllocator& Reference to this allocator
     */
    LinearAllocator& operator=(LinearAllocator&& other) noexcept;

    /**
     * @brief Destructor
     *
     * Calls free() to deallocate memory if the allocator owns it.
     */
    ~LinearAllocator();

    // Delete copy constructor and copy assignment to enforce move-only semantics
    LinearAllocator(const LinearAllocator&) = delete;
    LinearAllocator& operator=(const LinearAllocator&) = delete;
};

} // namespace alloc

// Include template implementation
#include "LinearAllocator.tpp"
