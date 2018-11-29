
CFLAGS = -m64 -Wall -g -c -nostdlib -fno-stack-protector

OBJS = task1.o   
EXEC = hexeditplus

$(EXEC): $(OBJS)
	gcc $(OBJS) -o $(EXEC)

%.o: %.c
	gcc $(CFLAGS) $*.c -o $*.o


#Clean the build directory
clean: 
	rm -f *.o $(EXEC)

