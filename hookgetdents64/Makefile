obj-m +=hook.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

# test:
# 	ls | grep my
# 	sudo insmod hook.ko
# 	ls
# 	find . -name myhidden
# 	ls -al | grep 'my'
# 	sudo rmmod hook