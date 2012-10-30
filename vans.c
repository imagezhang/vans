//============================================================================
// 
// UBIQUITOUS COMPUTING -- smart around. 
// 
// File Name: vans.c
// FIle Description:
// Main file for VANS (Virtual Alsa Network Soundcard).
//
//============================================================================

#include <linux/module.h>
#include <linux/kernel.h>

#include <sound/core.h>

//============================================================================


MODULE_AUTHOR("J. Zhang <imagezhang.tech@gmail.com>");
MODULE_DESCRIPTION("Virutal Alsa Network Soundcard");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{ALSA,Virtual Network Soundcard}}");


//============================================================================

#define SND_VANS_DRIVER "snd_vans"
#define SND_VANS_CARD_ID "snd_vans"

static struct platform_driver snd_vans_driver = {
	.probe		= snd_vans_probe,
	.remove		= __devexit_p(snd_vans_remove),
#ifdef CONFIG_PM
	.suspend	= snd_vans_suspend,
	.resume		= snd_vans_resume,
#endif
	.driver		= {
		.name	= SND_VANS_DRIVER
	},
};

struct snd_vans {
	struct snd_card *card;
	struct snd_pcm *pcm;
};

#define IDX_AUTO_ALLOCATE -1


module_param(spk_socket_info, charp, S_IRUSER|S_IWUSR|S_IRGRP|S_IWGRP);
MODULE_PARM_DESC(spk_socket_info, "network speaker socket info (ip:port)")
module_param(mic_socket_info, charp, S_IRUSER|S_IWUSR|S_IRGRP|S_IWGRP);
MODULE_PARM_DESC(mic_socket_info, "network microphone socket info (ip:port)")

//============================================================================

static int __init vans_init(void)
{
	int i = 0;
	struct platform_device *vans_device;
	
	// Create and register platform device.
	err = platform_driver_register(&snd_vans_driver);
	if (err < 0) {
		return err;
	}
	
	vans_device = platform_device_register_simple(SND_VANS_DRIVER,
						      i, NULL, 0);
	if (IS_ERR(vans_device)) {
		vans_device = NULL;
		return -ENODEV;
	}
	if (!platform_get_drvdata(vans_device)) {
		platform_device_unregister(vans_device);
		return -ENODEV;
	}
	
	// Device created.
	return 0;
}

static void vans_unregister(void)
{
	if (vans_device) {
		platform_device_unregister(vans_device);
	}
	platform_driver_unregister(&snd_vans_driver);
	return;
}

static void __exit vans_exit(void)
{
	vans_unregister();
}

//============================================================================

static int __devinit vans_probe(struct platform_device *devptr)
{
	struct snd_card *card;
	struct snd_vans *vans;

	int dev_id = devptr->id;

	err = snd_card_create(IDX_AUTO_ALLOCATE, SND_VANS_CARD_ID,
			      THIS_MODULE, sizeof(struct snd_vans), &card);
	if (err < 0) {
		return err;
	}
	vans = card->private_data;
	vans->card = card;


	
} 

//============================================================================

module_init(vans_init)
module_exit(vans_exit)
