#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


#define LOGIN 1
#define LOGOUT 2
#define UNREGISTER 3
#define EXIT 4
#define OPEN 5
#define CLOSE 6
#define MYAUCTIONS 7
#define MYBIDS 8
#define LIST 9
#define SHOW_ASSET 10
#define BID 11
#define SHOW_RECORD 12

int isalphanum(char *str);

int isalphanum_plus(char *str);

int isnum(char *str);

int read_word(char *word, char *c);

int login_check(char *uid);

int parse_uid(char *uid, char *c);

int parse_password(char *password, char *c);

int parse_login(char *message, char *uid, char *password, char *c);

int parse_logout(char *message, char *uid, char *password);

int parse_unregister(char *message, char *uid, char *password);

int parse_open(char *message, char *uid, char *password, char *c);

int parse_close(char *message, char *uid, char *password, char *c);

int parse_myauctions(char *message, char *uid);

int parse_mybids(char *message, char *uid);

int parse_show_asset(char *message, char *c);

int parse_bid(char *message, char *uid, char *password, char *c);

int parse_show_record(char *message, char *c);

int parse_action(char *action, char *message, char *uid, char *password);

int parser(char *message, char *uid, char *password, char *c);

#endif