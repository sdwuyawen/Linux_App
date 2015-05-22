serial_bluetooth: serial_bluetooth.o serial_api.o
	arm-linux-gcc -o $@ $^

%.o : %.S
	arm-linux-gcc -g -c -O0 -o $@ $^

%.o : %.c
	arm-linux-gcc -g -c -O0 -o $@ $^ -fno-builtin 

clean:
	rm *.o *.elf *.bin *.dis *~ serial_bluetooth -rf
