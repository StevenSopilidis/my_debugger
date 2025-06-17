.global main

.section .data
hex_format: .asciz "%#x" # directive for encoding string into ASCII with NULL termination
float_format: .asciz "%.2f"
long_float_format: .asciz "%.2Lf"

.section .text
.macro trap
    # Trap
    movq $62, %rax # kill syscall
    movq %r12, %rdi # pid of process to call kill on
    movq $5, %rsi # SIGTRAP
    syscall
.endm

main:
    push %rbp
    movq %rsp, %rbp

    # Get pid
    movq $39, %rax # getpid syscall
    syscall
    movq %rax, %r12 # save result of getpid syscall
    trap

    # Print contents of rsi
    leaq hex_format(%rip), %rdi
    movq $0, %rax
    call printf@plt
    movq $0, %rdi
    call fflush@plt # flush streams
    trap

    # Print content of mmo
    movq %mm0, %rsi
    leaq hex_format(%rip), %rdi
    movq $0, %rax
    call printf@plt
    movq $0, %rdi
    call fflush@plt
    trap

    # print content of xmm0
    leaq float_format(%rip), %rdi
    movq $1, %rax
    call printf@plt
    movq $0, %rdi
    call fflush@plt
    trap

    # print content of st0
    subq $16, %rsp # allocate 16 bytes on stack to store st0
    fstpt (%rsp) # pop st0 from top of FPU stack with the fstpt instruction
    leaq long_float_format(%rip), %rdi
    movq $0, %rax
    call printf@plt
    movq $0, %rdi
    call fflush@plt
    addq $16, %rsp # restore stack
    trap

    popq %rbp
    movq $0, %rax
    ret