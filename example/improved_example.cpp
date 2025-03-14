// example/improved_example.cpp
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
 * @brief Function that allocates and initializes points using an allocator
 *
 * Demonstrates passing the allocator by reference to a function.
 *
 * @param alloc Reference to a linear allocator
 * @param count Number of points to create
 * @return std::expected<Point*, alloc::AllocError> Pointer to the points or an error
 */
std::expected<Point*, alloc::AllocError> create_points(alloc::LinearAllocator& alloc, int count) {
    auto points_result = alloc.allocate<Point>(count);
    if (!points_result) {
        return std::unexpected(points_result.error());
    }

    Point* points = points_result.value();
    for (int i = 0; i < count; i++) {
        new (&points[i]) Point(i * 1.0f, i * 2.0f, i * 3.0f);
    }

    return points;
}

int main() {
    fmt::print("Improved Linear Allocator Example\n");
    fmt::print("================================\n\n");

    // Create a linear allocator with 1KB capacity
    auto allocator_result = alloc::LinearAllocator::create(1024);
    if (!allocator_result) {
        fmt::print("Failed to create allocator: {}\n",
                   static_cast<int>(allocator_result.error()));
        return 1;
    }

    auto& allocator = allocator_result.value();
    fmt::print("Created allocator with {} bytes capacity\n", allocator.capacity);

    // Allocate an array of points
    auto points_result = create_points(allocator, 3);
    if (!points_result) {
        fmt::print("Failed to allocate points: {}\n",
                   static_cast<int>(points_result.error()));
        return 1;
    }

    Point* points = points_result.value();
    fmt::print("Allocated 3 points:\n");
    for (int i = 0; i < 3; i++) {
        fmt::print("  Point {}: ({:.1f}, {:.1f}, {:.1f})\n",
                   i, points[i].x, points[i].y, points[i].z);
    }
    fmt::print("Used memory: {} bytes\n", allocator.used);

    // Demonstrate resize functionality
    fmt::print("\nResizing the points array from 3 to 5 points...\n");
    auto resized_points = allocator.resize<Point>(points, 3, 5);
    if (!resized_points) {
        fmt::print("Failed to resize points: {}\n",
                   static_cast<int>(resized_points.error()));
        return 1;
    }

    points = resized_points.value();
    new (&points[3]) Point(10.0f, 11.0f, 12.0f);
    new (&points[4]) Point(13.0f, 14.0f, 15.0f);

    fmt::print("Resized to 5 points, used memory: {} bytes\n", allocator.used);
    for (int i = 0; i < 5; i++) {
        fmt::print("  Point {}: ({:.1f}, {:.1f}, {:.1f})\n",
                   i, points[i].x, points[i].y, points[i].z);
    }

    // Demonstrate temporary memory
    {
        fmt::print("\nCreating temporary memory scope...\n");
        auto temp_mem = alloc::TempArenaMemory::begin(allocator);

        fmt::print("Used before temp allocations: {} bytes\n", allocator.used);

        // Allocate some temporary data
        auto temp_result = allocator.allocate<float>(100);
        if (!temp_result) {
            fmt::print("Failed to allocate temporary data: {}\n",
                       static_cast<int>(temp_result.error()));
            return 1;
        }

        float* temp_data = temp_result.value();
        for (int i = 0; i < 100; i++) {
            temp_data[i] = static_cast<float>(i);
        }

        fmt::print("Used during temp allocations: {} bytes\n", allocator.used);

        // Explicitly end the temporary memory
        temp_mem.end();
    }

    fmt::print("Used after temp memory ended: {} bytes\n", allocator.used);

    // Create another temporary memory but let it be cleaned up automatically when going out of scope
    {
        fmt::print("\nCreating another temporary memory scope (auto cleanup)...\n");
        auto temp_mem = alloc::TempArenaMemory::begin(allocator);

        auto temp_result = allocator.allocate<double>(50);
        if (!temp_result) {
            fmt::print("Failed to allocate temporary data: {}\n",
                       static_cast<int>(temp_result.error()));
            return 1;
        }

        fmt::print("Used during temp allocations: {} bytes\n", allocator.used);

        // Let the destructor clean up when the scope ends
    }

    fmt::print("Used after temp memory scope ends: {} bytes\n", allocator.used);

    // Create a linear allocator from an existing buffer
    fmt::print("\nCreating an allocator from an existing buffer...\n");
    uint8_t buffer[512];
    auto buffer_allocator_result = alloc::LinearAllocator::create_from_buffer(buffer, sizeof(buffer));
    if (!buffer_allocator_result) {
        fmt::print("Failed to create buffer allocator: {}\n",
                   static_cast<int>(buffer_allocator_result.error()));
        return 1;
    }

    auto& buffer_allocator = buffer_allocator_result.value();
    fmt::print("Created buffer allocator with {} bytes capacity\n", buffer_allocator.capacity);

    auto int_result = buffer_allocator.allocate<int>(10);
    if (!int_result) {
        fmt::print("Failed to allocate ints: {}\n",
                   static_cast<int>(int_result.error()));
        return 1;
    }

    int* ints = int_result.value();
    for (int i = 0; i < 10; i++) {
        ints[i] = i * 10;
    }

    fmt::print("Allocated 10 ints, values: ");
    for (int i = 0; i < 10; i++) {
        fmt::print("{} ", ints[i]);
    }
    fmt::print("\n");

    // Free the allocated memory from the first allocator
    std::free(allocator.buffer);

    return 0;
}
