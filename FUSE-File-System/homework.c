/*
 * file:        homework.c
 * description: skeleton file for CS 5600 homework 3
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Peter Desnoyers, updated April 2012
 * $Id: homework.c 452 2011-11-28 22:25:31Z pjd $
 */

#define FUSE_USE_VERSION 27

#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fuse.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "cs5600fs.h"
#include "blkdev.h"


/* 
 * disk access - the global variable 'disk' points to a blkdev
 * structure which has been initialized to access the image file.
 *
 * NOTE - blkdev access is in terms of 512-byte SECTORS, while the
 * file system uses 1024-byte BLOCKS. Remember to multiply everything
 * by 2.
 */

extern struct blkdev *disk;
static struct cs5600fs_super super_block;
static struct cs5600fs_dirent *directory;
static struct cs5600fs_entry *fat;

/* Splitting the line into words*/

static int strwrd(char *s, char **buf, int len, char *delim)
{
  char **temp;
  for(temp = buf; (*temp = strtok(s, delim)) != NULL; s = NULL)
    {
    if(++temp >= &buf[len])
      {
      break;
      }
    }
  return temp-buf;
}

/*Path contain the valid directory or not*/

static int validatePath(const char *path, struct cs5600fs_dirent *dir)
{
  char *directory_names[16];
  int length = 10;
  // char *delimeter = "/";

  int k, l, num_of_words;
  if(strcmp(path, "/")==0 || strcmp(path, "") == 0){
    memcpy(dir, directory, sizeof(struct cs5600fs_dirent));
    return 0;
  }
  char *path_variable=malloc((strlen(path)+1)*sizeof(char));
  memcpy(path_variable, path, strlen(path) + 1);
  num_of_words = strwrd((char *)path_variable, directory_names, length, "/");
  struct cs5600fs_dirent *tempdir =  directory;
  struct cs5600fs_dirent directory[16];
  for(k=0; k<num_of_words; k++)
    {
    disk->ops->read(disk, tempdir->start * 2, 2, (void*)directory);
    for(l=0; l<16; l++)
      {
      if(directory[l].valid == 1)
	{
	  if(strcmp(directory_names[k], directory[l].name)==0)
	    {
	      tempdir = &(directory[l]);
	      break;
	    }
	
	}
      }
    if(l>=16)
      {
	return -ENOENT;
      }
    }
  memcpy(dir, tempdir, sizeof(struct cs5600fs_dirent));
  return 0;
  }

static int num_blks(int length)
{
  if (length!=0)
    {
      return 1+((length-1)/1024);
    }
  else
    return 0;
}

void mainpath(const char* path, char* main, char* name)
{
  char* slashposition = strrchr(path, '/');
  int positionofSlash = slashposition - path;
  strncpy(main, path, positionofSlash);
  main[positionofSlash]= '\0';
  strncpy(name, path+positionofSlash+1, strlen(path)-positionofSlash-1);
  name[strlen(path)-positionofSlash-1] = '\0';
  if(strlen(main) ==0)
    {
      main[0]='/';
      main[1]='\0';
    }
}

/* init - this is called once by the FUSE framework at startup.
 * This might be a good place to read in the super-block and set up
 * any global variables you need. You don't need to worry about the
 * argument or the return value.
 */
void* hw3_init(struct fuse_conn_info *conn)
{
  char buf_superblock[FS_BLOCK_SIZE];
 
  disk->ops->read(disk, 0, 2, buf_superblock);
  memcpy(&super_block, buf_superblock, sizeof(super_block));
  
  int size_of_fat = FS_BLOCK_SIZE * super_block.fat_len;
  char buf_fat[size_of_fat];
  fat =calloc(size_of_fat, 1); 
  disk->ops->read(disk,2,super_block.fat_len*2, buf_fat);
  memcpy(fat, buf_fat, size_of_fat);
  
  directory = &(super_block.root_dirent);
  
  return NULL;
}

/* Note on path translation errors:
 * In addition to the method-specific errors listed below, almost
 * every method can return one of the following errors if it fails to
 * locate a file or directory corresponding to a specified path.
 *
 * ENOENT - a component of the path is not present.
 * ENOTDIR - an intermediate component of the path (e.g. 'b' in
 *           /a/b/c) is not a directory
 */

/* getattr - get file or directory attributes. For a description of
 *  the fields in 'struct stat', see 'man lstat'.
 *
 * Note - fields not provided in CS5600fs are:
 *    st_nlink - always set to 1
 *    st_atime, st_ctime - set to same value as st_mtime
 *
 * errors - path translation, ENOENT
 */
static int hw3_getattr(const char *path, struct stat *sb)
{
  
  struct cs5600fs_dirent newdirect;
  int result = validatePath((char *)path, &newdirect);
  if (result!=0)
    {
    return -ENOENT;
    }
  sb->st_dev=0;
  sb->st_ino=0;
  sb->st_mode= newdirect.mode | (newdirect.isDir ? S_IFDIR : S_IFREG);
  sb->st_nlink = 1;
  sb->st_uid = newdirect.uid;
  sb->st_gid = newdirect.gid;
  sb->st_rdev =0;
  sb->st_size =newdirect.length;
  sb->st_blksize = 1024;
  sb->st_blocks = num_blks(newdirect.length);
  sb->st_atime = sb->st_mtime = sb->st_ctime = newdirect.mtime;
  return 0;   
}




/* readdir - get directory contents.
 *
 * for each entry in the directory, invoke the 'filler' function,
 * which is passed as a function pointer, as follows:
 *     filler(buf, <name>, <statbuf>, 0)
 * where <statbuf> is a struct stat, just like in getattr.
 *
 * Errors - path resolution, ENOTDIR, ENOENT
 */
static int hw3_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
  char *name = NULL;
  struct cs5600fs_dirent directory[16]; // 1024/(64byte each directory) = 16
  struct cs5600fs_dirent workingdirectory;
  struct stat sb;
  int blk_num=0;
    
  //return -EOPNOTSUPP;
  /* NOT COMPLETE */

  int validated_dir;
  validated_dir = validatePath(path, &workingdirectory);
  if(validated_dir != 0)
    {
      return -ENOENT;
    }
  if(!workingdirectory.isDir)
    {
      return -ENOTDIR;
    }
  blk_num = workingdirectory.start;

  //printf("%d",blk_num);
  disk->ops->read(disk, blk_num*2, 2, (void*)directory);

  /* Example code - you have to iterate over all the files in a
   * directory and invoke the 'filler' function for each.
   */
  memset(&sb, 0, sizeof(sb));
    
  int dir_id=0;
  for (dir_id=0;dir_id<16;dir_id++) {

    if(directory[dir_id].valid==1){

      	sb.st_mode = directory[dir_id].mode | (directory[dir_id].isDir ? S_IFDIR : S_IFREG); /* permissions | (isdir ? S_IFDIR : S_IFREG) */
	sb.st_uid = directory[dir_id].uid;
	sb.st_gid = directory[dir_id].gid;
	sb.st_size = directory[dir_id].length; /* obvious */
	sb.st_atime = sb.st_ctime = sb.st_mtime = directory[dir_id].mtime; /* modification time */
	sb.st_blocks = num_blks(directory[dir_id].length);
	name = directory[dir_id].name;
	filler(buf, name, &sb, 0); /* invoke callback function */
      }      
}
    return 0;
}

/* create - create a new file with permissions (mode & 01777)
 *
 * Errors - path resolution, EEXIST
 *
 * If a file or directory of this name already exists, return -EEXIST.
 */
static int hw3_create(const char *path, mode_t mode,
			 struct fuse_file_info *fi)
{

  char calculated_path[200];
  struct cs5600fs_dirent dir[16];
  struct cs5600fs_dirent new_directory;
  char file_name[200];
  int valid_path =0;
  
  
  mainpath(path, calculated_path, file_name);
  valid_path = validatePath(path, &new_directory);

  if(valid_path==0) return -EEXIST;

  valid_path = validatePath(calculated_path,&new_directory);
  if(valid_path!=0) return -ENOENT;

  
  disk->ops->read(disk,new_directory.start*2,2,(void *)dir);
  int dir_id=0;
  int flag=0;

  for(dir_id=0;dir_id<16;dir_id++)
    {
      if(dir[dir_id].valid==1) flag++;

    }

  if(flag>=16) return -ENOSPC;
  
  dir[flag].valid = 1;
  dir[flag].isDir =0;
  dir[flag].mode = mode & 01777;
  dir[flag].pad =super_block.root_dirent.pad;
  dir[flag].gid=super_block.root_dirent.gid;
  dir[flag].uid=super_block.root_dirent.uid;
  dir[flag].mtime=time(NULL);
  dir[flag].length=0;
  strcpy(dir[flag].name,file_name);
  
  int flag2=0;
  for(dir_id=0;dir_id<super_block.fs_size;dir_id++)
    {
      if(fat[flag2].inUse) flag2++;
    }

  if(flag2>=super_block.fs_size) return -ENOSPC;

  fat[flag2].inUse=1;
  fat[flag2].eof=1;
  fat[flag2].next=0;
  dir[flag].start=flag2;


  disk->ops->write(disk, new_directory.start*2,2,(void *)dir);
  disk->ops->write(disk, 2, super_block.fat_len*2, (char *)fat);
  return 0;
}

/* mkdir - create a directory with the given mode.
 * Errors - path resolution, EEXIST
 * Conditions for EEXIST are the same as for create.
 */ 
static int hw3_mkdir(const char *path, mode_t mode)
{
  char dir_name[200];
  char calculated_path[200];
  int dir_id;
  int valid_path;
  //return -EOPNOTSUPP;

  int new_id =-5;
  struct cs5600fs_dirent new_directory;
  struct cs5600fs_dirent directory[16];
  mainpath(path, calculated_path,dir_name);
  
  valid_path = validatePath(calculated_path, &new_directory);

  if(valid_path!=0)
    {
      return -ENOENT;
    }


  
  disk->ops->read(disk, new_directory.start*2, 2, (void *)directory);
  // checking if 16 directories are already present if yes then returning operation not supported

  for(dir_id=0; dir_id<16; dir_id++)
    {
      if(directory[dir_id].valid && !strcmp(dir_name,directory[dir_id].name))
	{
	  return -EEXIST;
	  
	}
	
      if(!directory[dir_id].valid)
	{
	  if(new_id == -5)
	  {
	  //printf("inside\n");
	  new_id = dir_id; 
	  }
	}
	
    }
  //printf("%d", new_id);
  if(new_id == -5)
    {
      return -ENOSPC;
    }


  int first_blk = -5;

  for(dir_id=0;dir_id< super_block.fs_size; dir_id++)
    {
      if(fat[dir_id].inUse==0)
	{
	  first_blk=dir_id;
	  fat[dir_id].inUse =1;
	  fat[dir_id].eof=1;
	  fat[dir_id].next=0;
	  break;

	}
      
      
    }

  if(first_blk==-5)
    {
      return -ENOSPC;
    }

  
  directory[new_id].valid=1;
  directory[new_id].isDir=1;
  directory[new_id].pad = super_block.root_dirent.pad;
  directory[new_id].uid = super_block.root_dirent.uid;
  directory[new_id].gid = super_block.root_dirent.gid;
  directory[new_id].mode = mode;
  directory[new_id].mtime = time(NULL);
  directory[new_id].start = first_blk;
  directory[new_id].length = 0;
  strcpy(directory[new_id].name,dir_name);

  
  disk->ops->write(disk, new_directory.start*2,2,(void *)directory);
  disk->ops->write(disk, 2, super_block.fat_len*2, (char *)fat);
  return 0;
}

/* unlink - delete a file
 *  Errors - path resolution, ENOENT, EISDIR
 */
static int hw3_unlink(const char *path)
{
  struct cs5600fs_dirent direct_delete, path_delete;
  int k, fatblock,t;
  int flag = validatePath((char *) path, &direct_delete);
  if(flag!=0)
    return -ENOENT;
  if(direct_delete.isDir)
    return -EISDIR;
  char pathDirectory[512];
  char name[50];
  mainpath(path, pathDirectory, name);
  flag = validatePath((char *) pathDirectory, &path_delete);
  if(flag!=0)
    return -ENOENT;
  struct cs5600fs_dirent directory[16];
  disk->ops->read(disk,path_delete.start*2,2, (void *)directory);
  k=0;
  while(k<16)
    {
      if(directory[k].valid==1 && strcmp(direct_delete.name,directory[k].name)==0)
	{
	    directory[k].valid=0;
	    break; 
	}
      k++;
    }
  disk->ops->write(disk, path_delete.start*2,2,(void *)directory);
  fatblock = direct_delete.start;
  while(fatblock!=0)
    {
      fat[fatblock].inUse = 0;
      fat[fatblock].eof = 0;
      t = fat[fatblock].next;
      fat[fatblock].next = 0;
      fatblock = t;
    }
  disk->ops->write(disk, 2, super_block.fat_len*2, (char *)fat);
  return 0;
  
}

/* rmdir - remove a directory
 *  Errors - path resolution, ENOENT, ENOTDIR, ENOTEMPTY
 */
static int hw3_rmdir(const char *path)
{
  struct  cs5600fs_dirent original_direct, parent_direct;
  int convert_path_to_directory= validatePath(path, &original_direct);
  if (convert_path_to_directory!=0)
    {
      return -ENOENT;
    }
  if (!original_direct.isDir)
    {
      return -ENOTDIR;
    }
  struct cs5600fs_dirent read_blocks[16];
  disk->ops->read(disk,original_direct.start *2,2, (void *)read_blocks);
  int l;
  l=0;
  while(l<16)
    {
      if(read_blocks[l].valid == 1)
	{
	return -ENOTEMPTY;
	}
      l++;
    }
  fat[original_direct.start].inUse=0;
  fat[original_direct.start].eof=1;
  disk->ops->write(disk, 2, super_block.fat_len*2, (char *)fat);
  
  char p_path[512];
  char name[50];
  mainpath(path, p_path, name);
  validatePath(p_path, &parent_direct);
  struct cs5600fs_dirent read_blocks1[16];
  disk->ops->read(disk,parent_direct.start *2,2, (void *)read_blocks1);
  int k;
  k=0;
  while(k<16)
    {
      if(read_blocks1[k].valid==1)
	{
	  if(strcmp(read_blocks1[k].name, name)==0)
	    {
	      read_blocks1[k].valid=0;
	    }
	}
      k++;
    }
  disk->ops->write(disk,parent_direct.start*2,2, (void *)read_blocks1);
  return 0;
}

/* rename - rename a file or directory
 * Errors - path resolution, ENOENT, EINVAL, EEXIST
 *
 * ENOENT - source does not exist
 * EEXIST - destination already exists
 * EINVAL - source and destination are not in the same directory
 *
 * Note that this is a simplified version of the UNIX rename
 * functionality - see 'man 2 rename' for full semantics. In
 * particular, the full version can move across directories, replace a
 * destination file, and replace an empty directory with a full one.
 */
static int hw3_rename(const char *src_path, const char *dst_path)
{
  struct cs5600fs_dirent srcdirent, srcparentdirent;
  char *srcparent = malloc(strlen(src_path) + 1);
  char *dstparent = malloc(strlen(dst_path) + 1);
  char oldname[50], newname[50];
  int flag = validatePath((char *) src_path, &srcdirent);
  if(flag!=0)
    {
      return -ENOENT;
    }
  mainpath(src_path, srcparent, oldname);
  mainpath(dst_path, dstparent, newname);
  if(strcmp(srcparent, dstparent) != 0)
    {
      return -EINVAL;
    }
  flag = validatePath((char *) srcparent, &srcparentdirent);
  struct cs5600fs_dirent directoryblock[16];
  disk->ops->read(disk, srcparentdirent.start * 2, 2, (void *)directoryblock);
  int k;
  k=0;
  while(k<16)
    {
      if(directoryblock[k].valid == 1 && strcmp(directoryblock[k].name, newname)==0)
	{
	    return -EEXIST;
	}
      k++;
    }
  int l;
  l=0;
  while(l<16)
    {
      if(directoryblock[l].valid ==1 && strcmp(directoryblock[l].name, srcdirent.name)==0)
	{
	      strcpy(directoryblock[l].name, newname);
	      directoryblock[l].mtime = time(NULL);
	      break;
	}
      l++;
    }
  disk->ops->write(disk, srcparentdirent.start * 2, 2, (void *) directoryblock);
  return 0;
}

/* chmod - change file permissions
 
 * Errors - path resolution, ENOENT.
 */
static int hw3_chmod(const char *path, mode_t mode)
{
  struct cs5600fs_dirent srcdirent, srcparentdirent;
  char *srcparent = malloc(strlen(path) + 1);
  int flag = validatePath( path, &srcdirent);
  if(flag!=0)
    {
      return -ENOENT;
    }
  char nameoffile[50];
  mainpath(path, srcparent, nameoffile);
  flag = validatePath( srcparent, &srcparentdirent);
  struct cs5600fs_dirent directory_blocks[16];
  disk->ops->read(disk, srcparentdirent.start * 2, 2, (void *) directory_blocks);
  int j;
  j=0;
  while(j<16)
    {
      if(directory_blocks[j].valid == 1 && strcmp(directory_blocks[j].name, srcdirent.name)==0)
	{
	      directory_blocks[j].mode = mode;
	      break;
	}
      j++;
    }
  disk->ops->write(disk, srcparentdirent.start * 2, 2, (void *) directory_blocks);
  return 0;
}

/* utime - change access and modification times
 *         (for definition of 'struct utimebuf', see 'man utime')
 * Errors - path resolution, ENOENT.
 */

int hw3_utime(const char *path, struct utimbuf *ut)
{
  struct cs5600fs_dirent srcdirent, srcparentdirent;
  char *srcparent = malloc(strlen(path) + 1);
  int flag = validatePath((void *) path, &srcdirent);
  if(flag!=0)
    {
      return -ENOENT;
    }
  char nameoffile[50];
  mainpath(path, srcparent, nameoffile);
  flag = validatePath((void *) srcparent, &srcparentdirent);
  struct cs5600fs_dirent directory_blocks[16];
  disk->ops->read(disk, srcparentdirent.start * 2, 2, (void *) directory_blocks);
  int j;
  j=0;
  while(j<16)
    {
      if(directory_blocks[j].valid == 1 && strcmp(directory_blocks[j].name, srcdirent.name)==0)
	{
	      directory_blocks[j].mtime = ut->modtime;
	      break;
	}
      j++;
    }
  disk->ops->write(disk, srcparentdirent.start * 2, 2, (void *) directory_blocks);
  return 0;
}

/* truncate - truncate file to exactly 'len' bytes
 * Errors - path resolution, ENOENT, EISDIR, EINVAL
 *    return EINVAL if len > 0.
 */
static int hw3_truncate(const char *path, off_t len)
{
    /* you can cheat by only implementing this for the case of len==0,
     * and an error otherwise.
     */
    
    if (len != 0)
	return -EINVAL;		/* invalid argument */
    
    struct cs5600fs_dirent dest_path, main_direct;
    char file_name[50];
    int flag, block, j;
    flag = validatePath(path, &dest_path);
    if(flag!=0)
      {
	return -ENOENT;
      }
    if(dest_path.isDir)
      {
	return -EISDIR;
      }

    block = dest_path.start;
    while(1)
      {
	fat[block].inUse = 0;
	if(fat[block].eof==1)
	  {
	  break;
	  }
	block = fat[block].next;
      }
    fat[dest_path.start].eof=1;
    fat[dest_path.start].next = 0;
    fat[dest_path.start].inUse = 1;
    
    disk->ops->write(disk, 2, super_block.fat_len * 2, (void *)fat);
    
    char main_path[200];
    mainpath(path, main_path, file_name);
    flag = validatePath(main_path, &main_direct);
    struct cs5600fs_dirent directory_blocks[16];
    disk->ops->read(disk, main_direct.start*2, 2, (void *) directory_blocks);
    j=0;
    while(j<16)
      {
	if(directory_blocks[j].valid == 1)
	  {
	    if(strcmp(directory_blocks[j].name,dest_path.name)==0)
	      {
		directory_blocks[j].mtime = time(NULL);
		directory_blocks[j].length = 0;
	      }
	  }
	j++;
      }
    disk->ops->write(disk, main_direct.start*2, 2, (void *) directory_blocks);
    return 0;
}

/* read - read data from an open file.
 * should return exactly the number of bytes requested, except:
 *   - if offset >= len, return 0
 *   - on error, return <0
 * Errors - path resolution, ENOENT, EISDIR
 */
static int hw3_read(const char *path, char *buf, size_t len, off_t offset,
		    struct fuse_file_info *fi)
{
    struct cs5600fs_dirent dest_path;
    int block_offset = offset / FS_BLOCK_SIZE;
   
    int flag;
    flag = validatePath(path, &dest_path);
    if(flag!=0)
      {
	return -ENOENT;
      }
    if(dest_path.isDir)
      {
	return -EISDIR;
      }
    int directory_length= dest_path.length;
    if(offset >= directory_length)
      {
	return 0;
      }
   
    int starting_block = dest_path.start;
    int k;
    k=0;
    while(k<block_offset)
      {
	starting_block = fat[starting_block].next;
	k++;
      }
   
    int total_blocks;
    if((len + offset) > directory_length)
      {
      len = dest_path.length - offset;
      }
    total_blocks = ((len + offset)/1024) - block_offset + 1;
    
    char *buffer_temp = malloc(1024 * total_blocks);
    k = starting_block;
    int l;
    l=0;
    while(l<total_blocks)
      {
	disk->ops->read(disk, k*2, 2,  buffer_temp + (1024 * l));
	k=fat[k].next;
	l++;
      }
    memcpy(buf, buffer_temp + (offset%1024), len);
    return len;
}


/* write - write data to a file
 * It should return exactly the number of bytes requested, except on
 * error.
 * Errors - path resolution, ENOENT, EISDIR
 *  return EINVAL if 'offset' is greater than current file length.
 */
static int hw3_write(const char *path, const char *buf, size_t len,
		     off_t offset, struct fuse_file_info *fi)
{
  struct cs5600fs_dirent dir;
  struct cs5600fs_dirent main_dir;
  struct cs5600fs_dirent main_dir_array[16];

  int valid=0;

  valid = validatePath(path,&dir);

  if(valid!=0) return -ENOENT;
  
  if(dir.isDir==1) return -EISDIR;

  if(offset>dir.length) return -EINVAL;
  

  ////////////////////////////////////////////////////////


  char main_path[200];
  char entry_name[200];

  mainpath(path,main_path,entry_name);
  validatePath(main_path,&main_dir);


  ////////////////////////////////////////////////////////
  int start = dir.start;
  int off = offset/1024;

  int i;
  for(i=0;i<off;i++)
    {
      start = fat[start].next;
    }

  int total_blks;
  total_blks = ((offset+len)/1024)-off+1;


  char *buf2 = malloc(total_blks*1024);
  int index1 = start;
  int blk_count;

  for(blk_count=0;blk_count<total_blks;blk_count++)
    {
      disk->ops->read(disk,2*index1,2,(buf2)+(blk_count*1024));
      if(fat[index1].eof==1) break;
   
      index1=fat[index1].next;
    }
   
  memcpy((buf2)+(offset%1024),buf,len);
  
  int index2 = start;
  
  int flag_eof=0,x=0;
  
  for(i=0;i<total_blks;i++)
    {
      disk->ops->write(disk,index2*2,2,buf2+i*1024);


      if(fat[index2].eof==1 && flag_eof==0)
	{
	  fat[index2].eof=0;
	  flag_eof=1;
	}


      if(i!=total_blks-1)
	{
	  if(flag_eof!=0)
	    {

	      for(x=0;fat[x].inUse==1 && x<super_block.fs_size;x++);
		
		  
		  if(x>=super_block.fs_size)return -ENOSPC;
		  
		  fat[x].inUse=1;
		  fat[x].eof=0;
		  fat[x].next=0;
		  fat[index2].next=x;
		  index2=x;
	    }

	  else
	    {
	      index2=fat[index2].next;
	    }

	}

    }



  if(offset+len>dir.length)
    {
      fat[index2].eof=1;
      dir.length=offset+len;
     
    }

  disk->ops->read(disk,main_dir.start*2,2,(void *)main_dir_array);
  
  for(x=0;x<16;x++)
    {

      if(main_dir_array[x].valid && (strcmp(main_dir_array[x].name, dir.name)==0))
	{
	  main_dir_array[x].length = dir.length;
	  main_dir_array[x].mtime = time(NULL);
	}


    }


  disk->ops->write(disk, main_dir.start*2,2,(void *)main_dir_array);

  
  disk->ops->write(disk, 2, super_block.fat_len*2, (char *)fat);

  
  return len;
}

/* statfs - get file system statistics
 * see 'man 2 statfs' for description of 'struct statvfs'.
 * Errors - none. Needs to work.
 */
static int hw3_statfs(const char *path, struct statvfs *st)
{
  int k, blks_used=0;
  int fatcount;
  k=0;
  fatcount = super_block.fat_len*1024/4;
  while(k<fatcount)
    {
      if(fat[k].inUse)
	{
	  blks_used = blks_used + 1;
	}
      k++;
    }
  st->f_frsize =0;
  st->f_files=0;
  st->f_ffree=0;
  st->f_fsid=0;
  st->f_flag=0;
  st->f_namemax=43;
  st->f_bsize=super_block.blk_size;
  st->f_blocks=super_block.fs_size-(1+super_block.fat_len);
  st->f_bfree=st->f_blocks-blks_used;
  st->f_bavail=st->f_bfree;

  return 0;
    
}

/* operations vector. Please don't rename it, as the skeleton code in
 * misc.c assumes it is named 'hw3_ops'.
 */
struct fuse_operations hw3_ops = {
    .init = hw3_init,
    .getattr = hw3_getattr,
    .readdir = hw3_readdir,
    .create = hw3_create,
    .mkdir = hw3_mkdir,
    .unlink = hw3_unlink,
    .rmdir = hw3_rmdir,
    .rename = hw3_rename,
    .chmod = hw3_chmod,
    .utime = hw3_utime,
    .truncate = hw3_truncate,
    .read = hw3_read,
    .write = hw3_write,
    .statfs = hw3_statfs,
};
