#ifndef LIST
#define LIST

#include <stdint.h>

// HEAD_N explicitly checked for
#define TYPE_COUNT 2

#define STR_D 0
#define INT_D 1
#define HEAD_N 2

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

uint8_t error_check(void *ptr, const char *msg);
struct Node *new_node();
struct Node_s *new_sentinel();
struct Node_s *init_list();
struct Node_s *find_type(struct Node_s *root, void *data);
struct Node_s *find_range(struct Node_s *current, void *data);
struct Node_s *insert_marker(struct Node_s *current);
struct Node_s *insert(struct Node_s *root, void *data);
void print_list(struct Node_s *root);
void free_list(struct Node_s **root);

#endif
