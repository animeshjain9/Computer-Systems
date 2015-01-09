/*
 * file:        homework.c
 * description: skeleton code for CS 5600 Homework 3
 *
 * Peter Desnoyers, Northeastern Computer Science, 2011
 * $Id: homework.c 410 2011-11-07 18:42:45Z pjd $
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "blkdev.h"

/********** MIRRORING ***************/

/* example state for mirror device. See mirror_create for how to
 * initialize a struct blkdev with this.
 */
struct mirror_dev {
    struct blkdev *disks[2];    /* flag bad disk by setting to NULL */
    int nblks;
};
    
static int mirror_num_blocks(struct blkdev *dev)
{
    struct mirror_dev *mdev = dev->private;
    return mdev->nblks;
}

/* read from one of the sides of the mirror. (if one side has failed,
 * it had better be the other one...) If both sides have failed,
 * return an error.
 * Note that a read operation may return an error to indicate that the
 * underlying device has failed, in which case you should close the
 * device and flag it (e.g. as a null pointer) so you won't try to use
 * it again. 
 */
static int mirror_read(struct blkdev * dev, int first_blk,
                       int num_blks, void *buf)
{
  int flag_disk1, flag_disk2;
  struct mirror_dev *mdev = dev->private;

  if(first_blk<0 || first_blk+num_blks > mdev->nblks)
    return E_BADADDR;
  flag_disk1 = 1;
  flag_disk2 = 1;
  if(mdev->disks[0]!= NULL)
    {
      
      flag_disk1 =  mdev->disks[0]->ops->read(mdev->disks[0], first_blk, num_blks, buf);
      //printf("value of flag_disk1 is %d\n",flag_disk1);
      if (flag_disk1 == E_UNAVAIL)
	{
	  mdev->disks[0]->ops->close(mdev->disks[0]);
	  mdev->disks[0] = NULL;
	}
    }
  
  if(mdev->disks[1]!= NULL)
    {
      
      flag_disk2 =  mdev->disks[1]->ops->read(mdev->disks[1], first_blk, num_blks, buf);
     //printf("value of flag_disk1 is %d\n",flag_disk1);
      if (flag_disk2 == E_UNAVAIL)
	{
	  mdev->disks[1]->ops->close(mdev->disks[1]);
	  mdev->disks[1] = NULL;
	}
    }
 if(flag_disk1==0 || flag_disk2==0)
    {
      return SUCCESS;
    }
  else return E_UNAVAIL;
}

/* write to both sides of the mirror, or the remaining side if one has
 * failed. If both sides have failed, return an error.
 * Note that a write operation may indicate that the underlying device
 * has failed, in which case you should close the device and flag it
 * (e.g. as a null pointer) so you won't try to use it again.
 */
static int mirror_write(struct blkdev * dev, int first_blk,
                        int num_blks, void *buf)
{
   int flag_disk1, flag_disk2;
  struct mirror_dev *mdev = dev->private;

  if(first_blk<0 || first_blk+num_blks > mdev->nblks)
    return E_BADADDR;
  
  if(mdev->disks[0]!= NULL)
    {
      
      flag_disk1 =  mdev->disks[0]->ops->write(mdev->disks[0], first_blk, num_blks, buf);
      if (flag_disk1 ==  E_UNAVAIL)
	{
	  mdev->disks[0]->ops->close(mdev->disks[0]);
	  mdev->disks[0] = NULL;
	}
    }
  
  if(mdev->disks[1]!= NULL)
    {
      
      flag_disk2 =  mdev->disks[1]->ops->write(mdev->disks[1], first_blk, num_blks, buf);
      if (flag_disk2 ==  E_UNAVAIL)
	{
	  mdev->disks[1]->ops->close(mdev->disks[1]);
	  mdev->disks[1] = NULL;
	 
	}
    
    }

  if(flag_disk1==0 || flag_disk2==0)
    {
      return SUCCESS;
    }
  else return E_UNAVAIL;
}

/* clean up, including: close any open (i.e. non-failed) devices, and
 * free any data structures you allocated in mirror_create.
 */
static void mirror_close(struct blkdev *dev)
{
  struct mirror_dev *mdev = dev->private;
  mdev->disks[0]->ops->close(mdev->disks[0]);
  mdev->disks[1]->ops->close(mdev->disks[1]);
  free(mdev);
  free(dev);
}

struct blkdev_ops mirror_ops = {
    .num_blocks = mirror_num_blocks,
    .read = mirror_read,
    .write = mirror_write,
    .close = mirror_close
};

/* create a mirrored volume from two disks. Do not write to the disks
 * in this function - you should assume that they contain identical
 * contents. 
 */
struct blkdev *mirror_create(struct blkdev *disks[2])
{
    struct blkdev *dev = malloc(sizeof(*dev));
    struct mirror_dev *mdev = malloc(sizeof(*mdev));
    if((disks[0]->ops->num_blocks(disks[0])) != (disks[1]->ops->num_blocks(disks[1])))
      {
	printf("Disks size mismatched");
	return NULL;
      }
    mdev->disks[0] = disks[0];
    mdev->disks[1] = disks[1];
    mdev->nblks = disks[1]->ops->num_blocks(disks[1]);
    
    dev->private = mdev;
    dev->ops = &mirror_ops;

    return dev;
}

/* replace failed device 'i' (0 or 1) in a mirror. Note that we assume
 * the upper layer knows which device failed. You will need to
 * replicate content from the other underlying device before returning
 * from this call.
 */
int mirror_replace(struct blkdev *volume, int i, struct blkdev *newdisk)
{
   struct mirror_dev *mdev = volume->private;
   if(mdev->nblks != newdisk->ops->num_blocks(newdisk))
   {
     return E_SIZE;
   }
   int current_num_blocks = mdev->nblks;
   char buf[current_num_blocks * BLOCK_SIZE];
   volume->ops->read(volume, 0, current_num_blocks, buf);
   newdisk->ops->write(newdisk, 0, current_num_blocks, buf);
   //   if (i == 0)
   //	i = 1; 
   //else
   //	i = 0;
   mdev->disks[i] = newdisk;
  // struct mirror_dev *mdev1 = newdisk->private;
   return SUCCESS;
}

/**********  STRIPING ***************/



struct stripe_dev{
  struct blkdev **disks;
  int nofdisks;
  int nblks;
  int unit;

    };

int stripe_num_blocks(struct blkdev *dev)
{
  struct stripe_dev *sdev = dev->private;
  return sdev->nblks;
}

/* read blocks from a striped volume. 
 * Note that a read operation may return an error to indicate that the
 * underlying device has failed, in which case you should (a) close the
 * device and (b) return an error on this and all subsequent read or
 * write operations. 
 */
static int stripe_read(struct blkdev * dev, int first_blk,
                       int num_blks, void *buf)
{
  
  if(dev == NULL || dev->private == NULL)
    {
      return  E_UNAVAIL; 
    }
  struct stripe_dev *sdev = dev->private;
  if(first_blk<0 || first_blk+num_blks > sdev->nblks)
    {
      return  E_BADADDR;
    }
  int stripeNumber, chunkNumber, diskNumber, chunkOffset, bufOffset, result_temp, result, totalChunk, temp_num, i;

  chunkOffset = first_blk%(sdev-> unit);
  chunkNumber = first_blk/(sdev-> unit);
  diskNumber = chunkNumber%(sdev->nofdisks);
  stripeNumber = chunkNumber/(sdev->nofdisks);
  bufOffset = 0;
  result_temp = SUCCESS;
  result = SUCCESS;
  // printf("check the status");
  if(sdev->disks[diskNumber] != NULL)
    {
  if (chunkOffset!=0)
    {
      if (sdev->unit - chunkOffset >= num_blks)
	{
	  temp_num = num_blks;
	  result_temp = sdev->disks[diskNumber]->ops->read(sdev->disks[diskNumber], stripeNumber * sdev->unit + chunkOffset, temp_num, buf + bufOffset);
	  if (result_temp == E_UNAVAIL)
	    {
	        sdev->disks[diskNumber]->ops->close(sdev->disks[diskNumber]);
		sdev->disks[diskNumber] = NULL;
	        result = E_UNAVAIL;
	    }
	  return result;
	}
      else
	{
	  temp_num = sdev->unit - chunkOffset;
	  result_temp = sdev->disks[diskNumber]->ops->read(sdev->disks[diskNumber], stripeNumber * sdev->unit + chunkOffset, temp_num, buf + bufOffset);
	  if (result_temp == E_UNAVAIL)
	    {
	      sdev->disks[diskNumber]->ops->close(sdev->disks[diskNumber]);
	      sdev->disks[diskNumber] = NULL;
	      result = E_UNAVAIL;
	      return result;
	    }
	  bufOffset = bufOffset + temp_num * BLOCK_SIZE; 
	  chunkNumber++;
	  chunkOffset = 0;
	  diskNumber = chunkNumber%(sdev->nofdisks);
	  stripeNumber = chunkNumber/(sdev->nofdisks); 
	}
    }

  totalChunk = (num_blks - (bufOffset/ BLOCK_SIZE))/ sdev->unit;
  for (i=0; i<totalChunk; i++)
    {
      temp_num = sdev->unit;
      result_temp = sdev->disks[diskNumber]->ops->read(sdev->disks[diskNumber], stripeNumber * sdev->unit + chunkOffset, temp_num, buf + bufOffset);
      if (result_temp == E_UNAVAIL)
	{
	  sdev->disks[diskNumber]->ops->close(sdev->disks[diskNumber]);
	  sdev->disks[diskNumber] = NULL;
	  result = E_UNAVAIL;
	  return result;
	}
      bufOffset = bufOffset + temp_num * BLOCK_SIZE;
      chunkNumber++;
      chunkOffset = 0;
      diskNumber = chunkNumber%(sdev->nofdisks);
      stripeNumber = chunkNumber/(sdev->nofdisks);
    }
  if(bufOffset/BLOCK_SIZE < num_blks)
    {
      temp_num = num_blks - bufOffset/BLOCK_SIZE;
      result_temp = sdev->disks[diskNumber]->ops->read(sdev->disks[diskNumber], stripeNumber * sdev->unit + chunkOffset, temp_num, buf + bufOffset);
      if (result_temp == E_UNAVAIL)
	{
	  sdev->disks[diskNumber]->ops->close(sdev->disks[diskNumber]);
	  sdev->disks[diskNumber] = NULL;
	  result = E_UNAVAIL;
	  return result;
	}
      
    }
  if (result == E_UNAVAIL)
    {
      // int num_of_disks = sdev->nofdisks; 
      //int disk_id=0;
      /* for(disk_id=0; disk_id<num_of_disks; disk_id++)
	{
	  sdev->disks[disk_id]->ops->close(sdev->disks[disk_id]);
	  }*/
      return result;
    }

  return result;
    }
  else
    {
      return E_UNAVAIL;
    }
}

/* write blocks to a striped volume.
 * Again if an underlying device fails you should close it and return
 * an error for this and all subsequent read or write operations.
 */
static int stripe_write(struct blkdev * dev, int first_blk,
                        int num_blks, void *buf)
{
  //printf("check the status");
  if (dev== NULL || dev->private == NULL)
    {
      return E_UNAVAIL; 
    }
  struct stripe_dev *sdev = dev->private;

 
  if(first_blk<0 || first_blk+num_blks > sdev->nblks)
    {
      return  E_BADADDR;
    }
  int stripeNumber, chunkNumber, diskNumber, chunkOffset, bufOffset, result_temp, result, totalChunk, temp_num, i;
 
  chunkOffset = first_blk%(sdev->unit);
  chunkNumber = first_blk/(sdev->unit);
  diskNumber = chunkNumber%(sdev->nofdisks);
  stripeNumber = chunkNumber/(sdev->nofdisks);
  bufOffset = 0;
  result_temp = SUCCESS;
  result = SUCCESS;
  //printf("check the status");
  if(sdev->disks[diskNumber] != NULL)
    {
  if (chunkOffset!=0)
    {
      if (sdev->unit - chunkOffset >= num_blks)
	{
	  temp_num = num_blks;
	  result_temp = sdev->disks[diskNumber]->ops->write(sdev->disks[diskNumber], stripeNumber * sdev->unit + chunkOffset, temp_num, buf + bufOffset);
	  if (result_temp == E_UNAVAIL)
	    {
	      result = E_UNAVAIL;
	      sdev->disks[diskNumber]->ops->close(sdev->disks[diskNumber]);
	      sdev->disks[diskNumber] = NULL;
	    }
	  return result;
	}
      else
	{
	  temp_num = sdev->unit - chunkOffset;
	  result_temp = sdev->disks[diskNumber]->ops->write(sdev->disks[diskNumber], stripeNumber * sdev->unit + chunkOffset, temp_num, buf + bufOffset);
	  if (result_temp == E_UNAVAIL)
	    {
	    sdev->disks[diskNumber]->ops->close(sdev->disks[diskNumber]);
	    sdev->disks[diskNumber] = NULL;
	    result = E_UNAVAIL;
	    return result;
	    }
	  bufOffset = bufOffset + temp_num * BLOCK_SIZE;
	  chunkNumber++;
	  chunkOffset = 0;
	  diskNumber = chunkNumber%(sdev->nofdisks);
	  stripeNumber = chunkNumber/(sdev->nofdisks); 
	}
    }

  totalChunk = (num_blks - (bufOffset/ BLOCK_SIZE))/ sdev->unit;
  for (i=0; i<totalChunk; i++)
    {
      temp_num = sdev->unit;
      result_temp = sdev->disks[diskNumber]->ops->write(sdev->disks[diskNumber], stripeNumber * sdev->unit + chunkOffset, temp_num, buf + bufOffset);
      if (result_temp == E_UNAVAIL)
	{
	sdev->disks[diskNumber]->ops->close(sdev->disks[diskNumber]);
	sdev->disks[diskNumber] = NULL;
	result = E_UNAVAIL;
	return result;
	}
      bufOffset = bufOffset + temp_num * BLOCK_SIZE;
      chunkNumber++;
      chunkOffset = 0;
      diskNumber = chunkNumber%(sdev->nofdisks);
      stripeNumber = chunkNumber/(sdev->nofdisks);
    }
  if(bufOffset/BLOCK_SIZE < num_blks)
    {
      temp_num = num_blks - bufOffset/BLOCK_SIZE;
      result_temp = sdev->disks[diskNumber]->ops->write(sdev->disks[diskNumber], stripeNumber * sdev->unit + chunkOffset, temp_num, buf + bufOffset);
      if (result_temp == E_UNAVAIL)
	{
	  sdev->disks[diskNumber]->ops->close(sdev->disks[diskNumber]);
	  sdev->disks[diskNumber] = NULL;
	  result = E_UNAVAIL;
	  return result;
	}
      
    }
  if (result == E_UNAVAIL)
    {
      //int num_of_disks = sdev->nofdisks; 
      // int disk_id=0;
      /* for(disk_id=0; disk_id<num_of_disks; disk_id++)
	{
	  sdev->disks[disk_id]->ops->close(sdev->disks[disk_id]);
	  }*/
      return result;
      }

  return result;
    }
  else
    {
      return E_UNAVAIL;
    }
}

/* clean up, including: close all devices and free any data structures
 * you allocated in stripe_create. 
 */
static void stripe_close(struct blkdev *dev)
{

  struct stripe_dev *sdev = dev->private;
  int num_of_disks = sdev->nofdisks; 

  int disk_id=0;

  for(disk_id=0; disk_id<num_of_disks; disk_id++)
    {
      if (sdev->disks[disk_id] != NULL)
	{
      sdev->disks[disk_id]->ops->close(sdev->disks[disk_id]);
	}
    }

  free(sdev);
  free(dev);
  // free(sdev);
}

struct blkdev_ops strip_ops = 
{
    .num_blocks = stripe_num_blocks,
    .read = stripe_read,
    .write = stripe_write,
    .close = stripe_close
};

/* create a striped volume across N disks, with a stripe size of
 * 'unit'. (i.e. if 'unit' is 4, then blocks 0..3 will be on disks[0],
 * 4..7 on disks[1], etc.)
 * Check the size of the disks to compute the final volume size, and
 * fail (return NULL) if they aren't all the same.
 * Do not write to the disks in this function.
 */
struct blkdev *striped_create(int N, struct blkdev *disks[], int unit)
{
  struct blkdev *dev = malloc(sizeof(*dev));
  struct stripe_dev *sdev = malloc(sizeof(*sdev));
  int num_of_blocks = disks[0]->ops->num_blocks(disks[0]);
  int i;
 sdev->disks=disks;
  for(i=0;i< N; i++)
    {
      if(disks[i]->ops->num_blocks(disks[i]) != num_of_blocks)
	 return NULL;
    }
   
  int wastage = num_of_blocks%unit;
  sdev->nofdisks = N;
  sdev->unit = unit;
  sdev->nblks = N*(disks[0]->ops->num_blocks(disks[0])-wastage);
  dev->private = sdev;
  dev->ops = &strip_ops;
  return dev;

}

/**********   RAID 4  ***************/

/* helper function - compute parity function across two blocks of
 * 'len' bytes and put it in a third block. Note that 'dst' can be the
 * same as either 'src1' or 'src2', so to compute parity across N
 * blocks you can do: 
 *
 *     void **block[i] - array of pointers to blocks
 *     dst = <zeros[len]>
 *     for (i = 0; i < N; i++)
 *        parity(block[i], dst, dst);
 *
 * Yes, it's slow.
 */


struct raid4_dev{
  struct blkdev *strip; 
  int nofdisks;
  int nblks;
  int unit;
    };


int raid4_num_blocks(struct blkdev *dev)
{
  struct raid4_dev *rdev = dev->private;
  return rdev->nblks;
}



void parity(int len, void *src1, void *src2, void *dst)
{
    unsigned char *s1 = src1, *s2 = src2, *d = dst;
    int i;
    for (i = 0; i < len; i++)
        d[i] = s1[i] ^ s2[i];
}

int calculateParity(struct blkdev * dev, int stripeNumber, char *stripeBuffer)
{
  struct raid4_dev *rdev = dev->private;
  char *chunkBuffer =  malloc(rdev->unit * BLOCK_SIZE);
  char *bufferDescription;
  int i, diskParity, diskFail, result, readStartBlock, length, diskCount, stripeWidth;
  int noOfDiskFailure = 0;
  
  diskParity = rdev->nofdisks - 1;
  diskFail = -2;
  diskCount =  rdev->nofdisks;
  stripeWidth = rdev->unit;
  for (i =0; i< diskCount ; i++)
    {
      readStartBlock = (stripeNumber * diskCount + i) * stripeWidth;
      length =  stripeWidth;
      if (i == diskParity)
	{
	 
	  bufferDescription = stripeBuffer + stripeWidth * diskParity * BLOCK_SIZE;
	  
	}
      else
	{
	  bufferDescription = stripeBuffer + stripeWidth * i * BLOCK_SIZE;
	}
      result = rdev->strip->ops->read(rdev->strip, readStartBlock, length, bufferDescription);
      if (result!=SUCCESS)
	{
	  diskFail = i;
	  noOfDiskFailure++;
	}
    }
  if ( noOfDiskFailure > 1)
    {
      return E_UNAVAIL;
    }
  if (diskFail!=-2)
    {
      for( i = 0; i< diskCount; i++)
	{
	  if(i!=diskFail)
	    {
	      if(i==0 || (diskFail==0 && i==1))
		{
		  memcpy(chunkBuffer, stripeBuffer + i * stripeWidth * BLOCK_SIZE,  stripeWidth * BLOCK_SIZE);
		 }
	      else
		{
		  parity(stripeWidth * BLOCK_SIZE, stripeBuffer + i * stripeWidth * BLOCK_SIZE,chunkBuffer, chunkBuffer);
		}
	    }
	}
      memcpy(stripeBuffer + diskFail * stripeWidth * BLOCK_SIZE, chunkBuffer, stripeWidth * BLOCK_SIZE);
      return SUCCESS;
    }
  return SUCCESS;
}



/* read blocks from a RAID 4 volume.
 * If the volume is in a degraded state you may need to reconstruct
 * data from the other stripes of the stripe set plus parity.
 * If a drive fails during a read and all other drives are
 * operational, close that drive and continue in degraded state.
 * If a drive fails and the volume is already in a degraded state,
 * close the drive and return an error.
 */
static int raid4_read(struct blkdev * dev, int first_blk,
                      int num_blks, void *buf) 
{
  struct raid4_dev *rdev = dev->private;
  if(first_blk<0 || first_blk+num_blks > rdev->nblks)
    {
      return  E_BADADDR;
    }
  int i, stripeNumber, chunkNumber, diskNumber, chunkOffset, bufOffset,status, stripeLengthParse, totalChunk;

  chunkOffset = first_blk%(rdev-> unit);
  chunkNumber = first_blk/(rdev-> unit);
  diskNumber = chunkNumber%(rdev->nofdisks-1);
  stripeNumber = chunkNumber/(rdev->nofdisks-1);
  bufOffset = 0;
  char *stripeBuffer = malloc((rdev->nofdisks * rdev->unit * BLOCK_SIZE));
  char *startingPosition;
  int stripeWidth = rdev->unit;
  
  if (chunkOffset!=0 || diskNumber !=0)
    {
     status =  calculateParity(dev, stripeNumber, stripeBuffer);
     if (status == E_UNAVAIL)
       {
	 return E_UNAVAIL;
       }
     startingPosition =  stripeBuffer + ( diskNumber * stripeWidth + chunkOffset) * BLOCK_SIZE;
     stripeLengthParse = (rdev->nofdisks - 1) * stripeWidth - (diskNumber * stripeWidth + chunkOffset);
     if (num_blks <= stripeLengthParse)
       {
	 // printf("A");
	 memcpy(buf, startingPosition, num_blks * BLOCK_SIZE);
	 return SUCCESS;
       }
     else
       {
	 // printf("B");
	 memcpy(buf, startingPosition, stripeLengthParse * BLOCK_SIZE);
       }
     bufOffset = bufOffset + stripeLengthParse * BLOCK_SIZE;
     stripeNumber++;
    }

  totalChunk = (num_blks - (bufOffset/ BLOCK_SIZE))/ (stripeWidth * (rdev->nofdisks-1));
  // printf("C");
  for (i=0; i<totalChunk; i++)
    {
     status =  calculateParity(dev, stripeNumber, stripeBuffer);
     if (status == E_UNAVAIL)
       {
	 return E_UNAVAIL;
       }
     startingPosition = stripeBuffer;
     stripeLengthParse =  stripeWidth * (rdev->nofdisks-1);
     memcpy(buf + bufOffset, startingPosition, stripeLengthParse * BLOCK_SIZE);
     bufOffset =  bufOffset + stripeLengthParse * BLOCK_SIZE;
     stripeNumber++;
    }
  if(bufOffset/BLOCK_SIZE < num_blks)
    {
      // printf("D");
      status = calculateParity(dev, stripeNumber, stripeBuffer);
      if (status == E_UNAVAIL)
	{
	  return E_UNAVAIL;
	}
     startingPosition = stripeBuffer;
     stripeLengthParse =  num_blks - bufOffset/BLOCK_SIZE;
     // printf("length of the stripeLengthParse %d\n",stripeLengthParse);
     memcpy(buf + bufOffset, startingPosition, stripeLengthParse * BLOCK_SIZE);
     bufOffset =  bufOffset + stripeLengthParse * BLOCK_SIZE;
     stripeNumber++;   
    }
     
     
  return SUCCESS;
}




 int modifyBuffer(struct blkdev *dev, char *stripeBuffer, int size, int startingBlock, char *buf, int stripeNumber)
 {
   struct raid4_dev *rdev =  dev->private;
   int i;
   int stripeWidth = rdev->unit; 
   int chunkLength = stripeWidth * BLOCK_SIZE;
   char *bufferChunk = malloc(stripeWidth*BLOCK_SIZE);
   
   if(size == (rdev->nofdisks - 1) * stripeWidth)
     {
     if (startingBlock == 0)
       {
	 memcpy(stripeBuffer, buf, size * BLOCK_SIZE);
	 
	 for(i=0;i<(rdev->nofdisks - 1);i++)
	   {
	     if(i==0)
	       {
		 memcpy(bufferChunk, stripeBuffer+i*chunkLength, chunkLength);
	       }
	     else
	       {
		 parity(chunkLength, stripeBuffer+i*chunkLength, bufferChunk, bufferChunk); 
	       }
	     
	   }
	 memcpy(stripeBuffer+(rdev->nofdisks-1)*chunkLength, bufferChunk, chunkLength);
	
       }
     }
   else
     {
      int result =  calculateParity(dev, stripeNumber, stripeBuffer);
     
      if (result == SUCCESS)
	{
	  memcpy(stripeBuffer+(startingBlock * BLOCK_SIZE), buf, size * BLOCK_SIZE);
	  //char buffer[23];
	  //char *buf1 = stripeBuffer;
	  //buf1 = buffer;
	  //printf("Buffer:%s\n",buf1);
	  for(i=0;i<(rdev->nofdisks - 1);i++)
	   {
	     if(i==0)
	       {
		 memcpy(bufferChunk, stripeBuffer+i*chunkLength, chunkLength);
	       }
	     else
	       {
		 parity(chunkLength, stripeBuffer+i*chunkLength, bufferChunk, bufferChunk); 
	       }
	     
	   }
	 memcpy(stripeBuffer+(rdev->nofdisks-1)*chunkLength, bufferChunk, chunkLength);
	
	}
     else
	{
	return E_UNAVAIL; 
	}
	

     }
   free(bufferChunk);
   return SUCCESS;
 }

void final_writing_in_disk(int stripeNumber, struct blkdev *dev,char *stripeBuffer)
{
  struct raid4_dev  *rdev =  dev->private;
  char *finalBuffer;
  int stripeWidth = rdev->unit;
  int diskWithParity, firstBlkWrite;
  int i;
  diskWithParity = rdev->nofdisks-1 ;
  for(i=0; i<rdev->nofdisks;i++)
    {
      firstBlkWrite = (stripeNumber * rdev->nofdisks + i) * stripeWidth;
      if(i==diskWithParity)
	{
	  finalBuffer = stripeBuffer + stripeWidth * (rdev->nofdisks -1) * BLOCK_SIZE;
	}
      else
	{
	   finalBuffer = stripeBuffer + stripeWidth * i * BLOCK_SIZE;
	}
      rdev->strip->ops->write(rdev->strip, firstBlkWrite, stripeWidth, finalBuffer);
    }
}
      

/* write blocks to a RAID 4 volume.
 * Note that you must handle short writes - i.e. less than a full
 * stripe set. You may either use the optimized algorithm (for N>3
 * read old data, parity, write new data, new parity) or you can read
 * the entire stripe set, modify it, and re-write it. Your code will
 * be graded on correctness, not speed.
 * If an underlying device fails you should close it and complete the
 * write in the degraded state. If a drive fails in the degraded
 * state, close it and return an error.
 * In the degraded state perform all writes to non-failed drives, and
 * forget about the failed one. (parity will handle it)
 */
static int raid4_write(struct blkdev * dev, int first_blk,
                       int num_blks, void *buf)
{
  struct raid4_dev *rdev = dev->private;
  if(first_blk<0 || first_blk+num_blks > rdev->nblks)
  {
    return  E_BADADDR;
  }
  int i, stripeNumber, chunkNumber, diskNumber, chunkOffset, bufOffset, stripeLengthParse, totalChunk, startingBlock;
  //char buffer[23];
  //char *buf1 = buf;
  //*buf1 = buffer;
  //printf("Buffer:%s\n",buf1);
  chunkOffset = first_blk%(rdev-> unit);
  chunkNumber = first_blk/(rdev-> unit);
  diskNumber = chunkNumber%(rdev->nofdisks-1);
  stripeNumber = chunkNumber/(rdev->nofdisks-1);
  bufOffset = 0;
  char *stripeBuffer = malloc((rdev->nofdisks * rdev->unit * BLOCK_SIZE));
  int stripeWidth = rdev->unit;
  int final_result;

  if (chunkOffset!=0 || diskNumber !=0)
    {

     startingBlock = diskNumber * stripeWidth + chunkOffset;
     stripeLengthParse = (rdev->nofdisks - 1) * stripeWidth - (diskNumber * stripeWidth + chunkOffset);
     if (num_blks <= stripeLengthParse)
       {
	 final_result = modifyBuffer(dev, stripeBuffer, num_blks, startingBlock, buf, stripeNumber);
	 if(final_result == E_UNAVAIL)
	 {
		return E_UNAVAIL;
	 }
	 else
	 {
	 final_writing_in_disk(stripeNumber, dev, stripeBuffer);
	 return SUCCESS;
	 }
       }
     else
       {
	 final_result = modifyBuffer(dev, stripeBuffer, stripeLengthParse, startingBlock, buf, stripeNumber);
	 if(final_result == E_UNAVAIL)
	 {
		return E_UNAVAIL;
	 }
	 else
	 {
	 final_writing_in_disk(stripeNumber, dev, stripeBuffer);
  	 } 
       }
     bufOffset = bufOffset + stripeLengthParse * BLOCK_SIZE;
     stripeNumber++;
    }

  totalChunk = (num_blks - (bufOffset/ BLOCK_SIZE))/ (stripeWidth * (rdev->nofdisks-1));
  for (i=0; i<totalChunk; i++)
    {
     startingBlock=0;
     stripeLengthParse = (rdev->nofdisks-1) * rdev->unit;
     final_result = modifyBuffer(dev, stripeBuffer, stripeLengthParse, startingBlock, buf+bufOffset, stripeNumber);
     if(final_result == E_UNAVAIL)
     {
	return E_UNAVAIL;
     }
     else
     {
     final_writing_in_disk(stripeNumber, dev, stripeBuffer);
     }
     bufOffset =  bufOffset + stripeLengthParse * BLOCK_SIZE;
     stripeNumber++;
    }
  if(bufOffset/BLOCK_SIZE < num_blks)
    {
     startingBlock = 0;
     stripeLengthParse =  num_blks - bufOffset/BLOCK_SIZE;
     final_result = modifyBuffer(dev, stripeBuffer, stripeLengthParse, startingBlock, buf+bufOffset, stripeNumber);
     if(final_result == E_UNAVAIL)
     {
	return E_UNAVAIL;
     }
     else
     {
     final_writing_in_disk(stripeNumber, dev, stripeBuffer);
     }
     
    }
     
     
  return SUCCESS;
}

/* clean up, including: close all devices and free any data structures
 * you allocated in raid4_create. 
 */
static void raid4_close(struct blkdev *dev)
{


  struct raid4_dev *rdev = dev->private;
  rdev->strip->ops->close(rdev->strip);
  free(rdev);
  free(dev);

}

struct blkdev_ops raid4_ops = 
{
    .num_blocks = raid4_num_blocks,
    .read = raid4_read,
    .write = raid4_write,
    .close = raid4_close
};

/* Initialize a RAID 4 volume with stripe size 'unit', using
 * disks[N-1] as the parity drive. Do not write to the disks - assume
 * that they are properly initialized with correct parity. (warning -
 * some of the grading scripts may fail if you modify data on the
 * drives in this function)
 */
struct blkdev *raid4_create(int N, struct blkdev *disks[], int unit)
{
  struct blkdev *dev = malloc(sizeof(*dev));
  struct raid4_dev *rdev = malloc(sizeof(*rdev));
  int num_of_blocks_in_disk = disks[0]->ops->num_blocks(disks[0]);
  int i,j,k;
  for( i=0;i< N; i++)
    {
      if(disks[i]->ops->num_blocks(disks[i]) != num_of_blocks_in_disk)
	 return NULL;
    }

  int wastage_blocks = num_of_blocks_in_disk % unit;
  rdev->strip = striped_create(N, disks, unit);
  rdev->nofdisks = N;
  rdev->unit = unit;
  rdev->nblks = (N-1)*(disks[0]->ops->num_blocks(disks[0]) - wastage_blocks);
  // struct stripe_dev *sdev = rdev->strip->private;
  // struct blkdev *paritydisk = malloc(sizeof(*paritydisk));
  int sizeOfOneChunk = rdev->unit*BLOCK_SIZE;
  char *bufferData = malloc(sizeOfOneChunk * BLOCK_SIZE);
  char *bufferChunk = malloc(sizeOfOneChunk * BLOCK_SIZE);
  int sizeOfOneStripe = sizeOfOneChunk * rdev->nofdisks;
  int numberOfStripes = rdev->nblks / (rdev->unit * (rdev->nofdisks - 1));
  for ( j=0; j< numberOfStripes; j++)
    {
  for( k=0; k < rdev->nofdisks - 1; k++)  
    {
      rdev->strip->ops->read(rdev->strip, j*sizeOfOneStripe+k*sizeOfOneChunk ,sizeOfOneChunk, bufferChunk);  
      if(k==0)
	{
	  memcpy(bufferData, bufferChunk, sizeOfOneChunk * BLOCK_SIZE);
	
	}
      else
	{
	   parity(sizeOfOneChunk * BLOCK_SIZE, bufferChunk, bufferData, bufferData);
	}
    }
  
  rdev->strip->ops->write(rdev->strip, j*sizeOfOneStripe+k*sizeOfOneChunk, sizeOfOneChunk, bufferData);
    }
  dev->private = rdev;
  dev->ops = &raid4_ops;
  return dev;
  
}

/* replace failed device 'i' in a RAID 4. Note that we assume
 * the upper layer knows which device failed. You will need to
 * reconstruct content from data and parity before returning
 * from this call.
 */
int raid4_replace(struct blkdev *volume, int i, struct blkdev *newdisk)
{
  struct raid4_dev *rdev = volume->private;
  struct stripe_dev *sdev = rdev->strip->private;
  int sizeOfOneChunk = rdev->unit;
  char *bufferData = malloc(sizeOfOneChunk * BLOCK_SIZE);
  char *bufferChunk = malloc(sizeOfOneChunk * BLOCK_SIZE);
  int sizeOfOneStripe = sizeOfOneChunk * rdev->nofdisks;
  int numberOfStripes = rdev->nblks / (sizeOfOneChunk * (rdev->nofdisks - 1));
  int j,l;
  if(sdev->disks[1]->ops->num_blocks(sdev->disks[1])!=newdisk->ops->num_blocks(newdisk))
    {
      return E_SIZE;
    }

  for ( j=0; j< numberOfStripes; j++)
    {
  for( l=0; l < rdev->nofdisks; l++)  
    {
      if(l!=i)
	{
      rdev->strip->ops->read(rdev->strip, j*sizeOfOneStripe+l*sizeOfOneChunk ,sizeOfOneChunk, bufferChunk);  
      if((i==0 && l==1) || l==0 )
	{
	  memcpy(bufferData, bufferChunk, sizeOfOneChunk * BLOCK_SIZE);
	}
      else
	{
	   parity(sizeOfOneChunk * BLOCK_SIZE, bufferChunk, bufferData, bufferData);
	}
	}
    }
  newdisk->ops->write(newdisk, j*sizeOfOneChunk, sizeOfOneChunk, bufferData);
    }
  sdev->disks[i]=newdisk;
  return SUCCESS;
}






