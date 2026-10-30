/* spinlock.c - spinlock */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  spinlock  -   spinlock based on  test_and_set function
 *------------------------------------------------------------------------
 */


qid16 parking_q;

syscall	initlock( lock_t *l	/* Mutex flag to lock	*/) {
	l->flag = 0;
	l->guard = 0;
	parking_q = newqueue();
}

syscall	lock( lock_t *l	/* Mutex flag to spinlock	*/) {
	
	while (test_and_set(&l->guard , 1) == 1){/* spin here */}

	if(l->flag == 0) {
		l->flag = 1;
		l->guard = 0;
	}
	else {
		enqueue(currpid,parking_q);
		//set_park();
		l->guard = 0;
		//park();
	}
	
	return OK;
}

syscall	unlock(lock_t *l	/* Mutex flag to spinlock	*/ ) {

	while (test_and_set(&l->guard , 1) == 1){ /* Spin here */}
	
	if (isempty(parking_q)) {
		l->flag = 0;
	}
	else{
		//unpark(dequeue(parking_q));
	}
	l->guard = 0;
	
	return OK;
}