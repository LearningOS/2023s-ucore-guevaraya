#include "file.h"
#include "defs.h"
#include "fcntl.h"
#include "fs.h"
#include "proc.h"

//This is a system-level open file table that holds open files of all process.
struct file filepool[FILEPOOLSIZE];

//Abstract the stdio into a file.
struct file *stdio_init(int fd)
{
	struct file *f = filealloc();
	f->type = FD_STDIO;
	f->ref = 1;
	f->readable = (fd == STDIN || fd == STDERR);
	f->writable = (fd == STDOUT || fd == STDERR);
	return f;
}

//The operation performed on the system-level open file table entry after some process closes a file.
void fileclose(struct file *f)
{
	if (f->ref < 1)
		panic("fileclose");
	if (--f->ref > 0) {
		return;
	}
	switch (f->type) {
	case FD_STDIO:
		// Do nothing
		break;
	case FD_INODE:
		iput(f->ip);
		break;
	default:
		panic("unknown file type %d\n", f->type);
	}

	f->off = 0;
	f->readable = 0;
	f->writable = 0;
	f->ref = 0;
	f->type = FD_NONE;
}

//Add a new system-level table entry for the open file table
struct file *filealloc()
{
	for (int i = 0; i < FILEPOOLSIZE; ++i) {
		if (filepool[i].ref == 0) {
			filepool[i].ref = 1;
			return &filepool[i];
		}
	}
	return 0;
}

//Show names of all files in the root_dir.
int show_all_files()
{
	errorf("%s", __func__);
	return dirls(root_dir());
}

//Create a new empty file based on path and type and return its inode;
//if the file under the path exists, return its inode;
//returns 0 if the type of file to be created is not T_file
static struct inode *create(char *path, short type)
{
	struct inode *ip, *dp;
	debugf("%s", __func__);
	dp = root_dir(); //Remember that the root_inode is open in this step,so it needs closing then.
	ivalid(dp);
	if ((ip = dirlookup(dp, path, 0)) != 0) {
		warnf("create a exist file\n");
		iput(dp); //Close the root_inode
		ivalid(ip);
		if (type == T_FILE && ip->type == T_FILE)
			return ip;
		iput(ip);
		return 0;
	}
	if ((ip = ialloc(dp->dev, type)) == 0)
		panic("create: ialloc");

	tracef("create dinode and inode type = %d\n", type);

	ivalid(ip);
	iupdate(ip);
	if (dirlink(dp, path, ip->inum) < 0)
		panic("create: dirlink");

	iput(dp);
	return ip;
}

//A process creates or opens a file according to its path, returning the file descriptor of the created or opened file.
//If omode is O_CREATE, create a new file
//if omode if the others,open a created file.
int fileopen(char *path, uint64 omode)
{
	int fd;
	struct file *f;
	struct inode *ip;
	debugf("%s", __func__);
	if (omode & O_CREATE) {
		ip = create(path, T_FILE);
		if (ip == 0) {
			return -1;
		}
	} else {
		if ((ip = namei(path)) == 0) {
			return -1;
		}
		ivalid(ip);
	}
	if (ip->type != T_FILE)
		panic("unsupported file inode type\n");
	if ((f = filealloc()) == 0 ||
	    (fd = fdalloc(f)) <
		    0) { //Assign a system-level table entry to a newly created or opened file
		//and then create a file descriptor that points to it
		if (f)
			fileclose(f);
		iput(ip);
		return -1;
	}
	// only support FD_INODE
	f->type = FD_INODE;
	f->off = 0;
	f->ip = ip;
	f->readable = !(omode & O_WRONLY);
	f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
	if ((omode & O_TRUNC) && ip->type == T_FILE) {
		itrunc(ip);
	}
	return fd;
}
static char * base_name(char * path, char ** filename)
{
	char * p_end = path + strlen(path)-1;
	int count = 0;
	debugf("path:%s path_addr:%x", path, path);
	while(*p_end != '\\' && p_end != path){
		debugf("p_end:0x%x",p_end);	
		p_end--;
		count++;
	}
	if(p_end == path){
		*filename = path;
		return NULL;
	}else
	{
		*filename = p_end+1;
		*p_end = '\0';
		return path;
	}
}
int filelink(char *srcpath, char * linkpath, uint64 flags)
{
	struct inode *ip, *dp;
	char * basename, *filename;
	debugf("%s", __func__);
	if(0 == strncmp(linkpath, srcpath, 200))
		return -1;
	if ((ip = namei(srcpath)) == 0)
	{
		errorf("%s not found %s", __func__, srcpath);
		return -1;
	}

	if((basename = base_name(linkpath, &filename)) == NULL){
		dp = root_dir();
	}else{
		dp = namei(basename);
	}
	ivalid(ip);
	ivalid(dp);
	//dirls(dp);
	//iupdate(ip);
	if (dirlink(dp, linkpath, ip->inum) < 0){
		panic("%s: fail dirlink", __func__);
	}
	//dirls(dp);
	iput(dp);
	iput(ip);
	return 0;
}
int fileunlink(char * linkpath, uint64 flags)
{
	struct inode *dp;
	char basename[200], * filename;
	debugf("%s: unlink:%s", __func__, linkpath);

	strncpy(basename, linkpath, strlen(linkpath)+1);
	if(base_name(basename, &filename) == NULL){
		dp = root_dir();
	}else{
		dp = namei(basename);
	}
	ivalid(dp);
	if (dirunlink(dp, filename) < 0){
		panic(" %s filelink: fail dirlink", __func__);
	}
	iput(dp);
	return 0;
}
// Write data to inode.
uint64 inodewrite(struct file *f, uint64 va, uint64 len)
{
	int r;
	debugf("%s", __func__);
	ivalid(f->ip);
	if ((r = writei(f->ip, 1, va, f->off, len)) > 0)
		f->off += r;
	return r;
}

//Read data from inode.
uint64 inoderead(struct file *f, uint64 va, uint64 len)
{
	int r;
	debugf("%s", __func__);
	ivalid(f->ip);
	if ((r = readi(f->ip, 1, va, f->off, len)) > 0)
		f->off += r;
	return r;
}
