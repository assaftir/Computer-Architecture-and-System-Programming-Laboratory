CC = gcc
CFLAGS  = -m64 -g -Wall
LFLAGS  = -m64 -g -Wall -L\usr\lib

# All Targets
all: main

# Tool invocations
# Executable "count-words" depends on the files...

main: client/client.c server/server.c line_parser.c common.c
	@echo 'Building target: main'
	@echo 'Invoking: C Linker'
	$(CC) -o run_client common.c line_parser.c client/client.c $(LFLAGS)
	$(CC) -o run_server common.c line_parser.c server/server.c $(LFLAGS)
	@echo 'Finished building target: main'
	@echo ' '

# Depends on the source and header files
client.o: client.c
	$(CC) $(CFLAGS) -c  client/client.c
	
server.o: server.c
	$(CC) $(CFLAGS) -c  server/server.c
	
line_parser.o: line_parser.c
	$(CC) $(CFLAGS) -c  line_parser.c

common.o: common.c
	$(CC) $(CFLAGS) -c  common.c	

#Clean the build directory
clean:
	rm -f *.o run_client run_server

