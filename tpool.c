#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/completion.h>
#include "tpool.h"
#include <linux/sched.h>


static DEFINE_MUTEX(queue_lock);
static DECLARE_COMPLETION(queue_ready) ;


tpool_t* pool;





int work_routine(void* args)
{
   tpool_work_t* task = NULL;
   int count=0;

   
   while(1){
     mutex_lock(&queue_lock);
     //printk(KERN_INFO"tpool@work routine get lock!");
     printk(KERN_INFO"thread begin: %s[PID = %d]\n",current->comm,current->pid);
     if(!pool->tpool_head)
     {
          reinit_completion(&queue_ready);
          mutex_unlock(&queue_lock);
          wait_for_completion(&queue_ready);
          mutex_lock(&queue_lock);
     }

     /*tweak a work*/
     task = pool->tpool_head;
     pool->tpool_head=pool->tpool_head->next;
     mutex_unlock(&queue_lock);
     printk(KERN_INFO"thread end: %s[PID = %d]\n",current->comm,current->pid);
     task->work(task->args);
     kfree(task);
     
   }
        
   
return 0;
}


int create_tpool(tpool_t** pool,size_t max_thread_num)
{
   int i;
   
   (*pool) = (tpool_t*)kmalloc(sizeof(tpool_t),GFP_KERNEL);
   
   if(NULL == *pool){
        printk(KERN_INFO"Malloc tpool_t failed!");
       return -1;
   }
   (*pool)->maxnum_thread = max_thread_num;
   
   (*pool)->tpool_head = NULL;


   
  
   (*pool)->data = 2333;
   
   for(i=0; i < max_thread_num; i++){
      kthread_run(work_routine,NULL,"kthreadrun%d",i);    
   }
      
  // printk(KERN_INFO"tpool@create tpool success!");  
 
return 0;
}


/*void destroy_tpool(tpool_t* pool)
{
   kfree(pool);
}
*/
 
int add_task_2_tpool(tpool_t* pool,int (*work)(kthread_args_t*),kthread_args_t* args)
{
   tpool_work_t* newtask,*member;
 
   if(!work){
        //printk(KERN_INFO"tpool@rontine is null!\n");
        return -1;
   }
 
   
   newtask = (tpool_work_t*)kmalloc(sizeof(tpool_work_t),GFP_KERNEL);
   if(!newtask){
       // printk(KERN_INFO"tpool@Malloc work error!");
        
   }
   
   newtask->work = work;
   newtask->args = args;
   newtask->next = NULL;
   
 
   mutex_lock(&queue_lock);
   //printk(KERN_INFO"tpool@add task 2 pool, get lock");
  // printk(KERN_INFO"tpool@pool data: %d", pool->data); 
  
   if(!pool->tpool_head){
	//printk(KERN_INFO"tpool@tpool head is NULL, data is %d", newtask->args->data);
        pool->tpool_head = newtask;
        complete(&queue_ready);
	//printk(KERN_INFO"tpool@tpool head is newtask, data is %d", newtask->args->data);
   }
   else{
       // printk(KERN_INFO"tpool@tpool head is not NULL");
	member = pool->tpool_head;
        while(member->next){
        //  printk(KERN_INFO"tpool@tpool while loop");
           member = (tpool_work_t*)member->next;
        }
	//printk(KERN_INFO"tpool@add newtask to list");
        member->next = newtask;
   }
   
   //printk(KERN_INFO"tpool@add task to queue success!"); 
  // printk(KERN_INFO"tpool@add task to queue success 2!");  
   mutex_unlock(&queue_lock);
   return 0; 
   //notify the pool that new task arrived!

out:
   
   mutex_unlock(&queue_lock);
  // printk(KERN_INFO"tpool@add task, release lock!");
   return 0;
}









