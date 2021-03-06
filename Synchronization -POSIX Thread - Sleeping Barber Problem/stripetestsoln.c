#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "blkdev.h"

int main(int argc, char **argv)
{
    struct blkdev *disks[10];
    int i, ndisks, stripesize = atoi(argv[1]);

    for (i = 2, ndisks = 0; i < argc; i++)
        disks[ndisks++] = image_create(argv[i]);

    struct blkdev *striped = striped_create(ndisks, disks, stripesize);
    assert(striped != NULL);

    int nblks = disks[0]->ops->num_blocks(disks[0]);
    nblks = nblks - (nblks % stripesize);
    assert(striped->ops->num_blocks(striped) == ndisks*nblks);

    int one_chunk = stripesize * BLOCK_SIZE;
    char *buf = malloc(ndisks * one_chunk);
    for (i = 0; i < ndisks; i++)
        memset(buf + i * one_chunk, 'A'+i, one_chunk);

    int result, j;
    for (i = 0; i < 16; i++) {
        result = striped->ops->write(striped, i*ndisks*stripesize,
                                     ndisks*stripesize, buf);
        assert(result == SUCCESS);
    }

    char *buf2 = malloc(ndisks * one_chunk);

    for (i = 0; i < 16; i++) {
        result = striped->ops->read(striped, i*ndisks*stripesize,
                                    ndisks*stripesize, buf2);
        assert(result == SUCCESS);
        assert(memcmp(buf, buf2, ndisks * one_chunk) == 0);
    }

    /* now we test that the data got onto the disks in the right
     * places. 
     */
    for (i = 0; i < ndisks; i++) {
        result = disks[i]->ops->read(disks[i], i*stripesize,
                                     stripesize, buf2);
        assert(result == SUCCESS);
        assert(memcmp(buf + i*one_chunk, buf2, one_chunk) == 0);
    }

    /* now we test that small writes work
     */
    for (i = 0; i < ndisks; i++)
        memset(buf + i * one_chunk, 'a'+i, one_chunk);
    
    for (i = 0; i < 8; i++) {
        for (j = 0; j < ndisks*stripesize; j ++) {
            result = striped->ops->write(striped, i*ndisks*stripesize + j, 1,
                                         buf + j*BLOCK_SIZE);
            assert(result == SUCCESS);
        }
    }

    for (i = 0; i < 8; i++) {
        result = striped->ops->read(striped, i*ndisks*stripesize,
                                    ndisks*stripesize, buf2);
        assert(result == SUCCESS);
        assert(memcmp(buf, buf2, ndisks * one_chunk) == 0);
    }

    /* finally we test that large and overlapping writes work.
     */
    char *big = malloc(5 * ndisks*one_chunk);
    for (i = 0; i < 5; i++)
        for (j = 0; j < ndisks; j++)
            memset(big + j * one_chunk + i*ndisks*one_chunk, 'f'+i, one_chunk);

    int offset = ndisks*stripesize / 2;
    result = striped->ops->write(striped, offset, 5*ndisks*stripesize, big);
    assert(result == SUCCESS);

    char *big2 = malloc(5 * ndisks*one_chunk);
    result = striped->ops->read(striped, offset, 5*ndisks*stripesize, big2);
    assert(result == SUCCESS);
    assert(memcmp(big, big2, 5 * ndisks * one_chunk) == 0);

    /* and check we didn't muck up any previously-written data
     */
    result = striped->ops->read(striped, 0, offset, buf2);
    assert(result == SUCCESS);
    assert(memcmp(buf, buf2, offset*BLOCK_SIZE) == 0);

    result = striped->ops->read(striped, 5*ndisks*stripesize + offset, offset, buf2);
    assert(result == SUCCESS);
    assert(memcmp(buf + offset*BLOCK_SIZE, buf2, offset*BLOCK_SIZE) == 0);
    
    printf("Striping Test: SUCCESS\n");
    return 0;
}
