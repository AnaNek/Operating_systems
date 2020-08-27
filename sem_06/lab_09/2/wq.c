#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/workqueue.h>

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Anastasia Neklepaeva" );

static int irq = 1;
static int dev_id;
struct workqueue_struct *workqueue;
void work_func(struct work_struct *w);

// keyboard controller
#define DFN_IRQ 1

// поместить задачу в очередь работ
DECLARE_WORK(work, work_func);

// обработчик прерывания
static irqreturn_t irq_handler( int irq, void *dev_id ) 
{
    if (irq == DFN_IRQ)
    {
        // добавление задачи в очередь работ
        queue_work(workqueue, &work);
	return IRQ_HANDLED; // прерывание обработано
    }
    else return IRQ_NONE; // прерывание не обработано
}

void work_func(struct work_struct *w) 
{
    // получение кода нажатой клавиши клавиатуры
    int code = inb(0x60);
    char * ascii[84] = 
    {" ", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "+", "Backspace", 
     "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Ctrl",
     "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "\"", "'", "Shift (left)", "|", 
     "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "Shift (right)", 
     "*", "Alt", "Space", "CapsLock", 
     "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
     "NumLock", "ScrollLock", "Home", "Up", "Page-Up", "-", "Left",
     " ", "Right", "+", "End", "Down", "Page-Down", "Insert", "Delete"};
    if (code < 84) 
    {
        printk("+ workqueue: keyboard %s\n", ascii[code]);
    }
}

static int __init md_init( void )
{
    // регистрация обработчика аппаратного прерывания
    int rc = request_irq(irq, irq_handler, IRQF_SHARED, "my_irq_handler", &dev_id);
    if (rc)
    {
        printk("+ workqueue: register interrupt handler error!\n" );
        return rc;
    }

    //создание очереди работ
    workqueue = create_workqueue( "workqueue" );

    if (!workqueue)
    {
        printk("+ workqueue: create_workqueue error!\n" );
        return -1;
    }

    printk("+ workqueue: module loaded!\n" );
    return 0;
}

static void __exit md_exit( void )
{
    flush_workqueue(workqueue);
    destroy_workqueue(workqueue);
    free_irq( irq, &dev_id );
    printk("+ workqueue: module unloaded!\n" );
}

module_init( md_init );
module_exit( md_exit );
