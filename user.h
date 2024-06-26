#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT "58011"
#define LOGIN_NOK "RLI NOK"
#define LOGIN_ERR "RLI ERR"
#define LOGOUT_OK "RLO OK"
#define MAX_MSG_LEN 6002
#define CODE_SIZE 4
#define BUFFER_SIZE 1024

#define TIME_LIMIT 5

int fd, errcode;
int logout = 0;
ssize_t n;

socklen_t addrlen;

struct addrinfo hints, *res;
struct sockaddr_in addr;

char buffer[MAX_MSG_LEN];

int is_udp(char *code);

int udp_connect(char *message, int message_len);

int tcp_connect(char *message, int message_len);

void end_user();