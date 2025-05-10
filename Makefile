obj-m += chords.o

all: chords_test
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm chords_test

chords_test: chords_test.c
	gcc $< -o $@
