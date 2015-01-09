#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "blkdev.h"

int main(int argc, char **argv)
{
  /* here's some code you might use to run different tests from your
   * test script 
   */
  //int testnum;
  //testnum = atoi(argv[1]);
    
  /* now create your image devices like before, remembering that
   * you need an extra disk if you're testing failure recovery
   */

  //  switch (testnum) {
  //case 1:
  /* do one set of tests */
  //break;
  //case 2:
  /* do a different set of tests */
  //break;
  //default:
  //printf("unrecognized test number: %d\n", testnum);
  //}
   
  struct blkdev *disks[5];
  int i, ndisks, stripesize = atoi(argv[1]), result;
  //extern int image_devs_open;
  for (i = 2, ndisks = 0; i < argc; i++)
    disks[ndisks++] = image_create(argv[i]);
  /* and creates a striped volume with the specified stripe size
   */
  struct blkdev *raid4 = raid4_create(ndisks, disks, stripesize);
  assert(raid4 != NULL);


  char src[BLOCK_SIZE * stripesize * ndisks];
  char dst[BLOCK_SIZE * stripesize * ndisks];

  for (i = 0; i < ndisks-1; i++)
    {
      memset(src + (i * (BLOCK_SIZE * stripesize)), 65 + i, (BLOCK_SIZE * stripesize));
    }
  printf("Num of blocks: %d\n", raid4->ops->num_blocks(raid4));


  // Complete raid4 read test
    
  result = raid4->ops->write(raid4, 0, stripesize * (ndisks-1), src);
  assert(result == SUCCESS);
  result = raid4->ops->read(raid4, 0, stripesize * (ndisks-1), dst);
  assert(result == SUCCESS);
  assert(memcmp(src, dst, BLOCK_SIZE * stripesize * (ndisks-1)) == 0);
  // End of case1


  //overwrites the correct locations. i.e. write to your prepared disks
  //and check the results (using something other than your stripe
  //code) to check that the right sections got modified. 
  result = raid4->ops->write(raid4, (stripesize-1), (stripesize+1), (src + BLOCK_SIZE*(stripesize - 1)));
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
  result = raid4->ops->write(raid4, (stripesize-1), stripesize * ndisks, src);
  assert(result == SUCCESS);
  result = raid4->ops->read(raid4, (stripesize-1), stripesize * ndisks, dst);
  assert(result == SUCCESS);
  assert(memcmp(src, dst, BLOCK_SIZE * stripesize * ndisks) == 0);

  // small writes wrapping around the end of a stripe
  result = raid4->ops->write(raid4, (stripesize+1), stripesize * ndisks, src);
  assert(result == SUCCESS);
  result = raid4->ops->read(raid4, (stripesize+1), stripesize * ndisks, dst);
  assert(result == SUCCESS);
  assert(memcmp(src, dst, BLOCK_SIZE * stripesize * ndisks) == 0);

  //starting, ending in the middle of a stripe
  result = raid4->ops->write(raid4, (stripesize-1), (stripesize+1), (src + BLOCK_SIZE*(stripesize - 1)));
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
  result = raid4->ops->write(raid4, 0, 50 * stripesize * ndisks, srcl);
  assert(result == SUCCESS);
  result = raid4->ops->read(raid4, 0, 50 * stripesize * ndisks, dstl);
  assert(result == SUCCESS);
  assert(memcmp(srcl, dstl, BLOCK_SIZE * stripesize * ndisks) == 0);
		
	
	
  image_fail(disks[ndisks-2]);
  result = raid4->ops->write(raid4, stripesize * (ndisks-2), stripesize, src);
  assert(result == SUCCESS);
  // check for read failure
  result = raid4->ops->read(raid4, stripesize * (ndisks-2), stripesize, src);
  assert(result == SUCCESS);
  //printf("OPEN:%d\n",image_devs_open);


  image_fail(disks[ndisks-1]);
	
  result = raid4->ops->write(raid4, stripesize * (ndisks-2), stripesize, src);
  assert(result == E_UNAVAIL);
  // check for read failure
  result = raid4->ops->read(raid4, stripesize * (ndisks-2), stripesize, src);
  assert(result == E_UNAVAIL);
  //printf("OPEN:%d\n",image_devs_open);


  /* from the pdf: "RAID 4 tests are basically the same as the
   * stripe tests, combined with the failure and recovery tests from
   * mirroring"
   */
    
  printf("RAID4 Test: SUCCESS\n");
  return 0;
}
