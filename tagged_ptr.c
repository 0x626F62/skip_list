#include "tagged_ptr.h"

void *set_params(void *ptr, uint8_t type, uint16_t size) {
    // restrict type to 4 bits and size to 12 bits
    type &= 0xF;
    size &= 0x0FFF;

    // Combine type and size into one 16 bit var
    // Shift type over 12 bits, OR it with size, then store in params
    // ex: 1010 | 0000 1100 0011 == 1010 1100 0011
    uint16_t params = (uint16_t)((type << 12) | size);

    // Cast ptr to a 64bit integer type
    uintptr_t addr = (uintptr_t)ptr;

    // Clear the upper 16 bits of addr and embed params in them
    // ~(0xFFFFULL << 48) = 0x0000FFFFFFFFFFFF
    addr = (addr & ~(0xFFFFULL << 48)) | ((uintptr_t)params << 48);

    return (void *)addr;
}

// Mask out the lower 60 bits and return the highest 4 bits
uint8_t get_type(void *ptr) { return ((uintptr_t)ptr & (0xFULL << 60)) >> 60; }

// Mask out all bits except 59–48 and return the 12-bit size
uint16_t get_size(void *ptr) {
    return ((uintptr_t)ptr & (0x0FFFULL << 48)) >> 48;
}

// Clear the upper 16 bits, returning the original pointer
void *clear_params(void *ptr) {
    return (void *)((uintptr_t)ptr & ~(0xFFFFULL << 48));
}
