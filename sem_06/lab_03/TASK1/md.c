#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init_task.h>

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Anastasia Neklepaeva" );

static int __init md_init( void )
{
    struct task_struct *task = &init_task;
    do
    {
        printk("+ %s - %d  %s- %d\n", task->comm, task->pid, task->parent->comm, task->parent->pid);
    }
    while ((task = next_task(task)) != &init_task);
    printk("+ CURRENT %s - %d\n", current->comm, current->pid);
    return 0;
}

static void __exit md_exit( void )
{
   printk( "+ md: module md unloaded!\n" );
}

module_init( md_init );
module_exit( md_exit );
