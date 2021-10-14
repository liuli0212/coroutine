// Author: liuli
//
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "croutine.h"

void swtch(struct Task* old, struct Task* new);

static struct Task g_tasks[MAX_TASKS];
static int g_max_taskid = -1;

// Wrapper entry point for all coroutines.
// It's stack should have been arranged.
static void task_entry(struct Task* task) {
    task->entry(task, task->arg);
    //
    // Done. Finished task won't be scheduled again.
    task->state = FINISHED;
    yield(task->id);

    // Shouldn't come here anymore.
    assert(0);
}

static void recycle_task(struct Task* task) {
    // Not strictly right.
    if (g_max_taskid == task->id) g_max_taskid--;

    task->state = UNUSED;
    task->id = -1;
    task->sp = task->bp = NULL;
}

//  Prepare stack for coroutine's entry on its private stack.
//  IMPORTANT: Stack should be the same as swtch function uses, high to low.
//  |0          |
//  |task_entry  |
//  |arg        | -> poped into %rdi
//  |r12        |
//  |r13        |
//  |r14        |
//  |r15        |
//  |rbx        |
//  |rbp        | <- task->sp
static void make_stack(struct Task* task) {
    void* sp = task->sp;
    sp -= sizeof(void*);
    *(void **)(sp) = (void *)0;
    sp -= sizeof(void*);
    *(void **)(sp) = (void *)(task_entry);
    sp -= sizeof(void*);
    *(void **)(sp) = (void *)(task);

    static const int REGS = 6;
    sp -= REGS * sizeof(void*);
    memset(sp, 0, REGS * sizeof(void*));

    task->sp = sp;
}

struct Task* GetTask(Func f, void* arg) {
    for (int i = 0; i < MAX_TASKS; ++i) {
        if (g_tasks[i].state != UNUSED) {
            continue;
        }

        struct Task* task = &g_tasks[i];
        task->state = UNINITED;
        task->id = i;

        static const int STACK_SIZE = 1024*1024*2;
        if (task->stack == NULL)
            task->stack = (void*)malloc(STACK_SIZE);
        task->sp = task->bp = task->stack + STACK_SIZE;
        printf("sp: 0x%p, stack: 0x%p\n", task->sp, task->stack);

        task->entry = f;
        task->arg = arg;

        if (i > g_max_taskid) {
            g_max_taskid = i;
        }

        make_stack(task);
        return task;
    }
    fprintf(stderr, "Failed to find task slot.\n");
    return NULL;
}

void yield(int cur_id) {
    // g_tasks[0] is the main task, always in state READY.
    int next = 0;

    // Implements a random picking strategy
    // TODO(liuli): Fix this... it is not right.
    int start_id = random() % (g_max_taskid + 1) + 1;
    for (int i = start_id; i <= g_max_taskid; ++i) {
        if (g_tasks[i].state == READY) {
            next = i;
            break;
        } else if (g_tasks[i].state == FINISHED) {
            recycle_task(&g_tasks[i]);
        }
    }

    if (next == cur_id) return;

    struct Task* nextTask = &g_tasks[next];
    struct Task* curTask = &g_tasks[cur_id];
    swtch(curTask, nextTask);
}

int claim_main_task() {
    if (g_max_taskid >= 0) return 1;
    memset(&g_tasks[0], 0, sizeof(g_tasks[0]));
    g_tasks[0].state = READY;
    g_tasks[0].id = 0;

    g_max_taskid = 0;
    return 0;
}
