.section .text
.align 11
.global aarch64_vector_table
aarch64_vector_table:
    // Current EL with SP0
    .align 7
    b .
    .align 7
    b .
    .align 7
    b .
    .align 7
    b .

    // Current EL with SPx
    .align 7
    b .
    .align 7
    b .
    .align 7
    b .
    .align 7
    b .

    // Lower EL using AArch64
    .align 7
    b .
    .align 7
    b .
    .align 7
    b .
    .align 7
    b .

    // Lower EL using AArch32
    .align 7
    b .
    .align 7
    b .
    .align 7
    b .
    .align 7
    b .
