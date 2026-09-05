/////////////////////////////////////////
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
/////////////////////////////////////////
#include <linux/netdevice.h> // net driver device
#include <linux/etherdevice.h> // Ethernet device
/////////////////////////////////////////
#include <linux/if_arp.h> // main types of interfaces
/////////////////////////////////////////
#include <linux/device.h>
/////////////////////////////////////////
#include <linux/skbuff.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/delay.h>
//#include <sys/ioctl.h>
//#include <net/if.h>
//#include <errno.h>
/////////////////////////////////////////
#define DEVICE_NAME "SERGEEV"
#define DBGMSG(fmt, ...) printk(DEVICE_NAME": "fmt,##__VA_ARGS__)
/////////////////////////////////////////
static struct net_device* netDev = NULL;
static struct net_device_stats netDevStats = {};
//struct net_device* alloc_netdev(int sizeof_priv, char* name, unsigned char name_assign_type, void (*setup)(struct net_device*));
/////////////////////////////////////////
int ThreadProcMut1(void* Arg);
int ThreadProcMut2(void* Arg);
/////////////////////////////////////////
static int __init net_drv_init(void);
static void __exit net_drv_exit(void);
/////////////////////////////////////////
static int net_dev_open(struct net_device* dev);
static int net_dev_stop(struct net_device* dev);
static int net_dev_start_xmit(struct sk_buff* skb, struct net_device* dev);
static void net_dev_setup(struct net_device* dev);
static struct net_device_stats* net_dev_stats(struct net_device* dev);
static int net_dev_set_mac_address(struct net_device* dev, void* addr);
static int net_dev_init(struct net_device* dev);
static void net_dev_uninit(struct net_device* dev);
static int net_dev_change_mtu(struct net_device* dev, int new_mtu);
/////////////////////////////////////////
static struct net_device_ops dev_ops =
{
    .ndo_init = net_dev_init,
    .ndo_uninit = net_dev_uninit,
    .ndo_open = net_dev_open,
    .ndo_stop = net_dev_stop,
    .ndo_start_xmit = net_dev_start_xmit,
    .ndo_get_stats = net_dev_stats,
    .ndo_set_mac_address = net_dev_set_mac_address,
    .ndo_change_mtu = net_dev_change_mtu,
};
