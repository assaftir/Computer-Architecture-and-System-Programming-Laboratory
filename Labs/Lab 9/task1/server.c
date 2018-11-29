#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <limits.h>
#include "../common.h"
#define BACKLOG 1

char client_id[MAX_CLIENT_ID];
client_state cs;
int debug;
int quit = FALSE;

int main(int argc, char **argv) {   
    for (int i = 0 ; i < argc ; i++)
        if (strcmp(argv[i], "-d") == 0)
            debug = TRUE; 
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, bytes_recv;
    char buf[MAX_MESSAGE_LEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me    
    if(getaddrinfo(NULL, SERVER_PORT, &hints, &res) != 0){
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }   
    if((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if ((bind(sockfd, res->ai_addr, res->ai_addrlen)) < 0){
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if((listen(sockfd, BACKLOG) < 0)){
        perror("listen");
        exit(EXIT_FAILURE);
    }    
    addr_size = sizeof their_addr;
    while ((cs.sock_fd = accept(sockfd, (struct sockaddr *) &their_addr, &addr_size)) >= 0){
        quit = FALSE;
        while(!quit){
            memset(buf, 0, MAX_MESSAGE_LEN);
            if ((bytes_recv = recv(cs.sock_fd, buf, MAX_MESSAGE_LEN, 0)) < 0){
                perror("recv");
                exit(EXIT_FAILURE);
            }
            printf("incoming message from client: %s\n", buf);
            if (debug) {
                fprintf(stderr, "%s | Log: %s\n", cs.server_addr, buf);
            }
            if (strncmp(buf, "hello", strlen("hello")) == 0){
                hello_();
            }
            else if (strncmp(buf, "bye", strlen("bye")) == 0){
                if(bye_() < 0)
                    perror("bye failed");
                quit = TRUE;
            }
            else if (strncmp(buf, "ls", strlen("ls")) == 0){
                ls_();
            }
            else {
                fprintf(stderr, "%s | ERROR: Unknown message: %s\n", cs.client_id, buf);
                nok_("Unknown");
                break;
            }
        }
    }
    close(sockfd);
    return EXIT_SUCCESS;
}


int send_server_message(int fd, const char * message){
    if (send(fd, message, strlen(message), 0) < 0){
        return -1;
    }
    return 1;
}


void hello_(){
    if (cs.conn_state != IDLE){
        nok_("state");
        return;
    }
    cs.conn_state = CONNECTED;
    char message[MAX_MESSAGE_LEN];
    memset(message, 0, MAX_MESSAGE_LEN);
    sprintf(client_id, "%d", 1 + atoi(client_id));
    sprintf(message, "hello %s", client_id);    
    cs.client_id = client_id;
    if (send_server_message(cs.sock_fd, message) < 0){
        perror("send hello");
        return;
    }
    printf("Client %s connected\n", cs.client_id);
}

void ls_(){
    char cwd[PATH_MAX];
    char ldir[LS_RESP_SIZE];   
    send_server_message(cs.sock_fd, "ok ");
    getcwd(cwd, PATH_MAX);
    strcpy(ldir, list_dir());
    if (send_server_message(cs.sock_fd, ldir) < 0) {
        nok_("list_dir error");
        perror("send ls");
        return;
    }
}

int bye_(){
    if (cs.conn_state != CONNECTED){
        nok_("not connected (bye)");
        return -2;
    }
    printf("Client %s disconnected\n", cs.client_id);
    cs.client_id = NULL;
    cs.conn_state = IDLE;
    cs.sock_fd = -1;
    return 0;
}

void nok_(char* msg){
    char nok_msg[BUF_SIZE];
    memset(nok_msg, 0, BUF_SIZE);
    sprintf(nok_msg, "nok %s", msg);
    if (send_server_message(cs.sock_fd, nok_msg) < 0){
        perror("send nok");
        return;
    }
    if (close(cs.sock_fd) < 0){
        perror("close after nok");
        exit(EXIT_FAILURE);
    }
    cs.conn_state = IDLE;
    cs.sock_fd = -1;
}