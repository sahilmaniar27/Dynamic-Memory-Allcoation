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





////////////////////////////IMPLEMENTATION OF 4 REQUIRED FUNCTIONS START HERE/////////////////////////////////////




/*
get_free_node() has no input parameters. It should search through the
metadata node array for the first node with the node_state == free and
return the address of this node. If there are no free nodes then
get_free_node() should return a NULL pointer.
*/

	struct region_node *get_free_node()
	{
		struct region_node *pn;
  		pn = head->next;
  		for(int  i = 2; i < NUM_NODES; i++ )			//iterate through all the nodes
  		{
  			if(node[i].node_state==FREE)			//check for a free node
  			{
  				return &node[i];					//return the address of first available free node
  			}
  		}
  		
  		return NULL;								//return NULL if no node is found free untill we reach the dummy node
	}






/*
return_node() takes the address of a metadata node that has been unlinked
from the memory region list as its input parameter, and it simply
reinitializes the fields of the node structure:
*/

  void return_node( struct region_node * n )			// free the node by reinitializing
  {													
    n->node_state = FREE;
    n->region_state = FREE;
    n->start_address = NULL;
    n->size = 0;
    n->prev = NULL;
    n->next = NULL;
  }

/*
  alloc() takes the size for a requested memory allocation as its input
parameter. The size must be non-negative; otherwise, alloc() immediately
returns a NULL pointer. For a positive region size, alloc() searches the
memory region list in order, looking for a free memory region that is 
equal in size or larger than the requested size. If no such memory region
is found, alloc() returns a NULL pointer. If a memory region of equal size
is found, alloc() merely changes the memory region's state in the metadata
node to IN_USE and returns the starting address of the memory region. If
the size of the selected memory region is larger than the requested size,
alloc() must split the existing memory region into an in-use memory region
of the requested size and a new free memory region of whatever size is
left over. The in-use memory region always starts at the same address
as the selected memory region. A new free memory region resulting from a
split always starts at a higher address. For the new free memory region,
alloc() must get a free metadata node, link this node into the memory
region list, and set the other fields of the node to the appropriate
values. If get_free_node() returns a NULL address, alloc() should
allocate the complete memory region, even though it is larger than the
requested size.
*/
unsigned char *alloc( int size )
{

	struct region_node *pn;						//pointer used for iterating through all nodes	    
	struct region_node *pn_next;				//pointer nest to the iterating pointer
  	pn = head->next;

  	if(size<=0)									//check for positive size, if negative return NULL pointer
  	{
  		return NULL;
  	}
  	while( pn != head )							//iterate through all the nodes to find a IN_USE node with FREE memory
  	{
  		if(pn->region_state==FREE)				//checking for a free memory region
  		{

  			if(pn->size==size)					//if free block of required size is available,return address of the block
  			{									//and change its to state to IN_USE
  				pn->region_state=IN_USE;
  				return pn->start_address;
  			}
  			else if(pn->size < size)			//If no free block of required size is available return NULL
  			{
  				return NULL;
  			}
  			else if(pn->size > size)			//If free block of size greater than required is available,partition the block
  			{
  				pn_next = get_free_node();
  				if(pn_next==NULL)				//if no free nodes are available,entire free memory region is allocated
  				{
  					pn->region_state=IN_USE;
  					return pn->start_address;
  				}
  				else
  				{

  					pn_next->node_state=IN_USE;		//initializing the remaining memory with appropriate parameters
  					pn_next->region_state=FREE;
  					pn_next->start_address = (pn->start_address + size);
  					pn_next->size = (pn->size - size);
  					pn_next->prev = pn;
  					pn_next->next = pn->next;

  					pn->region_state=IN_USE;		//allocating block of only required size
  					pn->size=size;
  					pn->next=pn_next;

  					return pn->start_address;
  				}

  			}

  		}
  		if(pn->region_state==IN_USE)				//go to the next node if current node is IN_USE
  		{
  			pn = pn->next;
  			//pn++;
  		}
    	
  	}

  	return NULL;									//return Null pointer if no free block is available

}


/*release() takes the address of an in-use memory region as its input
parameter and passes back a return code of 0, 1, or 2, which indicates
the outcome of the call:
  0 - the call to release() was successful
  1 - the input parameter pointed to an existing FREE region
  2 - the input parameter was either NULL or an address that does not
      correspond to the staring address of an existing memory region

release() must also coalesce a newly-free memory region into a larger
free memory region whenever the predecessor and/or successor memory
region is (are) free. The following diagram shows the four possible
cases. "rn" is a pointer to the metadata node in the memory region list
that corresponds to the memory region being released. "pn" is a pointer
to the previous metadata node in the memory region list, and "nn" is a
pointer to the next metadata node in the memory region list. None, one,
or two of the metadata nodes may be returned, depending on how much
coalescing can be done.


        case 1     case 2     case 3     case 4
       +------+   +------+   +------+   +------+
  pn-> | FREE |   | FREE |   |IN_USE|   |IN_USE|
       +------+   +------+   +------+   +------+

       +------+   +------+   +------+   +------+
  rn-> | FREE |   | FREE |   | FREE |   | FREE |
       +------+   +------+   +------+   +------+

       +------+   +------+   +------+   +------+
  nn-> | FREE |   |IN_USE|   | FREE |   |IN_USE|
       +------+   +------+   +------+   +------+

       coalesce   coalesce   coalesce   all nodes
        all 3      top 2     bottom 2   remain in
        nodes      nodes      nodes     the list

       keep pn    keep pn    keep rn
       as FREE    as FREE    as FREE
       node in    node in    node in
        region     region     region
         list       list       list

       return rn  return rn  return nn
        and nn
*/
	int release( unsigned char *ptr )
	{
		struct region_node *pn,*pn_next,*pn_prev,*pn_next_next;		//pn - current iterating pointer,pn_next - next node,pn_next_next - next to next node,pn_prev - previous node pointer 
  																	//pn_prev similar to pn , rn similar to pn and nn similar to pn_next.
  		pn = head->next;	
  		while( pn != head )
  		{
  			if(pn->start_address==ptr)
  			{
  				if(pn->region_state==FREE)							//ptr points to already free region
  				{
  					return 1;
  				}
  				pn_next=pn->next;
  				pn_prev=pn->prev;
  				pn_next_next = pn_next->next;

  				//return_node(pn);

  				if(pn_next->region_state==IN_USE && pn_prev->region_state==IN_USE)		//case 4
  				{
  					pn->region_state=FREE;												//free the region
  					return 0;
  				}
  				else if(pn_next->region_state==FREE && pn_prev->region_state==FREE)		//case 1
  				{
  					pn_prev->size = pn_next->size + pn->size + pn_prev->size;			//change size of free region available
  					pn_prev->next = pn_next_next;
  					pn_next_next->prev = pn_prev;
  					return_node(pn_next);												//free node pointed by pn_next
  					return_node(pn);													//free node pointed by pn
  					return 0;
  				}
  				else if(pn_next->region_state==FREE && pn_prev->region_state==IN_USE)	//case 3
  				{
  					pn->size = pn_next->size + pn->size;								//change size of free region available
  					pn->next = pn_next_next;											
  					pn_next_next->prev = pn;
  					pn->region_state=FREE;												//free the specified region
  					return_node(pn_next);												//free node pointed by pn_next
  					return 0;
  				}
  				else if(pn_next->region_state==IN_USE && pn_prev->region_state==FREE)	//case 2
  				{
  					pn_prev->size = pn->size + pn_prev->size;							//change size of free region available
  					pn_prev->next = pn_next;
  					pn_next->prev = pn_prev;
  					return_node(pn);													//free node pointed by pn
  					return 0;
  				}
  			}
  			else if(pn->start_address!=ptr)									//iterate to next node if ptr points to some other memory region
  			{
  				pn = pn->next;
  			}
    		
	  	}

	  	return 2;							//no memory region corresponds to ptr and we reached back to the dummy node


	}




int main(){
  int rc;
  unsigned char *ptr[8];

  init_node_pool();

  print_regions();

/* general test */
  ptr[0] = alloc( 200 );
  printf("alloc 200\n");
  printf("%p:\n",ptr[0]);
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