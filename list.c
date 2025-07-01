#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tagged_ptr.h"

// Type Definitions
#define HEAD_N 1  // Mark the "head/tail" (it's circular)
#define STR_D 2   // string data node
#define INT_D 3   // int data node

// Interval to insert a new marker pair
#define NODE_COUNT 30

struct Node_s {
    struct Node_s *next;
    struct Node_s *prev;
    struct Node *below;
};

struct Node {
    void *data;            // Tagged type/size
    struct Node_s *above;  // If it's linked to a sentinel it's a marker
    struct Node *prev;
    struct Node *next;
};

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

// Called from insert() *current points to the marker node at the start of
// the type section.
// iterate through the sentinel list to find the marker node before the range
// the node needs to be inserted in.
struct Node_s *find_range(struct Node_s *current, void *data) {
    struct Node_s *range = current;

    // If 2 marker nodes are next to each other there has been no inserts yet.
    // only marker data nodes have non NULL above nodes. This will let us know
    // if 2 markers are next to each other and the range is empty
    if (range->below->above != NULL && range->next->below->above != NULL) {
        return range;
    }

    while (get_type(range->below->data) == get_type(data) &&
           get_type(range->below->data) != HEAD_N) {
        switch (get_type(data)) {
            case INT_D:  // if data >= first_n and data < last_n correct range
                if (*((int *)clear_params(data)) >
                        *((int *)clear_params(range->below->next->data)) &&
                    *((int *)clear_params(data)) <
                        *((int *)clear_params(  // clang-format =|
                            range->next->below->prev
                                ->data))) {  // should probably read the docs
                    return range;
                }
                break;

            case STR_D:
                if (strcmp(((char *)clear_params(data)),
                           ((char *)clear_params(range->below->next->data))) >=
                        0 &&
                    strcmp(((char *)clear_params(data)),
                           ((char *)clear_params(
                               range->next->below->prev->data))) < 0) {
                    return range;
                }
                break;

            default:
                fprintf(stderr, "Error find_range(): invalid type.\n");
                return NULL;
        }
    }
    return range;  // Should never reach here.
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
    current->below->data = set_params(current->below->data,
                                      get_type(current->below->data), splt_sz1);
    new_n->data =
        set_params(new_n->data, get_type(current->below->data), splt_sz2);

    return current;
}

// Insert a node to the list.
struct Node_s *insert(struct Node_s *root, void *data) {
    if (!error_check(root, "Error insert(): root null\n")) {
        return NULL;
    }
    if (!error_check(data, "Error insert(): data null\n")) {
        return NULL;
    }

    // Iterate through the sentinels until we
    // find the appropriate data type
    // If we reach back to the head the loop ends
    struct Node_s *current = root;
    do {
        if (get_type(current->below->data) == get_type(data)) break;

        current = current->next;
    } while (get_type(current->below->data) != HEAD_N);

    // Get a pointer to the sentinel at the start of our range to insert
    current = find_range(current, data);
    if (!error_check(current, "Error insert(): find_range() failed.\n")) {
        return NULL;
    }
    // Handle if the range is full
    if (get_size(current->below->data) > NODE_COUNT) {
        current = insert_marker(current);
        if (!error_check(current, "Error insert(): insert_marker() failed\n")) {
            return NULL;
        }
        // insert_marker() returns a pointer to the new sentinel for the marker
        // Since the range split we need to move back on sentinel then check
        // if it's in the first or second section of the split
        current = current->prev;
        current = find_range(current, data);
        if (!error_check(current, "Error insert(): find_range() failed\n")) {
            return NULL;
        }
    }
    // curr_data_n is our pointer to our marker node
    struct Node *curr_data_n = current->below;

    switch (get_type(curr_data_n->data)) {  // Should be at the marker
        case STR_D:

            // Loop until a marker is found and new node string > current string
            // Marker nodes will have an *above linked to a sentinel above. As
            // the list grows and sentinels are added we will know we're at the
            // end of a range.
            while (curr_data_n->next->above == NULL &&
                   strcmp((char *)clear_params(data),
                          (char *)clear_params(curr_data_n->next->data)) > 0) {
                curr_data_n = curr_data_n->next;
            }
            break;

        case INT_D:
            // Basically same as above except ints
            while (curr_data_n->next->above == NULL &&
                   *((int *)clear_params(data)) >
                       *((int *)clear_params(curr_data_n->next->data))) {
                curr_data_n = curr_data_n->next;
            }
            break;

        default:
            fprintf(stderr, "Error insert(): Invalid type.\n");
            return NULL;
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
    if (root == NULL || root->type != HEAD_N) {
        fprintf(stderr, "Error print_list() not a valid head\n");
        return;
    }
    struct Node *current = root->next;
    while (current) {
        if (current->data == NULL) {
            fprintf(stderr, "Error: data == NULL");
            return;
        }
        uint8_t type = get_type(current->data);
        uint16_t size = get_size(current->data);

        switch (type) {
            case STR_D:
                printf("str: %.*s\n", size,
                       (char *)clear_params(current->data));
                break;

            case INT_D:
                printf("int: %i\n", *(int *)clear_params(current->data));
                break;
            default:
                fprintf(stderr, "Invalid Type\n");
                break;
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
