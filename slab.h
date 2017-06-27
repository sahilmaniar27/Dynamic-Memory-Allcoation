#define NUM_NODES 64
#define MEM_SIZE 100000
#define FREE 0
#define IN_USE 1

struct region_node{
  int node_state, region_state;
  unsigned char *start_address;
  int size;
  struct region_node *prev, *next;
};

struct region_node *head, node[NUM_NODES];
unsigned char mem[MEM_SIZE];

struct region_node *get_free_node();
void return_node( struct region_node * n );
unsigned char *alloc( int size );
int release( unsigned char *ptr );
