// example/example_allocator.cpp
#include <cstdio>
#include <cstring>
#include <fmt/core.h>
#include "../src/LinearAllocator/LinearAllocator.hpp"

/**
 * @brief Example struct to demonstrate allocation
 */
struct Point {
    float x, y, z;

    Point(float x_val, float y_val, float z_val) : x(x_val), y(y_val), z(z_val) {}
};

/**
 * @brief Example struct to demonstrate alignment requirements
 */
struct alignas(16) AlignedData {
    double values[2];

    AlignedData(double val1, double val2) {
        values[0] = val1;
        values[1] = val2;
    }
};

/**
 * @brief Function that takes an allocator by reference and allocates from it
 *
 * This demonstrates passing the allocator by reference to functions.
 *
 * @param alloc Reference to a linear allocator
 * @return std::expected<Point*, alloc::AllocError> A pointer to the allocated Points or an error
 */
std::expected<Point*, alloc::AllocError> create_points(alloc::LinearAllocator& alloc) {
    // Allocate an array of 3 Points
    auto points_result = alloc.allocate<Point>(3);
    if (!points_result) {
        return std::unexpected(points_result.error());
    }

    // Construct the Points in-place
    Point* points = points_result.value();
    new (&points[0]) Point(1.0f, 2.0f, 3.0f);
    new (&points[1]) Point(4.0f, 5.0f, 6.0f);
    new (&points[2]) Point(7.0f, 8.0f, 9.0f);

    return points;
}

int main() {
    fmt::print("Linear Allocator Example\n");
    fmt::print("=======================\n\n");

    // Create a linear allocator with 1KB capacity
    auto allocator_result = alloc::LinearAllocator::create(1024);
    if (!allocator_result) {
        fmt::print("Failed to create allocator: {}\n", static_cast<int>(allocator_result.error()));
        return 1;
    }

    auto& allocator = allocator_result.value();
    fmt::print("Created allocator with {} bytes capacity\n", allocator.capacity);

    // Allocate a single integer
    auto int_result = allocator.allocate<int>();
    if (!int_result) {
        fmt::print("Failed to allocate int: {}\n", static_cast<int>(int_result.error()));
        return 1;
    }

    int* int_ptr = int_result.value();
    *int_ptr = 42;
    fmt::print("Allocated int with value: {}\n", *int_ptr);
    fmt::print("Used memory: {} bytes\n", allocator.used);

    // Allocate an array of floats
    auto float_result = allocator.allocate<float>(5);
    if (!float_result) {
        fmt::print("Failed to allocate floats: {}\n", static_cast<int>(float_result.error()));
        return 1;
    }

    float* floats = float_result.value();
    for (int i = 0; i < 5; i++) {
        floats[i] = static_cast<float>(i) * 1.5f;
    }

    fmt::print("Allocated float array with values: ");
    for (int i = 0; i < 5; i++) {
        fmt::print("{:.1f} ", floats[i]);
    }
    fmt::print("\n");
    fmt::print("Used memory: {} bytes\n", allocator.used);

    // Allocate an aligned struct
    auto aligned_result = allocator.allocate<AlignedData>(1, alignof(AlignedData));
    if (!aligned_result) {
        fmt::print("Failed to allocate aligned data: {}\n", static_cast<int>(aligned_result.error()));
        return 1;
    }

    AlignedData* aligned_data = aligned_result.value();
    new (aligned_data) AlignedData(3.14159, 2.71828);

    fmt::print("Allocated aligned data with values: {:.5f}, {:.5f}\n",
               aligned_data->values[0], aligned_data->values[1]);
    fmt::print("Used memory: {} bytes\n", allocator.used);
    fmt::print("AlignedData alignment: {}\n", alignof(AlignedData));

    // Use a function that takes the allocator by reference
    auto points_result = create_points(allocator);
    if (!points_result) {
        fmt::print("Failed to create points: {}\n", static_cast<int>(points_result.error()));
        return 1;
    }

    Point* points = points_result.value();
    fmt::print("Allocated points:\n");
    for (int i = 0; i < 3; i++) {
        fmt::print("  Point {}: ({:.1f}, {:.1f}, {:.1f})\n",
                   i, points[i].x, points[i].y, points[i].z);
    }
    fmt::print("Used memory: {} bytes\n", allocator.used);

    // Reset the allocator
    fmt::print("\nResetting allocator...\n");
    allocator.reset();
    fmt::print("Used memory after reset: {} bytes\n", allocator.used);

    // Create a second allocator from a user-provided buffer
    fmt::print("\nCreating a second allocator from a user-provided buffer...\n");
    uint8_t user_buffer[256];
    auto buffer_allocator_result = alloc::LinearAllocator::create_from_buffer(user_buffer, sizeof(user_buffer));

    if (!buffer_allocator_result) {
        fmt::print("Failed to create buffer allocator: {}\n", static_cast<int>(buffer_allocator_result.error()));
        return 1;
    }

    auto& buffer_allocator = buffer_allocator_result.value();
    fmt::print("Created buffer allocator with {} bytes capacity\n", buffer_allocator.capacity);

    // Allocate a string
    const char* message = "Hello, Linear Allocator!";
    size_t message_len = strlen(message) + 1;
    auto char_result = buffer_allocator.allocate<char>(message_len);

    if (!char_result) {
        fmt::print("Failed to allocate string: {}\n", static_cast<int>(char_result.error()));
        return 1;
    }

    char* char_ptr = char_result.value();
    strcpy(char_ptr, message);
    fmt::print("Allocated string: {}\n", char_ptr);
    fmt::print("Used memory: {} bytes\n", buffer_allocator.used);

    // Demonstrate allocation failure due to insufficient space
    fmt::print("\nAttempting to allocate more memory than available...\n");
    auto large_alloc_result = buffer_allocator.allocate<uint8_t>(1000);
    if (!large_alloc_result) {
        fmt::print("Expected allocation failure: {}\n",
                   static_cast<int>(large_alloc_result.error()) == static_cast<int>(alloc::AllocError::OutOfMemory)
                   ? "Out of Memory (as expected)"
                   : "Unexpected error");
    }

    // Clean up the allocator's memory
    std::free(allocator.buffer);
    // Note: We don't free user_buffer as it's a stack-allocated array

    return 0;
}
