#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/slab.h>

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Anastasia Neklepaeva" );

#define MAGIC_NUMBER 0x13131313

#define SLABNAME "my_cache"
struct kmem_cache *cache = NULL; 
static int number = 7;
module_param(number, int, 0); 

struct myfs_inode
{
    int i_mode;
    unsigned long i_ino;
};

static int size = sizeof(struct myfs_inode);

// деструктор суперблока
static void myfs_put_super(struct super_block * sb)
{
    printk(KERN_DEBUG "MYFS super block destroyed!\n" ) ;
}

// put_super хранит деструктор нашего суперблока
static struct super_operations const myfs_super_ops = 
{
    .put_super = myfs_put_super,
    .statfs = simple_statfs,
    .drop_inode = generic_delete_inode,

};

// размещение новой структуры inode 
// и заполнение ее значениями: размером и временами
static struct inode *myfs_make_inode(struct super_block * sb, int mode)
{
    struct inode *ret = new_inode(sb);
    struct myfs_inode *my_inode;

    if (ret)
    {
        inode_init_owner(ret, NULL, mode);
        ret->i_size = PAGE_SIZE;
        ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);

        my_inode = kmem_cache_alloc(cache, GFP_KERNEL);

        my_inode->i_mode = ret->i_mode;
        my_inode->i_ino = ret->i_ino;

        ret->i_private = my_inode;
    }
    return ret;
}

// инициализация суперблока, построение корневого каталога
static int myfs_fill_sb(struct super_block* sb, void* data, int silent)
{
    struct inode* root = NULL;
    sb->s_blocksize = PAGE_SIZE;
    sb->s_blocksize_bits = PAGE_SHIFT;
    sb->s_magic = MAGIC_NUMBER;
    sb->s_op = &myfs_super_ops;

    // создание inode для корневого каталога
    root =  myfs_make_inode(sb, S_IFDIR | 0755);
    if (!root)
    {
        printk (KERN_ERR "MYFS inode allocation failed !\n") ; 
        return -ENOMEM;
    }

    root->i_op  =  &simple_dir_inode_operations;
    root->i_fop = &simple_dir_operations;
    sb->s_root  = d_make_root(root) ;

    if (!sb->s_root)
    {
        printk(KERN_ERR " MYFS root creation failed !\n") ; 
        iput(root);
        return -ENOMEM;
    }

    return 0;
}

// функция вернёт структуру, описывающую корневой каталог файловой системы
static struct dentry* myfs_mount (struct file_system_type *type, int flags, 
                                    char const *dev, void *data)
{
    struct dentry* const entry = mount_nodev(type,  flags,  data, myfs_fill_sb) ;
    if (IS_ERR(entry))
        printk(KERN_ERR  "MYFS mounting failed !\n") ;
    else
        printk(KERN_DEBUG "MYFS mounted!\n") ;
    return entry;

}

// структура, "описывающая" создаваемую файловую систему
// owner отвечает за счетчик ссылок на модуль
// name хранит название файловой системы
static struct file_system_type myfs_type  =  
{
    .owner  =  THIS_MODULE,
    .name  =  "myfs",
    .mount  =  myfs_mount,
    .kill_sb  =  kill_anon_super,
};

static int __init myfs_init(void)
{
    // регистрация файловой системы
    int ret = register_filesystem(& myfs_type);
    if(ret != 0)
    {
        printk(KERN_ERR "MYFS_MODULE cannot register filesystem!\n");
        return ret;

    }
 
    cache = kmem_cache_create(SLABNAME, size, 0, SLAB_HWCACHE_ALIGN, NULL); 

    if (!cache)
    {
        printk( KERN_ERR "kmem_cache_create error\n"); 
        kmem_cache_destroy(cache); 
	return -ENOMEM;
    }

    printk(KERN_DEBUG "MYFS_MODULE loaded !\n"); 
    return 0;
}


static void __exit myfs_exit(void)
{
    int ret;

    // дерегистрация файловой системы
    ret = unregister_filesystem(&myfs_type); 
    if (ret != 0)
        printk(KERN_ERR "MYFS_MODULE cannot unregister filesystem !\n");
    printk(KERN_DEBUG "MYFS_MODULE unloaded !\n");

    kmem_cache_destroy(cache);
}

module_init( myfs_init );
module_exit( myfs_exit );
