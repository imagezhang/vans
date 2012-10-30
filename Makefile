obj-m += vans.o

vans-objs := net.o vans.o

all:
	make -C /ubicomp/android/kernel/tegra M=$(PWD) modules

clean:
	make -C /ubicomp/android/kernel/tegra M=$(PWD) clean
