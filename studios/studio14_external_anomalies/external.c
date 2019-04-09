
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
# define SCHED_FIFO   1
# define SCHED_RR     2
# define SCHED_OTHER  0
#define KTIME_ARR_SIZE 2
#define MILLION 1000000
struct sched_param {
    int sched_priority;
};

ktime_t ktime_arr[KTIME_ARR_SIZE];
struct hrtimer hr_timer;
struct task_struct *kthread;
int sched;
static ktime_t interval;

static char* policy = "RR";
static int core = 0;
static ulong period = 1000;
static int iter = 3;

module_param(policy, charp, 0644);
module_param(core, int, 0644);
module_param(period, ulong, 0644);
module_param(iter, int, 0644);

static enum hrtimer_restart hrtimer_fn( struct hrtimer *timer_for_restart )
{
    ktime_t currtime;
    wake_up_process(kthread);
    
    currtime  = ktime_get();
    hrtimer_forward(timer_for_restart, currtime , interval);
    return HRTIMER_RESTART;
}
static void task_fn(void){
    struct timespec diff;
    int i = 0;
    ktime_arr[0] = ktime_get();
    while(i<iter){
        ktime_arr[1]=ktime_get();
        i+=1;
    }
    diff = ktime_to_timespec(ktime_sub(ktime_arr[1],ktime_arr[0]));

    printk(KERN_INFO "iter: %d, latency for creating kthread: %lld.%.9ld\n", iter, (long long)diff.tv_sec, diff.tv_nsec);

}
static int kthread_fn(void *data) {

    while (!kthread_should_stop()){ 
        set_current_state(TASK_INTERRUPTIBLE);
        schedule();
        printk(KERN_INFO "data is %x",(int)data);
        task_fn();
        printk(KERN_INFO "In thread");
    }
    
    return 0;

}

static int external_init (void) {

    struct sched_param param;
    int ret;
    param.sched_priority = 10;

    printk(KERN_ALERT "multithread module is being loaded!\n");

    printk(KERN_INFO "policy: %s\n", policy);
    printk(KERN_INFO "core_index: %d\n", core);
    printk(KERN_INFO "period: %lums\n", period);
    printk(KERN_INFO "iter_num: %d\n", iter);
    /*sched init*/
    if (strncmp(policy, "RR",2) == 0) {

      sched = SCHED_RR;

    } else if (strncmp(policy, "FIFO",4) == 0){

      sched = SCHED_FIFO;
    } else if (strncmp(policy, "OTHER",5) == 0){
      sched= SCHED_OTHER;
    }else{
        printk(KERN_ALERT "policy invalid!!\n");
        return -1;
    }
   

    kthread = kthread_create(kthread_fn, &iter, "kthread");
    kthread_bind(kthread, core);
    ret=sched_setscheduler(kthread, sched, &param);
    if(ret==-1){
        printk(KERN_ALERT "sched_setscheduler failed");
        return -1;  
    }
    /* hrtimer init and start
    */
    interval=ktime_set(0,period*MILLION);
    hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    hr_timer.function = &hrtimer_fn;

    wake_up_process(kthread);
    hrtimer_start(&hr_timer,interval, HRTIMER_MODE_REL);

    printk(KERN_ALERT "external module initialized\n");
    return 0;
}

static void external_exit (void) {
    int ret;
    hrtimer_cancel(&hr_timer);
    ret = kthread_stop(kthread);
    if(!ret)
        printk(KERN_INFO "Thread stopped");
    // struct timespec diff;
    // diff = ktime_to_timespec(ktime_sub(ktime_arr[1], ktime_arr[0]));

    // printk(KERN_INFO "latency for creating kthread: %lld.%.9ld\n",  (long long)diff.tv_sec, diff.tv_nsec);
    printk(KERN_ALERT "multithread module is being unloaded\n");


}

module_init (external_init);
module_exit (external_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Jiangnan Liu, Qitao Xu, Zhe Wang");
MODULE_DESCRIPTION ("Observing external anomalies");