/*****************************************************************************\
	dma_memcpy.c
 	Rabbit Semiconductor, 2006

	This program is used with Rabbit 4000 series microprocessors and protoboards.

   This sample program demonstrates the DMA used as a memcpy or strncpy.
	It moves 256 bytes of data from one memory location to another and prints it
	out.
	This also demonstrates the use of the DMA stop-match feature by calling
	the DMAmatchSetup function.

\*****************************************************************************/

// Uncomment the following line to debug DMA API functions.
//#define DMA_DEBUG

#ximport "data\paragraph.txt"	xmem_src
unsigned int file_size, length;
dma_addr_t src, dest;

int main()
{
	int err;
   dma_chan_t handle;
   char str1[256], str2[256], str3[256];

   src = xmem_src + sizeof(long);
   xmem2root((void *)&file_size, xmem_src, 4);
   dest = xalloc(file_size);

   // clear initial strings
   memset(str1, 0, 256);
   memset(str2, 0, 256);
   memset(str3, 0, 256);

   // Set parameters to send 64 bytes at a time.
   err = DMAsetParameters(0, 0, DMA_IDP_FIXED, 64, 25);
   if(err){
   	printf("Error: DMAsetParameters.\n");
      exit(err);
   }

   xmem2root(str1, src, file_size);
   printf("Original: \n%s\n", str1);

   // Allocate a channel
   handle = DMAalloc(DMA_CHANNEL_ANY, 0);
   if(handle == DMA_CHANNEL_NONE){
   	printf("Error: DMAalloc.\n");
      exit(EINVAL);
   }

   // transfer data
   err = DMAmem2mem(handle, dest, src, file_size, 0);
   if(err){
   	printf("Error: DMAmem2mem.\n");
      exit(err);
   }

   while(!DMAcompleted(handle, NULL))
   	;

   xmem2root(str2, dest, file_size);
   printf("\nCopied: \n%s\n", str2);

   if(strcmp(str1, str2) != 0){
   	printf("Error: strings do not match.\n");
      exit(EINVAL);
   }

	printf("\nNow for just the first line:\n");

   // clear destination
   root2xmem(dest, str3, file_size);
   DMAmatchSetup(handle, 0xFF, '\n');

   // transfer data
   err = DMAmem2mem(handle, dest, src, file_size, DMA_F_STOP_MATCH);
   if(err){
   	printf("Error: DMAmem2mem.\n");
      exit(err);
   }

   // loop until done
   while(!DMAcompleted(handle, NULL))
   	;

   xmem2root(str3, dest, file_size);
   printf("%s\n", str3);

   // free up the channel
   err = DMAunalloc(handle);
   if(err){
   	printf("Error: DMAunalloc.\n");
      exit(err);
   }

   printf("Test successful.\n");
}



