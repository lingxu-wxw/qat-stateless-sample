#ifndef T_POOL
#define T_POOL


#include <linux/kthread.h>
#include <linux/ctype.h>
#include <linux/mutex.h>
#include <linux/sched.h>


typedef struct kthread_args
{
      struct zram *zram;
      int offset;
      struct bio_vec bvec;
      struct bio *bio;
      u32 index;

}kthread_args_t;


typedef struct tpool_work
{
    /* data */
    int (*work)(kthread_args_t*);//function to be called
    struct kthread_args* args;                     //arguments
    struct tpool_work* next;
}tpool_work_t;


typedef struct tpool
{
    /* data */
    size_t maxnum_thread; //maximum of thread
    struct task_struct **thread; //a array of threads
    struct tpool_work* tpool_head; //tpool_work queue
    int data;
}tpool_t;




/***************************************************
*@brief:
*       create thread pool
*@args:   
*       max_thread_num ---> maximum of threads
*       pool           ---> address of thread pool
*@return value: 
*       0       ---> create thread pool successfully
*       othres  ---> create thread pool failed
***************************************************/
int create_tpool(tpool_t** pool,size_t max_thread_num);
 
/***************************************************
*@brief:
*       destroy thread pool
*@args:
*        pool  --->  address of pool
***************************************************/
void destroy_tpool(tpool_t* pool);
 
/**************************************************
*@brief:
*       add tasks to thread pool
*@args:
*       pool     ---> thread pool
*       routine  ---> entry function of each thread
*       args     ---> arguments
*@return value:
*       0        ---> add ok
*       others   ---> add failed        
**************************************************/
int add_task_2_tpool(tpool_t* pool,int (*work)(kthread_args_t*), kthread_args_t* args);




#endif
