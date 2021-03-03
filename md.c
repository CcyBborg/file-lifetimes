#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <linux/workqueue.h>

#define WORK_INTERVAL 1000

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Golik Konstantin");

void handle(struct work_struct *);

DECLARE_DELAYED_WORK(work, handle);

void handle(struct work_struct * _)
{
    printk("WORK!!!!!!!!!!");
    schedule_delayed_work(&work, WORK_INTERVAL);
}

static int __init md_init(void)
{ 
    printk("Init started\n");
    schedule_delayed_work(&work, WORK_INTERVAL);
    printk("Initialized successfully");
    return 0; 
}

static void __exit md_exit(void) 
{
    cancel_delayed_work_sync(&work);
    printk("Terminated\n"); 
} 

module_init(md_init); 
module_exit(md_exit);
