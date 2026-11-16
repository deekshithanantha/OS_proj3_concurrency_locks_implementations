/* spinlock.c - spinlock */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  spinlock  -   spinlock based on  test_and_set function
 *------------------------------------------------------------------------
 */

//	2. Implement a spinlock based on your test_and_set function. The spinlock should have an initialization, a 
//	lock and an unlock functions declared as specified below. You are free to define the sl_lock_t data type 
//	as you wish. 
//	syscall sl_initlock(sl_lock_t *l); 
//	syscall sl_lock(sl_lock_t *l); 
//	syscall sl_unlock(sl_lock_t *l); 
//	The spinlock implementation should be in a system/spinlock.c file. Include in lock.h constant 
//	NSPINLOCKS defining the maximum number of spinlocks that can be used, and initialize this constant to 20. 


static int32	sp_locks_count = 0;
static uint32 	sp_init_guard = 0;

syscall	sl_initlock(sl_lock_t *l) /* Mutex flag to spinlock	*/
{
	while (test_and_set(&sp_init_guard,1) == 1)
	{
	  /* this is done to avoid two processes from updating the count simultaneously */
	}
		
		if (sp_locks_count>=NSPINLOCKS)
		{
			sp_init_guard = 0;
			return SYSERR;
		}

		l->flag = 0;
		l->owner = -1;
		sp_locks_count++; 

	sp_init_guard = 0;

	return OK;
}

syscall	sl_lock( sl_lock_t *l)	/* Mutex flag to spinlock	*/
{
	while (test_and_set(&l->flag , 1) == 1)
	{
		/* spin here */
	}
	
	l->owner = currpid; 
	return OK;
}

syscall	sl_unlock( sl_lock_t *l )	/* Mutex flag to spinlock	*/
{
	if(l->owner != currpid) return SYSERR; 

	l->owner = -1;
	l->flag=0;

	return OK;
}