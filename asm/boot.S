.section ".text.boot"

.global _start

_start:
    // Check processor ID is 0 (primary core), else loop
    mrs     x0, mpidr_el1
    and     x0, x0, #0xFF
    cbz     x0, master
    // CPU ID > 0, stop
halt:
    wfe
    b       halt

master:
    // Set up stack pointer
    ldr     x0, =_start
    mov     sp, x0

    // Clear BSS
    ldr     x0, =__bss_start
    ldr     x1, =__bss_end
    bl      clear_bss

    // Jump to C code
    bl      kernel_main

    // If kernel_main returns, halt
    b       halt

// Clear BSS section
clear_bss:
    cmp     x0, x1
    b.ge    clear_bss_end
    str     xzr, [x0], #8
    b       clear_bss
clear_bss_end:
    ret