SECTIONS
{
    /* Kernel starts at 0x80000 for Raspberry Pi 4 */
    . = 0x80000;
    
    /* Ensure the entry point (_start) is at the beginning */
    .text : {
        KEEP(*(.text.boot))
        *(.text*)
    }
    
    /* Read-only data */
    .rodata : {
        *(.rodata*)
    }
    
    /* Read-write data (initialized) */
    .data : {
        *(.data*)
    }
    
    /* Read-write data (uninitialized) and stack */
    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }
   
   /* Align to 8-byte boundary */
   . = ALIGN(8);
   
   /* End of kernel image */
   __end = .;
}