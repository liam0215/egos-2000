/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: process switching, communication and termination
 * system calls are basically inter-process communication
 */

#include "egos.h"
#include "grass.h"
#include <string.h>

int proc_nprocs;
#define MAX_NPROCESS  64
struct process proc_set[MAX_NPROCESS];

static void timer_or_syscall(int id);
void proc_init() {
    earth->intr_register(timer_or_syscall);

    proc_nprocs = 0;
    memset(proc_set, 0, sizeof(struct process) * MAX_NPROCESS);
}

int proc_alloc() {
    proc_nprocs++;
    for (int i = 0; i < MAX_NPROCESS; i++) {
        if (proc_set[i].pid == 0) {
            proc_set[i].pid = proc_nprocs;
            proc_set[i].status = PROC_UNUSED;
            return proc_nprocs;
        }
    }
    FATAL("Reach the limit of %d processes", MAX_NPROCESS);
    return -1;
}

void proc_set_running(int pid) {
    for (int i = 0; i < MAX_NPROCESS; i++) {
        if (proc_set[i].pid == pid) {
            proc_set[i].status = PROC_RUNNING;
            return;
        }
    }
}

void proc_set_runnable(int pid) {
    for (int i = 0; i < MAX_NPROCESS; i++) {
        if (proc_set[i].pid == pid) {
            proc_set[i].status = PROC_RUNNABLE;
            return;
        }
    }    
}

static void timer_or_syscall(int id) {
    if (id == INTR_ID_TMR) {
        /* timer interrupt for scheduling */
        timer_reset();
    } else if (id == INTR_ID_SOFT) {
        /* software interrupt for system call */
        struct syscall *sc = (struct syscall*)SYSCALL_ARGS_BASE;
        sc->type = SYS_UNUSED;
        *((int*)RISCV_CLINT0_MSIP_BASE) = 0;

        INFO("Got system call #%d with arg %d", sc->type, sc->args.exit.status);        
    } else {
        FATAL("Got unknown interrupt #%d", id);
    }
}