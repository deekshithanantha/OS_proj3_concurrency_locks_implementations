/* chprio.c - chprio */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  chprio  -  Change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
pri16	chprio(
	  pid32		pid,		/* ID of process to change	*/
	  pri16		newprio		/* New priority			*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	pri16	oldprio;		/* Priority to return		*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return (pri16) SYSERR;
	}
	prptr = &proctab[pid];
	oldprio = prptr->prprio;


	if (prptr->prstate == PR_READY && oldprio != newprio) {
		getitem(pid);
		prptr->prprio = newprio;
		insert(pid, readylist, newprio);
	} 
	
	else {
		prptr->prprio = newprio;
	}

	restore(mask);
	return oldprio;
}


void pi_restoreprio(void)
{
	intmask mask = disable();
	proctab[currpid].prprio = proctab[currpid].prprio_pi;
	proctab[currpid].pi_guard_active = FALSE;
	restore(mask);
}
