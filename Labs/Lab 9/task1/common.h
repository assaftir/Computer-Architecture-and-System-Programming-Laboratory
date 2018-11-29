#ifndef COMMON__
#define COMMON__

#define DIR_MAX_SIZE 2048
#define LS_RESP_SIZE 2048
#define FILE_BUFF_SIZE 1024
#define MAX_MESSAGE_LEN 2048
#define MAX_CLIENT_ID 128
#define BUF_SIZE 128
#define TRUE 1
#define FALSE !TRUE

#include "line_parser.h"
static const char CONNECT_COMMAND[] = "conn";
static const char DISCONNECT_COMMAND[] = "bye";
static const char LIST_DIR[] = "ls";
static const char SERVER_IP[] = "127.0.0.1";
static const char SERVER_PORT[] = "2018";


typedef enum {
	IDLE,
	CONNECTING,
	CONNECTED,
	DOWNLOADING,
	SIZE
} c_state;
	
typedef struct {
	char* server_addr;	// Address of the server as given in the [connect] command. "nil" if not connected to any server
	c_state conn_state;	// Current state of the client. Initially set to IDLE
	char* client_id;	// Client identification given by the server. NULL if not connected to a server.
	int sock_fd;
} client_state;

long file_size(char * filename);
char* list_dir();

//Client

void init_client(client_state *);
int exec(cmd_line *);
int connect_(cmd_line *);
void update_client_state(int, char *, char *);
int send_client_message(const char * message);

//Server

int send_server_message(int fd, const char * message);
void hello_();
void ls_();
int bye_();
void nok_();

#endif