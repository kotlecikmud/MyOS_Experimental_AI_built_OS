.syntax unified
.cpu arm1176jzf-s
.arm

.section .text.boot, "ax"
.global _start
.type _start, %function
_start:
    cpsid if
    ldr sp, =__stack_top
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
1:
    cmp r0, r1
    strlo r2, [r0], #4
    blo 1b
    bl kernel_main
2:
    wfi
    b 2b
.size _start, . - _start

.section .text, "ax"
.align 5
.global armv6_vector_table
armv6_vector_table:
    ldr pc, reset_addr
    ldr pc, undef_addr
    ldr pc, swi_addr
    ldr pc, prefetch_addr
    ldr pc, data_addr
    ldr pc, reserved_addr
    ldr pc, irq_addr
    ldr pc, fiq_addr
reset_addr:     .word 0
undef_addr:     .word 0
swi_addr:       .word 0
prefetch_addr:  .word 0
data_addr:      .word 0
reserved_addr:  .word 0
irq_addr:       .word 0
fiq_addr:       .word 0
