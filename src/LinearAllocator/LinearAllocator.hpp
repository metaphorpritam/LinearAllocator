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
 * @brief A linear/arena allocator that allocates memory linearly from a pre-allocated block
 *
 * This allocator provides O(1) allocation time and doesn't support freeing individual
 * allocations. The entire memory block can be reset at once.
 *
 * Based on the memory allocation strategy described in "Memory Allocation Strategies - Part 2"
 * by Ginger Bill.
 */
struct LinearAllocator {
    uint8_t* buffer;    ///< Pointer to the memory buffer
    size_t capacity;    ///< Total capacity of the allocator in bytes
    size_t used;        ///< Current number of bytes used
    size_t prev_used;   ///< Offset of the previous allocation (for resize operations)
    bool zero_on_alloc; ///< Whether to zero memory on allocation

    /**
     * @brief Construct a new Linear Allocator
     *
     * @param buffer_ptr Pointer to the memory buffer
     * @param capacity_in_bytes Capacity of the allocator in bytes
     * @param zero_memory Whether to zero memory on allocation
     */
    LinearAllocator(uint8_t* buffer_ptr, size_t capacity_in_bytes, bool zero_memory = true)
        : buffer(buffer_ptr), capacity(capacity_in_bytes),
          used(0), prev_used(0), zero_on_alloc(zero_memory) {}

    /**
     * @brief Create a new Linear Allocator with a given capacity
     *
     * @param size_in_bytes The total size of the allocator in bytes
     * @param zero_memory Whether to zero memory on allocation
     * @return std::expected<LinearAllocator, AllocError> A new allocator or an error
     */
    static std::expected<LinearAllocator, AllocError> create(
        size_t size_in_bytes, bool zero_memory = true);

    /**
     * @brief Create a new Linear Allocator from an existing buffer
     *
     * @param buffer_ptr Pointer to the memory buffer
     * @param size_in_bytes The total size of the buffer in bytes
     * @param zero_memory Whether to zero memory on allocation
     * @return std::expected<LinearAllocator, AllocError> A new allocator or an error
     */
    static std::expected<LinearAllocator, AllocError> create_from_buffer(
        uint8_t* buffer_ptr, size_t size_in_bytes, bool zero_memory = true);

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
    std::expected<void*, AllocError> allocate_bytes(size_t size_in_bytes,
                                                   size_t alignment = alignof(std::max_align_t));

    /**
     * @brief Resize the last allocation made
     *
     * If the memory being resized was the last allocation made, this will
     * efficiently resize it in place. Otherwise, it will allocate new memory
     * and copy the old data.
     *
     * @tparam T The type of the allocation
     * @param old_ptr Pointer to the old memory
     * @param old_count Old element count
     * @param new_count New element count
     * @param alignment Alignment requirements
     * @return std::expected<T*, AllocError> Pointer to resized memory or an error
     */
    template <typename T>
    std::expected<T*, AllocError> resize(T* old_ptr, size_t old_count,
                                         size_t new_count,
                                         size_t alignment = alignof(T));

    /**
     * @brief Reset the allocator, effectively freeing all allocations
     *
     * This simply resets the used counters so the memory can be reused.
     */
    void reset() {
        used = 0;
        prev_used = 0;
    }
};

/**
 * @brief Temporary memory savepoint for a LinearAllocator
 *
 * Allows creating a savepoint in the allocator and returning to that point later.
 * Useful for short-lived allocations.
 */
struct TempArenaMemory {
    LinearAllocator* allocator; ///< Pointer to the allocator
    size_t saved_used;          ///< The saved 'used' offset

    /**
     * @brief Create a temporary arena memory savepoint
     *
     * @param alloc The allocator to create a savepoint for
     * @return TempArenaMemory A savepoint that can be used to roll back allocations
     */
    static TempArenaMemory begin(LinearAllocator& alloc) {
        return TempArenaMemory{&alloc, alloc.used};
    }

    /**
     * @brief End the temporary arena memory, rolling back any allocations made since creation
     */
    void end() {
        if (allocator) {
            allocator->used = saved_used;
            allocator = nullptr; // Mark as ended
        }
    }

    /**
     * @brief Destructor that automatically ends the temporary memory if not already ended
     */
    ~TempArenaMemory() {
        if (allocator) {
            end();
        }
    }
};

} // namespace alloc

// Include template implementation
#include "LinearAllocator.tpp"
