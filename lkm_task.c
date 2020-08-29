#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
MODULE_LICENSE("GPL");
MODULE_VERSION("0.01");
#define DEVICE_NAME "lkm_task"

#define MSG_BUFFER_LEN 250
#define MSG_QUEUE_LEN 15

/* Prototypes for device functions */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static int major_num;
static int device_open_count = 0;
static char msg_buffer[MSG_QUEUE_LEN][MSG_BUFFER_LEN];
static char *msg_ptr;

static int q_len;
static int front_index, back_index;

/* This structure points to all of the device functions */
static struct file_operations file_ops = {
 .read = device_read,
 .write = device_write,
 .open = device_open,
 .release = device_release
};

/* When a process reads from our device, this gets called. */
static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset)
{
   int bytes_read = 0;

   if (q_len > 0)
   {
      msg_ptr = msg_buffer[front_index];

      if(*msg_ptr)
      {
         if (len > MSG_BUFFER_LEN)
         {
            len = MSG_BUFFER_LEN;
         }
         while (len && *msg_ptr)
         {
            put_user(*(msg_ptr++), buffer++);
            len--;
            bytes_read++;
         }

         q_len--;
         memset(msg_buffer[front_index], 0, MSG_BUFFER_LEN);
         front_index++;
         if (front_index == MSG_QUEUE_LEN)
         {
            front_index = 0;
         }

         printk(KERN_INFO "Queue len = %d\n", q_len);
      }
   }
   else
   {
      printk(KERN_INFO "Empty queue\n");
   }

   return bytes_read;
}

/* Called when a process tries to write to our device */
static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset)
{
   int bytes_written = 0;

   if (q_len < MSG_QUEUE_LEN)
   {
      msg_ptr = msg_buffer[back_index];

         if (len > MSG_BUFFER_LEN)
         {
            len = MSG_BUFFER_LEN;
         }
         while (len > 0)
         {
            get_user(*(msg_ptr++), buffer++);
            len--;
            bytes_written++;
         }

         q_len++;
         back_index++;
         if (back_index == MSG_QUEUE_LEN)
         {
            back_index = 0;
         }

         printk(KERN_INFO "Queue len = %d\n", q_len);
   }
   else
   {
      printk(KERN_INFO "Queue is full\n");
   }

   return bytes_written;
}

/* Called when a process opens our device */
static int device_open(struct inode *inode, struct file *file) {
 /* If device is open, return busy */
 if (device_open_count) {
 return -EBUSY;
 }
 device_open_count++;
 try_module_get(THIS_MODULE);
 return 0;
}

/* Called when a process closes our device */
static int device_release(struct inode *inode, struct file *file) {
 /* Decrement the open counter and usage count. Without this, the module would not unload. */
 device_open_count--;
 module_put(THIS_MODULE);
 return 0;
}

static int __init lkm_example_init(void)
{
   q_len = 0;
   front_index = back_index = 0;

   memset(*msg_buffer, 0, MSG_BUFFER_LEN * MSG_QUEUE_LEN);

   /* Try to register character device */
   major_num = register_chrdev(0, DEVICE_NAME, &file_ops);
   if (major_num < 0)
   {
      printk(KERN_ALERT "Could not register device: %d\n", major_num);
      return major_num;
   }
   else
   {
      printk(KERN_INFO "queue module is loaded with device major number %d\n", major_num);
      return 0;
   }

   return 0;
}

static void __exit lkm_example_exit(void)
{
   /* Remember — we have to clean up after ourselves. Unregister the character device. */
   unregister_chrdev(major_num, DEVICE_NAME);
   printk(KERN_INFO "Goodbye, World!\n");
}


/* Register module functions */
module_init(lkm_example_init);
module_exit(lkm_example_exit);
