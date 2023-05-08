#include "syscall.h"
#include "defs.h"
#include "loader.h"
#include "timer.h"
#include "trap.h"
#include "syscall_ids.h"
#include "proc.h"

uint64 sys_write(int fd, uint64 va, uint len)
{
	debugf("sys_write fd = %d va = %x, len = %d", fd, va, len);
	if (fd != STDOUT)
		return -1;
	struct proc *p = curr_proc();
	char str[MAX_STR_LEN];
	int size = copyinstr(p->pagetable, str, va, MIN(len, MAX_STR_LEN));
	debugf("size = %d", size);
	for (int i = 0; i < size; ++i) {
		console_putchar(str[i]);
	}
	return size;
}

__attribute__((noreturn)) void sys_exit(int code)
{
	exit(code);
	__builtin_unreachable();
}

uint64 sys_sched_yield()
{
	yield();
	return 0;
}

uint64 sys_gettimeofday(TimeVal *val, int _tz) // TODO: implement sys_gettimeofday in pagetable. (VA to PA)
{
	TimeVal kTimeVal;
	struct proc *p = curr_proc();
	// YOUR CODE


	/* The code in `ch3` will leads to memory bugs*/

	uint64 cycle = get_cycle();
	kTimeVal.sec = cycle / CPU_FREQ;
	kTimeVal.usec = (cycle % CPU_FREQ) * 1000000 / CPU_FREQ;
	copyout(p->pagetable, (uint64)val, (char*)&kTimeVal,sizeof(TimeVal));
	return 0;
}

uint64 sys_sbrk(int n)
{
	uint64 addr;
        struct proc *p = curr_proc();
        addr = p->program_brk;
        if(growproc(n) < 0)
                return -1;
        return addr;	
}



// TODO: add support for mmap and munmap syscall.
// hint: read through docstrings in vm.c. Watching CH4 video may also help.
// Note the return value and PTE flags (especially U,X,W,R)
/*
* LAB1: you may need to define sys_task_info here
*/
int64 sys_task_info(TaskInfo * ti)
{	
	int offset_time;
	int ret;

	if(!ti)
		return -1;
	TaskInfo *ktaskinfo = &curr_proc()->ti;
	ret = copyout(curr_proc()->pagetable, (uint64)ti, (char*)ktaskinfo,sizeof(TaskInfo));
	offset_time = get_time()- ktaskinfo->time;
	ret = copyout(curr_proc()->pagetable,(uint64)&ti->time, (char*)&offset_time, sizeof(int)); 
	return ret;
}
uint64 sys_getpid()
{
	return curr_proc()->pid;
}
uint64 sys_mmap(void* start, unsigned long long len, int port, int flag, int fd)
{
	uint64 ret, left_size;
	char *mem;
	int xperm;
	if(!start)return -1;
	if(len > MAXVA)return -1;
	if(!PGALIGNED((uint64)start)) return -1;
	if((port&(~7)|| port == 0))return -1;
	xperm = PTE_U;
	if((port&1))xperm |=PTE_R;
	if((port&2))xperm |=PTE_W;
	if((port&4))xperm |=PTE_X;

	left_size = len;
	while(left_size>0)
	{
		mem = kalloc();
		if(mem == 0){
			return -1;
		}
		memset(mem, 0, PGSIZE);
		tracef("start[0x%x] mem: %x port:%x xperm:0x%x", start, (uint64)mem, port, xperm);
		ret = mappages(curr_proc()->pagetable, (uint64)start, PGSIZE, (uint64)mem, xperm);
		tracef("start[0x%x] ret:%d", start, ret);
		if (ret != 0) 
		{ 
			kfree(mem);
			return ret;
		}
		tracef("start[0x%x].pa = 0x%x\n", start, walkaddr(curr_proc()->pagetable, (uint64)start));
		if(left_size <= PGSIZE){

			return 0;
		}else{
			left_size -=PGSIZE;
			start +=PGSIZE;
		}
	}
	return 0;
}
uint64 sys_munmap(void* start, unsigned long long len)
{
	uint64 page_num;
	if(!start)return -1;
	if(len > MAXVA ||  len%PGSIZE != 0)return -1;
	if(!PGALIGNED((uint64)start)) return -1;


	page_num = (len-1)/PGSIZE+1;
	tracef("munmap start[%x] len:%x page_num:%x", start, len, page_num);
	uvmunmap(curr_proc()->pagetable, (uint64)start, page_num, 1);
	return 0;
}
extern char trap_page[];

void syscall()
{
	struct trapframe *trapframe = curr_proc()->trapframe;
	int id = trapframe->a7, ret;
	uint64 args[6] = { trapframe->a0, trapframe->a1, trapframe->a2,
			   trapframe->a3, trapframe->a4, trapframe->a5 };
	tracef("syscall %d args = [%x, %x, %x, %x, %x, %x]", id, args[0],
	       args[1], args[2], args[3], args[4], args[5]);
	/*
	* LAB1: you may need to update syscall counter for task info here
	*/
	curr_proc()->ti.syscall_times[id]++;
	switch (id) {
	case SYS_write:
		ret = sys_write(args[0], args[1], args[2]);
		break;
	case SYS_exit:
		sys_exit(args[0]);
		// __builtin_unreachable();
	case SYS_sched_yield:
		ret = sys_sched_yield();
		break;
	case SYS_gettimeofday:
		ret = sys_gettimeofday((TimeVal *)args[0], args[1]);
		break;
	case SYS_sbrk:
		ret = sys_sbrk(args[0]);
		break;
	/*
	* LAB1: you may need to add SYS_taskinfo case here
	*/
	case SYS_task_info:
		//errorf("syscall_time[%d]:%d before", id,  curr_proc()->ti.syscall_times[id]);
		ret = sys_task_info((TaskInfo*)args[0]);
		//errorf("syscall_time[%d]:%d after", id,   curr_proc()->ti.syscall_times[id]);
		break;
	case SYS_getpid:
		ret = sys_getpid();
		break;
	case SYS_mmap:
		ret = sys_mmap((void*)args[0], (unsigned long long)args[1], (int)args[2], 
												(int)args[3], (int)args[4]);
		break;
	case SYS_munmap:
		ret = sys_munmap((void*)args[0], (unsigned long long)args[1]);
		break;
	default:
		ret = -1;
		errorf("unknown syscall %d", id);
	}
	trapframe->a0 = ret;
	tracef("syscall ret %d", ret);
}
