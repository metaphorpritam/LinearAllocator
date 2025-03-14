# Linear Allocator Implementation

## Overview

This implementation provides a comprehensive linear/arena allocator following data-oriented programming principles. The allocator follows the design described in "Memory Allocation Strategies - Part 2" by Ginger Bill, enhanced with modern C++ features.

The allocator supports:

- Precise memory alignment for any data type
- Efficient O(1) allocation time
- Temporary memory savepoints for short-lived allocations
- Resize functionality for the last allocation
- Using custom user-provided memory buffers
- Memory zeroing control
- Reset operation for reusing memory
- Error handling via `std::expected` (C++23)
- Passing by reference to functions that need to allocate memory

## Directory Structure

```
LinearAllocator/
├── CMakeLists.txt                         # Main CMake build configuration
├── src/
│   └── LinearAllocator/                   # Linear allocator library
│       ├── LinearAllocator.hpp            # Header file with declarations
│       └── LinearAllocator.tpp            # Template implementation
├── example/
│   ├── simplified_example.cpp             # Simple usage example
│   ├── example_allocator.cpp              # Full usage example
│   └── improved_example.cpp               # Example showing advanced features
├── doc/
│   ├── DOXYGEN.in                         # Doxygen configuration
│   ├── mainpage.dox                       # Main documentation page
│   └── library_usage.dox                  # Library usage documentation
└── docs/                                  # Files for github pages
```

## Implementation Details

### LinearAllocator Struct

The `LinearAllocator` follows data-oriented design with:

- Public data members (no getters/setters)
- Methods defined inline within the struct
- No private encapsulation
- Static factory methods for creation

### Memory Management

The allocator works by:

1. Tracking a contiguous memory block with a pointer and usage counter
2. Incrementing the usage counter as allocations are made
3. Ensuring proper alignment by padding allocations as needed
4. Supporting resize operations for the last allocation
5. Providing temporary memory savepoints for short-lived allocations
6. Offering reset operations to reuse memory without deallocation

### TempArenaMemory

The `TempArenaMemory` feature allows:

1. Creating a savepoint in the allocator's state
2. Allocating temporary memory after the savepoint
3. Rolling back to the savepoint, effectively freeing all temporary allocations
4. Automatic rollback when the TempArenaMemory goes out of scope

### Error Handling

Using `std::expected`, the allocator returns either a valid pointer or an error:

- `OutOfMemory`: Not enough space remains
- `InvalidAlignment`: Alignment is not a power of 2

## Key Design Choices

1. **Public Members**: All data members are public, avoiding getters/setters.
2. **Alignment Handling**: Padding ensures proper alignment for any data type.
3. **Reset & Temporary Memory**: Reset and temporary memory savepoints allow reusing memory without reallocation.
4. **Resize**: Efficient resizing of the last allocation.
5. **Buffer Management**: Create allocators from existing buffers or let the allocator manage memory.
6. **Zero Memory Control**: Option to control whether memory is zeroed on allocation.
7. **Struct-based Design**: Adheres to data-oriented programming principles.
8. **Manual Memory Management**: User is responsible for freeing memory when done.

## Example Usage

```cpp
// Create allocator
auto allocator_result = alloc::LinearAllocator::create(1024);
auto& allocator = allocator_result.value();

// Allocate some memory
auto int_result = allocator.allocate<int>();
int* value = int_result.value();
*value = 42;

// Create a temporary memory savepoint
{
    auto temp = alloc::TempArenaMemory::begin(allocator);
    
    // Allocate temporary memory
    auto float_result = allocator.allocate<float>(100);
    // Use temporary memory...
    
    // Memory is automatically freed when temp goes out of scope
}

// Remember to free the allocator's memory when done
std::free(allocator.buffer);
```

## Building and Using

The project uses CMake for building. To build the examples:

```bash
mkdir build && cd build
cmake ..
make
```

See `doc/library_usage.dox` for detailed information on integrating this allocator into your own projects.
