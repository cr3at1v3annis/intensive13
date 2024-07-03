/*! This is a simple secret storage module */
#include "ssmod.h"
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/string.h>
#include <linux/gfp.h>
#include <linux/semaphore.h>

unsigned transaction_id;
int size = 57;
HT *ht;
volatile int mt = 0;
#define HT_MAX_KEYLEN 64
struct hashtable_node {
    void  *data;
    size_t size;
    __u64  key;
    struct hashtable_node *next;
};
typedef struct ht{
    struct hashtable_node **tbl;
    int size;
} HT;

unsigned long _hash(char *key)
{
    unsigned long hash = 5381;
    int c;
    
    while (c =*key++)
        hash = ((hash << 5) + hash) + c;
    
    return hash;
}
HT *ht_create(int size)
{
    HT *ht_temp= kcalloc(1, sizeof(HT), GFP_KERNEL);
    
    ht_temp->size = size;
    
    ht_temp->tbl = kcalloc(1, size * sizeof(struct hashtable_node *), GFP_KERNEL);
    return ht_temp;
}
void lock(void) 
{
    while (__sync_lock_test_and_set(&mt,1)){}
}
void unlock(void)
{
    __sync_synchronize();
    mt = 0;
}
static void free_callback(void *data){}
unsigned int keyvalue_poll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    printk("keyvalue_poll called. Process queued\n");
    return mask;
}
static long keyvalue_ioctl(struct file *filp, unsigned int cmd,
                           unsigned long arg)
{
    switch (cmd) 
    {
        case KEYVALUE_IOCTL_GET:
            return keyvalue_get((void __user *) arg);
        case KEYVALUE_IOCTL_SET:
            return keyvalue_set((void __user *) arg);
        case KEYVALUE_IOCTL_DELETE:
            return keyvalue_delete((void __user *) arg);
        default:
            return -ENOTTY;
    }
}
static int keyvalue_mmap(struct file *filp, struct vm_area_struct *vma)
{
    return 0;
}
static const struct file_operations keyvalue_fops = {
    .owner                = THIS_MODULE,
    .unlocked_ioctl       = keyvalue_ioctl,
    .mmap                 = keyvalue_mmap,
};
static struct miscdevice keyvalue_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "keyvalue",
    .fops = &keyvalue_fops,
};
static int __init keyvalue_init(void)
{
    int ret;
    ht=ht_create(size);
    if ((ret = misc_register(&keyvalue_dev)))
        printk(KERN_ERR "Unable to register \"keyvalue\" misc device\n");
    return ret;
}
static void __exit keyvalue_exit(void)
{
    misc_deregister(&keyvalue_dev);
}
static long keyvalue_get(struct keyvalue_get __user *ukv)
{
    unsigned long result;
    unsigned long index = ukv->key  % ht->size;
    struct hashtable_node *n = ht->tbl[index];
    lock();
    while (n) {
        if (ukv->key==n->key){
            result=copy_to_user(ukv->data, (char *)n->data, (__u64)n->size);
            if (result)
            {
                unlock();
                return -1;
            }
            put_user(n->size, ukv->size);
            transaction_id++;
            unlock();;
            return transaction_id;
        }
        n = n->next;
    }
    unlock();
    return -1;
}
static long keyvalue_set(struct keyvalue_set __user *ukv)
{
    printk("start------------------------ %d\n", ukv->key);
    struct keyvalue_set *kv;
    char *data;
    unsigned long result;
    unsigned long index = ukv->key  % ht->size;
    lock();
    struct hashtable_node *n = ht->tbl[index];
    n->key=ht->tbl[index]->key;
    while (n) {
        if (ukv->key == n->key)
        {
            kfree(n->data);
            n->data = kcalloc(1, ukv->size, GFP_KERNEL);
            if (!n->data)
            {
                unlock();
                return -1;
            }
            result=copy_from_user(n->data, ukv->data, ukv->size);
            if (result)
            {
                unlock();
                return -1;
            }
            n->size = ukv->size;
            transaction_id++;
            unlock();
            return transaction_id;
        }
        
        n = n->next;
    }
    printk("3\n");
    struct hashtable_node *n_new;
    n_new = kcalloc(1, sizeof(struct hashtable_node), GFP_KERNEL);
    if (!n_new )
    {
        unlock();
        return -1;
    }
    n_new->data = kcalloc(1, ukv->size, GFP_KERNEL);
    if (!n_new->data)
    {
        kfree(n_new);
        unlock();
        return -1;
    }
    n_new->size = ukv->size;
    n_new->key = ukv->key;
    result=copy_from_user(n_new->data, ukv->data, ukv->size);
    if (result)
    {
        kfree(n_new->data);
        kfree(n_new);
        unlock();
        return -1;
    }
    
    n_new->next = ht->tbl[index];
    ht->tbl[index] = n_new;
    transaction_id++;
    
    unlock();
    printk("end\n");
    return transaction_id;
}
static long keyvalue_delete(struct keyvalue_delete __user *ukv)
{
    unsigned long index = ukv->key  % ht->size;
    struct hashtable_node *p = NULL, *n =ht->tbl[index];
    lock();
    while (n) {
        if (ukv->key==n->key) 
        {
            if (p)
                p->next = n->next;
            kfree (n->data);
            n->key = NULL;
            if (ht->tbl[index] == n)
                ht->tbl[index] = NULL;
            kfree (n);
            n = NULL;
            transaction_id++;
            unlock();
            return transaction_id;
        }
        p = n;
        n = n->next;
    }
    unlock();
    return -1;
}

MODULE_LICENSE("GPL");
module_init(keyvalue_init);
module_exit(keyvalue_exit);
