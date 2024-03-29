#include "stack_allocator.h"

#include <stdlib.h>
#include <cassert>
#define _unused(x) ((void)(x))

StackAllocator::StackAllocator(void* memoryPointer, size_t  memorySizeBytes)
:  Allocator(memoryPointer, memorySizeBytes), _stackMarker(0) {
}

void* StackAllocator::allocate(size_t sizeBytes, size_t alignment) {
    assert(alignment >= 1);
    assert(alignment <= 128);
    assert((alignment & (alignment - 1)) == 0); // verify power of 2

    // Determine total amount of memory to allocate.
    size_t expandSize_bytes = sizeBytes + alignment;
    _unused(expandSize_bytes);

    // Assert that the stack can allocate the block
    assert(_stackMarker + expandSize_bytes <= _memorySizeBytes);

    // Cast so we can perform arithmetic on stack pointer
    uintptr_t sPtr = reinterpret_cast<uintptr_t>(_memoryPointer);

    // Allocate unaligned block & convert address to uintptr_t.
    uintptr_t rawAdress = reinterpret_cast<uintptr_t>(sPtr + _stackMarker);

    // Calculate the adjustment by masking off the lower bits
    // of the address, to determine how "misaligned" it is.
    size_t mask = (alignment - 1);
    uintptr_t misalignment = (rawAdress & mask);
    ptrdiff_t adjustment = alignment - misalignment;

    // Calculate the adjusted address, return as pointer.
    uintptr_t alignedAddress = rawAdress + adjustment;

    // Store the adjustment in the byte immediately
    // preceding the adjusted address.
    assert(adjustment < 256);
    uint8_t* pAlignedMem = reinterpret_cast<uint8_t*>(alignedAddress);
    pAlignedMem[-1] = static_cast<uint8_t>(adjustment);

    // Increment the stack marker
    _stackMarker += sizeBytes + static_cast<size_t>(adjustment);

    return static_cast<void*> (pAlignedMem);
}

void* StackAllocator::allocUnaligned(size_t sizeBytes) {
     // Assert that the stack can allocate the block
    assert(_stackMarker + sizeBytes <= _memorySizeBytes);

    // cast so we can perform arithmetic on stack pointer
    uintptr_t sPtr = reinterpret_cast<uintptr_t>(_memoryPointer);

    // Find the address
    void* address = reinterpret_cast<void*>(sPtr + _stackMarker);
    // Increment the stack marker
    _stackMarker += sizeBytes;

    return address;
}

void StackAllocator::free(void* pointer) {
        const uint8_t* pAlignedMem = reinterpret_cast<const uint8_t*>(pointer);

        uintptr_t alignedAddress = reinterpret_cast<uintptr_t>(pointer);
        ptrdiff_t adjustment = static_cast<ptrdiff_t>(pAlignedMem[-1]);

        uintptr_t rawAdress = alignedAddress - adjustment;
        void* pRawMem = reinterpret_cast<void*>(rawAdress);

        freeUnaligned(pRawMem);
    }

void StackAllocator::freeUnaligned(void* pointer) {
    uintptr_t ptr = reinterpret_cast<uintptr_t>(pointer);
    uintptr_t sPtr = reinterpret_cast<uintptr_t>(_memoryPointer);

    ptrdiff_t diff = ptr - sPtr;
    // Make sure that the pointer is within the stack
    assert(ptr >= sPtr);
    assert(diff >= 0);
    assert(static_cast<size_t>(diff) < _memorySizeBytes);

    // The updated stack marker should point to the 
    // Marker that the pointer mapps to
    _stackMarker = static_cast<size_t>(diff);
}

void StackAllocator::clear() {
    _stackMarker = 0;
}
