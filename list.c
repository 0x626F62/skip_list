#include "list.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helper_funcs.h"
#include "tagged_ptr.h"

uint8_t error_check(void *ptr, const char *msg) {
    if (ptr == NULL) {
        fprintf(stderr, "%s", msg);
        return 0;
    }
    return 1;
}

// Create a new empty node
struct Node *new_node() {
    struct Node *node_n = malloc(sizeof(struct Node));
    if (!error_check(node_n, "Error new_node(): malloc() failed\n")) {
        return NULL;
    }
    node_n->above = NULL;
    node_n->prev = node_n->next = node_n->data = NULL;

    return node_n;
}

// Create a new sentinel node
struct Node_s *new_sentinel() {
    struct Node_s *node_s = malloc(sizeof(struct Node_s));
    if (!error_check(node_s, "Error new_sentinel(): malloc() failed\n")) {
        return NULL;
    }
    node_s->below = NULL;
    node_s->prev = node_s->next = NULL;
    return node_s;
}

// Init our list
struct Node_s *init_list() {
    // Create our sentinel nodes
    struct Node_s *head_s = new_sentinel();
    struct Node_s *str_s = new_sentinel();
    struct Node_s *int_s = new_sentinel();

    // data nodes
    struct Node *head_n = new_node();
    struct Node *str_n = new_node();
    struct Node *int_n = new_node();

    if (head_n == NULL || str_n == NULL || int_n == NULL || head_s == NULL ||
        str_s == NULL || int_s == NULL) {
        fprintf(stderr, "Error init_list(): malloc() failed\n");
        free(head_n);
        free(str_n);
        free(int_n);
        free(head_s);
        free(str_s);
        free(int_s);
        return NULL;
    }

    // Link sentinel nodes circularly
    head_s->below = head_n;
    str_s->below = str_n;
    int_s->below = int_n;

    head_s->next = str_s;
    head_s->prev = int_s;

    str_s->next = int_s;
    str_s->prev = head_s;

    int_s->next = head_s;
    int_s->prev = str_s;

    // Link data nodes circularly
    head_n->above = head_s;
    head_n->next = str_n;
    head_n->prev = int_n;

    str_n->above = str_s;
    str_n->next = int_n;
    str_n->prev = head_n;

    int_n->above = int_s;
    int_n->next = head_n;
    int_n->prev = str_n;

    head_n->data = set_params(head_n->data, HEAD_N, 0);
    str_n->data = set_params(str_n->data, STR_D, 0);
    int_n->data = set_params(int_n->data, INT_D, 0);

    if (get_type(head_n->data) != HEAD_N || get_type(str_n->data) != STR_D ||
        get_type(int_n->data) != INT_D) {
        fprintf(stderr, "Error init_list(): set_params() failed\n");
        free(head_n);
        free(str_n);
        free(int_n);
        free(head_s);
        free(str_s);
        free(int_s);
        return NULL;
    }
    return head_s;
}

// Locate the start of the appropriate type ranges
struct Node_s *find_type(struct Node_s *root, void *data) {
    struct Node_s *current = root;
    do {
        if (get_type(current->below->data) == get_type(data)) break;

        current = current->next;
    } while (get_type(current->below->data) != HEAD_N);

    if (get_type(current->below->data) == HEAD_N) {
        fprintf(stderr, "Error find_type(): Couldn't find type.\n");
        return NULL;
    }

    return current;
}

// Called from insert() *current points to the marker node at the start of
// the type section.
// iterate through the sentinel list to find the marker node before the range
// the node needs to be inserted in.
// Refining the comparison helper functions and working out edge cases.
struct Node_s *find_range(struct Node_s *current, void *data) {
    if (!error_check(current, "Error find_range(): current NULL.\n") ||
        !error_check(clear_params(data), "Error find_range(): data NULL.\n")) {
        return NULL;
    }
    struct Node_s *range = current;

    // If 2 marker nodes are next to each other there has been no inserts yet.
    // only marker data nodes have non NULL above nodes. This will let us know
    // if 2 markers are next to each other and the range is empty
    if (range->below->above != NULL && range->next->below->above != NULL) {
        return range;
    }

    uint8_t type_index = TYPE_COUNT;
    for (int i = 0; i < TYPE_COUNT; i++) {
        if (range_checks[i].type == get_type(data)) {
            type_index = i;
        }
    }
    if (type_index == TYPE_COUNT) {
        fprintf(stderr, "Error find_range(): Type not found.\n");
        return NULL;
    }

    // while the type matches at the start and end of range loop
    // if it hits a different type loop stops
    while (get_type(range->below->data) == get_type(data) &&
           get_type(range->next->below->data) == get_type(data)) {
        // if the cmp_func has a match it'll return 1
        if (range_checks[type_index].cmp_func(range, data) &&
            get_type(range->below->data) == get_type(data)) {
            return range;
        }
        range = range->next;
    }
    // if range is set to something other than our data type
    // something was missed in the comparison func.
    if (get_type(range->below->data) == HEAD_N ||
        get_type(range->below->data) != get_type(data)) {
        fprintf(stderr, "Error find_range(): Something broke.\n");
        return NULL;
    }

    // there is only one range
    return range;
}

// Insert a new sentinel/marker node pair
// Called from insert() not to be called on its own. Starts from the current
// sentinel node before the range that the data node needs to be inserted in
struct Node_s *insert_marker(struct Node_s *current) {
    struct Node_s *new_s = malloc(sizeof(struct Node_s));
    struct Node *new_n = malloc(sizeof(struct Node));
    if (!error_check(new_n, "Error insert_marker(): malloc() failed.\n") ||
        !error_check(new_s, "Error insert_marker(): malloc() failed.\n")) {
        free(new_n);
        free(new_s);
        return NULL;
    }

    // Find the middle node in the chunk
    // data for the marker nodes under a sentinel are laid out type/range_count
    // marker nodes are the only nodes with a !NULL above member so checking
    // to make double check we stop at a marker.
    struct Node *mid_node = current->below;
    for (int i = 0;
         i <= get_size(current->below->data) / 2 && mid_node->above == NULL;
         i++)
        mid_node = mid_node->next;

    new_n->data = set_params(new_n->data, get_type(current->below->data), 0);
    new_n->above = new_s;
    new_n->prev = mid_node;
    new_n->next = mid_node->next;
    mid_node->next->prev = new_n;
    mid_node->next = new_n;

    new_s->next = current->next;
    new_s->prev = current;
    new_s->below = new_n;
    current->next->prev = new_s;
    current->next = new_s;

    uint16_t splt_sz1 = 0, splt_sz2 = 0;
    splt_sz1 = splt_sz2 = NODE_COUNT / 2;
    // In case NODE_COUNT is odd we want to account for the 1 extra
    if (get_size(current->below->data) % 2) {
        splt_sz1 = get_size(current->below->data) / 2;
        splt_sz2 = splt_sz1 + 1;
    }
    // Update our types and counts for our 2 new ranges
    current->below->data = set_params(current->below->data,
                                      get_type(current->below->data), splt_sz1);
    new_n->data =
        set_params(new_n->data, get_type(current->below->data), splt_sz2);

    return current;
}

// Insert a node to the list.
// Old logic for type detection hasn't been updated yet to use the
// comparison functions.
struct Node_s *insert(struct Node_s *root, void *data) {
    if (!error_check(root, "Error insert(): root null\n")) {
        return NULL;
    }
    if (!error_check(data, "Error insert(): data null\n")) {
        return NULL;
    }

    // Get the sentinel node at the start of the type section
    struct Node_s *current = find_type(root, data);
    if (!error_check(current, "Error insert(): find_type() failed.\n")) {
        return NULL;
    }

    // Get a pointer to the sentinel at the start of our range to insert
    current = find_range(current, data);
    if (!error_check(current, "Error insert(): find_range() failed.\n")) {
        return NULL;
    }
    // Handle if the range is full
    if (get_size(current->below->data) >= NODE_COUNT) {
        current = insert_marker(current);
        if (!error_check(current, "Error insert(): insert_marker() failed\n")) {
            return NULL;
        }
        // insert_marker() returns a pointer to the new sentinel for the marker
        // Since the range split we need to move back one sentinel then check
        // if it's in the first or second section of the split
        current = current->prev;
        current = find_range(current, data);
        if (!error_check(current, "Error insert(): find_range() failed\n")) {
            return NULL;
        }
    }
    // curr_data_n is our pointer to our marker node
    struct Node *curr_data_n = current->below;

    // match our type and determine appropriate spot the insert
    uint8_t type_index = TYPE_COUNT;
    for (int i = 0; i < TYPE_COUNT; i++) {
        if (node_checks[i].type == get_type(data)) {
            type_index = i;
        }
    }
    if (type_index == TYPE_COUNT) {
        fprintf(stderr, "Error insert(): No type match\n");
        return NULL;
    }
    // Matched our type now iterate until we find the proper spot to insert.
    while (clear_params(curr_data_n->next->above) == NULL) {
        if (node_checks[type_index].cmp_func(curr_data_n, data)) {
            break;  // we're on the node we want to insert after
        }
        curr_data_n = curr_data_n->next;
    }
    // Create a new node
    struct Node *new_n = new_node();
    if (!error_check(new_n, "Error insert(): new_node() failed.\n")) {
        fprintf(stderr, "Error insert(): new_node() failed. Freeing data.\n");
        free(clear_params(data));
        return NULL;
    }
    new_n->data = data;

    // Update the count for our sentinel
    uint16_t count = get_size(current->below->data);
    current->below->data =
        set_params(current->below->data, get_type(new_n->data), ++count);

    // Update pointers to insert new node
    new_n->prev = curr_data_n;
    new_n->next = curr_data_n->next;

    curr_data_n->next = new_n;
    curr_data_n->next->prev = new_n;

    return root;  // We always want to track our head sentinel node.
}

// Print our list. Each data pointer has embedded meta data so handle
// accordingly -- Old needs updated
void print_list(struct Node_s *root) {
    if (!error_check(root, "Error print_list(): *root is NULL\n")) {
        return;
    }
    struct Node *current = root->below->next;
    while (get_type(current->next->data) != HEAD_N) {
        if (current->above != NULL) {
            printf("Marker Node\n");
            continue;
        }
        if (!error_check(current->data, "Marker Node\n")) {
            uint8_t type = get_type(current->data);
            out_funcs[type].output_func(current);
        }
        current = current->next;
    }
}

// Free memory occupied by the list
// Old needs updated
void free_list(struct Node **root) {
    struct Node *current = *root;
    struct Node *next = current->next;
    while (current) {
        next = current->next;
        if (current->data) free(clear_params(current->data));

        free(current);
        current = next;
    }
    *root = NULL;
}

int main() { return 0; }
