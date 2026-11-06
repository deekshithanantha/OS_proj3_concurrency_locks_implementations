/* pi_lock.c - FIFO locks with transitive priority inheritance */

#include <xinu.h>

static int32 locks_count = 0;
static uint32 locks_init_guard = 0;


static uint32 pi_guard = 0;
static pi_lock_t *locks_list[NPILOCKS];

static void pi_enter(void)
{
    proctab[currpid].pi_guard_active = TRUE;
    chprio(currpid, MAXPRIO);
    while (test_and_set(&pi_guard, 1) == 1) { /* spin */ }
}

static bool8 registered_lock(pi_lock_t *l)
{
    int i;
    for (i = 0; i < locks_count; i++) {
        if (locks_list[i] == l) {
            return TRUE;
        }
    }
    return FALSE;
}

static void priority_changer(pid32 pid, pri16 prio_before, pri16 prio_after)
{
    if (prio_before == prio_after) {
        return;
    }
    proctab[pid].prprio_pi = prio_after;
    

    if (!proctab[pid].pi_guard_active) {
        chprio(pid, prio_after);
    }
    kprintf("priority_change=P%d::%d-%d\n", pid, prio_before, prio_after);
}



static void update_priorities(void)
{
    pri16 priority[NPROC];
    pid32 pid, waiter;
    int pass, i;
    uint32 n;
    bool8 changed;
    pi_lock_t *l;

    for (pid = 0; pid < NPROC; pid++) {
        priority[pid] = proctab[pid].prprio_og;
    }
    for (pass = 0; pass < NPROC; pass++) {
        changed = FALSE;
        for (i = 0; i < locks_count; i++) {
            l = locks_list[i];
            if (!l->flag || isbadpid(l->owner)) {
                continue;
            }
            for (n = 0; n < l->count; n++) {
                waiter = l->waiting_pids_list[(l->head + n) % NPROC];
                if (!isbadpid(waiter) && priority[waiter] > priority[l->owner]) {
                    priority[l->owner] = priority[waiter];
                    changed = TRUE;
                }
            }
        }
        if (!changed) {
            break;
        }
    }
    for (pid = 0; pid < NPROC; pid++) {
        if (!isbadpid(pid)) {
            priority_changer(pid, proctab[pid].prprio_pi, priority[pid]);
        }
    }
}

syscall pi_initlock(pi_lock_t *l)
{
    if (l == NULL) {
        return SYSERR;
    }
    pi_enter();
    while (test_and_set(&locks_init_guard, 1) == 1) { /* spin */ }

    if (locks_count >= NPILOCKS ||
        (registered_lock(l) && (l->flag || l->count))) {
        locks_init_guard = 0;
        pi_guard = 0;
        pi_restoreprio();
        return SYSERR;
    }

    l->flag = 0;
    l->guard = 0;
    l->owner = -1;
    l->count = 0;
    l->head = 0;
    l->tail = 0;
    locks_list[locks_count++] = l;
    locks_init_guard = 0;
    pi_guard = 0;
    pi_restoreprio();
    return OK;
}

syscall pi_lock(pi_lock_t *l)
{
    pi_enter();
    if (l == NULL || !registered_lock(l)) {
        pi_guard = 0;
        pi_restoreprio();
        return SYSERR;
    }
    while (test_and_set(&l->guard, 1) == 1) { /* spin */ }

    if (l->flag == 0) {
        l->flag = 1;
        l->owner = currpid;
        l->guard = 0;
        pi_guard = 0;
        pi_restoreprio();
    } else {
        if (l->count >= NPROC) {
            l->guard = 0;
            pi_guard = 0;
            pi_restoreprio();
            return SYSERR;
        }
        l->waiting_pids_list[l->tail] = currpid;
        l->tail = (l->tail + 1) % NPROC;
        l->count++;
        update_priorities();

        set_park(currpid);
        l->guard = 0;
        pi_guard = 0;
        pi_restoreprio();
        park(currpid);
    }
    return OK;
}

syscall pi_unlock(pi_lock_t *l)
{
    pid32 pid = -1;

    pi_enter();
    if (l == NULL || !registered_lock(l)) {
        pi_guard = 0;
        pi_restoreprio();
        return SYSERR;
    }
    while (test_and_set(&l->guard, 1) == 1) { /* spin */ }

    if (l->owner != currpid) {
        l->guard = 0;
        pi_guard = 0;
        pi_restoreprio();
        return SYSERR;
    }

    if (l->count == 0) {
        l->flag = 0;
        l->owner = -1;
    } else {
        pid = l->waiting_pids_list[l->head];
        l->head = (l->head + 1) % NPROC;
        l->count--;
        l->owner = pid;  
    }


    update_priorities();
    l->guard = 0;
    pi_guard = 0;

    
    if (pid != -1) {
        unpark(pid);
    }
    pi_restoreprio();
    yield();
    return OK;
}
