#include "helper_funcs.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "list.h"
#include "tagged_ptr.h"

// If larger than the next node
uint8_t compare_str(struct Node *marker, void *data) {
    if (strcmp((char *)clear_params(data),
               (char *)clear_params(marker->next->data)) < 0) {
        return 1;
    }
    return 0;
}

// if larger than next node
uint8_t compare_int(struct Node *marker, void *data) {
    if (*((int *)clear_params(data)) <
        *((int *)clear_params(marker->next->data))) {
        return 1;
    }

    return 0;
}

// Functions for comparisons to find ranges
// strcmp() > 0 first string larger 0 == equal < 0 less than
// first condition checks if it fits between the first and first node
// of next range
// else if beginning of range
uint8_t str_cmp_range(struct Node_s *marker, void *data) {
    if (strcmp(((char *)clear_params(data)),
               ((char *)clear_params(marker->below->next->data))) >= 0 &&
        strcmp(((char *)clear_params(data)),
               ((char *)clear_params(marker->next->below->next->data))) < 0) {
        return 1;
    } else if (strcmp(((char *)clear_params(data)),
                      ((char *)clear_params(marker->below->next->data))) < 0) {
        return 1;
    }
    return 0;
}

// if start < x < start of next || x < start
uint8_t int_cmp_range(struct Node_s *marker, void *data) {
    if (*((int *)clear_params(data)) >=
            *((int *)clear_params(marker->below->next->data)) &&
        *((int *)clear_params(data)) <
            *((int *)clear_params(  // clang-format =|
                marker->next->below->next->data))) {
        return 1;
    } else if (*((int *)clear_params(data)) <
               *((int *)clear_params(marker->below->next->data))) {
        return 1;
    }
    return 0;
}

// Print out funcs
void print_str(struct Node *node) {
    if (get_type(node->data) != STR_D) {
        fprintf(stderr, "Error print_str(): *data not a string\n");
        return;
    }
    if (!error_check(clear_params(node->data),
                     "Error print_str(): data is null\n")) {
        return;
    }
    uint16_t size = get_size(node->data);
    for (uint16_t i = 0; i < size; i++) {
        char c = ((char *)clear_params(node->data))[i];
        putchar(c);
    }
}

void print_int(struct Node *node) {
    if (get_type(node->data) != INT_D) {
        fprintf(stderr, "Error print_int(): *data not an int\n");
        return;
    }

    printf("*data: %i\n", *((int *)clear_params(node->data)));
}
// Update as needed to add more types
// Compare to type then call the corresponding comparison function
const struct Range_cmps range_checks[TYPE_COUNT] = {
    {.type = STR_D, .cmp_func = str_cmp_range},
    {.type = INT_D, .cmp_func = int_cmp_range}};

const struct Node_cmps node_checks[TYPE_COUNT] = {
    {.type = STR_D, .cmp_func = compare_str},
    {.type = INT_D, .cmp_func = compare_int}};

const struct Output_funcs out_funcs[TYPE_COUNT] = {
    {.type = STR_D, .output_func = print_str},
    {.type = INT_D, .output_func = print_int}};
