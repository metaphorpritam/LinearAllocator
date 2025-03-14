// src/LinearAllocator/LinearAllocator.tpp
#pragma once

#include <cstring>
#include <new>
#include <utility>


namespace alloc {

inline std::expected<LinearAllocator, AllocError> LinearAllocator::create(size_t size_in_bytes) {
    if (size_in_bytes == 0) {
        return std::unexpected(AllocError::OutOfMemory);
    }

    uint8_t* mem = static_cast<uint8_t*>(std::malloc(size_in_bytes));
    if (!mem) {
        return std::unexpected(AllocError::OutOfMemory);
    }

    return LinearAllocator(mem, size_in_bytes, true);
}

inline std::expected<LinearAllocator, AllocError> LinearAllocator::create_from_buffer(uint8_t* existing_buffer, size_t size_in_bytes) {
    if (!existing_buffer || size_in_bytes == 0) {
        return std::unexpected(AllocError::NullPointer);
    }

    return LinearAllocator(existing_buffer, size_in_bytes, false);
}

inline LinearAllocator::LinearAllocator(LinearAllocator&& other) noexcept
    : buffer(other.buffer),
      capacity(other.capacity),
      used(other.used),
      owns_memory(other.owns_memory) {
    other.buffer = nullptr;
    other.capacity = 0;
    other.used = 0;
    other.owns_memory = false;
}

inline LinearAllocator& LinearAllocator::operator=(LinearAllocator&& other) noexcept {
    if (this != &other) {
        free();
        buffer = other.buffer;
        capacity = other.capacity;
        used = other.used;
        owns_memory = other.owns_memory;
        other.buffer = nullptr;
        other.capacity = 0;
        other.used = 0;
        other.owns_memory = false;
    }
    return *this;
}

inline LinearAllocator::~LinearAllocator() {
    free();
}

inline void LinearAllocator::reset() {
    used = 0;
}

inline void LinearAllocator::free() {
    if (owns_memory && buffer) {
        std::free(buffer);
        buffer = nullptr;
    }
    capacity = 0;
    used = 0;
    owns_memory = false;
}

inline std::expected<void*, AllocError> LinearAllocator::allocate_bytes(size_t size_in_bytes, size_t alignment) {
    if (!buffer) {
        return std::unexpected(AllocError::NullPointer);
    }

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
    void* result = buffer + used + adjustment;
    used += adjustment + size_in_bytes;

    return result;
}

template <typename T>
inline std::expected<T*, AllocError> LinearAllocator::allocate(size_t count, size_t alignment) {
    if (count == 0) {
        return static_cast<T*>(nullptr);
    }

    auto result = allocate_bytes(count * sizeof(T), alignment);
    if (!result) {
        return std::unexpected(result.error());
    }

    return static_cast<T*>(result.value());
}

} // namespace alloc
