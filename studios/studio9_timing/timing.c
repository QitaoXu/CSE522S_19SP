#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/sched.h> 
#include <linux/time.h>

#define KTIME_ARR_SIZE 8
ktime_t ktime_arr[KTIME_ARR_SIZE];

static int timing_init (void) {

    int i = 0; 

    for (i = 0; i < KTIME_ARR_SIZE; i++) {

        ktime_arr[i] = ktime_set(0, 0);
    }

    printk(KERN_ALERT "timing module is being loaded!\n");

    return 0;

}

static void timing_exit (void) {

    int i = 0;
    struct timespec ts;

    for (i = 0; i < KTIME_ARR_SIZE; i++) {

        ts = ktime_to_timespec(ktime_arr[i]);
        printk(KERN_INFO "ktime[%d] = %lld.%.9ld\n", i, (long long)ts.tv_sec, ts.tv_nsec);
    }
	
    printk(KERN_ALERT "timing module is being unloaded\n");
}

module_init (timing_init);
module_exit (timing_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Jiangnan Liu, Qitao Xu, Zhe Wang");
MODULE_DESCRIPTION ("Observing Timing Events");