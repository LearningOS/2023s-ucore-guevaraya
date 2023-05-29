#ifndef FILE_H
#define FILE_H

#include "fs.h"
#include "proc.h"
#include "types.h"

#define PIPESIZE (512)
#define FILEPOOLSIZE (NPROC * FD_BUFFER_SIZE)

// in-memory copy of an inode,it can be used to quickly locate file entities on disk
struct inode {
	uint dev; // Device number
	uint inum; // Inode number
	int ref; // Reference count
	int valid; // inode has been read from disk?
	short type; // copy of disk inode
	uint size;
	uint addrs[NDIRECT + 1];
	// LAB4: You may need to add link count here
	uint nlink;
};

// Defines a file in memory that provides information about the current use of the file and the corresponding inode location
struct file {
	enum { FD_NONE = 0, FD_INODE, FD_STDIO } type;
	int ref; // reference count
	char readable;
	char writable;
	struct inode *ip; // FD_INODE
	uint off;
};
struct Stat {
	uint64 dev;     // 文件所在磁盘驱动号，该实现写死为 0 即可。
	uint64 ino;     // inode 文件所在 inode 编号
	uint mode;    // 文件类型
	uint nlink;   // 硬链接数量，初始为1
	uint64 pad[7];  // 无需考虑，为了兼容性设计
};

// 文件类型只需要考虑:
#define DIR 0x040000              // directory
#define FILE 0x100000             // ordinary regular file

//A few specific fd
enum {
	STDIN = 0,
	STDOUT = 1,
	STDERR = 2,
};

extern struct file filepool[FILEPOOLSIZE];

void fileclose(struct file *);
struct file *filealloc();
int fileopen(char *, uint64);
uint64 inodewrite(struct file *, uint64, uint64);
uint64 inoderead(struct file *, uint64, uint64);
struct file *stdio_init(int);
int show_all_files();
int filelink(char *srcpath, char * dstpath, uint64 flags);
int fileunlink(char * dstpath, uint64 flags);
#endif // FILE_H
