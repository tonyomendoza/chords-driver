#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

static int major;
static int scale[7] = { 0, 1, 2, 3, 4, 5, 6 };
static char *degrees[7] = { "C", "D", "E", "F", "G", "A", "B" };
static const int D_ROOT = 0;
static const int D_2nd = 1;
static const int D_3rd = 2;
static const int D_4th = 3;
static const int D_5th = 4;
static const int D_6th = 5;
static const int D_7th = 6;
static const int D_Oct = 7;

static int getScaleOffset(const char *musicScale, int l){
    for(int i = 0; i < 7; i++){
        printk("CHORDS: Comparing pitch: '%s' with '%s'\n", musicScale, degrees[i]);

        if(strncmp(musicScale, degrees[i], l) == 0){
            return i;
        }
    }
    return -1;
}

static char *getPitch(int offset, int pos){
    int val = (offset + pos) % 7;
    return degrees[scale[val]];
}

static ssize_t my_write(struct file *f, const char __user *u, size_t l, loff_t *o)
{
    printk("CHORDS: Writing...\n");
    printk("CHORDS: Input chord name and receive notes in that chord.\n");

    if (!u) {
        return -EINVAL;
    }
    if (!access_ok(u, l)) {
        return -EFAULT;
    }
    if (l == 0) {
        return 0;
    }

    char kernel_buffer[16] = {0};
    copy_from_user(kernel_buffer, u, min(l, sizeof(kernel_buffer)));

    char pitch[3] = {0};
    size_t copy_size = (l >= 2) ? 2 : 1;
    strncpy(pitch, kernel_buffer, copy_size);
    pitch[copy_size] = '\0';  // Ensure null termination
    printk("buffer=%s", pitch);

    int offset = getScaleOffset(pitch, copy_size);
    if (offset < 0) {
        printk("CHORDS: Pitch does not exist in this driver :C\n");
        return -EFAULT;
    }

    char result[16] = {0};

    // Triad Chords (root, third, fifth)
    strcpy(result, getPitch(offset, D_ROOT));
    strcat(result, " ");
    strcat(result, getPitch(offset, D_3rd));
    strcat(result, " ");
    strcat(result, getPitch(offset, D_5th));

    printk("CHORDS: %s\n", result);

    copy_to_user(u, result, strlen(result));

    return strlen(result);
}



static struct file_operations fops = {
    .owner = THIS_MODULE,
	.write = my_write
};

static int __init my_init(void)
{
	major = register_chrdev(0, "hello_cdev", &fops);
	if (major < 0) {
		pr_err("hello_cdev - Error registering chrdev\n");
		return major;
	}
	printk("CHORDS: Hello! (Major Device Number: %d\n)", major);
	return 0;
}

static void __exit my_exit(void)
{
	unregister_chrdev(major, "chords");
	printk("CHORDS: Goodbye!");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TonyOM");
MODULE_DESCRIPTION("A simple driver for generating a musical chord based on given parameters.");