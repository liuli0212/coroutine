// A very simple single threaded coroutine implementation.
// Example usage see the test file.
//
// TODO(liuli):
//  * scheduler needs to be improved.
//  * multi-thread processing.
//  * probably stack space recylcing.
//
// Author: liuli
//
#ifndef CROUTINE_H
#define CROUTINE_H

#define MAX_TASKS 1000
// Tasks[0] is the main task with system allocated stack.
#define MAIN_TASK 0;

enum task_state {
    UNUSED,
    UNINITED,
    READY,
    FINISHED
};

struct Task;
typedef void (*func)(struct Task*, void*);

struct Task {
    // sp & bp will be setup in swtch.S
    void* sp;
    void* bp;

    // Stack base (lowest addrss)
    void* stack;

    // Worker function entry.
    func entry;
    void* arg;

    // State of the coroutine.
    enum task_state state;

    // id of Task.
    int id;
};

// Task of cur_id yield.
void yield(int cur_id);

// Claim I am the main task with id 0.
int claim_main_task();

struct Task* GetTask(func f, void* arg);

#endif
