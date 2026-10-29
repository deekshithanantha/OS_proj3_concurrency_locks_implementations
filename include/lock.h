/*lock.h*/


/* Synchronization types */
typedef struct lock {
    uint32 flag;
} sl_lock_t;

#define NSPINLOCKS 20	/* maximum number of spinlocks that can be used */