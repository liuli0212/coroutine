// Author: liuli
//
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "croutine.h"

#define TRACE(msg) \
    fprintf(stderr, "%s(%d): %s\n", __FILE__, __LINE__, msg)

void foo(struct Task* task, void* arg) {
    TRACE("foo called\n");
    int* f = arg;

    int i = 10;
    while (i--) {
        if (f) (*f)++;
        yield(task->id);
        fprintf(stderr, "2.. foo called for id: %d\n", task->id);
        TRACE("foo called\n");
    }
}

int main() {
    if (claim_main_task() != 0) {
        fprintf(stderr, "Another main task runing... fail.\n");
        return 1;
    }

    int f1= 0, f2 = 0, f3 = 0;

    struct Task* t = GetTask(foo, &f1);
    t->state = READY;

    t = GetTask(foo, &f2);
    t->state = READY;

    t = GetTask(foo, &f3);
    t->state = READY;

    for (int i = 4; i < MAX_TASKS; ++i) {
        t = GetTask(foo, NULL);
        t->state = READY;
    }

    srandom(time(NULL));

    int f4 = 0;
    while (1) {
        printf("main called f1: %d, f2: %d, f3: %d, f4: %d\n", f1, f2, f3, f4);
        yield(0);
        f4++;
        printf("yield called\n");
        //usleep(500*1000);
    }

    return 0;
}
