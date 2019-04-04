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

#define KTIME_ARR_SIZE 16
#define MILLION 1000000

ktime_t ktime_arr[KTIME_ARR_SIZE];

struct task_struct * kthread0;
struct task_struct * kthread1;
struct task_struct * kthread2;
struct task_struct * kthread3;

atomic_t barrier;

struct sched_param{
	int sched_priority;
};

static void barrier_set_up(int i, atomic_t * v){

    atomic_set(v, i);
}

static void barrier_wait(void){

    atomic_dec_return(&barrier);

    while (1) {

        if (atomic_read(&barrier) == 0) {

            break;
        }
    }
    
}


static int kthread_fn(void *data) {

    int *index = (int *)data;

    printk(KERN_INFO "This is kthread: %s function!, index = %d\n", current->comm, *index);

    ktime_arr[(*index) * 2] = ktime_get();

    barrier_wait();

    ktime_arr[(*index) * 2 + 1] = ktime_get();

    return 0;
}

static int multithread_init (void) {

    int i = 0; 
    int j = 0;
    int k = 1;
    int m = 2;
    int n = 3;
    
    struct sched_param param;
    param.sched_priority = 5;

    printk(KERN_ALERT "multithread module is being loaded!\n");

    barrier_set_up(4, &barrier);

    for (i = 0; i < KTIME_ARR_SIZE; i++) {

        ktime_arr[i] = ktime_set(0, 0);
    }

    /* kthrread creation 
    */
    kthread0 = kthread_create(kthread_fn, (void*)(&j), "kthread0");
    kthread_bind(kthread0, 0);
    sched_setscheduler(kthread0, SCHED_FIFO, &param);

    kthread1 = kthread_create(kthread_fn, (void*)(&k), "kthread1");
    kthread_bind(kthread1, 1);
    sched_setscheduler(kthread1, SCHED_FIFO, &param);

    kthread2 = kthread_create(kthread_fn, (void*)(&m), "kthread2");
    kthread_bind(kthread2, 2);
    sched_setscheduler(kthread2, SCHED_FIFO, &param);

    kthread3 = kthread_create(kthread_fn, (void*)(&n), "kthread3");
    kthread_bind(kthread3, 3);
    sched_setscheduler(kthread3, SCHED_FIFO, &param);

    wake_up_process(kthread0);
    wake_up_process(kthread1);
    wake_up_process(kthread2);
    wake_up_process(kthread3);

    return 0;

}

static void multithread_exit (void) {

    int i = 0;
    struct timespec ts;
    struct timespec zero_one_diff;
    struct timespec two_three_diff;
    struct timespec four_five_diff;
    struct timespec six_seven_diff;

    for (i = 0; i < KTIME_ARR_SIZE; i++) {

        ts = ktime_to_timespec(ktime_arr[i]);
        printk(KERN_INFO "ktime[%d] = %lld.%.9ld\n", i, (long long)ts.tv_sec, ts.tv_nsec);
    }

    zero_one_diff = ktime_to_timespec(ktime_sub(ktime_arr[1], ktime_arr[0]));
    two_three_diff = ktime_to_timespec(ktime_sub(ktime_arr[3], ktime_arr[2]));
    four_five_diff = ktime_to_timespec(ktime_sub(ktime_arr[5], ktime_arr[4]));
    six_seven_diff = ktime_to_timespec(ktime_sub(ktime_arr[7], ktime_arr[6]));
    
    printk(KERN_INFO "barrier for kthread0: %lld.%.9ld\n",  (long long)zero_one_diff.tv_sec, zero_one_diff.tv_nsec);
    printk(KERN_INFO "barrier for kthread1: %lld.%.9ld\n",  (long long)two_three_diff.tv_sec, two_three_diff.tv_nsec);
    printk(KERN_INFO "barrier for kthread2: %lld.%.9ld\n",  (long long)four_five_diff.tv_sec, four_five_diff.tv_nsec);
    printk(KERN_INFO "barrier for kthread3: %lld.%.9ld\n",  (long long)six_seven_diff.tv_sec, six_seven_diff.tv_nsec);

	
    printk(KERN_ALERT "multithread module is being unloaded\n");
}

module_init (multithread_init);
module_exit (multithread_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Jiangnan Liu, Qitao Xu, Zhe Wang");
MODULE_DESCRIPTION ("Observing Timing Events");