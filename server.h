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
#include <time.h>

//"USERS/103074/HOSTED/001.txt"
//"AUCTIONS/001/bids/"
// YYYY-MM-DD HH:MM:SS

// Common strings to use
#define USERS_DIR "USERS/"
#define AUCTIONS_DIR "AUCTIONS/"

#define DEFAULT 128

// Filepaths -> LENGTH & directories -> SIZE (convention used)
#define USER_DIR_SIZE 13

#define USER_SUB_DIR_LENGTH 20
#define MAX_FILE_LENGTH 30
#define USER_HOSTED_AUCTION_LENGTH 28

#define AUCTIONS_DIR_SIZE 13
#define ASSET_DIR_SIZE 20
#define BIDS_DIR_SIZE 19

#define AUCTION_ASSET_LENGTH 40
#define AUCTION_START_LENGTH 27
#define START_MAX_SIZE 82

#define AUCTION_END_LENGHT 25

// Variables size
#define USER_UID_SIZE 7
#define PASS_SIZE 9
#define AUCT_NAME 11
#define START_VALUE 7
#define TIME_ACTIVE 6
#define ASSET_NAME 25
#define AID 4
#define CURR_TIME 20
#define FULLTIME 5


// Possible Returns from functions
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

#define RCL_NLG 2
#define RCL_EAU 3
#define RCL_EOW 4
#define RCL_END 5

// Fix size of answers
#define LIN_SIZE 20
#define LOU_SIZE 20
#define UNR_SIZE 20
#define LMA_SIZE 11
#define LMB_SIZE 11
#define LST_SIZE 4
#define SRC_SIZE 8
#define OPEN_SIZE 16        //103074 password 
#define CLOSE_SIZE 20       //103074 password 001
#define BID_SIZE 26         //103074 password 001 50000

// Port
#define PORT "58011"

// Main function conventions
#define MAX_MSG_LEN 6002
#define CODE_SIZE 4

int udp_fd, tcp_fd, tcp_newfd, udp_n, tcp_n, udp_errcode, tcp_errcode;
socklen_t udp_addrlen, tcp_addrlen;
struct addrinfo tcp_hints, udp_hints, *udp_res, *tcp_res;
struct sockaddr_in udp_addr, tcp_addr;
pid_t child_pid;
int aid = 0;