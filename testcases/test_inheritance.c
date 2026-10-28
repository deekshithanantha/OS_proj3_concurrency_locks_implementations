/*  main.c  - main */

#include <xinu.h>

#define M 10
#define P 10

syscall sync_printf(char *fmt, ...)
{
        intmask mask = disable();
        void *arg = __builtin_apply_args();
        __builtin_apply((void*)kprintf, arg, 100);
        restore(mask);
        return OK;
}

uint32 get_timestamp(){
	return ctr1000;
}

void run_for_ms(uint32 time){
	uint32 start = proctab[currpid].runtime;
	while ((proctab[currpid].runtime-start) < time);
}

process p1(lock_t *l1, uint32 runtime){
        lock(l1);
        run_for_ms(runtime);
        unlock(l1);
	return OK;
} 

process p1_pi(pi_lock_t *l1, uint32 runtime){
	pi_lock(l1);
	run_for_ms(runtime);
	pi_unlock(l1);
	return OK;
}

process p2_pi(pi_lock_t *l1, pi_lock_t *l2, uint32 runtime, uint32 runtime2){
	pi_lock(l1);
	run_for_ms(runtime);
	pi_lock(l2);		
	run_for_ms(runtime2);
	pi_unlock(l2);
	run_for_ms(runtime2);
	pi_unlock(l1);		
	return OK;
}

process run(uint32 num_threads, pri16 priority, uint32 runtime){
	uint32 i;
	for (i=0; i<num_threads;i++)
		resume(create((void *)run_for_ms, INITSTK, priority, "p", 1,runtime));
	return OK;
}

lock_t mutex;
pi_lock_t pimutex[M];
pid32 pid[P];
	
process	main(void)
{

	uint32 i;
	pid32 p;
	uint32 ts1;
	
	/* initialize locks */
	initlock(&mutex);	
	for (i=0; i<M; i++) pi_initlock(&pimutex[i]);

	kprintf("\n\n=========== TEST 1 for priority inversion  ===================\n\n");

	/* high, low and 10 medium priority threads */
	kprintf("* lock_t:: 1 high, 1 low and 10 middle priority threads...\n");	
	pid[0] = create((void *)p1, INITSTK, 1, "lowp", 2, &mutex, 1000);
	pid[1] = create((void *)p1, INITSTK, 3, "highp", 2, &mutex, 1000);
	resume(pid[0]);
	sleepms(10);
	ts1 = get_timestamp();	
	resume(pid[1]);
	sleepms(10);	
	run(10, 2, 1000);

        for (i=0; i<12; i++){
		p=receive();
		if (p==pid[1]) kprintf("high priority process execution time = %d ms\n", get_timestamp()-ts1);
	}	

	sleepms(100); //let all the threads complete

	kprintf("\n* pi_lock_t:: 1 high, 1 low and 10 middle priority threads...\n");	
	pid[0] = create((void *)p1_pi, INITSTK, 1, "lowp", 2, &pimutex[0], 1000);
        pid[1] = create((void *)p1_pi, INITSTK, 3, "highp", 2, &pimutex[0], 1000);
        resume(pid[0]);
        sleepms(10);
        ts1 = get_timestamp();
        resume(pid[1]);
	sleepms(10);	
        run(10, 2, 1000);
               
        for (i=0; i<12; i++){
                p=receive();
                if (p==pid[1]) kprintf("high priority process execution time = %d ms\n", get_timestamp()-ts1);
        }

        sleepms(100); //let all the threads complete
	
	kprintf("\n\n=========== TEST 2 for priority inversion - transitivity  ===================\n\n");
	pid[0] = create((void *)p2_pi, INITSTK, 1, "lowp", 4, &pimutex[2], &pimutex[3], 1000, 1000);	
	pid[1] = create((void *)p2_pi, INITSTK, 2, "mediump", 4, &pimutex[1], &pimutex[2], 1000, 1000);
	pid[2] = create((void *)p2_pi, INITSTK, 3, "highp", 4, &pimutex[1], &pimutex[3], 1000, 1000);

	resume(pid[0]);
	sleepms(100);
	resume(pid[1]);
	sleepms(100);
	resume(pid[2]);

	receive();
	receive();
	receive();
        
	sleepms(100); //let all the threads complete

	kprintf("\n\n=========== TEST 3 for priority restoration - many threads hold same lock  ===================\n\n");
	for (i=1; i<6;i++){
		pid[i] = create((void *)p1_pi, INITSTK, i, "p", 2, &pimutex[0], 1000);
		resume(pid[i]);
		sleepms(10);
	};
	
	for (i=1; i<6;i++){
		receive();
	}
	
	sleepms(100); //let all the threads complete

	kprintf("\n\n=========== TEST 4 for priority restoration - many threads hold same lock  ===================\n\n");
	pid[0] = create((void *)p2_pi, INITSTK, 1, "p", 4, &pimutex[1], &pimutex[2], 10, 1500);
	resume(pid[0]);
	for (i=1; i<5;i++){
		pid[i] = create((void *)p1_pi, INITSTK, i, "p", 2, &pimutex[1], 1000);
		resume(pid[i]);
		sleepms(10);
	}
	for (i=5; i<9;i++){
		pid[i] = create((void *)p1_pi, INITSTK, i, "p", 2, &pimutex[2], 1000);
		resume(pid[i]);
		sleepms(10);
	}
	
	for (i=0; i<9;i++){
		receive();
	}
        
	sleepms(100); //let all the threads complete

	return OK;
}
