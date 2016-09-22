/*Carlos Alva*/
/*COP 4610 Lab1, Spring 2016*/
#include <linux/module.h>
#include <linux/slab.h>
static int hello_init(void)
{
 printk("Testing New Call\n");
// printk("Hello world 2, this is Carlos Alva\n");
// printk("Hello world 3, this is Carlos Alva\n");
//  return 0;

	int i,j;

        for (j = 1; j <= 10; j++) {

                for(i = 1; i <= 1000; i++)
                {
                        kmalloc(250, GFP_KERNEL);
                }
        }

//        printk("\nClaimed: %ld\n", syscall(__NR_get_slob_amt_claimed));
//      free(a);
//        printk("\nFree: %ld\n", syscall(__NR_get_slob_amt_freed));

        return 0;



}

static void hello_exit(void)
{
 printk("Goodbye world, this is Carlos Alva\n");
 printk("Goodbye world 2, this is Carlos Alva\n");
 printk("Goodbye world 3, this is Carlos Alva\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("Carlos Alva");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Carlos Alva's Hello World");
