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
#include <linux/string.h>
#include <linux/slab.h>

#define KTIME_ARR_SIZE 2

struct sched_param {
    int sched_priority;
};

ktime_t ktime_arr[KTIME_ARR_SIZE];
struct hrtimer timer;
struct task_struct *kthread;
int sched_policy;

static char* policy = "RR";
static int core_index = 0;
static int period = 1000;
static int iter_num = 3;

module_param(policy, charp, 0764);
module_param(core, int, 0764);
module_param(period, int, 0764);
module_param(iter, int, 0764);

static enum hrtimer_restart hrtimer_fn(struct hrtimer * timer) {

    /* wake up kthread
    */
    wake_up_process(kthread);

    printk(KERN_INFO "This is in hrtimer_fn!\n");

    return HRTIMER_NORESTART;
}

static int kthread_fn(void *data) {

    int *iter = (int *)data;

    printk(KERN_INFO "This is kthread: %s function!, iter_num = %d\n", current->comm, *iter);

    ktime_arr[1] = ktime_get();

    *iter -= 1;

    set_current_state(TASK_INTERRUPTIBLE);

    schedule();

    return 0;
}

static int external_init (void) {

   
    struct sched_param param;
    param.sched_priority = 10;

    printk(KERN_ALERT "multithread module is being loaded!\n");

    printk(KERN_INFO "policy: %s\n", policy);
    printk(KERN_INFO "core_index: %d\n", core);
    printk(KERN_INFO "period: %dms\n", period);
    printk(KERN_INFO "iter_num: %d\n", iter);

    if (strcmp(policy, "RR") == 0) {

        sched_policy = SCHED_RR;
    } else {

        sched_policy = SCHED_FIFO;
    }

    kthread = kthread_create(kthread_fn, &iter_num, "kthread");
    kthread_bind(kthread, core_index);
    sched_setscheduler(kthread, sched_policy, &param);

    /* hrtimer init and start
    */
    hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    timer.function = &hrtimer_fn;

    ktime_arr[0] = ktime_get();

    while (iter_num > 0) {

        hrtimer_start(&timer, ktime_set(0, period * 1000000), HRTIMER_MODE_REL);

        schedule();

    }

    return 0;
}

static void external_exit (void) {

    struct timespec diff;
    diff = ktime_to_timespec(ktime_sub(ktime_arr[1], ktime_arr[0]));

    printk(KERN_INFO "latency for creating kthread: %lld.%.9ld\n",  (long long)diff.tv_sec, diff.tv_nsec);
    printk(KERN_ALERT "multithread module is being unloaded\n");


}

module_init (external_init);
module_exit (external_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Jiangnan Liu, Qitao Xu, Zhe Wang");
MODULE_DESCRIPTION ("Observing external anomalies");