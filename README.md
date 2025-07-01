# skip_list
Test implementation of a circular skip list with tagged pointers to track type and size
Each type has it's own section marked by a sentinel/data node pair
nodes are inserted in order.
NODE_COUNT specifies how many data nodes should be linked before another marker node
pair is inserted. The sentinel nodes are used to skip over sections of nodes until
the appropriate range is found then perform the required function i.e.
insert() search() delete()
at no time (unless it's a function designed to) should the amount of data nodes 
traversed exceed NODE_COUNT

TODO:
finish print_list(), free_list()
add search() and delete() 
create a test main() implementation.

I would like to devise a way to remove hardcoded type detection logic
from insert() (i.e. switch(type { INT_D: ...}))
make it scalable and easier to handle different types.

Declutter insert() primarily

