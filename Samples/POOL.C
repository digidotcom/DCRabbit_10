/*******************************************************************************
        Samples\pool.c
        Z-World, 2003

        This sample program demonstrates use of POOL.LIB, for dynamic
        memory allocation.
*******************************************************************************/
#class auto

// Define debugging
#define POOL_VERBOSE
#define POOL_DEBUG

// Include the pool library
#use "pool.lib"

// Define some pools.  One root, one xmem.
Pool_t  root_pool;
Pool_t  xmem_pool;

// Define element sizes
#define ROOT_POOL_SIZE		20
#define XMEM_POOL_SIZE		120

// Deifne number of elements
#define ROOT_ELS		4
#define XMEM_ELS		8
#define ROOT_ELS_XTRA	2
#define XMEM_ELS_XTRA	3

// Define data storage for root pool.
char root_data[ROOT_POOL_SIZE * ROOT_ELS];
char root_data_xtra[ROOT_POOL_SIZE * ROOT_ELS_XTRA];

// Define pointer to xmem pool data (will be xalloc'd)
long xmem_data;
long xmem_data_xtra;


int main()
{
	void * r;
   long   x;
	void * r0;
   long   x0;
   int i;
   int do_xtra;

   do_xtra = 1;

	// Allocate the xmem pool data area
   xmem_data = xalloc(XMEM_POOL_SIZE * XMEM_ELS);
   xmem_data_xtra = xalloc(XMEM_POOL_SIZE * XMEM_ELS_XTRA);

   // Init the pools
   pool_init(&root_pool, root_data, ROOT_ELS, ROOT_POOL_SIZE);
   pool_xinit(&xmem_pool, xmem_data, XMEM_ELS, XMEM_POOL_SIZE);

   // Turn linking on, so we can easily iterate through allocated elements
   pool_link(&root_pool, 1);
   pool_link(&xmem_pool, 1);

_again:

   printf("Available in root pool: %u\n", pavail(&root_pool));
   printf("Available in xmem pool: %u\n", pavail(&xmem_pool));
   printf("Elements in root pool: %u\n", pnel(&root_pool));
   printf("Elements in xmem pool: %u\n", pnel(&xmem_pool));

   for (i = 0; i < 10; ++i) {
		r = palloc(&root_pool);
      if (r)
      	printf("Got root element at %04X\n", r);
      if (!i)
      	r0 = r;
		x = pxalloc(&xmem_pool);
      if (x)
      	printf("Got xmem element at %08lX\n", x);
      if (!i)
      	x0 = x;
   }

   printf("Available in root pool: %u\n", pavail(&root_pool));
   printf("Available in xmem pool: %u\n", pavail(&xmem_pool));

   // Now free the first elements we remembered
	printf("Freeing up %04X and %08lX...\n", r0, x0);
   pfree(&root_pool, r0);
   pxfree(&xmem_pool, x0);

   printf("Available in root pool: %u\n", pavail(&root_pool));
   printf("Available in xmem pool: %u\n", pavail(&xmem_pool));

   printf("Maximum used in root pool: %u\n", phwm(&root_pool));
   printf("Maximum used in xmem pool: %u\n", phwm(&xmem_pool));

   printf("Deleting everything...\n");
   for (r = pfirst(&root_pool); r; r = r0) {
   	r0 = pnext(&root_pool, r);
      pfree(&root_pool, r);
   }

   for (x = pxfirst(&xmem_pool); x; x = x0) {
   	x0 = pxnext(&xmem_pool, x);
      pxfree(&xmem_pool, x);
   }

   if (do_xtra) {
   	do_xtra = 0;
      printf("Doing it all again, with appended data areas...\n");
      pool_append(&root_pool, root_data_xtra, ROOT_ELS_XTRA);
   	pool_xappend(&xmem_pool, xmem_data_xtra, XMEM_ELS_XTRA);
      goto _again;
   }

	return 0;
}