# include <linux/init.h>
# include <linux/module.h>
# include <linux/kernel.h>
# include <linux/sched.h>
# include <linux/netlink.h>
# include <net/sock.h>
# include <net/net_namespace.h>
# define NETLINK_TEST 17 

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

    printk(KERN_INFO "Initializing Netlink Socket\n");
    socket_ptr = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
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