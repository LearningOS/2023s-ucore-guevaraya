#问答作业
* 正确进入 U 态后，程序的特征还应有：使用 S 态特权指令，访问 S 态寄存器后会报错。请同学们可以自行测试这些内容（参考 前三个测例 ，描述程序出错行为，同时注意注明你使用的 sbi 及其版本。
[ERROR 0]unknown trap: 0x0000000000000007, stval = 0x0000000000000000 sepc = 0x0000000080400002
地址0 为非法地址，具体应该是Rustsbi和QEMU内置的，内核加载地址只能从0x80200000 开始
[ERROR 0]IllegalInstruction in application, epc = 0x0000000080400002, core dumped.
sret 属于S级命令返回命令 ，U态命令访问S态会出现错误的
[ERROR 0]IllegalInstruction in application, epc = 0x0000000080400002, core dumped.
x[rd] = CSRs[csr]  读取控制状态寄存器 需要特权模式
RustSBI version 0.3.0-alpha.2, adapting to RISC-V SBI v1.0.0
 请结合用例理解 trampoline.S 中两个函数 userret 和 uservec 的作用，并回答如下几个问题:
L79: 刚进入 userret 时，a0、a1 分别代表了什么值。
* uservec 和 userret 分布是进入trap 保存和恢复上下文的操作，其中a0代表系统调用的第一个参数a1 为系统调用的第二参数
返回a0 代表系统调用的返回值

L87-L88: sfence 指令有何作用？为什么要执行该指令，当前章节中，删掉该指令会导致错误吗？
* sfence 内存屏障指令，切换上下文可同步刷新虚拟地址的页表缓存TLB,此时删除不会导致错误，只会减少命中率，系统执行效率不高

csrw satp, a1
sfence.vma zero, zero
L96-L125: 为何注释中说要除去 a0？哪一个地址代表 a0？现在 a0 的值存在何处？
a0 需要作为返回值，TRAPFRAME等于a0，a0 保存到sscratch

# restore all but a0 from TRAPFRAME
ld ra, 40(a0)
ld sp, 48(a0)
ld t5, 272(a0)
ld t6, 280(a0)
userret：中发生状态切换在哪一条指令？为何执行之后会进入用户态？
sret 返回U态

L29： 执行之后，a0 和 sscratch 中各是什么值，为什么？
a0 : 代表TRAPFRAME, sscratch:代表用户空间上下文
csrrw a0, sscratch, a0
L32-L61: 从 trapframe 第几项开始保存？为什么？是否从该项开始保存了所有的值，如果不是，为什么？
第六项开始保存，前五项都是通用的，是的，
sd ra, 40(a0)
sd sp, 48(a0)
...
sd t5, 272(a0)
sd t6, 280(a0)
进入 S 态是哪一条指令发生的？
jr t0
L75-L76: ld t0, 16(a0) 执行之后，`t0`中的值是什么，解释该值的由来？
t0 代表 kernel_trap 
ld t0, 16(a0)
jr t0
