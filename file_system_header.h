#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>


static struct fuse_operations file_system_oper {
	.init       = file_system_init,
	.getattr    = file_system_getattr,
	//.readdir    = file_system_readdir,
	//.mkdir		= file_system_mkdir,
	//.rmdir		= file_system_rmdir,
	//.rename		= file_system_rename,
	//.truncate	= file_system_truncate,
	.open       = file_system_open,
	.create     = file_system_create,
	.read       = file_system_read,
	.write      = file_system_write,
	//.unlink		= file_system_rm,
};


typedef struct {
	int magic;			//denoting number of file types
	size_t blocks;		//number of data blocks
	size_t iblocks;		//number of inode bitmap blocks
	size_t inodes;		//number of inodes
} __attribute__((packed, aligned(1))) sblock;

//static int file_system_rmdir(const char *path);
/*static int file_system_rename(const char *from, const char *to, unsigned int flags);
static int file_system_truncate(const char *path, off_t size, struct fuse_file_info *fi);*/
static int file_system_create(const char *path, mode_t mode, struct fuse_file_info *fi);
static int file_system_open(const char *path, struct fuse_file_info *fi);
static int file_system_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int file_system_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
//static int file_system_rm(const char *path);

static void *file_system_init(struct fuse_conn_info *conn, struct fuse_config *cfg);
static int file_system_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi);
/*static int file_system_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags);
static int file_system_mkdir(const char *path, mode_t mode);*/




typedef struct {
	char *data;					// pointer to data block
	bool directory;				// true if its a directory else false
	int last_accessed;			// Last accessed time
	int last_modified;			// Last modified time
	int link_count; 			// 2 in case its a directory, 1 if its a file
	int children;
	bool used;                  // valid inode or not
	int id;						// ID for the inode
	size_t size;				// Size of the file
	

} __attribute__((packed, aligned(1))) inode;	//38 bytes


typedef struct {
	int id;			// inode id
	inode *in;      // pointer to inode
}FILE_REGISTER;

typedef struct{
	char *filename;
	inode *file_inode;
	int children;
}directory_record;

#define SBLK_SIZE 16
#define BLK_SIZE 4096

#define N_INODES 50
#define DBLKS_PER_INODE 1
#define DBLKS 50

#define FREEMAP_BLKS 1
#define INODE_BLKS 1

#define FS_SIZE (INODE_BLKS + FREEMAP_BLKS + DBLKS) * BLK_SIZE


#define MAX_NO_OF_OPEN_FILES 10

#define TEST

//helper functions 

int initialise_inodes(inode* i);
int initialise_freemap(int* map);
inode* next_inode(inode *i);
int next_datablock(int *freemap);
void find_inode(const char* path, inode **ino);
void allocate_inode(char *path, inode **ino, bool dir);
void print_inode(inode *i);
directory_record* get_dirent(char *path);


char *fs;												// The start of the FileSystem in the memory
inode *inodes;											// The start of the inode block
int *freemap;											// The start of the free-map block
char *datablks;											// The start of the data_blocks
FILE_REGISTER open_table[MAX_NO_OF_OPEN_FILES]; // The open file array

directory_record *root; // Address of the block representing the root directory
