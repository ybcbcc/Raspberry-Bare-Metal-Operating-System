# boot.S的汇编语言介绍
## mrs     x0, mpidr_el1
mrs：ARM 汇编指令，用于 从系统寄存器读取值到通用寄存器。  
mpidr_el1 只读寄存器
![image](https://github.com/user-attachments/assets/d1a469f0-f6cf-4ea9-bfd0-78655ad107d9)
- Aff0（bit 7:0） → 代表 核心编号（即 CPU ID）。
- Aff1（bit 15:8） → 代表 簇编号（Cluster ID，多个核心的分组）。
- Aff2（bit 23:16） → 更高层的分组，一般在更复杂的多核架构中才会用到。
- U（bit 30） → 0 表示是多核系统的一部分，1 表示单核系统。
- MT（bit 24） → 0 表示核心是独立的，1 可能表示 multithreading 或核心间高度依赖。
## and     x0, x0, #0xFF
这里的and是与的意思，取第八位，从上文中mpidr_el1寄存器的定义可以看出，低八位可以看出CPU的。
## cbz     x0, master
如果x0等于0，跳转master。
## 定义指针
```
    ldr     x0, =_start 
    mov     sp, x0
```
将指针移至程序开始位置，进行堆栈设置，这里有个问题，堆栈和汇编程序开始的位置重合，堆栈可能会覆盖汇编程序，考虑到和硬件结构有关，这个问题在之后解决，之后的内存分配应该考虑这个问题。
## 清空BSS段
```
    ldr     x0, =__bss_start
    ldr     x1, =__bss_end
    bl      clear_bss
```
BSS 段（.bss）用于存放未初始化的全局变量，需要清零。设置地址后，跳转清楚函数
## 跳转main函数
```
    // Jump to C code
    bl      kernel_main

    // If kernel_main returns, halt
    b       halt
```
跳转到主函数也就是kernel.c。如果主函数意外崩溃返回，进入到下面的等待函数死循环避免核心崩溃。

## 其他核心
```
halt:
    wfe
    b       halt
```
如果cbz不为0的核心就会进入等待的死循环，之后会介绍一种唤醒机制，帮助等待的核心唤醒来实现多线程。
