# boot.S的汇编语言介绍
## 从_start开始程序
mrs：ARM 汇编指令，用于 从系统寄存器读取值到通用寄存器。  
mpidr_el1 只读寄存器
![image](https://github.com/user-attachments/assets/d1a469f0-f6cf-4ea9-bfd0-78655ad107d9)
- Aff0（bit 7:0） → 代表 核心编号（即 CPU ID）。
- Aff1（bit 15:8） → 代表 簇编号（Cluster ID，多个核心的分组）。
- Aff2（bit 23:16） → 更高层的分组，一般在更复杂的多核架构中才会用到。
- U（bit 30） → 0 表示是多核系统的一部分，1 表示单核系统。
- MT（bit 24） → 0 表示核心是独立的，1 可能表示 SMT（超线程）或核心间高度依赖。
