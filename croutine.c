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

static struct Task gTasks[MAX_TASKS];
static int gMaxTaskId = -1;

// Wrapper entry point for all coroutines.
// It's stack should have been arranged.
static void TaskEntry(struct Task* task) {
    task->entry(task, task->arg);
    //
    // Done. Finished task won't be scheduled again.
    task->state = FINISHED;
    yield(task->id);

    // Shouldn't come here anymore.
    assert(0);
}

static void RecycleTask(struct Task* task) {
    // Not strictly right.
    if (gMaxTaskId == task->id) gMaxTaskId--;

    task->state = UNUSED;
    task->id = -1;
    task->sp = task->bp = NULL;
}

//  Prepare stack for coroutine's entry on its private stack.
//  IMPORTANT: Stack should be the same as swtch function uses, high to low.
//  |0          |
//  |TaskEntry  |
//  |arg        | -> poped into %rdi
//  |r12        |
//  |r13        |
//  |r14        |
//  |r15        |
//  |rbx        |
//  |rbp        | <- task->sp
static void MakeStack(struct Task* task) {
    void* sp = task->sp;
    sp -= sizeof(void*);
    *(void **)(sp) = (void *)0;
    sp -= sizeof(void*);
    *(void **)(sp) = (void *)(TaskEntry);
    sp -= sizeof(void*);
    *(void **)(sp) = (void *)(task);

    static const int REGS = 6;
    sp -= REGS * sizeof(void*);
    memset(sp, 0, REGS * sizeof(void*));

    task->sp = sp;
}

struct Task* GetTask(Func f, void* arg) {
    for (int i = 0; i < MAX_TASKS; ++i) {
        if (gTasks[i].state != UNUSED) {
            continue;
        }

        struct Task* task = &gTasks[i];
        task->state = UNINITED;
        task->id = i;

        static const int STACK_SIZE = 1024*1024*2;
        if (task->stack == NULL)
            task->stack = (void*)malloc(STACK_SIZE);
        task->sp = task->bp = task->stack + STACK_SIZE;
        printf("sp: 0x%p, stack: 0x%p\n", task->sp, task->stack);

        task->entry = f;
        task->arg = arg;

        if (i > gMaxTaskId) {
            gMaxTaskId = i;
        }

        MakeStack(task);
        return task;
    }
    fprintf(stderr, "Failed to find task slot.\n");
    return NULL;
}

void yield(int cur_id) {
    // gTasks[0] is the main task, always in state READY.
    int next = 0;

    // Implements a random picking strategy
    // TODO(liuli): Fix this... it is not right.
    int start_id = random() % (gMaxTaskId + 1) + 1;
    for (int i = start_id; i <= gMaxTaskId; ++i) {
        if (gTasks[i].state == READY) {
            next = i;
            break;
        } else if (gTasks[i].state == FINISHED) {
            RecycleTask(&gTasks[i]);
        }
    }

    if (next == cur_id) return;

    struct Task* nextTask = &gTasks[next];
    struct Task* curTask = &gTasks[cur_id];
    swtch(curTask, nextTask);
}

int claim_main_task() {
    if (gMaxTaskId >= 0) return 1;
    memset(&gTasks[0], 0, sizeof(gTasks[0]));
    gTasks[0].state = READY;
    gTasks[0].id = 0;

    gMaxTaskId = 0;
    return 0;
}
