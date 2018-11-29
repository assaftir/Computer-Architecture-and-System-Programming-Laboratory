CC = gcc
CFLAGS  = -m32 -g -Wall
LFLAGS  = -m32 -g -Wall -L\usr\lib

# All Targets
all: wordCounter

# Tool invocations
# Executable "wordCounter" depends on the files...

wordCounter: wordCounter.o
	@echo 'Building target: wordCounter'
	@echo 'Invoking: C Linker'
	$(CC) -o wordCounter wordCounter.c $(LFLAGS)
	@echo 'Finished building target: wordCounter'
	@echo ' '

# Depends on the source and header files
wordCounter.o: wordCounter.c
	$(CC) $(CFLAGS) -c  wordCounter.c

#Clean the build directory
clean:
	rm -f *.o wordCounter
