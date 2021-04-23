.data
strnn:
.asciz "\n\n"
.align 8

.text
.globl main
lbl_fun_sum: 
pushq %rbp
movq %rsp, %rbp
addq $16, %rbp
subq $32, %rsp
movq %rdi, -32(%rbp)
movq %rsi, -40(%rbp)
movq -32(%rbp), %rax
movq -40(%rbp), %rbx
addq %rbx, %rax
movq %rax, -24(%rbp)
movq -24(%rbp), %rax
jmp lbl_0
lbl_0: addq $32, %rsp
popq %rbp
retq
lbl_fun_difference: 
pushq %rbp
movq %rsp, %rbp
addq $16, %rbp
subq $32, %rsp
movq %rdi, -32(%rbp)
movq %rsi, -40(%rbp)
movq -32(%rbp), %rax
movq -40(%rbp), %rbx
subq %rbx, %rax
movq %rax, -24(%rbp)
movq -24(%rbp), %rax
jmp lbl_1
lbl_1: addq $32, %rsp
popq %rbp
retq
lbl_fun_sum_difference_product: 
pushq %rbp
movq %rsp, %rbp
addq $16, %rbp
subq $64, %rsp
movq %rdi, -48(%rbp)
movq %rsi, -56(%rbp)
movq %rdx, -64(%rbp)
movq %rcx, -72(%rbp)
movq -48(%rbp), %rdi
movq -56(%rbp), %rsi
callq lbl_fun_sum
movq %rax, -24(%rbp)
movq -64(%rbp), %rdi
movq -72(%rbp), %rsi
callq lbl_fun_difference
movq %rax, -32(%rbp)
movq -24(%rbp), %rax
movq -32(%rbp), %rbx
imulq %rbx
movq %rax, -40(%rbp)
movq -40(%rbp), %rax
jmp lbl_2
lbl_2: addq $64, %rsp
popq %rbp
retq
main:
pushq %rbp
movq %rsp, %rbp
addq $16, %rbp
subq $48, %rsp
movq $10, %rax
movq %rax, -24(%rbp)
movq $20, %rax
movq %rax, -32(%rbp)
movq $30, %rax
movq %rax, -40(%rbp)
movq $15, %rax
movq %rax, -48(%rbp)
movq $10, %rdi
callq printByte
movq $10, %rdi
movq $20, %rsi
movq $30, %rdx
movq $15, %rcx
callq lbl_fun_sum_difference_product
movq %rax, -56(%rbp)
movq -56(%rbp), %rdi
callq printInt
movq $strnn, %rdi
callq printString
lbl_3: addq $48, %rsp
popq %rbp
retq
