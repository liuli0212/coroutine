// void swtch(struct Task* old, struct Task* new);
// offset 0 of struct Task is the sp, offset 8 is the bp.
//
.globl swtch
swtch:
    pushq %rdi
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq %rbx
    pushq %rbp

    movq %rdi, %rax  // old ptr
    movq %rsi, %rdx  // new ptr

    // Save old stack ptr to old task.
    movq %rsp, (%rdi)
    // This is not strictly necessary, but makes debugging easier.
    movq %rbp, 8(%rdi)

    // End of using old stack, now it looks like:
    // |    ....        |
    // | prev_contents  |
    // | ret addr       |
    // | rdi            |
    // | r12            |
    // | r13            |
    // | r14            |
    // | r15            |
    // | rbx            |
    // | rbp            |

    // load new stack ptr from new task.
    movq 8(%rsi), %rbp
    movq (%rsi), %rsp


    //
    // Now we are using the stack for new task. It is the same layout
    // as the current old stack, with different content.
    // Old stack will be freezed untiled being swtched to.
    //
    popq %rbp
    popq %rbx
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rdi

    // ret will return to the ret addr on the new stack.
    ret
