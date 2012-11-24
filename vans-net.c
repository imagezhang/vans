#include <linux/module.h>
#include <linux/kernel.h>

static int __init net_sound_init(void)
{
	printk(KERN_INFO "%s: init\n", __FUNCTION__);
	return 0;
}

static void __exit net_sound_exit(void)
{
	printk(KERN_INFO "%s: exit\n", __FUNCTION__);
	return;
	
}

module_init(net_sound_init)
module_exit(net_sound_exit)
