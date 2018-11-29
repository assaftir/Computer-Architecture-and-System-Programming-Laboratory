#include <stdlib.h>

#include "lab4_util.h"


//GLOBAL vars
int verbose = 0;
char* name = NULL; // filename to search for
char* user_command = NULL; //command to execute on the required file
int found_filename = FALSE; //flag to handle non-existing file
void(*func)(ent* entp, char* path) = print_all; //the func which will be executed in work() (line 74), depends on the user flag

//int main(int argc, char* argv[], char* envp[]){
//    print_message(simple_itoa(argc));
//    print_message("\n");
//    print_message(envp[0]);
//    print_message("\n");
//    print_message(envp[1]);
//    print_message("\n");
//    print_message(envp[2]);
//    print_message("\n");
//}

int main(int argc, char* argv[]) {
    int i;
    char* path = ".";
    for (i = 1; i < argc; i++) {
        if (simple_strcmp(argv[i], "-e") == 0) {
            name = argv[++i];
            user_command = argv[++i];
            remove_nl(user_command);
            func = execute;
        }
        if (simple_strcmp(argv[i], "-n") == 0) {
            name = argv[++i];
            remove_nl(name);
            func = print_specific_name;
        }
        if (simple_strcmp(argv[i], "-d") == 0)
            verbose = 1;
    }
    work(path);
    if (user_command && !found_filename) {
        print_message("The file ");
        print_message(name);
        print_message(" Does not found.\n");
    }
    exit_(EXIT_SUCCESS);
}

void work(char* path) {
    int fd, count;
    ent* entp;
    int bpos;
    char type;
    char buf[MAX_DIR_SIZE];

    fd = openfile(path, RDONLY, 0777);
    if (fd == -1)
        print_error("open error");

    for (;;) {
        count = getdents(fd, buf);
        if (count == -1)
            print_error("getdents error");

        if (count == 0)
            break;

        for (bpos = 0; bpos < count;) {
            entp = (ent*) (buf + bpos);
            type = *(buf + bpos + entp->len - 1);
            bpos += entp->len;
            func(entp, path);
            if (type == DIR_TYPE && simple_strcmp(entp->name, ".") != 0 && simple_strcmp(entp->name, "..") != 0) {
                int dirname_len = simple_strlen(path);
                char subdir[PATH_MAX + 1] = {0};
                simple_strcat(subdir, path);
                simple_strcat(subdir + dirname_len, "/");
                simple_strcat(subdir + dirname_len + 1, entp->name);
                work(subdir); //recursive call
            }
        }
    }
    close_fd(fd);
}

int getdents(int fd, char* entp) {
    return print_debug(SYS_GETDENTS, system_call(SYS_GETDENTS, fd, entp, MAX_DIR_SIZE));
}

void print_message(char* str) {
    print_debug(SYS_WRITE, system_call(SYS_WRITE, STDOUT, str, simple_strlen(str)));
}

void print_error(char* str) {
    system_call(SYS_WRITE, STDERR, str, simple_strlen(str));
}

int openfile(const char* path, int flags, int mode) {
    return print_debug(SYS_OPEN, system_call(SYS_OPEN, path, flags, mode));
}

int readChars(int input_fd, char* buf, int size) {
    return print_debug(SYS_READ, system_call(SYS_READ, input_fd, buf, size));
}

int write_chars(int output_fd, char* buf, int size) {
    return print_debug(SYS_WRITE, system_call(SYS_WRITE, output_fd, buf, size));
}

int close_fd(int fd) {
    return print_debug(SYS_CLOSE, system_call(SYS_CLOSE, fd));
}

int exit_(int status){
    return print_debug(SYS_EXIT, system_call(SYS_EXIT, status));
}

int print_debug(int syscall_id, int ret_value) {
    if (verbose) {
        print_error("----System call invoked----    ID: ");
        system_call(SYS_WRITE, STDERR, simple_itoa(syscall_id), sizeof (int));
        print_error(", Return value: ");
        system_call(SYS_WRITE, STDERR, simple_itoa(ret_value), sizeof (int));
        print_error("\n");
    }
    return ret_value;
}

void print_specific_name(ent* entp, char* path) {
    if (simple_strcmp(name, entp->name) == 0) {
        print_message(path);
        print_message("/");
        print_message(entp->name);
        print_message("\n");
    }
}

void print_all(ent* entp, char* path) {
    print_message(path);
    print_message("/");
    print_message(entp->name);
    print_message("\n");
}

void execute(ent* entp, char* path) {
    if (simple_strcmp(name, entp->name) == 0) {
        if (user_command != NULL) {
            found_filename = TRUE;
            int command_len = simple_strlen(user_command);
            int path_len = simple_strlen(path);
            char shell_command[PATH_MAX + 1] = {0};
            simple_strcat(shell_command, user_command);
            simple_strcat(shell_command + command_len, " ");
            simple_strcat(shell_command + command_len + 1, path);
            simple_strcat(shell_command + command_len + path_len, "/");
            simple_strcat(shell_command + command_len + path_len + 1, name);
            simple_system(shell_command);
        }
    }
}

void remove_nl(char * s) {
    int len = simple_strlen(s) - 1;
    if (*s && s[len] == '\n')
        s[len] = 0;
}
