CFLAGS=-m64 -ggdb -g -no-pie -Wall -W

%.o: %.S
	gcc $(CFLAGS) -c $<

%.o: %.c
	gcc $(CFLAGS) -c $<

croutine_test: swtch.o croutine.o
	gcc $(CFLAGS) -o croutine_test croutine_test.c swtch.o croutine.o

clean:
	rm *.o

.PHONY: croutine_test
