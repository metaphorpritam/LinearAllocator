// src/LinearAllocator/LinearAllocator.tpp
#pragma once

#include <cstdlib>
#include <cstring>

namespace alloc {

inline std::expected<LinearAllocator, AllocError> LinearAllocator::create(
    size_t size_in_bytes, bool zero_memory) {

    if (size_in_bytes == 0) {
        return std::unexpected(AllocError::OutOfMemory);
    }

    uint8_t* mem = static_cast<uint8_t*>(std::malloc(size_in_bytes));
    if (!mem) {
        return std::unexpected(AllocError::OutOfMemory);
    }

    // Zero the initial buffer if requested
    if (zero_memory) {
        std::memset(mem, 0, size_in_bytes);
    }

    return LinearAllocator(mem, size_in_bytes, zero_memory);
}

inline std::expected<LinearAllocator, AllocError> LinearAllocator::create_from_buffer(
    uint8_t* buffer_ptr, size_t size_in_bytes, bool zero_memory) {

    if (!buffer_ptr || size_in_bytes == 0) {
        return std::unexpected(AllocError::OutOfMemory);
    }

    // Zero the buffer if requested
    if (zero_memory) {
        std::memset(buffer_ptr, 0, size_in_bytes);
    }

    return LinearAllocator(buffer_ptr, size_in_bytes, zero_memory);
}

inline std::expected<void*, AllocError> LinearAllocator::allocate_bytes(
    size_t size_in_bytes, size_t alignment) {

    // Ensure alignment is a power of 2
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        return std::unexpected(AllocError::InvalidAlignment);
    }

    // Calculate aligned address
    uintptr_t current = reinterpret_cast<uintptr_t>(buffer + used);
    uintptr_t aligned = (current + alignment - 1) & ~(alignment - 1);
    size_t adjustment = aligned - current;

    // Check if we have enough space
    if (used + adjustment + size_in_bytes > capacity) {
        return std::unexpected(AllocError::OutOfMemory);
    }

    // Get the result pointer and update the used counter
    prev_used = used + adjustment; // Track prev_used for resize
    void* result = buffer + prev_used;
    used = prev_used + size_in_bytes;

    // Zero the memory if requested
    if (zero_on_alloc && size_in_bytes > 0) {
        std::memset(result, 0, size_in_bytes);
    }

    return result;
}

template <typename T>
inline std::expected<T*, AllocError> LinearAllocator::allocate(
    size_t count, size_t alignment) {

    if (count == 0) {
        return static_cast<T*>(nullptr);
    }

    auto result = allocate_bytes(count * sizeof(T), alignment);
    if (!result) {
        return std::unexpected(result.error());
    }

    return static_cast<T*>(result.value());
}

template <typename T>
inline std::expected<T*, AllocError> LinearAllocator::resize(
    T* old_ptr, size_t old_count, size_t new_count, size_t alignment) {

    if (!old_ptr || old_count == 0) {
        return allocate<T>(new_count, alignment);
    }

    size_t old_size = old_count * sizeof(T);
    size_t new_size = new_count * sizeof(T);

    // Check if this is the last allocation made
    uintptr_t old_ptr_addr = reinterpret_cast<uintptr_t>(old_ptr);
    uintptr_t buffer_addr = reinterpret_cast<uintptr_t>(buffer);
    size_t old_offset = old_ptr_addr - buffer_addr;

    if (old_offset == prev_used) {
        // This was the last allocation, we can resize in place
        if (prev_used + new_size <= capacity) {
            used = prev_used + new_size;

            // Zero any new memory if expanding
            if (zero_on_alloc && new_size > old_size) {
                std::memset(
                    reinterpret_cast<uint8_t*>(old_ptr) + old_size,
                    0,
                    new_size - old_size
                );
            }

            return old_ptr;
        }
        return std::unexpected(AllocError::OutOfMemory);
    } else {
        // Not the last allocation, allocate new memory and copy
        auto new_mem = allocate<T>(new_count, alignment);
        if (!new_mem) {
            return std::unexpected(new_mem.error());
        }

        // Copy old data to new memory, limited by the smaller of the two sizes
        size_t copy_size = old_size < new_size ? old_size : new_size;
        std::memcpy(new_mem.value(), old_ptr, copy_size);

        return new_mem;
    }
}

} // namespace alloc
