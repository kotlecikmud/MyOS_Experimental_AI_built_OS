OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kernel_main)

PHDRS
{
    requests PT_LOAD FLAGS((1 << 1) | (1 << 2));             /* Read, Write */
    text     PT_LOAD FLAGS((1 << 0) | (1 << 2));             /* Read, Execute */
    rodata   PT_LOAD FLAGS((1 << 2));                         /* Read */
    data     PT_LOAD FLAGS((1 << 1) | (1 << 2));             /* Read, Write */
}

SECTIONS
{
    . = 0xffffffff80000000;

    .requests : {
        KEEP(*(.requests_start_marker))
        KEEP(*(.requests))
        KEEP(*(.requests_end_marker))
    } :requests

    . = ALIGN(4K);

    .text : {
        *(.text .text.*)
    } :text

    . = ALIGN(4K);

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    . = ALIGN(4K);

    .data : {
        *(.data .data.*)
    } :data

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /DISCARD/ : {
        *(.note.*)
        *(.comment)
        *(.eh_frame .eh_frame_hdr)
    }
}
