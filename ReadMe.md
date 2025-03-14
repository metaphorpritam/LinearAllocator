# Linear Allocator Implementation

## Overview

This implementation provides a simplified linear allocator with alignment specification following data-oriented programming principles. The allocator supports:

- Precise memory alignment for any data type
- Using `uint8_t*` instead of `char*` for pointer arithmetic
- Passing the allocator by reference to functions
- Memory reset operation for reusing memory
- Error handling via `std::expected` (C++23)

## Directory Structure

```.txt
project/
├── src/
│   └── LinearAllocator/
│       ├── LinearAllocator.hpp  # Header file with declarations
│       └── LinearAllocator.tpp  # Template implementation
├── example/
│   └── simplified_example.cpp   # Example usage
├── doc/
│   ├── DOXYGEN.in              # Doxygen configuration
│   ├── mainpage.dox            # Main documentation page
│   └── library_usage.dox       # Library usage documentation
|── docs/..                     # Files for github pages
└── CMakeLists.txt              # Build configuration
```

## Implementation Details

### LinearAllocator Struct

The `LinearAllocator` follows data-oriented design with:

- Public data members (no getters/setters)
- Methods defined inline within the struct
- No private encapsulation
- Methods for allocation and reset

### Memory Management

The allocator works by:

1. Tracking a contiguous memory block with a pointer and usage counter
2. Incrementing the usage counter as allocations are made
3. Ensuring proper alignment by padding allocations as needed
4. Supporting reset operations to reuse memory without deallocation

### Error Handling

Using `std::expected`, the allocator returns either a valid pointer or an error:

- `OutOfMemory`: Not enough space remains
- `InvalidAlignment`: Alignment is not a power of 2

## Key Design Choices

1. **Public Members**: All data members are public, avoiding getters/setters.
2. **Alignment Handling**: Padding ensures proper alignment for any data type.
3. **Reset**: Reset allows reusing memory without reallocation.
4. **Struct-based Design**: Adheres to data-oriented programming principles.
5. **Manual Memory Management**: User is responsible for freeing memory when done.
