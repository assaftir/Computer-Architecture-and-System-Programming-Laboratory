#include "../common.h"
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "../line_parser.h"

client_state cs;
int debug;

int client_ls();
int disconnect_();



int main(int argc, char **argv) {
    
    struct cmd_line * command;
    char input[MAX_INPUT];
    init_client(&cs);
    
    for(;;){	
        printf("server: %s> ", cs.server_addr);
        fgets(input, MAX_INPUT, stdin);
        if (strcmp(input, "quit\n") == 0 || strcmp(input, "q\n") == 0)
            exit(EXIT_SUCCESS);       
        command = parse_cmd_lines(input);  //Build the command line, returns null for enter
        if (!command) continue;  //Incase it is an empty command line, happens when user enter <enter>
        exec(command);
    }
    return 0;
}

void init_client(client_state * cs){
    cs->client_id = NULL;
    cs->server_addr = NULL;
    cs->sock_fd = -1;
    cs->conn_state = IDLE;
}

int exec(cmd_line * cmd){
    if(strcmp(cmd->arguments[0], CONNECT_COMMAND) == 0)
        return connect_(cmd);
    else if(strcmp(cmd->arguments[0], DISCONNECT_COMMAND) == 0)
        return disconnect_();
    else if(strcmp(cmd->arguments[0], LIST_DIR) == 0)
        return client_ls();
    else
        perror("Invalid input\n");
    return 0;
}

int connect_(cmd_line * cmd){
    char message[MAX_MESSAGE_LEN];
    int bytes_recv;
    if (cs.conn_state != IDLE){
        perror("connect");
        return -2;    
    }
    struct addrinfo hints, *res;
    int sockfd;      
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;   
    if (getaddrinfo(cmd->arguments[1], SERVER_PORT, &hints, &res) != 0){
        perror("getaddrinfo");
        return -1;
    }      
    if((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
        perror("socket");
        return -1;
    }
    connect(sockfd, res->ai_addr, res->ai_addrlen); 
    if (send(sockfd, "hello", strlen("hello"), 0) < 0){
        perror("send");
        return -1;
    }
    cs.conn_state = CONNECTING;
    if ((bytes_recv = recv(sockfd, message, MAX_MESSAGE_LEN - 1, 0)) < 0){
        perror("recv");
        exit(EXIT_FAILURE);
    }
    if(strncmp(message, "hello", strlen("hello")) != 0){
        fprintf(stderr, "Unexpected response for connect command.\n");
        return -1;
    }
    update_client_state(sockfd, message, cmd->arguments[1]);
    printf("server message: %s\n", message);
    freeaddrinfo(res);
    return 0;
}

int disconnect_(){
    if (cs.conn_state != CONNECTED){
        perror("disconnect");
        return -2;
    }   
    char buf[MAX_MESSAGE_LEN];
    memset(buf, 0, MAX_MESSAGE_LEN);
    int bytes_recv;
    if (send_client_message("bye") < 0){
        perror("send bye");
        return -1;
    }
    if (debug) {
        if ((bytes_recv = recv(cs.sock_fd, buf, MAX_MESSAGE_LEN, 0)) < 0){
            perror("recv after bye");
            return -1;
        }
        fprintf(stderr, "%s | Log: %s\n", cs.server_addr, buf);
    }
    if (close(cs.sock_fd) < 0){
        perror("close");
        exit(EXIT_FAILURE);
    }  
    cs.client_id = NULL;
    cs.conn_state = IDLE;
    cs.sock_fd = -1;
    cs.server_addr = NULL;
    return 0;
}

int client_ls() {
    char buf[MAX_MESSAGE_LEN];
    memset(buf, 0, MAX_MESSAGE_LEN);
    if (send_client_message(LIST_DIR) < 0){
        perror("send ls");
        return -1;
    }
    if (recv(cs.sock_fd, buf, 3, 0) < 0){
        perror("recv ls");
        return -1;
    }
    if (strncmp(buf, "ok ", 3) == 0){
        if (recv(cs.sock_fd, buf, LS_RESP_SIZE, 0) < 0){
            perror("recv ok ls");
            return -1;
        }
        printf("%s", buf);
    }
    else if (strncmp(buf, "nok", 3) == 0){
        if (recv(cs.sock_fd, buf, MAX_MESSAGE_LEN, 0) < 0){
            perror("recv nok ls");
            return -1;
        }
        fprintf(stderr, "error: %s\n", buf);
        disconnect_();
    }
    else {
        perror("Unknown\n");
    }
    return 0;
}

int send_client_message(const char * message){
    if (send(cs.sock_fd, message, strlen(message), 0) == -1){
        return -1;
    }
    return 0;
}

void update_client_state(int sockfd, char * inc_message, char * server_addr){
    cs.client_id = inc_message + 6; // sorry for the magic, it's just len(hello ) MALLOC?
    cs.conn_state = CONNECTED;
    cs.sock_fd = sockfd;
    cs.server_addr = server_addr;
}