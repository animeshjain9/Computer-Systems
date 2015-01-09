#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "blkdev.h"

/* example main() function that takes several disk image filenames as
 * arguments on the command line.
 * Note that you'll need three images to test recovery after a failure.
 */
int main(int argc, char **argv)
{
  struct blkdev *d1 = image_create(argv[1]);
    struct blkdev *d2 = image_create(argv[2]);

    struct blkdev *disks[] = {d1, d2};
    struct blkdev *mirror = mirror_create(disks);
    assert(mirror != NULL);
    assert(mirror->ops->num_blocks(mirror) == d1->ops->num_blocks(d1));
        
    char src[BLOCK_SIZE * 16];
    FILE *fp = fopen("/dev/urandom", "r");
    assert(fread(src, sizeof(src), 1, fp) == 1);
    fclose(fp);
    
    int i, result;
    for (i = 0; i < 128; i += 16) {
        result = mirror->ops->write(mirror, i, 16, src);
        assert(result == SUCCESS);
    }

    char dst[BLOCK_SIZE * 16];
    for (i = 0; i < 128; i += 16) {
        result = mirror->ops->read(mirror, i, 16, dst);
        assert(result == SUCCESS);
        assert(memcmp(src, dst, 16*BLOCK_SIZE) == 0);
    }
    image_fail(d1);
    for (i = 0; i < 128; i += 16) {

      result = mirror->ops->read(mirror, i, 16, dst);
      assert(result == SUCCESS);
      assert(memcmp(src, dst, 16*BLOCK_SIZE) == 0);
    }

    
    struct blkdev *d3 = image_create(argv[3]);
    assert(mirror_replace(mirror, 0, d3) == SUCCESS);
    for (i = 0; i < 128; i += 16) {
        result = mirror->ops->read(mirror, i, 16, dst);
        assert(result == SUCCESS);
        assert(memcmp(src, dst, 16*BLOCK_SIZE) == 0);
	}

    image_fail(d2);
    for (i = 0; i < 128; i += 16) {
        result = mirror->ops->read(mirror, i, 16, dst);
        assert(result == SUCCESS);
        assert(memcmp(src, dst, 16*BLOCK_SIZE) == 0);
    }


	printf("Mirroring Test: SUCCESS\n");
	return 0;
}
