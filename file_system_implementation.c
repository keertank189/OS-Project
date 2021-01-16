#include<file_system_header.h>


static void* file_sys_init(struct fuse_conn_info *conn,struct fuse_config *cfg)
{
	printf("File System is initializing\n");
	return NULL;
}

static int file_sys_open(const char *file_path, struct fuse_file_info *fi)
{

	/*opens a file by returning it's corresponding inode, based off the file_path provided, 
	and returns -1 if file cannot be found*/
	#ifdef TEST
	printf("Opening File - %s\n", file_path);
	#endif
	inode *file_inode;
	find_inode(file_path, &file_inode);		
	printf("inode returned = %u\n", file_inode);
	if(file_inode == NULL){
		return -1;
	}
	#ifdef TEST
	printf("Successful open\n");
	#endif
	return 0;
}

static int file_sys_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{
	#ifdef TEST
	printf("\nRead Called, Path: %s, ", path);
	#endif
	inode *ino;
	find_inode(path, &ino);
	size_t len;
	(void) fi;
	if(ino == NULL)
		return -ENOENT;

	len = ino->size;
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, (ino -> data) + offset, size);
	} else
		size = 0;

	return size;
}

static int file_sys_write(const char *path, const char *buf, size_t size,off_t offset, struct fuse_file_info *fi)
{
	#ifdef TEST
	printf("\nWrite called, Path = %s, buffer: %s\n", path, buf);
	#endif

	inode* ino;
	find_inode(path, &ino);
	if(ino==NULL)
		return -1;

	memcpy((ino -> data + offset), (buf), size);
	ino -> size = ((ino -> size) + size);
	return 0;
}

static int file_sys_create(const char *path_orig, mode_t mode,struct fuse_file_info *fi)
{
	printf("\nCreate : %s\n", path_orig);

	char *path = (char*)malloc(strlen(path_orig)+1);
	strcpy(path, path_orig);
	if(strcmp(path, "/")==0)
		return -1;
	char *prevtok = strtok(path, "/");		//tokenize the first part of the path
	char *tok = strtok(NULL, "/");
	directory_record *rel_root = (directory_record*)root;
	while(tok!=NULL)
	{
		int x = rel_root->children;
		rel_root = (directory_record*)(rel_root->file_inode->data);
		while(x!=0 && (rel_root -> filename != NULL) && (strcmp(rel_root -> filename, prevtok) != 0))
		{
			rel_root++;
			x--;
		}
		if(x==0 || (rel_root->filename==NULL))
			return -1;

		prevtok = tok;
		tok = strtok(NULL, "/");
	}

	if(rel_root==NULL || rel_root->file_inode->directory==false)
		return -1;
	directory_record *newentry = ((directory_record*)rel_root->file_inode->data)+rel_root->children;
	rel_root->children++;
	rel_root->file_inode->children++;
	newentry->children = 0;
	allocate_inode(path, &(newentry->file_inode), false);
	newentry->filename = (char*)malloc(15);
	strcpy(newentry->filename, prevtok);
	return 0;
}

static int file_sys_getattr(const char *path, struct stat *stbuf,
		       struct fuse_file_info *fi)
{
	printf("\nGetAttr: %s\n", path);

	int result = 0;
	memset(stbuf, 0, sizeof(struct stat));

	inode *ino;
	find_inode(path, &ino);

	int directory_flag = -1;
	if(ino!=NULL)
		directory_flag = ino -> directory;
	if (directory_flag == 1) {
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
	}
	else if (directory_flag == 0){
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = ino -> size;
	}
	else{
		result = -ENOENT;
	}
	return result;
}