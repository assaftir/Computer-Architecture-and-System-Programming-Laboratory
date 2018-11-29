CC = gcc
CFLAGS  = -m64 -g -Wall
LFLAGS  = -m64 -g -Wall -L\usr\lib

# All Targets
all: main

# Tool invocations
# Executable "count-words" depends on the files...

main: main.o toy_printf.o
	@echo 'Building target: main'
	@echo 'Invoking: C Linker'
	$(CC) -o main main.o toy_printf.o $(LFLAGS)
	@echo 'Finished building target: main'
	@echo ' '

# Depends on the source and header files
main.o: main.c
	$(CC) $(CFLAGS) -c  main.c
	
toy_printf.o: toy_printf.c
	$(CC) $(CFLAGS) -c  toy_printf.c

#Clean the build directory
clean:
	rm -f *.o main


