/*
 * Kernel/boot.S - Kernel Entry Point for HailOS
 */
.section .boot
.global _start

/*
 * _start is the entry point called by FrostLoader.
 * FrostLoader passes a pointer to bootinfo_t in the RDI register
 * (first argument according to System V AMD64 ABI).
 */
_start:
    /* 1. Set up a stack.
     * We'll use a simple statically allocated stack in the .bss section for now.
     * The kernel_stack_top symbol will be defined at the end of this file.
     * Make sure the stack grows downwards.
     */
    movabs $kernel_stack_top, %rsp

    /* 2. Clear Direction Flag (DF) in RFLAGS.
     * Good practice for C functions that use string operations.
     */
    cld

    /* 3. Zero the .bss section. (Handled by FrostLoader) */
    /*
    extern _kernel_bss_start, _kernel_bss_end // This would also be an error; use .global for assembly symbols if needed for export
    movabs $_kernel_bss_start, %rdi
    movabs $_kernel_bss_end, %rsi
    sub %rdi, %rsi          // rsi = length of bss
    xor %al, %al            // al = 0
    rep stosb               // Zero BSS
    */

    /* 4. The pointer to bootinfo_t is already in RDI (passed by FrostLoader).
     * We just need to call our C main function.
     * The linker will find the 'main' symbol from your C object files.
     */
    // extern main /* REMOVE THIS LINE */
    call main /* This is void main(bootinfo_t*) in hailkrnl.c */

    /* 5. If main returns (it shouldn't for a kernel), halt the system. */
hang_loop:
    cli /* Disable interrupts */
    hlt /* Halt the processor */
    jmp hang_loop

/*
 * Define a stack in the .bss section.
 * This stack will be automatically zeroed if FrostLoader handles .bss based on p_memsz.
 * Size: 16KB. Adjust as needed.
 * The stack grows downwards, so kernel_stack_top is the initial RSP value.
 */
.section .bss
.align 16 /* Ensure stack is 16-byte aligned for ABI compatibility */
kernel_stack_bottom:
    .skip 4096 * 4  /* 16KB stack (4 pages) */
kernel_stack_top:
    /* RSP will be set to this address, which is the top of the allocated stack space */