#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

//"USERS/103074/103074_login.txt"
//"USERS/103074/BIDDED"

#define USERS_DIR "USERS/"
#define AUCTIONS_DIR "AUCTIONS/"

#define WRONG_FORMAT "ERR Wrong Format."

#define DEFAULT 128
#define MAX_FILE_LENGTH 30
#define USER_SUB_DIR_LENGTH 20
#define AUCTIONS_DIR_LENGTH 11
#define USER_DIR_SIZE 13
#define USER_UID_SIZE 7
#define PASS_SIZE 9
#define AUCT_NAME 11
#define START_VALUE 7
#define TIME_ACTIVE 6
#define AID 4

#define ERR -1
#define STATUS_OK 0
#define STATUS_NOK 1

#define LOGIN_REG 2
#define ALR_LOGIN 3
#define UNREG_UNR 2
#define LOGOUT_UNR 2
#define RMA_NLG 2
#define RMB_NLG 2
#define ROA_NLG 2

#define LIN_SIZE 20
#define LOU_SIZE 20
#define UNR_SIZE 20
#define LMA_SIZE 11
#define LMB_SIZE 11
#define LST_SIZE 4
#define SRC_SIZE 8
#define OPEN_SIZE 16 //103074 password 

#define PORT "58011"

#define MAX_MSG_LEN 6002
#define CODE_SIZE 4

int udp_fd, tcp_fd, tcp_newfd, udp_n, tcp_n, udp_errcode, tcp_errcode;
socklen_t udp_addrlen, tcp_addrlen;
struct addrinfo tcp_hints, udp_hints, *udp_res, *tcp_res;
struct sockaddr_in udp_addr, tcp_addr;
pid_t child_pid;