        global    find_max
        section   .text
find_max:
        push    rbp
        mov     rbp, rsp
        jmp     .L2
.L4:
        cdqe
        add     rax, rdx
        cdqe
        add     rax, rdx
.L2:
        jl      .L4
        pop     rbp
        ret