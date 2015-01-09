#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "blkdev.h"

int main(int argc, char **argv)
{
    /* This code reads arguments <stripesize> d1 d2 d3 ...
     * Note that you don't need any extra images, as there's no way to
     * repair a striped volume after failure.
     */ 
    struct blkdev *disks[10];
    int i, ndisks, stripesize = atoi(argv[1]), result;
    extern int image_devs_open;
    for (i = 2, ndisks = 0; i < argc; i++)
        disks[ndisks++] = image_create(argv[i]);
    /* and creates a striped volume with the specified stripe size
     */
    struct blkdev *striped = striped_create(ndisks, disks, stripesize);
    assert(striped != NULL);

    /* your tests here */

    /* from pdf:
       - reports the correct size
       - reads data from the right disks and locations. (prepare disks
         with known data at various locations and make sure you can read it back) 
       - overwrites the correct locations. i.e. write to your prepared disks
         and check the results (using something other than your stripe
         code) to check that the right sections got modified. 
       - fail a disk and verify that the volume fails.
       - large (> 1 stripe set), small, unaligned reads and writes
         (i.e. starting, ending in the middle of a stripe), as well as small
         writes wrapping around the end of a stripe. 
       - Passes all your other tests with different stripe sizes (e.g. 2,
         4, 7, and 32 sectors) and different numbers of disks.  
     */
 
	//printf("Size of disks: %d\n", sizeof(struct blkdev *));
	char src[BLOCK_SIZE * stripesize * ndisks];
	char dst[BLOCK_SIZE * stripesize * ndisks];
	for (i = 0; i < ndisks; i++)
	{
		memset(src + (i * (BLOCK_SIZE * stripesize)), 65 + i, (BLOCK_SIZE * stripesize));
	}
      	printf("Num of blocks: %d\n", striped->ops->num_blocks(striped));  

	// reads data from the right disks and locations. (prepare disks
    // with known data at various locations and make sure you can read it back) 
	// Case1: Entire Stripe
	result = striped->ops->write(striped, 0, stripesize * ndisks, src);
	assert(result == SUCCESS);
	result = striped->ops->read(striped, 0, stripesize * ndisks, dst);
	assert(result == SUCCESS);
	assert(memcmp(src, dst, BLOCK_SIZE * stripesize * ndisks) == 0);
	// End of case1
	//overwrites the correct locations. i.e. write to your prepared disks
    //and check the results (using something other than your stripe
    //code) to check that the right sections got modified. 
	result = striped->ops->write(striped, (stripesize-1), (stripesize+1), (src + BLOCK_SIZE*(stripesize - 1)));
	assert(result == SUCCESS);
	char dst_disks[BLOCK_SIZE * (stripesize+1)];
	result = disks[0]->ops->read(disks[0], (stripesize-1), 1, dst_disks);
	assert(result == SUCCESS);
	result = disks[1]->ops->read(disks[1], 0, stripesize, dst_disks+BLOCK_SIZE);
	assert(result == SUCCESS);
	assert(memcmp(src + BLOCK_SIZE*(stripesize - 1), dst_disks, BLOCK_SIZE * (stripesize+1)) == 0);

	//large (> 1 stripe set), small, unaligned reads and writes
    //(i.e. starting, ending in the middle of a stripe), as well as small
    //writes wrapping around the end of a stripe. 	
	
	// small writes wrapping around the end of a stripe
	result = striped->ops->write(striped, (stripesize-1), stripesize * ndisks, src);
	assert(result == SUCCESS);
	result = striped->ops->read(striped, (stripesize-1), stripesize * ndisks, dst);
	assert(result == SUCCESS);
	assert(memcmp(src, dst, BLOCK_SIZE * stripesize * ndisks) == 0);

	// small writes wrapping around the end of a stripe
	result = striped->ops->write(striped, (stripesize+1), stripesize * ndisks, src);
	assert(result == SUCCESS);
	result = striped->ops->read(striped, (stripesize+1), stripesize * ndisks, dst);
	assert(result == SUCCESS);
	assert(memcmp(src, dst, BLOCK_SIZE * stripesize * ndisks) == 0);

	//starting, ending in the middle of a stripe
	result = striped->ops->write(striped, (stripesize-1), (stripesize+1), (src + BLOCK_SIZE*(stripesize - 1)));
	assert(result == SUCCESS);
	result = disks[0]->ops->read(disks[0], (stripesize-1), 1, dst_disks);
	assert(result == SUCCESS);
	result = disks[1]->ops->read(disks[1], 0, stripesize, dst_disks+BLOCK_SIZE);
	assert(result == SUCCESS);
	assert(memcmp(src + BLOCK_SIZE*(stripesize - 1), dst_disks, BLOCK_SIZE * (stripesize+1)) == 0);

	//large (> 1 stripe set)
	char srcl[50 * BLOCK_SIZE * stripesize * ndisks];
	char dstl[50 * BLOCK_SIZE * stripesize * ndisks];
	for (i = 0; i < 50*ndisks; i++)
	{
		memset(srcl + (i * (BLOCK_SIZE * stripesize)), 65 + i, (BLOCK_SIZE * stripesize));
	}  
	// reads data from the right disks and locations. (prepare disks
    // with known data at various locations and make sure you can read it back) 
	// Entire Stripe
	result = striped->ops->write(striped, 0, 50 * stripesize * ndisks, srcl);
	assert(result == SUCCESS);
	result = striped->ops->read(striped, 0, 50 * stripesize * ndisks, dstl);
	assert(result == SUCCESS);
	assert(memcmp(srcl, dstl, BLOCK_SIZE * stripesize * ndisks) == 0);
		

	//fail a disk and verify that the volume fails.
	// check for write failure
	image_fail(disks[ndisks-2]);
	result = striped->ops->write(striped, stripesize * (ndisks-2), stripesize, src);
	assert(result == E_UNAVAIL);
	// check for read failure
	result = striped->ops->read(striped, stripesize * (ndisks-2), stripesize, src);
	assert(result == E_UNAVAIL);
	//check if other disks are working
	result = striped->ops->write(striped, 0, stripesize*(ndisks-2), src);
	assert(result == SUCCESS);
	result = striped->ops->write(striped, stripesize*(ndisks-1), stripesize, src + BLOCK_SIZE*stripesize*(ndisks-2));
	assert(result == SUCCESS);
	result = striped->ops->read(striped, 0, stripesize*(ndisks-2), dst);
	assert(result == SUCCESS);
	result = striped->ops->read(striped, stripesize*(ndisks-1), stripesize, dst + BLOCK_SIZE*stripesize*(ndisks-2));
	assert(result == SUCCESS);

	//printf("%d\n", image_devs_open);
	striped->ops->close(striped);
	//printf("%d\n", image_devs_open);
	assert(image_devs_open == 0);
	//result = striped->ops->read(striped, 0, stripesize, dst);
        //assert(result == E_UNAVAIL);

    printf("Striping Test: SUCCESS\n");
    return 0;
}
