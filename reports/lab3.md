## stride 算法原理非常简单，但是有一个比较大的问题。例如两个 pass = 10 的进程，使用 8bit 无符号整形储存 stride， p1.stride = 255, p2.stride = 250，在 p2 执行一个时间片后，理论上下一次应该 p1 执行。
* 实际情况是轮到 p1 执行吗？为什么？
如果stride采用无符号8位存储，则p2.stide=250+10=4  p1.stride=255 这样下个还是p2 得到时间片
因此需要考虑stride的溢出情况 改进stide调度的算法,字段溢出范围的stride和实际stride是有区别的

## 我们之前要求进程优先级 >= 2 其实就是为了解决这个问题。可以证明，在不考虑溢出的情况下, 在进程优先级全部 >= 2 的情况下，如果严格按照算法执行，那么 STRIDE_MAX – STRIDE_MIN <= BigStride / 2。
* 为什么？尝试简单说明（传达思想即可，不要求严格证明）。
已知以上结论，在考虑溢出的情况下，假设我们通过逐个比较得到 Stride 最小的进程，请设计一个合适的比较函数，用来正确比较两个 Stride 的真正大小：

前置条件，fork和spawn新建的进程stride初始值是父进程的话，结合上面的结论 两个进程之前的stride相差最大为BigStride/2
```
typedef unsigned long long Stride_t;
const Stride_t BIG_STRIDE = 0xffffffffffffffffULL;
int cmp(Stride_t a, Stride_t b) {
    // YOUR CODE HERE
    if(a > b)
    {
    	((a - b) > BigStride/2)?-1:1 
    }else if(a < b)
    {

    	((b - a) > BigStride/2)?1:-1 
    }else{
	return 0;
    }

}

```

#实验过程中问题集锦
## 问题0

lab3 中的ch5_mergetest 中发现大量 Usertests: Running (null) 失败，可以尝试在user仓中CMakelists.txt 的C编译选项中添加

```
 -no-pie -fno-pie

```

## 问题1

```
W: GPG 错误：https://openmediavault-plugin-developers.github.io/packages/debian shaitan-testing InRelease: 由于没有公钥，无法验证下列签名                          ： NO_PUBKEY 326A835E697B890A
E: 仓库 “https://openmediavault-plugin-developers.github.io/packages/debian shaitan-testing InRelease” 没有数字签名。
N: 无法安全地用该源进行更新，所以默认禁用该源。
N: 参见 apt-secure(8) 手册以了解仓库创建和用户配置方面的细节。
make: *** [Makefile:31：ubuntu_setenv] 错误 100

```

解决方法：
apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv  326A8                          35E697B890A

riscv64-linux-musl-gcc

Using built-in specs.

COLLECT_GCC=riscv64-linux-musl-gcc

COLLECT_LTO_WRAPPER=/home/codespace/riscv64-linux-musl-cross/bin/../libexec/gcc/riscv64-linux-musl/10.2.1/lto-wrapper

Target: riscv64-linux-musl

Configured with: ../src_gcc/configure --enable-languages=c,c++,fortran CC='gcc -static --static' CXX='g++ -static --static' FC='gfortran -static --static' CFLAGS='-g0 -O2 -fno-align-functions -fno-align-jumps -fno-align-loops -fno-align-labels -Wno-error' CXXFLAGS='-g0 -O2 -fno-align-functions -fno-align-jumps -fno-align-loops -fno-align-labels -Wno-error' FFLAGS='-g0 -O2 -fno-align-functions -fno-align-jumps -fno-align-loops -fno-align-labels -Wno-error' LDFLAGS='-s -static --static' --enable-default-pie --enable-static-pie --disable-bootstrap --disable-assembly --disable-werror --target=riscv64-linux-musl --prefix= --libdir=/lib --disable-multilib --with-sysroot=/riscv64-linux-musl --enable-tls --disable-libmudflap --disable-libsanitizer --disable-gnu-indirect-function --disable-libmpx --enable-libstdcxx-time=rt --enable-deterministic-archives --enable-libstdcxx-time --enable-libquadmath --enable-libquadmath-support --disable-decimal-float --with-build-sysroot=/tmp/m1064/build/local/riscv64-linux-musl/obj_sysroot AR_FOR_TARGET=/tmp/m1064/build/local/riscv64-linux-musl/obj_binutils/binutils/ar AS_FOR_TARGET=/tmp/m1064/build/local/riscv64-linux-musl/obj_binutils/gas/as-new LD_FOR_TARGET=/tmp/m1064/build/local/riscv64-linux-musl/obj_binutils/ld/ld-new NM_FOR_TARGET=/tmp/m1064/build/local/riscv64-linux-musl/obj_binutils/binutils/nm-new OBJCOPY_FOR_TARGET=/tmp/m1064/build/local/riscv64-linux-musl/obj_binutils/binutils/objcopy OBJDUMP_FOR_TARGET=/tmp/m1064/build/local/riscv64-linux-musl/obj_binutils/binutils/objdump RANLIB_FOR_TARGET=/tmp/m1064/build/local/riscv64-linux-musl/obj_binutils/binutils/ranlib READELF_FOR_TARGET=/tmp/m1064/build/local/riscv64-linux-musl/obj_binutils/binutils/readelf STRIP_FOR_TARGET=/tmp/m1064/build/local/riscv64-linux-musl/obj_binutils/binutils/strip-new --build=x86_64-pc-linux-gnu --host=x86_64-pc-linux-gnu 'CC_FOR_BUILD=cc -static --static' 'CXX_FOR_BUILD=g++ -static --static'

Thread model: posix

Supported LTO compression algorithms: zlib

gcc version 10.2.1 20210227 (GCC)

## 问题2

cmake生成Makefile，make时不打印详细信息。

Makefile中有MAKESILENT变量。

###### 解决方法

在[CMakeLists](https://so.csdn.net/so/search?q=CMakeLists&spm=1001.2101.3001.7020).txt中加入set(CMAKE_VERBOSE_MAKEFILE 1)即可

[Cmakelists使用小记](https://blog.csdn.net/roujian0985/article/details/125077138)

**引用文献**

1、[CMake 基本用法介绍](https://zhjwpku.com/2019/11/15/cmake-basic-commands-intro.html)

2、 [CMakeList常用命令 - 简书](https://www.jianshu.com/p/be0f3c19e836)

[3、Android NDK开发之旅(5)：Android Studio中使用CMake进行NDK/JNI开发(高级)\_无名之辈FTER的博客-CSDN博客](https://blog.csdn.net/AndrExpert/article/details/82909572?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_title~default-1-82909572-blog-115343852.pc_relevant_default&spm=1001.2101.3001.4242.2&utm_relevant_index=3)

## 问题3

在完成CH5开始的时候，合入CH4修改的过程中发现：ch4_mmap0 系统退出的时候出现freewalk: leaf

经过分析，ch4_mmap0 这个程序做了mmap后，没有主动unmmap，因此程序下exit退出的时候发现存在叶子页面没有释放的问题，需要在exit里面主动释放，但是默认exit 只释放了5个页表，这个5个页表是在loader的时候初始化，没有包含用mmap的页表

这有两个方案：mmap的时候保存页面映射链表，另一个简单粗暴的方法是将proc->max_pages 扩大，这样uvmunmap的时候就直接全部释放；

```c
@@ -158,15 +158,17 @@ uint64 sys_mmap(void* start, unsigned long long len, int port, int flag, int fd)
                        return -1;
                }
                memset(mem, 0, PGSIZE);
                ret = mappages(curr_proc()->pagetable, (uint64)start, PGSIZE, (uint64)mem, xperm);
                if (ret != 0) 
                { 
                        kfree(mem);
                        return -1;
                }
+               max_npage = (uint64)(start)/PGSIZE + 1;
+               if(max_npage>curr_proc()->max_page)curr_proc()->max_page = max_npage; 
                if(left_size <= PGSIZE){
 
                        return 0;

```

## 问题4：ch5_usertest 运行过程中阻塞

问题现象是阻塞住，怀疑系统调度出了问题，尝试其他分析后，只能在调度进程上下文加打印了：

I am child 10

\[ERROR 65]front:24 tail:34

\[ERROR 65]index:24 value:9 pid:10 stride:81920

\[ERROR 65]index:25 value:14 pid:24 stride:77824

\[ERROR 65]index:26 value:0 pid:1 stride:77824

\[ERROR 65]index:27 value:33 pid:35 stride:77824

\[ERROR 65]index:28 value:32 pid:34 stride:77824

\[ERROR 65]index:29 value:12 pid:13 stride:81920

\[ERROR 65]index:30 value:39 pid:60 stride:77824

\[ERROR 65]index:31 value:11 pid:12 stride:77824

\[ERROR 65]index:32 value:7 pid:8 stride:77824

\[ERROR 65]index:33 value:1 pid:2 stride:81920

\[ERROR 65]index:34 value:4 pid:58 stride:73728

\[ERROR 65]find low_index:34, pid:4

\[ERROR 65]fetch task 4(pid=58, state:5) to task queue

task4(pid=58)获得调度进程后， index:34 理论上存储着 index:24 value:9 pid:10 从front移动过来

\[ERROR 58]yield: set runnable 58

\[ERROR 58]add task 4(pid=58) to task queue

tail 35 应该为task4(pid=58) 但下面实际为 **index:35 value:5 pid:6，并且pid:6的状态为 **ZOMBIE 说明已经执行完成，处于僵尸进程

\[ERROR 58]front:25 tail:35

\[ERROR 58]index:25 value:14 pid:24 stride:77824

\[ERROR 58]index:26 value:0 pid:1 stride:77824

\[ERROR 58]index:27 value:33 pid:35 stride:77824

\[ERROR 58]index:28 value:32 pid:34 stride:77824

\[ERROR 58]index:29 value:12 pid:13 stride:81920

\[ERROR 58]index:30 value:39 pid:60 stride:77824

\[ERROR 58]index:31 value:11 pid:12 stride:77824

\[ERROR 58]index:32 value:7 pid:8 stride:77824

\[ERROR 58]index:33 value:1 pid:2 stride:81920

\[ERROR 58]index:34 value:4 pid:58 stride:77824

**\[****\*\***\*\*\*\*\*\*\*\*ERROR 58]index:35 value:5 pid:6 stride:53248  # 是怎么加到runnable列表的？ \*\*

\[ERROR 58]find low_index:35, pid:5

**\[****\*\***\*\*\*\*\*\*\*\*ERROR 58]fetch task 5(pid=6, state:5) to task queue\*\*

\[ERROR 6]yield: set runnable 6

\[ERROR 6]add task 5(pid=6) to task queue

\[ERROR 6]front:26 tail:36

\[ERROR 6]index:26 value:0 pid:1 stride:77824

\[ERROR 6]index:27 value:33 pid:35 stride:77824

\[ERROR 6]index:28 value:32 pid:34 stride:77824

\[ERROR 6]index:29 value:12 pid:13 stride:81920

\[ERROR 6]index:30 value:39 pid:60 stride:77824

\[ERROR 6]index:31 value:11 pid:12 stride:77824

\[ERROR 6]index:32 value:7 pid:8 stride:77824

\[ERROR 6]index:33 value:1 pid:2 stride:81920

\[ERROR 6]index:34 value:4 pid:58 stride:77824

\[ERROR 6]index:35 value:5 pid:6 stride:57344

\[ERROR 6]index:36 value:0 pid:1 stride:77824

\[ERROR 6]find low_index:35, pid:5

\[ERROR 6]fetch task 5(pid=6, state:3) to task queue

## 问题5 CI失败

```
2023-05-18T09:07:36.5736291Z    12 |  uint64 origin_brk = sbrk(0);
2023-05-18T09:07:36.5737160Z       |                      ^~~~
2023-05-18T09:07:38.9033746Z /__w/2023s-ucore-guevaraya/2023s-ucore-guevaraya/ucore-tutorial-ci/workplace/user/src/ch6b_filetest_simple.c: In function ‘main’:
2023-05-18T09:07:38.9034656Z /__w/2023s-ucore-guevaraya/2023s-ucore-guevaraya/ucore-tutorial-ci/workplace/user/src/ch6b_filetest_simple.c:18:6: warning: unused variable ‘read_len’ [-Wunused-variable]
2023-05-18T09:07:38.9035519Z    18 |  int read_len = read(fd, buf, 16);
2023-05-18T09:07:38.9035750Z       |      ^~~~~~~~
2023-05-18T09:07:39.2297071Z /__w/2023s-ucore-guevaraya/2023s-ucore-guevaraya/ucore-tutorial-ci/workplace/user/src/ch8_sem1_deadlock.c: In function ‘main’:
2023-05-18T09:07:39.2297913Z /__w/2023s-ucore-guevaraya/2023s-ucore-guevaraya/ucore-tutorial-ci/workplace/user/src/ch8_sem1_deadlock.c:88:45: warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]
2023-05-18T09:07:39.2298421Z    88 |   threads[i] = thread_create(deadlock_test, (void *)i);
2023-05-18T09:07:39.2298698Z       |                                             ^
2023-05-18T09:07:47.2484250Z make: *** [Makefile:40: test] Error 1
2023-05-18T09:07:47.2554426Z ##[error]Process completed with exit code 2.

```

在训练营群里沟通，是目录下没有reports的原因 详细参照指导书https://ucore-rv-64.github.io/uCore-RV-64-doc/index.html

