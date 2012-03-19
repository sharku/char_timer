obj-m += char_timer.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -o test driver_test.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
