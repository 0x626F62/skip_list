#ifndef TAGGED_PTR
#define TAGGED_PTR

#include <stdint.h>

void *set_params(void *ptr, uint8_t type, uint16_t size);
uint8_t get_type(void *ptr);
uint16_t get_size(void *ptr);
void *clear_params(void *ptr);

#endif
