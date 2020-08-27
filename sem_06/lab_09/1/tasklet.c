#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Anastasia Neklepaeva" );

static int irq = 1;
static int dev_id;
void tasklet_func( unsigned long data );
char tasklet_data[] = "tasklet_func was called";

// keyboard controller
#define DFN_IRQ 1

// статическое создание тасклета
DECLARE_TASKLET( tasklet, tasklet_func, (unsigned long) &tasklet_data );

// обработчик прерывания
static irqreturn_t irq_handler( int irq, void *dev_id ) 
{
    if (irq == DFN_IRQ)
    {
        // запланирование тасклета на выполнение
        tasklet_schedule( &tasklet );
	return IRQ_HANDLED; // прерывание обработано
    }
    else return IRQ_NONE; // прерывание не обработано
}

void tasklet_func(unsigned long data) 
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
        printk("+ tasklet: keyboard %s\n", ascii[code]);
    }
}

static int __init md_init( void )
{
    // регистрация обработчика аппаратного прерывания
    int rc = request_irq(irq, irq_handler, IRQF_SHARED, "my_irq_handler", &dev_id);
    if (rc)
    {
        printk("+ tasklet: register interrupt handler error!\n" );
        return rc;
    }
    printk("+ tasklet: module loaded!\n" );
    return 0;
}

static void __exit md_exit( void )
{
    tasklet_kill( &tasklet );
    free_irq( irq, &dev_id );
    printk("+ tasklet: module unloaded!\n" );
}

module_init( md_init );
module_exit( md_exit );
