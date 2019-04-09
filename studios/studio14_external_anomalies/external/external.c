#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/sched.h> 
#include <linux/time.h>
#include <linux/hrtimer.h>
#include <linux/types.h>

static char* policy = "RR";
static int core_index = 0;
static int period = 1000;
static int iter_num = 3;

module_param(policy, char *, 0764);
module_param(core_index, int, 0764);
module_param(period, int, 0764);
module_param(iter_num, int, 0764);

static int kthread_fn(void *data) {

    int *index = (int *)data;

    printk(KERN_INFO "This is kthread: %s function!, index = %d\n", current->comm, *index);

    return 0;
}

static int external_init (void) {

    printk(KERN_ALERT "multithread module is being loaded!\n");

    printk(KERN_INFO "policy: %s\n", policy);
    printk(KERN_INFO "core_index: %d\n", core_index);
    printk(KERN_INFO "period: %dms\n", period);
    printk(KERN_INFO "iter_num: %d\n", iter_num);

    return 0;
}

static void external_exit (void) {

    printk(KERN_ALERT "multithread module is being unloaded\n");
}

module_init (external_init);
module_exit (external_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Jiangnan Liu, Qitao Xu, Zhe Wang");
MODULE_DESCRIPTION ("Observing external anomalies");