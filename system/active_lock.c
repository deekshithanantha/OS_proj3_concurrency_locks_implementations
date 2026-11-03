/* spinal_lock.c - spinal_lock */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  spinal_lock  -   spinal_lock based on  test_and_set function
 *------------------------------------------------------------------------
 */
static int32	al_locks_count = 0;
static uint32 	al_locks_init_guard = 0;




void park(pid32 pid);
void unpark(pid32 pid);
void set_park(pid32 pid);

bool8 al_trylock(al_lock_t *l ) {
    
    pri16 old_priority = chprio(currpid,MAXPRIO);

	while (test_and_set(&l->guard , 1) == 1){/* spin here */}


	if(l->flag == 0) {
		l->flag = 1;
		l->owner = currpid; 
		l->guard = 0;

		chprio(currpid,old_priority);
		return TRUE;		
	}
	l->guard = 0;
	chprio(currpid,old_priority);
	return FALSE;
}

syscall	al_initlock( al_lock_t *l	/* Mutex flag to al_lock	*/) {

	pri16 old_priority = chprio(currpid,MAXPRIO);

	while (test_and_set(&al_locks_init_guard,1) == 1)
	{
	  /* this is done to avoid two processes from updating the count simultaneously */
	}
	
	if (al_locks_count>=NALOCKS)
	{
		al_locks_init_guard = 0;
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

	al_locks_count++;
	al_locks_init_guard = 0;

	chprio(currpid,old_priority);

	return OK;
}

syscall	al_lock( al_lock_t *l	/* Mutex flag to spinal_lock	*/) {

	pri16 old_priority = chprio(currpid,MAXPRIO);

	while (test_and_set(&l->guard , 1) == 1){/* spin here */}


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

        //deadlock detection code
		proctab[currpid].wt_mutex = l;
		
		pid32 pid_trace = l->owner;
		bool8 DL_FOUND = FALSE;

		bool8 pid_vist_sts[NPROC] = {FALSE};
		pid32 pid_trace_list[NPROC];
		int trace_count = 0;

		while (TRUE) {

			if(currpid == pid_trace) {
				DL_FOUND = TRUE;
				break;
			}
			
			if(pid_vist_sts[pid_trace] == TRUE) {
				// a case where the DL parent is someone else in the loop, so exit it 
				break;
			}
			
			pid_vist_sts[pid_trace] = TRUE;
			pid_trace_list[trace_count++] = pid_trace;

			al_lock_t *lock_trace;
			lock_trace = proctab[pid_trace].wt_mutex;

			if (lock_trace == NULL) {
				break;
			}

			pid_trace = lock_trace->owner;

		}

		if(DL_FOUND) {

			int i;
			printf("deadlock_detected=P%d",&currpid);
			for(i = 0;i<trace_count;i++) {
				pid32 temp = pid_trace_list[i];
				printf("-P%d",&temp);
			}

		} 

		set_park(currpid);
		l->guard = 0;

		chprio(currpid,old_priority);
		park(currpid);

	}
	
	return OK;
}

syscall	al_unlock( al_lock_t *l	/* Mutex flag to spinal_lock	*/ ) {

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
		proctab[pid].wt_mutex = NULL;  

		l->guard = 0;
		chprio(currpid,old_priority);
		
		unpark(pid);
		
		
		return OK;
	}
	
	l->guard = 0;
	chprio(currpid,old_priority);
	return OK;
}

