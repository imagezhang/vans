//============================================================================
// 
// UBIQUITOUS COMPUTING -- smart around. 
// 
// File Name: vans.c
// FIle Description:
// Main file for VANS (Virtual Alsa Network Soundcard). Based on alsa dummy
// driver.
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

#define IDX_AUTO_ALLOCATE -1
#define VANS_SUBSTREAMS 1

/* defaults */
#define MAX_BUFFER_SIZE         (64*1024)
#define MIN_PERIOD_SIZE         64
#define MAX_PERIOD_SIZE         MAX_BUFFER_SIZE
#define USE_FORMATS             SNDRV_PCM_FMTBIT_S16_LE
#define USE_RATE                SNDRV_PCM_RATE_48000
#define USE_RATE_MIN            48000
#define USE_RATE_MAX            48000
#define USE_CHANNELS_MIN        1
#define USE_CHANNELS_MAX        1
#define USE_PERIODS_MIN         1
#define USE_PERIODS_MAX         1024

//============================================================================

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

static struct snd_pcm_hardware vans_pcm_hardware = {
	.info =             (SNDRV_PCM_INFO_MMAP |
			     SNDRV_PCM_INFO_INTERLEAVED |
			     SNDRV_PCM_INFO_RESUME |
			     SNDRV_PCM_INFO_MMAP_VALID),
	.formats =          USE_FORMATS,
	.rates =            USE_RATE,
	.rate_min =         USE_RATE_MIN,
	.rate_max =         USE_RATE_MAX,
	.channels_min =     USE_CHANNELS_MIN,
	.chaneels_max =     USE_CHANNELS_MAX,
	.buffer_bytes_max = MAX_BUFFER_SIZE,
	.period_bytes_min = MIN_PERIOD_SIZE,
	.period_bytes_max = MAX_PERIOD_SIZE,
	.periods_min =      USE_PERIODS_MIN,
	.periods_max =      USE_PERIODS_MAX,
	.fifo_size =        0,
};

struct snd_vans {
	struct snd_card *card;
	struct snd_pcm *pcm;

	struct snd_pcm_hardware pcm_hw;

};

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
	
	err = snd_card_vans_pcm(vans, 0, VANS_SUBSTREAMS);
	if (err < 0) {
		goto __nodev;
	}
	
	vans->pcm_hw = vans_pcm_hardware;

/*	err = snd_card_vans_new_mixer(vans);
	if (err < 0) {
		goto __nodev;
	}
*/	
	strcpy(card->driver, "vans");
	strcpy(card->shortname, "vans");
	sprintf(card->longname, "vans");
	
	vans_proc_init(vans);
	
	snd_card_set_dev(card, &devptr->dev);
	
	err = snd_card_register(card);
	if (err == 0) {
		platform_set_drvdata(devptr, card);
		return 0;
	}

__nodev:
	snd_card_free(card);
	return err;
} 

static int __devinit snd_card_vans_pcm(struct snd_vans *vans, int device,
				       int substreams)
{
	struct snd_pcm *pcm;
	struct snd_pcm_ops *ops;
	int err;

	err = snd_pcm_new(vans->card, "vans PCM", device,
			  substreams, substreams, &pcm);
	if (err < 0) {
		return err;
	}

	vans->pcm = pcm;
	ops = &vans_pcm_ops;

	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, ops);
	pcm->private_data = vans;
	pcm->info_flags = 0;
	strcpy(pcm->name, "vans PCM");
	
	snd_pcm_lib_preallocate_pages_for_all(
		pcm, SNDRV_DMA_TYPE_CONTINUOUS,
		snd_dma_continuous_data(GFP_KERNEL),
		0, 64*1024);
	
	return 0;
}

//============================================================================

static struct snd_pcm_ops vans_pcm_ops = {
	.open      = vans_pcm_open,
	.close     = vans_pcm_close,
	.ioctl     = snd_pcm_lib_ioctl,
	.hw_params = vans_pcm_hw_params,
	.hw_free   = vans_pcm_hw_free,
	.prepare   = vans_pcm_prepare,
	.trigger   = vans_pcm_trigger,
	.pointer   = vans_pcm_pointer,
};

static struct snd_pcm_hardware vans_pcm_hardware = {
	.info =                 (SNDRV_PCM_INFO_MMAP |
                                 SNDRV_PCM_INFO_INTERLEAVED |
                                 SNDRV_PCM_INFO_RESUME |
					 SNDRV_PCM_INFO_MMAP_VALID),
	.formats =              USE_FORMATS,
	.rates =                USE_RATE,
	.rate_min =             USE_RATE_MIN,
	.rate_max =             USE_RATE_MAX,
	.channels_min =         USE_CHANNELS_MIN,
	.channels_max =         USE_CHANNELS_MAX,
	.buffer_bytes_max =     MAX_BUFFER_SIZE,
	.period_bytes_min =     64,
	.period_bytes_max =     MAX_PERIOD_SIZE,
	.periods_min =          USE_PERIODS_MIN,
	.periods_max =          USE_PERIODS_MAX,
	.fifo_size =            0,
};

static int vans_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_vans *vans = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	int err;
	
	vans->timer_ops = &vans_hrtimer_ops;
	
	err = vans->timer_ops->create(substream);
	if (err < 0) {
		return err;
	}

	runtime->hw = vans->pcm_hw;
	runtime->hw.info &= ~SNDRV_PCM_INFO_INTERLEAVED;
	runtime->hw.info |= SNDRV_PCM_INFO_NONINTERLEAVED;
	
	return 0;
}

static int vans_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct snd_vans *vans = snd_pcm_substream_chip(substream);
	
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		return vans->timer_ops->start(substream);
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		return vans->timer_ops->stop(substream);
	}
	
	return -EINVAL;
}

static int vans_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct snd_vans *vans = snd_pcm_substream_chip(substream);
	
	return vans->timer_ops->prepare(substream);
}

static int vans_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_vans *vans = snd_pcm_substream_chip(substream);
	
	return vans->timer_ops->pointer(substream);
}

static int vans_pcm_hw_params(struct snd_pcm_substream *substream,
			      struct snd_pcm_hw_params *hw_params)
{
	return snd_pcm_lib_malloc_pages(substream,
					params_buffer_bytes(hw_params));
}

static int vans_pcm_hw_free(struct snd_pcm_substream *substream)
{
	return snd_pcm_lib_free_pages(substream);
}

//============================================================================
static struct vans_timer_ops vans_hrtimer_ops = {
	.create = vans_hrtimer_create,
	.free = vans_hrtimer_free,
	.prepare = vans_hrtimer_prepare,
	.start = vans_hrtimer_start,
	.stop = vans_hrtimer_stop,
	.pointer = vans_hrtimer_pointer
};

struct vans_hrtimer_pcm {
	ktime_t base_time;
	ktime_t period_time;
	atomic_t running;
	struct hrtimer timer;
	struct tasklet_struct tasklet;
	struct snd_pcm_substream *substream;
}

static snd_pcm_uframes_t vans_hrtimer_pointer(
	struct snd_pcm_substream *sbustream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct van_hrtimer_pcm *dpcm = runtime->private_data;
	u64 delta;
	u32 pos;

	delta = ktime_us_delta(hrtimer_cb_get_time(&dpcm->timer),
			       dpcm->base_time);
	delta = div_u64(delta * runtime->rate + 999999, 1000000);
	div_u64_rem(delta, runtime->buffer_size, &pos);
	return pos;
}

static int vans_hrtimer_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct vans_hrtimer_pcm *dpcm = runtime->private_data;
	
	unsigned int period, rate;
	long sec;
	unsigned long nsecs;

	vans_hrtimer_sync(dpcm);
	period = runtime->period_size;
	rate = runtime->rate;
	sec = period/rate;
	
	peroid %= rate;
	nsecs = div_u64((u64)period * 1000000000UL + rate - 1, rate);
	dpcm->period_time = ktime_set(sec, nsecs);

	return 0;
}  

static int vans_hrtimer_create(struct snd_pcm_substream *substream)
{
	struct vans_hrtimer_pcm *dpcm;
	dpcm = kzalloc(sizeof(*dpcm), GFP_KERNEL);
	if (!dpcm) {
		return -ENOMEM;
	}

	substream->runtime->private_date = dpcm;
	hrtimer_init(&dpcm->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	dpcm->timer.function = vans_hrtimer_callback;
	dpcm->substream = substream;
	atomic_set(&dpcm->running, 0);
	tasklet_init(&dpcm->tasklet, vans_hrtimer_pcm_elapsed,
		     (unsigned long)dpcm);
	return 0;
}

static enum hrtimer_restart vans_hrtimer_callback(struct hrtimer *timer)
{
	struct vans_hrtimer_pcm *dpcm;
	
	dpcm = container_of(timer, struct dummy_hrtimer_pcm, timer);
	if (!atomic_read(&dpcm_running)) {
		return HRTIMER_NORESTART;
	}
	
	tasklet_schedule(&dpcm->tasklet);
	hrtimer_forward_now(timer, dpcm->period_time);
	return HRTIMER_RESTART;
}

static void vans_hrtimer_pcm_elapsed(unsigned long priv)
{
	struct vans_hrtimer_pcm *dpcm;
	dpcm = (struct vans_hrtimer_pcm *)priv;

	if (atomic_read(dpcm->running)) {
		snd_pcm_period_elapsed(dpcm->substream);
	}
	
	return;
}

static int vans_hrtimer_start(struct snd_pcm_substream *substream)
{
	struct vans_hrtimer_pcm *pcm = substream->runtime->private_data;

	dpcm->base_time = hrtimer_cb_get_time(&dpcm->timer);
	hrtimer_start(&dpcm->timer, dpcm->period_time, HRTIMER_MODE_REL);
	atomic_set(&dpcm->running, 1);
	return 0;
}


static int vans_hrtimer_stop(struct snd_pcm_substream *substream)
{
	struct vans_hrtimer_pcm *pcm = substream->runtime->private_data;
	
	atomic_set(&dpcm->running, 0);
	hrtimer_cancel(&dpcm->timer);
	return 0;
}

static inline void vans_hrtimer_sync(struct vans_hrtimer_pcm *dpcm)
{
	tasklet_kill(&dpcm->tasklet);
}

//============================================================================

module_init(vans_init)
module_exit(vans_exit)