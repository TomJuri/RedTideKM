#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/pid.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/capability.h>

#define DEVICE_NAME "redtide"
#define CLASS_NAME "memclass"
#define MEM_READ  _IOWR('k', 1, struct mem_ioctl_data)
#define MEM_WRITE _IOWR('k', 2, struct mem_ioctl_data)
#define UNUSED(x) (void)(x)

static int safe_long_to_int(long value) {
    return (value >= INT_MIN && value <= INT_MAX) ? (int)value : -1;
}

static int major;
static struct class *mem_class;

struct mem_ioctl_data {
    pid_t pid;
    unsigned long addr;
    int size;
    char __user *user_buffer;
};

static long mem_ioctl(const struct file *file, const unsigned int cmd, const unsigned long arg)
{
    UNUSED(file);

    struct mem_ioctl_data data;
    long retval = 0;
    long bytes_copied;

    if (!capable(CAP_SYS_PTRACE))
        return -EPERM;

    if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
        return -EFAULT;

    /* Validate parameters */
    if (data.size <= 0 || data.size > PAGE_SIZE)
        return -EINVAL;

    if (!access_ok(data.user_buffer, data.size))
        return -EFAULT;

    struct pid* pid_struct = find_get_pid(data.pid);
    if (!pid_struct)
        return -ESRCH;

    struct task_struct* task = pid_task(pid_struct, PIDTYPE_PID);
    put_pid(pid_struct);
    if (!task)
        return -ESRCH;

    void* kernel_buffer = kmalloc(data.size, GFP_KERNEL);
    if (!kernel_buffer)
        return -ENOMEM;

    switch (cmd) {
    case MEM_READ:
        bytes_copied = access_process_vm(task, data.addr,
                         kernel_buffer, data.size, 0);
        if (bytes_copied <= 0) {
            retval = bytes_copied;
            goto cleanup;
        }
        if (copy_to_user(data.user_buffer, kernel_buffer, bytes_copied)) {
            retval = -EFAULT;
            goto cleanup;
        }
        retval = bytes_copied;
        break;

    case MEM_WRITE:
        if (copy_from_user(kernel_buffer, data.user_buffer, data.size)) {
            retval = -EFAULT;
            goto cleanup;
        }
        bytes_copied = access_process_vm(task, data.addr,
                         kernel_buffer, data.size, 1);
        retval = bytes_copied;
        break;

    default:
        retval = -ENOTTY;
        break;
    }

cleanup:
    kfree(kernel_buffer);
    return retval;
}

static struct file_operations fops = {
    .unlocked_ioctl = mem_ioctl,
};

static int __init memdriver_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0)
        return major;

    mem_class = class_create(CLASS_NAME);
    if (IS_ERR(mem_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        return safe_long_to_int(PTR_ERR(mem_class));
    }

    const struct device* dev = device_create(mem_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(dev)) {
        class_destroy(mem_class);
        unregister_chrdev(major, DEVICE_NAME);
        return safe_long_to_int(PTR_ERR(dev));
    }

    return 0;
}

static void __exit memdriver_exit(void)
{
    device_destroy(mem_class, MKDEV(major, 0));
    class_destroy(mem_class);
    unregister_chrdev(major, DEVICE_NAME);
}

module_init(memdriver_init);
module_exit(memdriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sarpedon");