/*lock.h*/

#define NSPINLOCKS 20	/* maximum number of spinlocks that can be used */
#define NLOCKS 20	/* maximum number of locks that can be used */

extern qid16 parking_q;


/* Synchronization types */
typedef struct sp_lock {
    uint32 flag;
    pid32 owner;

} sl_lock_t;



typedef struct lock {
    uint32 flag;
    uint32 guard;
    pid32 owner;

    pid32 waiting_pids_list[NPROC];
    uint32 count;
    uint32 head;
    uint32 tail;
    

} lock_t;
