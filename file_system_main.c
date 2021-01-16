int initialise_inodes(inode* i){
	for(int x = 0; x < N_INODES; x++){
		(i+x) -> used = false;
	}
	return 0;
}
int initialise_freemap(int* map){
	int x;
	*(map) = 1;
	*(map+1) = 1;
	*(map+2) = 1;	//inodes, bitmap, root directory entry
	for(x = 0; x < DBLKS; x++){
		*(map + x + 3) = 0;
	}
	*(map + x) = -1;
	return 0;
}
inode* next_inode(inode *i){
	int x = 0;
	while(x<N_INODES && (i+x)->used==true)
	{
		x++;
	}
	if(x==N_INODES)
		return NULL;
	(i+x)->used = true;
	return i+x;
}
int next_datablock(int *freemap){
	int x = 0;
	while(x<DBLKS && freemap[x+2]==1)
		x++;
	if(x==DBLKS)
		return -1;
	freemap[x+2] = 1;
	return x+2;
}
void allocate_inode(char *path, inode **ino, bool dir){
	*ino = next_inode(inodes);
	(*ino) -> id = rand() % 10000;
	(*ino) -> size = 0;
	(*ino) -> data = ( datablks + (next_datablock(freemap)*BLK_SIZE));
	(*ino) -> directory = dir;
	(*ino) -> last_accessed = 0;
	(*ino) -> last_modified = 0;
	(*ino) -> link_count = 2;
	(*ino) -> children = 0;
}
void print_inode(inode *i){
	printf("used : %d\n", i -> used);
	printf("id : %d\n", i -> id);
	printf("size : %d\n", i -> size);
	printf("data : %u\n", i -> data);
	printf("directory : %d\n", i -> directory);
	printf("last_accessed : %d\n", i -> last_accessed);
	printf("last_modified : %d\n", i -> last_modified);
	printf("link_count : %d\n", i -> link_count);
}
directory_record* get_dirent(char *path)
{
	if(strcmp(path, '/')==0)
		return NULL;
	char *prevtok = strtok(path, '/');
	char *tok = strtok(NULL, '/');
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
			return NULL;

		prevtok = tok;
		tok = strtok(NULL, '/');
	}
	return rel_root;

}
int main(int argc, char *argv[])
{
	fs = calloc(1, FS_SIZE);

	inodes = (inode *)fs;
	freemap = (int *)(fs + BLK_SIZE);
	datablks = (char *)(fs + 2*BLK_SIZE);

	printf("fs = %u\n", fs);
  	printf("inodes = %u\n", inodes);
  	printf("freemap = %u\n", freemap);
  	printf("datablks = %u\n", datablks);

	initialise_inodes(inodes);
	initialise_freemap(freemap);

	root = (directory_record *)datablks;

	(root -> filename) = (char *)malloc(10);
	strcpy(root -> filename, "ROOT");

	root -> file_inode = next_inode(inodes);
	root -> children = 0;
	(root -> file_inode) -> id = 1;
	(root -> file_inode) -> size = 30;
	(root -> file_inode) -> data = ( datablks + (next_datablock(freemap)*BLK_SIZE));
	(root -> file_inode) -> directory = true;
	(root -> file_inode) -> last_accessed = 0;
	(root -> file_inode) -> last_modified = 0;
	(root -> file_inode) -> link_count = 1;
	printf("Created Root\n");
	file_sys_create("/new", 0, 0);
	file_sys_write("/new", "HELLO", 5, 0, NULL);
	char* buff = (char*)malloc(6);
	file_sys_read("/new", buff, 5, 0, NULL);
	printf("%s\n", buff);
	return fuse_main(argc, argv, &file_sys_oper, NULL);
}
void find_inode(const char* path, inode **ino){

	if(strcmp("/", path) == 0){
		*ino = (root->file_inode);
	}
	else{
		char *path_copy = (char*)malloc(strlen(path)+1);
		strcpy(path_copy, path);
		char *token = strtok(path_copy, "/");
		directory_record *temp = root;
    while (token != NULL)
    {
    		int i = 0;
    		int children = temp->children;
    		if((temp -> file_inode) -> directory){
					temp = (directory_record *)((temp -> file_inode) -> data);
			while(i<children && (temp -> filename != NULL) && (strcmp(temp -> filename, token) != 0)){
				temp++;
				i++;
			}
			if(i>=children || (temp -> filename) == NULL){
				#ifdef TEST
				printf("NO SUCH PATH: %s\n", path);
				#endif
				*ino = NULL;
				return;
			}
			else{
				*ino = (temp -> file_inode);
				}
			}
			token = strtok(NULL, "/");
		}
	}
}