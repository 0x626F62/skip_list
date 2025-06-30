#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tagged_ptr.h"

// Type Definitions
#define HEAD_N 1      // Mark the "head/tail" (it's circular)
#define STR_MARKER 2  // Mark the start of str section
#define INT_MARKER 3  // Mark the start of int section
#define STR_D 4       // string data node
#define INT_D 5       // int data node

struct Node_s {
    struct Node_s *next;
    struct Node_s *prev;
    struct Node *below;  // Tagged type/node_count
};

struct Node {
    void *data;  // Tagged type/size
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
    head_n->next = str_n;
    head_n->prev = int_n;

    str_n->next = int_n;
    str_n->prev = head_n;

    int_n->next = head_n;
    int_n->prev = str_n;

    // set_params()
    head_s->below = set_params(head_s->below, HEAD_N, 0);
    str_s->below = set_params(str_s->below, STR_MARKER, 0);
    int_s->below = set_params(int_s->below, INT_MARKER, 0);

    head_n->data = set_params(head_n->data, HEAD_N, 0);
    str_n->data = set_params(str_n->data, STR_MARKER, 0);
    int_n->data = set_params(int_n->data, INT_MARKER, 0);

    if (get_type(head_s->below) != HEAD_N || get_type(head_n->data) != HEAD_N ||
        get_type(str_s->below) != STR_MARKER ||
        get_type(str_n->data) != STR_MARKER ||
        get_type(int_s->below) != INT_MARKER ||
        get_type(int_n->data) != INT_MARKER) {
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

// Used to traverse the list of sentinels and find the closest to insert point
struct Node_s *find_range(struct Node_s **root) { return *root; }

// Insert a node to the list.
struct Node_s *insert(struct Node_s **root, void *data) {
    if (!error_check((*root), "Error insert(): root null\n")) {
        return NULL;
    }
    if (!error_check(data, "Error insert(): data null\n")) {
        return NULL;
    }

    uint8_t data_type = get_type(data);
    struct Node_s *current = *root;
    do {
        if (get_type(current->below) == data_type) break;

        current = current->next;
    } while (get_type(current->below) != HEAD_N);

    struct Node *new_n = NULL;
    switch (get_type(current->below)) {
        case STR_MARKER:
            if (clear_params(current->below->next->data) != NULL) {
                int res = strcmp(clear_params(data),
                                 clear_params(current->below->next->data));
            }
            break;

        case INT_MARKER:
            // handle int
            break;

        default:
            fprintf(stderr, "Error insert(): Invalid type.\n");
            return NULL;
    }
    return current;  // this might change we'll see how the logic plays out
}

// Print our list. Each data pointer has embedded meta data so handle
// accordingly
void print_list(struct Node *root) {
    if (root == NULL || root->data == NULL || get_type(root->data) != HEAD_N) {
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
            case STRING:
                printf("str: %.*s\n", size,
                       (char *)clear_params(current->data));
                break;

            case INT:
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
