#include "compare_funcs.h"

#include <stdint.h>
#include <string.h>

#include "list.h"
#include "tagged_ptr.h"

// Marker nodes have a non NULL above pointer will traverse
// until it reaches a large value or hits the marker
// same for int. Comparison logic is just different
uint8_t compare_str(struct Node *marker, void *data) {
    if (strcmp((char *)clear_params(data),
               (char *)clear_params(marker->next->data)) > 0) {
        return 1;
    }
    return 0;
}

uint8_t compare_int(struct Node *marker, void *data) {
    if (*((int *)clear_params(data)) >
        *((int *)clear_params(marker->next->data))) {
        return 1;
    }

    return 0;
}

// Functions for comparisons to find ranges
uint8_t str_cmp_range(struct Node_s *marker, void *data) {
    if (strcmp(((char *)clear_params(data)),
               ((char *)clear_params(marker->below->next->data))) >= 0 &&
        strcmp(((char *)clear_params(data)),
               ((char *)clear_params(marker->next->below->prev->data))) < 0) {
        return 1;
    }
    return 0;
}

// if start < x < end || x < start || end < x && next != type
uint8_t int_cmp_range(struct Node_s *marker, void *data) {
    if (*((int *)clear_params(data)) >
            *((int *)clear_params(marker->below->next->data)) &&
        *((int *)clear_params(data)) <
            *((int *)clear_params(  // clang-format =|
                marker->next->below->prev->data))) {
        return 1;
    } else if (*((int *)clear_params(data)) <
               *((int *)clear_params(marker->below->next->data))) {
        return 1;
    } else if (*((int *)clear_params(data)) >
                   *((int *)clear_params(marker->next->below->prev->data)) &&
               get_type(marker->next->below->data) != get_type(data)) {
        return 1;
    }
    return 0;
}

// Update as needed to add more types
const struct Range_cmps range_checks[2] = {
    {.type = STR_D, .cmp_func = str_cmp_range},
    {.type = INT_D, .cmp_func = int_cmp_range}};

const struct Node_cmps node_checks[2] = {
    {.type = STR_D, .cmp_func = compare_str},
    {.type = INT_D, .cmp_func = compare_int}};
