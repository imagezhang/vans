obj-m += vans.o
vans-y := vans-net.o vans-alsa.o

obj-m += dummy.o

#KERNEL_PATH = /ubicomp/android/kernel/tegra
KERNEL_PATH = /usr/src/linux-source-3.2.0/linux-source-3.2.0

all:
	make -C $(KERNEL_PATH) M=$(PWD) modules

clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean
