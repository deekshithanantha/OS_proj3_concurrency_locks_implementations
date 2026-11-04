/* lock.c - lock */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  lock  -   lock based on  test_and_set function
 *------------------------------------------------------------------------
 */
static int32	locks_count = 0;
static uint32 	locks_init_guard = 0;




syscall	initlock( lock_t *l	/* Mutex flag to lock	*/) {

	pri16 old_priority = chprio(currpid,MAXPRIO);

	while (test_and_set(&locks_init_guard,1) == 1)
	{
	  /* this is done to avoid two processes from updating the count simultaneously */
	}
	
	if (locks_count>=NLOCKS)
	{
		locks_init_guard = 0;
		chprio(currpid,old_priority);
		return SYSERR;
	}


	l->flag = 0;
	l->guard = 0;
	l->owner = -1; 

	// for the waiting list
	l->count = 0;
	l->head = 0; 
	l->tail = 0; 	

	locks_count++;
	locks_init_guard = 0;

	chprio(currpid,old_priority);

	return OK;
}

syscall	lock( lock_t *l	/* Mutex flag to lock	*/) {

	pri16 old_priority = chprio(currpid,MAXPRIO);

	while (test_and_set(&l->guard , 1) == 1){/* spin here */}

	// if a ctxsw happens here while guard is held by a Low priority Thread, the High priority thread will be
	// stuck in the the above loop so we take the maximim priority for this process.
	if(l->flag == 0) {
		l->flag = 1;
		l->owner = currpid; 
		l->guard = 0;

		chprio(currpid,old_priority);
		
	}
	else {
		l->waiting_pids_list[l->tail] = currpid;
		l->tail = (l->tail +1) % NPROC ;
		l->count++;

		set_park(currpid);
		l->guard = 0;

		chprio(currpid,old_priority);
		park(currpid);

	}
	
	return OK;
}

syscall	unlock(lock_t *l	/* Mutex flag to lock	*/ ) {

	pri16 old_priority = chprio(currpid,MAXPRIO);

	while (test_and_set(&l->guard , 1) == 1){ /* Spin here */}
	
	if(l->owner != currpid) 
	{
		l->guard = 0;
		chprio(currpid,old_priority);
		return SYSERR;
	}
	
	 
	if (l->count == 0) {
		l->flag = 0;
		l->owner = -1;
	}
	else{
		pid32 pid = l->waiting_pids_list[l->head];
		l->head = (l->head +1) % NPROC;
		l->count--;
		l->owner = pid;

		l->guard = 0;
		chprio(currpid,old_priority);

		unpark(pid);
		
		
		return OK;
	}
	
	l->guard = 0;
	chprio(currpid,old_priority);
	return OK;
}

void set_park(pid32 pid) {
	intmask	mask = disable();
		proctab[pid].set_park = TRUE;
	restore(mask);
}

void park( pid32 pid ) {
	intmask	mask = disable();	

		if(isbadpid(pid) || pid != currpid)
		{
			restore(mask);
			return;
		}
		
		if(proctab[pid].set_park == TRUE) {
			proctab[pid].prstate = PR_SUSP;
			resched();
		}
		
	 
	restore(mask);
}

void unpark(pid32 pid) {
	intmask	mask = disable();
	
		if(isbadpid(pid))
		{
			restore(mask);
			return;
		}
		if(proctab[pid].set_park == TRUE) {
			proctab[pid].set_park = FALSE;

			if (proctab[pid].prstate == PR_SUSP) {
				ready(pid);
			}

		}
	restore(mask);
}


// Low priority Tid takes lock
// XINU will alwasy schedule High prio TID
// So low priority thread will never release a lock