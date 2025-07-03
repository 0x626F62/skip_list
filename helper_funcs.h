#ifndef COMPARE_FUNCS
#define COMPARE_FUNCS

// Comparison helper functions
#include <stdint.h>

#include "list.h"

struct Node_cmps {
    uint8_t type;
    uint8_t (*cmp_func)(struct Node *marker, void *data);
};

struct Range_cmps {
    uint8_t type;
    uint8_t (*cmp_func)(struct Node_s *marker, void *data);
};

extern const struct Range_cmps range_checks[TYPE_COUNT];
extern const struct Node_cmps node_checks[TYPE_COUNT];

uint8_t compare_str(struct Node *marker, void *data);
uint8_t compare_int(struct Node *marker, void *data);
uint8_t str_cmp_range(struct Node_s *marker, void *data);
uint8_t int_cmp_range(struct Node_s *marker, void *data);

#endif
