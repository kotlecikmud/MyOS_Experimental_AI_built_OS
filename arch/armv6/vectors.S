.section .text
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
