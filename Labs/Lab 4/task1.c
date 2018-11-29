#include "lab4_util.h"
#include <stdio.h>
#include <string.h>
//typedef unsigned int size_t;

#define STDOUT 1
#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_LSEEK 8
//#define SEEK_SET 0
//#define SEEK_CUR 1
//#define SEEK_END 2


/*system_call(...) wrappers*/
void exit(int status);
void printMessage(char* str, size_t length);
size_t _strlen(const char *s);
extern int system_call();
int main (int argc , char* argv[], char* envp[]){
	if (argc < 3)
		exit(-1);
	char* filepath = argv[1];
	char* newname = argv[2];
	int fd = system_call(SYS_OPEN, filepath, 2, 0777);
	system_call(SYS_LSEEK, fd, 0x1015, SEEK_SET);
	system_call(SYS_WRITE, fd, newname, _strlen(newname));
	system_call(SYS_CLOSE, fd);
  return 0;
}

void printMessage(char* str, size_t length) {
	system_call(SYS_WRITE, STDOUT, str, _strlen(str));
}

void exit(int status) {
	system_call(60, status, 0, 0);
}

size_t _strlen(const char *s) {
    const char *p = s;
    while (*s) ++s;
    return s - p;
}

