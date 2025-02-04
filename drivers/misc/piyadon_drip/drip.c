#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h> // for copy_from_user
#include "piyaart.h"       // ASCII art header

#define PROC_NAME "piyadonthedripper"

static ssize_t proc_read(struct file *file, char __user *buffer, size_t count, loff_t *offset);
static ssize_t proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *offset);

static const struct proc_ops proc_file_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

static ssize_t proc_read(struct file *file, char __user *buffer, size_t count, loff_t *offset) {
    static char kernel_buffer[65536]; // Buffer to hold ASCII art
    size_t len = 0;                 // Total length of data
    int copied = 0;

    // Build the buffer only on the first read
    if (*offset == 0) {
        for (int i = 0; ascii_art[i] != NULL; i++) {
            len += snprintf(kernel_buffer + len, sizeof(kernel_buffer) - len, "%s\n", ascii_art[i]);
            if (len >= sizeof(kernel_buffer)) {
                pr_warn("Kernel buffer full, truncating ASCII art\n");
                break;
            }
        }
    }

    // If we've reached the end, return 0 (EOF)
    if (*offset >= len) {
        return 0;
    }

    // Calculate remaining data
    size_t remaining = len - *offset;
    size_t to_copy = min(count, remaining);

    // Copy the data to the user buffer
    copied = copy_to_user(buffer, kernel_buffer + *offset, to_copy);
    if (copied != 0) {
        pr_err("Failed to copy %d bytes to user\n", copied);
        return -EFAULT;
    }

    // Update offset and return the number of bytes read
    *offset += to_copy;
    return to_copy;
}


static ssize_t proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *offset) {
    char kernel_buffer[128];

    if (count > sizeof(kernel_buffer) - 1) {
        pr_warn("Input too long, truncating\n");
        count = sizeof(kernel_buffer) - 1;
    }

    if (copy_from_user(kernel_buffer, buffer, count)) {
        pr_err("Failed to copy data from user\n");
        return -EFAULT;
    }

    kernel_buffer[count] = '\0'; // Null-terminate
    pr_info("Proc file written: %s\n", kernel_buffer);

    return count;
}

static int __init helloworld_init(void) {
    if (!proc_create(PROC_NAME, 0666, NULL, &proc_file_ops)) {
        pr_err("Failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s created\n", PROC_NAME);
    return 0;
}

static void __exit helloworld_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
    pr_info("/proc/%s removed\n", PROC_NAME);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Notmycode Foundation");
MODULE_DESCRIPTION("Piyadon Dripper");
MODULE_VERSION("1.0");

module_init(helloworld_init);
module_exit(helloworld_exit);
