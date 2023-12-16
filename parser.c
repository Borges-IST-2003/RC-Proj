#include <stdio.h>
#include <string.h>

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

int isalphanum(char *str){
    return strspn(str, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890") == strlen(str);
}

int isalphanum_plus(char *str){
    return strspn(str, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890._-") == strlen(str);
}

int isnum(char *str){
    return strspn(str, "123456780") == strlen(str);
}

//return 0 if user not logged in and 1 if is logged in
int login_check(char *uid){
    if (uid[0] == '\0')
        return 0;
    return 1;
}

/*le uid do stdin e verifica se estifer corretamente formatado
    o uid novo fica guardado na variavel passada a funcao*/
int parse_uid(char *uid){
    scanf("%s", uid);

    if (strlen(uid) != 6){
        printf("Comprimento errado UID!\n");
        return -1;
    }
    if (!isnum(uid)){
        return -1;
        printf("Formatação errada UID!\n");
    }
    return 0;
}

/*le password do stdin e verifica se estifer corretamente formatada
    a password fica guardada na variavel passada a funcao*/
int parse_password(char *password){
    scanf("%s", password);

    if (strlen(password) != 8){
        printf("Comprimento errado password!\n");
        return -1;
    }

    return 0;
}

/*valida os dados de login e escreve toda a mensagem a ser enviada para o AS*/
int parse_login(char *message, char *uid, char *password){
    if (login_check(uid) == 1){
        printf("User still logged in. Logout first!\n");
        return -1;
    }
    if (parse_uid(uid) == -1 || parse_password(password) == -1){
        printf("Falha Login!\n");
        memset(message, 0, strlen(message));
        memset(uid, 0, strlen(uid));
        memset(password, 0, strlen(password));
        return -1;
    }

    strcat(message, uid);
    strcat(message, " \0");
    strcat(message, password);

    return 0;
}

/*escerve a mensagem de logout a enviar ao AS*/
int parse_logout(char *message, char *uid, char *password){
    if(login_check(uid) == 0){
        printf("User not logged in\n");
        memset(message, 0, strlen(message));
        return -1;
    }
    strcat(message, uid);
    strcat(message, " \0");
    strcat(message, password);
    return 0;
}

/*escreve a mensagem de unregister para enviar ao AS*/
int parse_unregister(char *message, char *uid, char *password){
    if(login_check(uid) == 0){
        printf("User not logged in\n");
        memset(message, 0, strlen(message));
        return -1;
    }
    strcat(message, uid);
    strcat(message, " \0");
    strcat(message, password);
    return 0;
}

/*valida todos os parametros dados pelo utilizador, escreve parte da mensagem
    a ser enviada para o AS, ate ao Fname + ' '     */
int parse_open(char *message, char *uid, char *password){
    char name[11] = {0};
    char fname[25] = {0};
    char start_value[7] = {0};
    char time_active[6] = {0};

    if (login_check(uid) == 0){
        printf("User not logged in\n");
        return -1;
    }

    strcat(message, uid);
    strcat(message, " \0");
    strcat(message, password);
    strcat(message, " \0");

    if (scanf("%s", name) == -1 || strlen(name) > 10 || !isalphanum(name)){
        memset(message, 0, strlen(message));
        printf("Name error\n");
        return -1;
    }
    if (scanf("%s", fname) == -1 || strlen(fname) > 24 || !isalphanum_plus(fname)){
        memset(message, 0, strlen(message));
        printf("Fname error\n");
        return -1;
    }
    if (scanf("%s", start_value) == -1 || strlen(start_value) > 6 || !isnum(start_value)){
        memset(message, 0, strlen(message));
        printf("Start Value error\n");
        return -1;
    }
    if (scanf("%s", time_active) == -1 || 0 >= strlen(time_active) > 5 || !isnum(time_active)){
        memset(message, 0, strlen(message));
        printf("Time actie error\n");
        return -1;
    }

    strcat(message, name);
    strcat(message, " \0");
    strcat(message, start_value);
    strcat(message, " \0");
    strcat(message, time_active);
    strcat(message, " \0");
    strcat(message, fname);
    strcat(message, " \0");

    return 0;
}

/*Valida os parametros de input e escreve toda a mesnagem a ser enviada ao AS*/
int parse_close(char *message, char *uid, char *password){
    char aid[4] = {0};
    if (login_check(uid) == 0){
        printf("User not logged in\n");
        return -1;
    }

    strcat(message, uid);
    strcat(message, " \0");
    strcat(message, password);
    strcat(message, " \0");

    if (scanf("%s", aid) == -1 || strlen(aid) != 3 || !isnum(aid)){
        memset(message, 0, strlen(message));
        printf("Invalid AID\n");
        return -1;
    }
    strcat(message, aid);
    return 0;
}

/*verifica se o user esta logged in e escreve a mensagem a enviar ao AS*/
int parse_myauctions(char *message, char *uid){
    
    if(login_check(uid) == 0){
        printf("User not logged in\n");
        memset(message, 0, strlen(message));
        return -1;
    }

    strcat(message, uid);
    return 0;
}

/*verifica se o user esta logged in e escreve a mensagem a enviar ao AS*/
int parse_mybids(char *message, char *uid){
    
    if(login_check(uid) == 0){
        printf("User not logged in\n");
        memset(message, 0, strlen(message));
        return -1;
    }

    strcat(message, uid);
    return 0;
}

/*Valida o parametro de input e escreve toda a mensagem a ser enviada ao AS*/
int parse_show_asset(char *message){
    char aid[4] = {0};
    if (scanf("%s", aid) == -1 || strlen(aid) != 3 || !isnum(aid)){
        memset(message, 0, strlen(message));
        printf("Invalid AID\n");
        return -1;
    }
    strcat(message, aid);
    return 0;
}

int parse_bid(char *message, char *uid, char *password){
    char aid[4] = {0};
    char value[7] = {0};

    if (login_check(uid) == 0){
        printf("User not logged in\n");
        return -1;
    }

    strcat(message, uid);
    strcat(message, " \0");
    strcat(message, password);
    strcat(message, " \0");

    if (scanf("%s", aid) == -1 || strlen(aid) != 3 || !isnum(aid)){
        memset(message, 0, strlen(message));
        printf("Invalid AID\n");
        return -1;
    }
    strcat(message, aid);
    strcat(message, " \0");

    if (scanf("%s", value) == -1 || strlen(value) > 5 || !isnum(value)){
        memset(message, 0, strlen(message));
        printf("Invalid value\n");
        return -1;
    }
    strcat(message, value);
    strcat(message, " \0");
    return 0;
}

int parse_show_record(char *message){
    char aid[4] = {0};
    if (scanf("%s", aid) == -1 || strlen(aid) != 3 || !isnum(aid)){
        memset(message, 0, strlen(message));
        printf("Invalid AID\n");
        return -1;
    }
    strcat(message, aid);
    return 0;
}

int parse_action(char *action, char *message, char *uid, char *password){
    if (!strcmp(action, "login")){
        strcpy(message, "LIN ");
        return LOGIN;
    }if (!strcmp(action, "logout")){
        strcpy(message, "LOU ");
        return LOGOUT;
    }if (!strcmp(action, "unregister")){
        strcpy(message, "UNR ");
        return UNREGISTER;
    }if (!strcmp(action, "exit")){
        strcpy(message, "EXIT");
        return EXIT;
    }if (!strcmp(action, "open")){
        strcpy(message, "OPA ");
        return OPEN;
    }if (!strcmp(action, "close")){
        strcpy(message, "CLS ");
        return CLOSE;
    }if (!strcmp(action, "myauctions") || !strcmp(action, "ma")){
        strcpy(message, "LMA ");
        return MYAUCTIONS;
    }if (!strcmp(action, "mybids") || !strcmp(action, "mb")){
        strcpy(message , "LMB ");
        return MYBIDS;
    }if (!strcmp(action, "list") || !strcmp(action, "l")){
        strcpy(message, "LST");
        return LIST;
    }if (!strcmp(action, "show_asset") || !strcmp(action, "sa")){
        strcpy(message, "SAS ");
        return SHOW_ASSET;
    }if (!strcmp(action, "bid") || !strcmp(action, "b")){
        strcpy(message, "BID ");
        return BID;
    }if (!strcmp(action, "show_record") || !strcmp(action, "sr")){
        strcpy(message, "SRC ");
        return SHOW_RECORD;
    }return -1;
}

int parser(char *message, char *uid, char *password){
    char action_in[20] = {0};
    int action;
    
    scanf("%s", action_in);
    if ((action = parse_action(action_in, message, uid, password)) == -1){
        printf("Ação inválida\n");
        return -1;
    }
    
    switch (action){
        case LOGIN:
            if (parse_login(message, uid, password) == -1){
                printf("Login error\n");
                return -1;
            }
            break;
        case LOGOUT:
            if (parse_logout(message, uid, password) == -1){
                printf("Logout error\n");
                return -1;
            }
            break;
        case UNREGISTER:
            if (parse_unregister(message, uid, password) == -1){
                printf("Unregister error\n");
                return -1;
            }
            break;
        case EXIT:
            break;
        case OPEN:
            if (parse_open(message, uid, password) == -1){
                printf("Opening error\n");
                return -1;
            }
            break;
        case CLOSE:
            if (parse_close(message, uid, password) == -1){
                printf("Closing error\n");
                return -1;
            }
            break;
        case MYAUCTIONS:
            if (parse_myauctions(message, uid) == -1){
                printf("Myauctions error\n");
                return -1;
            }
            break;
        case MYBIDS:
            if (parse_mybids(message, uid) == -1){
                printf("Mybids error\n");
                return -1;
            }
            break;
        case LIST:

            break;
        case SHOW_ASSET:
            if (parse_show_asset(message) == -1){
                printf("Show_asset error\n");
                return -1;
            }
            break;
        case BID:
            if (parse_bid(message, uid, password) == -1){
                printf("Bidding error\n");
                return -1;
            }
            break;
        case SHOW_RECORD:
            if (parse_show_record(message) == -1){
                printf("Show_record error\n");
                return -1;
            }
            break;
        defualt:
            printf("Action error\n");
            return -1;
    }
}


int main(){
    char message[100] = {0};
    char uid[7] = {0};
    char password[9] = {0};

    while(strcmp(message, "EXIT")){
        if(parser(message, uid, password) == -1){
            printf("%s\n", message);
        }else
            printf("%s\n", message);
        
        memset(message, 0, strlen(message));
        //printf("%s--%s\n", uid, password);
    }
    return 0;
}