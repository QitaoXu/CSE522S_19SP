#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/netlink.h>
#include <linux/string.h>
#include <net/sock.h>
#include <net/net_namespace.h>

#include <linux/fs.h>      // Needed by filp
#include <asm/uaccess.h>   // Needed by segment descriptors
#define NETLINK_TEST 17 

static struct sock *socket_ptr = NULL;

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

static int netlink_module_init(void) {

    // Create variables
    struct file *f;
    char buf[1024];
    mm_segment_t fs;
    // int i;

    socket_ptr = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
    printk(KERN_INFO "Initializing Netlink Socket\n");


    // Init the buffer with 0
    memset(buf, 0, 1024);
    // To see in /var/log/messages that the module is operating
    printk(KERN_INFO "My module is loaded\n");
    // I am using Fedora and for the test I have chosen following file
    // Obviously it is much smaller than the 128 bytes, but hell with it =)
    f = filp_open("/proc/stat", O_RDONLY, 0);
    if(f == NULL)
        printk(KERN_ALERT "filp_open error!!.\n");
    else{
        // Get current segment descriptor
        fs = get_fs();
        // Set segment descriptor associated to kernel space
        set_fs(get_ds());
        // Read the file
        f->f_op->read(f, buf, 1024, &f->f_pos);
        // Restore segment descriptor
        set_fs(fs);
        // See what we read from file
        printk(KERN_INFO "buf:%s\n",buf);
    }
    filp_close(f,NULL);



    return 0;
}

static void netlink_module_exit(void) {

    printk(KERN_INFO "netlink module unloaded!\n");
    sock_release(socket_ptr->sk_socket);
    
}

module_init (netlink_module_init);
module_exit (netlink_module_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Zhe Wang, Jiangnan Liu, Qitao Xu");
MODULE_DESCRIPTION ("Course Project");