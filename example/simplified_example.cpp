// example/simplified_example.cpp
#include <fmt/core.h>
#include "../src/LinearAllocator/LinearAllocator.hpp"

/**
 * @brief Example struct to demonstrate allocation
 */
struct Item {
    int id;
    float value;

    Item(int i, float v) : id(i), value(v) {}
};

/**
 * @brief Function that takes an allocator by reference and allocates from it
 *
 * @param alloc Reference to a linear allocator
 * @return std::expected<Item*, alloc::AllocError> A pointer to the allocated Item or an error
 */
std::expected<Item*, alloc::AllocError> create_item(alloc::LinearAllocator& alloc) {
    // Allocate an Item from the allocator
    auto item_result = alloc.allocate<Item>(1);
    if (!item_result) {
        return std::unexpected(item_result.error());
    }

    // Construct the Item in-place
    Item* item = item_result.value();
    new (item) Item(42, 3.14f);

    return item;
}

/**
 * @brief Function that allocates multiple items from an allocator
 *
 * @param alloc Reference to a linear allocator
 * @param count Number of items to allocate
 * @return std::expected<Item*, alloc::AllocError> A pointer to the allocated Items or an error
 */
std::expected<Item*, alloc::AllocError> create_items(alloc::LinearAllocator& alloc, int count) {
    // Allocate multiple Items from the allocator
    auto items_result = alloc.allocate<Item>(count);
    if (!items_result) {
        return std::unexpected(items_result.error());
    }

    // Construct each Item in-place
    Item* items = items_result.value();
    for (int i = 0; i < count; i++) {
        new (&items[i]) Item(i, i * 1.5f);
    }

    return items;
}

int main() {
    fmt::print("Simplified Linear Allocator Example\n");
    fmt::print("==================================\n\n");

    // Create a linear allocator with 1KB capacity
    auto allocator_result = alloc::LinearAllocator::create(1024);
    if (!allocator_result) {
        fmt::print("Failed to create allocator: {}\n",
                   static_cast<int>(allocator_result.error()));
        return 1;
    }

    auto& allocator = allocator_result.value();
    fmt::print("Created allocator with {} bytes capacity\n", allocator.capacity);

    // Create a single item
    auto item_result = create_item(allocator);
    if (!item_result) {
        fmt::print("Failed to create item: {}\n",
                   static_cast<int>(item_result.error()));
        return 1;
    }

    Item* item = item_result.value();
    fmt::print("Created item: id={}, value={:.2f}\n", item->id, item->value);
    fmt::print("Used memory: {} bytes\n", allocator.used);

    // Create multiple items
    auto items_result = create_items(allocator, 5);
    if (!items_result) {
        fmt::print("Failed to create items: {}\n",
                   static_cast<int>(items_result.error()));
        return 1;
    }

    Item* items = items_result.value();
    fmt::print("\nCreated 5 items:\n");
    for (int i = 0; i < 5; i++) {
        fmt::print("  Item {}: id={}, value={:.2f}\n", i, items[i].id, items[i].value);
    }
    fmt::print("Used memory: {} bytes\n", allocator.used);

    // Reset the allocator and reuse it
    fmt::print("\nResetting allocator...\n");
    allocator.reset();
    fmt::print("Used memory after reset: {} bytes\n", allocator.used);

    // Allocate again after reset
    auto new_items_result = create_items(allocator, 3);
    if (!new_items_result) {
        fmt::print("Failed to create new items: {}\n",
                   static_cast<int>(new_items_result.error()));
        return 1;
    }

    Item* new_items = new_items_result.value();
    fmt::print("\nCreated 3 new items after reset:\n");
    for (int i = 0; i < 3; i++) {
        fmt::print("  Item {}: id={}, value={:.2f}\n", i, new_items[i].id, new_items[i].value);
    }
    fmt::print("Used memory: {} bytes\n", allocator.used);

    // Note: We're not freeing the allocator's memory in this example
    // In a real application, you would need to free it manually since we removed the destructor
    std::free(allocator.buffer);

    return 0;
}
