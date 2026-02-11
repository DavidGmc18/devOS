bits 32

extern main
global entry

section .entry
entry:
    jmp short start
    nop


section .start
start:
    jmp main