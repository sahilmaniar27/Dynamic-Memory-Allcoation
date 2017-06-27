
#include <stdio.h>
#include "pfl.h"

void init_node_pool(){
  int i;
  node[0].node_state = IN_USE;          // node 0 is dummy node
  node[0].region_state = IN_USE;        // always in-use to simplify
  node[0].start_address = NULL;         //   the coalescing logic
  node[0].size = 0;
  node[0].prev = &node[1];
  node[0].next = &node[1];
  node[1].node_state = IN_USE;          // node 1 is initial free region
  node[1].region_state = FREE;
  node[1].start_address = mem;
  node[1].size = MEM_SIZE;
  node[1].prev = &node[0];
  node[1].next = &node[0];
  for( i = 2; i < NUM_NODES; i++ ){     // other nodes are available for
    node[i].node_state = FREE;          //   splitting a free region into
    node[i].region_state = FREE;        //   an allocated region and a
    node[i].start_address = NULL;       //   remaining free region
    node[i].size = 0;
    node[i].prev = NULL;
    node[i].next = NULL;
  }
  head = &node[0];
}

void print_regions(){
  struct region_node *pn;
  printf("\nregion list - addr NS RS size\n");
  pn = head->next;
  while( pn != head ){
    printf("%p: %d %d %d\n",pn->start_address,pn->node_state,
      pn->region_state,pn->size);
    pn = pn->next;
  }
  printf("\n");
}

int main(){
  int rc;
  unsigned char *ptr[8];

  init_node_pool();

  print_regions();

/* general test */
  ptr[0] = alloc( 200 );
  printf("alloc 200\n");
  print_regions();
  rc = release( ptr[0] );
  printf( "alloc 200, release, expect rc of 0, rc = %d\n", rc );
  print_regions();
  rc = release( ptr[0] );
  printf( "re-release, expect rc of 1, rc = %d\n", rc );
  print_regions();

/* edge case tests */
  ptr[0] = alloc( -1 );
  printf( "alloc -1, expect NULL, ptr = %p\n", ptr[0] );
  ptr[0] = alloc( 0 );
  printf( "alloc 0, expect NULL, ptr = %p\n", ptr[0] );
  rc = release( ptr[0] );
  printf( "release, expect rc of 2, rc = %d\n", rc );
  print_regions();

  ptr[0] = alloc( 1 );
  printf("alloc 1\n");
  print_regions();
  rc = release( ptr[0] );
  printf( "alloc 1, release, expect rc of 0, rc = %d\n", rc );
  print_regions();

  ptr[0] = alloc( MEM_SIZE );
  printf("alloc MEM_SIZE\n");
  print_regions();
  rc = release( ptr[0] );
  printf( "alloc MEM_SIZE, release, expect rc of 0, rc = %d\n", rc );
  print_regions();

  ptr[0] = alloc( MEM_SIZE + 1 );
  rc = release( ptr[0] );
  printf( "alloc MEM_SIZE+1, release, expect rc of 2, rc = %d\n", rc );
  print_regions();

  ptr[0] = alloc( MEM_SIZE );
  ptr[1] = alloc( 1 );
  printf( "alloc MEM_SIZE then 1, expect NULL, ptr = %p\n", ptr[1] );
  rc = release( ptr[0] );
  print_regions();



/* coalescing tests */

  ptr[0] = alloc( 256 );
  ptr[1] = alloc( 256 );
  ptr[2] = alloc( 256 );
  ptr[3] = alloc( 256 );
  ptr[4] = alloc( 256 );

  print_regions();
  release( ptr[2] );
  print_regions();

  ptr[2] = alloc( 256 );
  print_regions();
  release( ptr[1] );
  release( ptr[2] );
  print_regions();

  ptr[1] = alloc( 256 );
  ptr[2] = alloc( 256 );
  print_regions();
  release( ptr[3] );
  release( ptr[2] );
  print_regions();

  ptr[2] = alloc( 256 );
  ptr[3] = alloc( 256 );
  print_regions();
  release( ptr[1] );
  release( ptr[3] );
  release( ptr[2] );
  print_regions();

  return 0;
}