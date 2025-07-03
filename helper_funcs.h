#ifndef HELPER_FUNCS
#define HELPER_FUNCS

// Comparison helper functions
#include <stdint.h>
#include <stdio.h>

#include "list.h"
#include "tagged_ptr.h"

struct Node_cmps {
    uint8_t type;
    uint8_t (*cmp_func)(struct Node *marker, void *data);
};

struct Range_cmps {
    uint8_t type;
    uint8_t (*cmp_func)(struct Node_s *marker, void *data);
};

struct Output_funcs {
    uint8_t type;
    void (*output_func)(struct Node *node);
};

extern const struct Range_cmps range_checks[TYPE_COUNT];
extern const struct Node_cmps node_checks[TYPE_COUNT];
extern const struct Output_funcs out_funcs[TYPE_COUNT];

// Comparison functions
uint8_t compare_str(struct Node *marker, void *data);
uint8_t compare_int(struct Node *marker, void *data);
uint8_t str_cmp_range(struct Node_s *marker, void *data);
uint8_t int_cmp_range(struct Node_s *marker, void *data);

// Print out functions
void print_str(struct Node *node);
void print_int(struct Node *node);

#endif
