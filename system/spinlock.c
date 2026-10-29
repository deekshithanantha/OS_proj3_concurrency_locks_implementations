/* spinlock.c - spinlock */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  spinlock  -   spinlock based on  test_and_set function
 *------------------------------------------------------------------------
 */

syscall	sl_initlock(
	  sl_lock_t *l	/* Mutex flag to spinlock	*/
	)
{
	*l = 0;
	return OK;
}

syscall	sl_lock(
	  sl_lock_t *l	/* Mutex flag to spinlock	*/
	)
{
	while (test_and_set(&l , 1) == 1)
	{
		/* spin here */
	}
	return OK;
}

syscall	sl_unlock(
	  sl_lock_t *l	/* Mutex flag to spinlock	*/
	)
{
	*l=0;
	return OK;
}