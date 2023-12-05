#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

//"USERS/103074/103074_login.txt"

#define USERS_DIR "USERS/"
#define AUCTIONS_DIR "AUCTIONS/"

#define MAX_FILE_LENGH 30
#define USER_DIR_SIZE 13
#define USER_UID_SIZE 7
#define PASS_SIZE 9

#define ERR -1
#define STATUS_OK 0
#define STATUS_NOK 1

#define LOGIN_REG 2
#define UNREG_UNR 2
#define LOGOUT_UNR 2

#define PORT "58011"

#define MAX_MSG_LEN 128
#define CODE_SIZE 4

int udp_fd, tcp_fd, tcp_newfd, udp_errcode, tcp_errcode;
int stop = 0;
ssize_t udp_n, tcp_n;
socklen_t udp_addrlen, tcp_addrlen;
struct addrinfo tcp_hints, udp_hints, *udp_res, *tcp_res;
struct sockaddr_in udp_addr, tcp_addr;
char buffer[128];