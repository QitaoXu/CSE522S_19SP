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
#include <linux/slab.h>
#define NETLINK_TEST 17 
const char d[2] = " ";
static struct sock *socket_ptr = NULL;
static unsigned long period_sec= 1;
static unsigned long period_nsec=0;
static struct hrtimer hr_timer;
static ktime_t interval;
static struct task_struct *thread1;
long prev_idle=0;
long prev_total=0;
// static char *statCPU="/proc/stat";
struct sched_param{
	int sched_priority;
};
module_param(period_sec,ulong, 0);
module_param(period_nsec, ulong, 0);

static void netlink_recv_msg(struct sk_buff * skb) {

    struct nlmsghdr * nlh = NULL;

    if (skb == NULL) {
        printk(KERN_INFO "skb is NULL \n");

        return ;
    }

    nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "%s: received netlink message payload: %s\n", __FUNCTION__, (char *)NLMSG_DATA(nlh));

}

struct netlink_kernel_cfg cfg = {
    .input = netlink_recv_msg,
};
/* static function for kernel thread*/

// struct file *file_open(const char *path, int flags, int rights) 
// {
//     struct file *filp = NULL;
//     mm_segment_t oldfs;
//     int err = 0;

//     oldfs = get_fs();
//     set_fs(get_ds());
//     filp = filp_open(path, flags, rights);
//     set_fs(oldfs);
//     if (IS_ERR(filp)) {
//         err = PTR_ERR(filp);
//         return NULL;
//     }
//     return filp;
// }
// //Close a file (similar to close):

// void file_close(struct file *file) 
// {
//     filp_close(file, NULL);
// }
// //Reading data from a file (similar to pread):

// int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) 
// {
//     mm_segment_t oldfs;
//     int ret;

//     oldfs = get_fs();
//     set_fs(get_ds());

//     ret = vfs_read(file, data, size, &offset);

//     set_fs(oldfs);
//     return ret;
// }   
// //Writing data to a file (similar to pwrite):

// int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) 
// {
//     mm_segment_t oldfs;
//     int ret;

//     oldfs = get_fs();
//     set_fs(get_ds());

//     ret = vfs_write(file, data, size, &offset);

//     set_fs(oldfs);
//     return ret;
// }
// //Syncing changes a file (similar to fsync):

// int file_sync(struct file *file) 
// {
//     vfs_fsync(file, 0);
//     return 0;
// }


//https://stackoverflow.com/questions/1184274/read-write-files-within-a-linux-kernel-module
static int thread_fn(void * data){
	int i;
	struct file *f;
	int ret;
    // char buf[1024];
    char* token;
    char* buffer;
    long total;
    long total_idle;
    long idle;
    long split;
    long long percentage=0;
    mm_segment_t fs;

    socket_ptr = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
    printk(KERN_INFO "Initializing Netlink Socket\n");
    token=kmalloc(sizeof(char*),GFP_KERNEL);
    if (!token)
    	printk(KERN_ALERT "Allocation error!!.\n");
    buffer=kmalloc_array(sizeof(char*),1024,GFP_KERNEL);
    if (!buffer)
    	printk(KERN_ALERT "Allocation error!!.\n");
    // Init the buffer with 0
    // memset(buf, 0, 1024);
    // To see in /var/log/messages that the module is operating
    printk(KERN_INFO "My module is loaded\n");
    // I am using Fedora and for the test I have chosen following file
    // Obviously it is much smaller than the 128 bytes, but hell with it =)
    
	while (!kthread_should_stop()){ 
		set_current_state(TASK_INTERRUPTIBLE);
  		schedule();
		f = filp_open("/proc/stat", O_RDONLY, 0);
	    if(f == NULL)
	        printk(KERN_ALERT "filp_open error!!.\n");
	    else{
	        // Get current segment descriptor
	        fs = get_fs();
	        // Set segment descriptor associated to kernel space
	        set_fs(get_ds());
	        // Read the file
	        f->f_op->read(f, buffer, 1024, &f->f_pos);
	        // Restore segment descriptor
	        set_fs(fs);
	        // See what we read from file
	        printk(KERN_INFO "buf:%s",buffer);
	    }
	    filp_close(f,NULL);
	    total=0;
	    idle=0;
	    i=0;
	    percentage=0;
	    total_idle=0;
	    // buffer=&buf;
	    while( (token = strsep(&buffer,d)) != NULL &&i<10){
        	printk(KERN_INFO "%s %d \n",token,i);
        	if(i!=0&&i!=1){
        		ret=kstrtol(token,10,&split);
    	 		if(ret!=0)
    	 			printk(KERN_ALERT "Conversion error!!.\n");
    	 		total=total+split;
    	 	}
    	 	
    	 	if(i==5||i==6){
    	 		ret=kstrtol(token,10,&idle);
    	 		if(ret!=0)
    	 			printk(KERN_ALERT "Conversion error!!.\n");
    	 		total_idle+=idle;
	    	}
        	i+=1;
	    }
	   
	    // percentage=(1.0 - (idle-prev_idle)*1.0/(total-prev_total))*100;
	   	prev_idle=total_idle;
	   	prev_total=total;

      	printk(KERN_INFO "In thread1 %ld %ld",idle,total);
	}
	kfree(buffer);
	kfree(token);
	sock_release(socket_ptr->sk_socket);
	//printk(KERN_INFO "In thread1");
	// struct file *f;

	
	return 0;
}


/*timer expiration*/
static enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart )
{
	ktime_t currtime;
	wake_up_process(thread1);
  	currtime  = ktime_get();
  	hrtimer_forward(timer_for_restart, currtime , interval);
	// set_pin_value(PIO_G,9,(cnt++ & 1)); //Toggle LED 
	return HRTIMER_RESTART;
}
/* init function - logs that initialization happened, returns success */

static int simple_init (void) {
	int ret;
	char name[8]="thread1";
	struct sched_param param= { . sched_priority=95};
	
	interval=ktime_set(period_sec,period_nsec);
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function=&timer_callback;
	thread1=kthread_create(thread_fn,NULL,name);
	kthread_bind(thread1, 1);
	ret=sched_setscheduler(thread1, SCHED_FIFO, &param);
	if(ret==-1){
  		printk(KERN_ALERT "sched_setscheduler");
		return 1;  
	}
	wake_up_process(thread1);
	hrtimer_start(&hr_timer,interval, HRTIMER_MODE_REL);

    printk(KERN_ALERT "simple module initialized\n");
    return 0;
}

/* exit function - logs that the module is being removed */
static void simple_exit (void) {
	int ret;
	hrtimer_cancel(&hr_timer);
 	ret = kthread_stop(thread1);
 	if(!ret)
  		printk(KERN_INFO "Thread stopped");
    printk(KERN_ALERT "simple module is being unloaded\n");
}

module_init (simple_init);
module_exit (simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Jiangnan Liu, Qitao Xu, Zhe Wang");
MODULE_DESCRIPTION ("Enforcing Real Time Behavior");
