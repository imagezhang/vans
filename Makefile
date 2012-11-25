obj-m += vans.o
vans-y := vans-net.o vans-alsa.o

#KERNEL_PATH = /ubicomp/android/kernel/tegra
KERNEL_PATH = /home/jian/linux/linux-3.2.0

all:
	make -C $(KERNEL_PATH) M=$(PWD) modules

clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean
