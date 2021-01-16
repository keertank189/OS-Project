#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>
#include <libgen.h>
#include <limits.h>
#include <math.h>

#define BLOCKSIZE 4096
#define N 100
#define PATH_MAX_1 100
#define MARKER ")"

typedef struct NODE{
   char name[PATH_MAX_1];
   struct NODE *children[N];
   struct stat statit;
   char *data;
}NODE;
NODE *root;
NODE *temp_node_cxt;
char names[100][100];
int count_names=0,j=0;
int count_compare=0;

NODE *newNode(char *name)
{
    NODE *temp = malloc(sizeof(NODE));
    strcpy(temp->name,name);
    for (int i = 0; i < N; i++)
        temp->children[i] = NULL;
    temp->statit.st_nlink=0;
    temp->statit.st_size = 0;
    temp->data=NULL;
    return temp;
}
void get_names(char *s)
{
	for(int i=0;i<100;i++)
		names[i][0]='\0';
	char *dir=malloc(sizeof(s));
	int k=1;
	strcpy(names[0],basename(s));
	strcpy(dir,dirname(s));
	while(strcmp(basename(dir),"/")!=0)
	{
		strcpy(names[k++],basename(dir));
		strcpy(dir,dirname(dir));
	}
	int i=0;
	while(names[i][0]!='\0')
	{
		i++;
		count_names++;
	}

}
void getNodecxt(NODE *root1,char *path)
{
	
	if(root1)
	{
		if(strcmp(root1->name,names[j])==0)
		{
			count_compare+=1;
			temp_node_cxt=root1;
			if(j>0)
			{
				getNodecxt(temp_node_cxt,names[--j]);
			}
			return;
		}
		else
		{
			for(int i=0;i<100;i++)
			{
				if(root1->children[i]!=NULL)
					getNodecxt(root1->children[i],path);
			}
		}
	}
}
int search(const char *s)
{
	temp_node_cxt=NULL;
	count_names=0;
	count_compare=0;
	char path_prevent[strlen(s)],path_prevent1[strlen(s)];
	strcpy(path_prevent,s);
	strcpy(path_prevent1,s);
	get_names(path_prevent);
        strcpy(names[count_names],"root");
	j=count_names;
	getNodecxt(root,s);
	if(strcmp(path_prevent1,"/")==0)
		strcpy(path_prevent1,"root");
	if(strcmp(temp_node_cxt->name,basename(path_prevent1))!=0)  
		return 0;
	if(strcmp(temp_node_cxt->name,basename(path_prevent1))==0 && count_compare==(j))
		return 0;
	return 1;
}
static int getattr_f( const char *path, struct stat *st )
{
	printf("---------------GETATTR FOR %s--------------\n", path);
	st->st_uid = getuid(); 
	st->st_gid = getgid(); 
	st->st_atime = time( NULL ); 
	st->st_mtime = time( NULL ); 
	if ( strcmp( path, "/" ) == 0 )
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; 
	}
	else if(path[0]=='/' && (path[1]=='.' || (path[1]=='a' && path[2]=='u')))
	{
		st->st_mode = S_IFDIR | 0777;
		st->st_nlink = 1;
	 	st->st_size = 1024;
	}
	else
	{
		if(path[0]=='/')
		{
			if(!search(path))
			{
				printf("---------------PATH NOT FOUND--------------\n");
				return -ENOENT;
			}
			else
			{
				NODE *cur=temp_node_cxt;
				st->st_mode = cur->statit.st_mode;
				st->st_nlink = cur->statit.st_nlink;
				st->st_size = cur->statit.st_size;
				st->st_blocks = cur->statit.st_blocks;
			}
		}

		
	}
	
	return 0;
}
static int chmod_f(const char *path,mode_t mode)
{
	if(!search(path))
	{
		return -ENOENT;
	}
	NODE *cur=temp_node_cxt;
	cur->statit.st_mode=mode;
	return 0;
}
static int mkdir_f(const char *path,mode_t mode)
{
	printf("--------------------MKDIR-------------------\n");
	int found=1;
	char path_prevent[strlen(path)];
	strcpy(path_prevent,path);
	if(!search(dirname(path_prevent)))
	{
		printf("----------------INVALID PATH----------------\n");
		found=0;
		return -ENOENT;
	}
	NODE *cur=temp_node_cxt;
	if(found==1)
	{
		printf("-----------------VALID PATH-----------------\n");
		for(int i=0;i<100;i++)
		{
			if(cur->children[i]==NULL)
			{
				cur->children[i]=newNode(basename(path));
				cur->children[i]->statit.st_mode=S_IFDIR | 0777;
				cur->children[i]->statit.st_nlink =2;
				cur->children[i]->statit.st_size=0;
				break;
			}
		}
	}
	else
		printf("----------------INVALID PATH----------------\n");
	for(int i=0;i<100;i++)
	{
		if(cur->children[i]!=NULL)
			printf("Child %d:%s\n",i+1,cur->children[i]->name);
	}
	
	return 0;
}
void traverse(NODE *root,void *buf,fuse_fill_dir_t filler)
{
    if (root)
    {
        for (int i = 0; i < N; i++)
		{
			if(root->children[i]!=NULL)
				filler(buf,root->children[i]->name,NULL,0);
			else
				break;
		}
	}
}
static int rmdir_f(const char *path)
{
	if(!search(path))
	{
		printf("----------------INVALID PATH----------------\n");
		return -ENOENT;
	}
	else
	{
		NODE *cur=temp_node_cxt;
		cur=NULL;
	}
	return 0;
}
static int mv_f(const char *path,const char *new)
{
	if(!search(path))
	{
		printf("----------------INVALID PATH----------------\n");
		return -ENOENT;
	}
	else
	{
		NODE *cur=temp_node_cxt;
		strcpy(cur->name,basename(new));
	}
	return 0;
}
static int mknod_f(const char *path,mode_t mode,dev_t dev)
{
	printf("-------------------MKNODE-------------------\n");
	char *path_prevent=malloc(sizeof(char)*strlen(path));
	strcpy(path_prevent,path);
	if(!search(dirname(path_prevent)))
	{
		return -ENOENT;
	}
	NODE *cur=temp_node_cxt;
	for(int i=0;i<N;i++)
	{
		if(cur->children[i]==NULL)
		{
			cur->children[i]=newNode(basename(path));
			cur->children[i]->statit.st_mode=mode;
			cur->children[i]->statit.st_rdev=dev;
			cur->children[i]->statit.st_size=1;
			cur->children[i]->statit.st_nlink=1;
			break;
		}

	}
	return 0;
}
static int readdir_f(const char *path,void *buf, fuse_fill_dir_t filler, off_t offset,struct fuse_file_info *fi)
{
	printf("---------------READDIR FOR %s--------------\n", path);
	if(strcmp(path,"/")==0)
		search(path);
	else
	{
		int found=1;
		if(!search(path))
		{
			found=0;
			return -ENOENT;
		}
		if(temp_node_cxt==NULL)
			printf("----------------INVALID PATH----------------\n");
		
	}
	
	NODE *cur=temp_node_cxt;
	int i=0;
	if(cur==NULL)
	{
		printf("----------------IDK LOL----------------\n");
		return 0;
	}
	/*else if(!S_ISDIR(cur->statit.st_mode))
	{
		printf("----what is happening-----");
		return -ENOTDIR;
	}*/
	else
	{

		traverse(cur,buf,filler);
	}
}

static int write_f(const char *path, const char *buf, size_t size, off_t offset,
		      struct fuse_file_info *f)
{
	printf("---------------WRITE FOR %s--------------\n", path);
	if(!search(path))
		return -ENOENT;
	NODE *cur=temp_node_cxt;
	if(offset<0)
		return 0;
	int l;
	if(cur->data!=NULL)
		l=strlen(cur->data);
	else 
		l=0;
	if((offset+size)>l)
	{
		cur->data=realloc(cur->data,sizeof(char)*(offset+size));
		cur->statit.st_size=offset+size;
                cur->statit.st_blocks += (offset+size)/4096;
		if((offset+size)%4096 != 0 )
			cur->statit.st_blocks +=1;
		printf("--------------SIZE: %d--------------\n",cur->statit.st_size);
		//exit(0);
	}
	int k=0;
	for(int i=offset;i<(offset+size);i++)
	{
		cur->data[i]=buf[k++];
	}
	printf("------DATA:%s------",cur->data);
	return size;
	
}

static int truncate_f(const char *path,size_t size)
{
	printf("-----------------TRUNCATE-----------------\n");
	printf("----size is:%lu----",size);
	return 0;
	
}

static int open_f(const char *path,struct fuse_file_info *f)
{
	printf("---------------OPEN FILE---------------\n");
	if(!search(path))
		return -ENOENT;
	printf("-------------FLAGS:%d-------------",f->flags);
	return 0;
}

static int read_f(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
	printf("-------------READ FOR %s-------------\n",path);
	if(!search(path))
	{
		return -ENOENT;
	}
	NODE *cur=temp_node_cxt;
	if(cur->data==NULL)
		return 0;
	//buf=malloc(sizeof(char)*strlen(cur->data));
	int l=strlen(cur->data);
	if(offset<0 || offset>l)
		return 0;
	else
	{
		//int l=strlen(cur->data);
		int flag=1;
		char s[l];
		strcpy(s,cur->data);
		printf("After write read: %s\n",cur->data);
		char t[l];
		int k=0;
		if(l<(offset+size))
			flag=0;
		if(flag==0)
		{
			printf("Data: %s\n",s);
			for(int i=offset;i<=l;i++)
				t[k++]=s[i];
			strcpy(buf,t);
			printf("l-offset:%d\n",l-offset);
			//fflush(buf);
			return (l-offset);
		}	
		else
		{
			for(int i=offset;i<=(offset+size);i++)
				t[k++]=s[i];
			strcpy(buf,t);
			return size;
		}
		
	}
	
}

static int unlink_f(const char *path)
{
	printf("-----------------REMOVE-----------------\n");
	if(!search(path))
	{
		return -ENOENT;
	}
	NODE *cur=temp_node_cxt;
	cur=NULL;
	return 0;
}

void serialize(NODE *root, FILE *fp)
{
    if(root == NULL) 
    	return;
    fprintf(fp, "%s", root->name);
    fprintf(fp,"%s","#");
   if(root->data!=NULL)
    	fprintf(fp,"%s",root->data);
    fprintf(fp,"%s",">");
    fwrite(&root->statit,sizeof(struct stat),1,fp);
    for (int i = 0; i < 100 && root->children[i]; i++)
         serialize(root->children[i],  fp);
    fprintf(fp, "%s", MARKER);
}

int deSerialize(NODE **root, FILE *fp)
{
	char *val=calloc(1,sizeof(char));
	char *data=calloc(1,sizeof(char));
	int i=0;
	val[i]=fgetc(fp);
	if(val[i]==EOF)
		return 1;
	if(val[i]==')')
		return 1;
	while(val[i])
	{
		if(val[i]=='#')
		{
			val[i]='\0';
			printf("%s\n",val);
			*root = newNode(val);
			break;
		}
		i+=1;
		val=realloc(val,strlen(val)+1);
		val[i]=fgetc(fp);
	}
	i=0;
	data[i]=fgetc(fp);
	while(data[i])
	{
		if(data[i]=='>')
		{
			data[i]='\0';
			printf("whattttt---%s\n",data);
			if(root!=NULL)
				(*root)->data=data;
			break;
		}
		i+=1;
		data=realloc(data,strlen(data)+1);
		data[i]=fgetc(fp);
 	}
    fread(&((*root)->statit),sizeof(struct stat),1,fp);
	if(*root!=NULL)
	{
	for (int i = 0; i < N; i++)
		if(deSerialize(&((*root)->children[i]), fp))
			break;
	}
	return 0;
}
static struct fuse_operations operations = {
    .mkdir= mkdir_f,
    .readdir = readdir_f,
    .getattr = getattr_f,
    .rmdir = rmdir_f,
    .rename = mv_f,
    .open = open_f,
    .read = read_f,
    .write = write_f,
    .truncate = truncate_f,
    .mknod = mknod_f,
    .chmod = chmod_f,
    .unlink = unlink_f,
};
int main(int argc,char *argv[])
{
	FILE *fd=fopen("vsfs_capture","r+");
	if(fd==NULL)
		perror("open");
	root = NULL;
	deSerialize(&root,fd);
	fd=freopen("vsfs_capture","r+",fd);
	if(root==NULL)
		root=newNode("root");
	umask(0);
	fuse_main( argc, argv, &operations, NULL );
	serialize(root,fd);
	return 0;
 

}

