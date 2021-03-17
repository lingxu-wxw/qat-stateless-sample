#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/completion.h>
#include "tpool.h"
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>



static DECLARE_COMPLETION(queue_ready) ;


tpool_t* pool;
rwlock_t queue_lock;
unsigned long flags;


int work(kthread_args_t* args)
{
  
  
  msleep(5000);
  printk(KERN_INFO"tpool@work I'm thread!%d",args->data);
   
   return 0;
}


int work_routine(void* args)
{
   tpool_work_t* task = NULL;
   int count=0;

   
   while(1){
     write_lock_irqsave(&queue_lock,flags);
     printk(KERN_INFO"tpool@work_routine get lock!");
     printk(KERN_INFO"tpool@work_routine thread begin: %s[PID = %d]\n",current->comm,current->pid);
     if(!pool->tpool_head)
     {
          write_unlock_irqrestore(&queue_lock,flags);
          reinit_completion(&queue_ready);
          wait_for_completion(&queue_ready);
          write_lock_irqsave(&queue_lock,flags);
     }

     /*tweak a work*/
     task = pool->tpool_head;
     pool->tpool_head=pool->tpool_head->next;
     write_unlock_irqrestore(&queue_lock, flags);
     
     task->work(task->args);

     
     
     kfree(task);
     printk(KERN_INFO"Free suceess!");
     
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
   
   
   write_lock_irqsave(&queue_lock,flags);
   //printk(KERN_INFO"tpool@add task 2 pool, get lock");
  
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
           member = member->next;
        }
	//printk(KERN_INFO"tpool@add newtask to list");
        member->next = newtask;
   }
   
   //printk(KERN_INFO"tpool@add task to queue success!"); 
  // printk(KERN_INFO"tpool@add task to queue success 2!");  
   write_unlock_irqrestore(&queue_lock, flags);
   return 0; 
   //notify the pool that new task arrived!

out:
   
  
  // printk(KERN_INFO"tpool@add task, release lock!");
   return 0;
}


static int __init color_init(void)
{
   int i; 
   rwlock_init(&queue_lock);
   init_completion(&queue_ready);
  // printk(KERN_INFO"tpool@Module start!");
   
   if(0 != create_tpool(&pool,5)){
        //printk(KERN_INFO"tpool@create_tpool failed!\n");
        return -1;
   }

   //printk(KERN_INFO"tpool@add task to pool start!");

   for(i = 0; i < 5; i++){
	kthread_args_t *args;
        args = (kthread_args_t*)kmalloc(sizeof(kthread_args_t),GFP_KERNEL);
        args->data = i;

        if(add_task_2_tpool(pool,work,args)==-1){
           return -1;
        }
	//printk(KERN_INFO"tpool@_init add task success");
   }
   


  
// printk(KERN_INFO"tpool@Module init return!");
   return 0;

}


static void __exit color_exit(void)
{
    
    printk(KERN_INFO"tpool@Module finish!");

}



module_init(color_init);
module_exit(color_exit);


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Nitin Gupta <ngupta@vflare.org>");
MODULE_DESCRIPTION("kthread pool test!");


