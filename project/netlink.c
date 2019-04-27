#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/sched.h> 
#include <linux/fs.h>
#include <net/sock.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/netlink.h>
#include <linux/buffer_head.h>
#include <net/net_namespace.h>
#include <linux/string.h>
#include <linux/slab.h>

#define NETLINK_TEST 17 
#define BUFFER_SIZE 256
#define UTIL_THRESHOLD 1250 // 80/100
#define UTIL_PRECISION 1000

static unsigned long period_sec = 1;
static unsigned long period_nsec = 0;
static unsigned long test = UTIL_THRESHOLD; //for test

long prev_idle = 0;
long prev_total = 0;

static struct hrtimer hr_timer;
static ktime_t interval;

static struct task_struct *main_thread;

static struct sock *socket_ptr = NULL;

struct sched_param {
	int sched_priority;
};

module_param(period_sec, ulong, 0);
module_param(period_nsec, ulong, 0);
module_param(test, ulong, 0);

static void netlink_recv_msg(struct sk_buff * skb) {
    struct nlmsghdr * nlh = NULL;
    if (skb == NULL) {
        printk(KERN_INFO "skb is NULL \n");
        return;
    }
    nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "%s: received netlink message payload: %s\n", __FUNCTION__, (char *)NLMSG_DATA(nlh));
}

struct netlink_kernel_cfg cfg = {
    .input = netlink_recv_msg,
};

/*
input is utilization threshold
return three stats: 
	-1 	func running error
	0	utilization is low
	1	utilization is high
*/
//https://supportcenter.checkpoint.com/supportcenter/portal?eventSubmit_doGoviewsolutiondetails=&solutionid=sk65143
//https://stackoverflow.com/questions/1184274/read-write-files-within-a-linux-kernel-module
static int do_analysis_proc_stat(int threshold) {
	struct file *f;
	char buffer[BUFFER_SIZE] = {'\0'};
	mm_segment_t fs;
	long idle = 0;
	long idlecpu = 0;
	long iowait = 0;
	long total = 0;
	long totald = 0;
	long idled = 0;
	long split;
	int i = 0;
	int ret;
	char* token;
	char *cur;

	f = filp_open("/proc/stat", O_RDONLY, 0);
	if(!f){
		printk(KERN_ALERT "filp_open error!!.\n");
		filp_close(f, NULL);
		return -1;
	} else {
		fs = get_fs();
		set_fs(get_ds());
		f->f_op->read(f, buffer, BUFFER_SIZE, &f->f_pos);
		set_fs(fs);
		filp_close(f, NULL);

		cur = buffer;
		while( (token = strsep(&cur, "  ")) != NULL &&i<9){
			if(i==0||i==1){
				i++;
				continue;
			} else {
				//printk(KERN_INFO "%s %d", token, i);
				ret = kstrtol(token, 10, &split);
				if(ret!=0) {
					printk(KERN_ALERT "Conversion1 error!!\n");
					return -1;
				}
				total = total + split;

				if(i==5){ //idle cpu time
					ret = kstrtol(token, 10, &idlecpu);
					if(ret!=0) {
						printk(KERN_ALERT "Conversion2 error!!\n");
						return -1;
					}
				}
				if(i==6){ //i/o wait
					ret = kstrtol(token, 10, &iowait);
					if(ret!=0) {
						printk(KERN_ALERT "Conversion2 error!!\n");
						return -1;
					}
				}
				i++;
			}
		}
		idle = idlecpu + iowait;
		totald = total - prev_total;
		idled = idle - prev_idle;

		printk(KERN_INFO "utilization = %lu/%lu", totald-idled, totald);

		prev_total = total;
		prev_idle = idle;

		printk("1/utilization = %lu, threshold = %d (< leads to high util)", (totald)*UTIL_PRECISION/(totald-idled), threshold);
		if ((totald)*UTIL_PRECISION/(totald-idled) < threshold) {
			return 1;
		} else {
			return 0;
		}
	}
}

static int thread_fn(void * data) {
	int system_util_stat = -1;
	while (!kthread_should_stop()){ 
		set_current_state(TASK_INTERRUPTIBLE);
  		schedule();
		system_util_stat = do_analysis_proc_stat(test);
		if (system_util_stat==-1) { //func running error
			continue;
		} else if (system_util_stat==0) {
			printk(KERN_INFO "low utilization");
			continue;
		} else if (system_util_stat==1) {
			printk(KERN_INFO "high utilization");
			continue;
			//TODO

		} else {
			printk(KERN_ALERT "wrong system_util_stat: %d", system_util_stat);
		}
	}
	return 0;
}

/*timer expiration*/
static enum hrtimer_restart timer_callback(struct hrtimer *timer_for_restart) {
	ktime_t currtime;
	wake_up_process(main_thread);
  	currtime  = ktime_get();
  	hrtimer_forward(timer_for_restart, currtime , interval);
	return HRTIMER_RESTART;
}

/* init function - logs that initialization happened, returns success */
static int simple_init (void) {
	int ret;
	struct sched_param param= {.sched_priority=95};
	socket_ptr = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
    printk(KERN_INFO "link created");
	interval=ktime_set(period_sec,period_nsec);
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function=&timer_callback;

	main_thread = kthread_create(thread_fn, NULL ,"main_thread");
	kthread_bind(main_thread, 1);
	ret=sched_setscheduler(main_thread, SCHED_FIFO, &param);
	if(ret==-1){
  		printk(KERN_ALERT "sched_setscheduler failed");
		return 1;  
	}
	wake_up_process(main_thread);
	hrtimer_start(&hr_timer,interval, HRTIMER_MODE_REL);

    printk(KERN_ALERT "simple module initialized\n");
    return 0;
}

/* exit function - logs that the module is being removed */
static void simple_exit (void) {
	int ret;
	sock_release(socket_ptr->sk_socket);
	hrtimer_cancel(&hr_timer);
 	ret = kthread_stop(main_thread);
 	if(!ret)
  		printk(KERN_INFO "Thread stopped");
    printk(KERN_ALERT "simple module is being unloaded\n");
}

module_init (simple_init);
module_exit (simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Jiangnan Liu, Qitao Xu, Zhe Wang");
MODULE_DESCRIPTION ("Enforcing Real Time Behavior");
