#include "server.h"

int create_login(char* user_uid, char* login_filepath){
    FILE *file;

    if((file = fopen(login_filepath, "w")) == NULL){
        printf("Error creating login file.\n");
        return ERR;
    }

    if(fprintf(file, "%s", user_uid) != USER_UID_SIZE - 1){
        printf("Error writing on login file.\n");
        return ERR;
    }

    if (fclose(file) != 0) {
        printf("Error closing login file.\n");
        return ERR;
    }

    return STATUS_OK;                               // returns 0
}

int check_password(char* password, char* pass_filepath){
    FILE *file;
    char pass[PASS_SIZE] = {0};                     // pass read from the file '_pass.txt'

    if((file = fopen(pass_filepath, "r")) == NULL){
        printf("Error opening the pass file.\n");
        return ERR;
    }

    if(fread(pass, sizeof(char), PASS_SIZE-1, file) != PASS_SIZE-1){
        printf("Error reading the pass file.\n");
        return ERR;
    }

    if(fclose(file) != 0){
        printf("Error closing pass file.\n");
        return ERR;
    }

    if(strcmp(pass, password) != 0)
        return STATUS_NOK;

    return STATUS_OK;
}

int user_create(char *login_filepath, char *pass_filepath, char *user_uid, char *password) { // Creates user
    FILE *file;
    char user_dir[USER_DIR_SIZE] = {0};

    // Get's the directory to be created (eg. user_dir = 'USERS/103074/')
    memcpy(user_dir, login_filepath, USER_DIR_SIZE - 1);

    // Creates the directory of the user (eg. USERS/'103074'/)
    if (mkdir(user_dir, 0777) != 0) {
        if (errno != EEXIST) {
            printf("Error creating user directory.\n");
            return ERR;
        }
    }

    // Creates password file. (eg. USERS/103074/'103074_pass.txt')
    if((file = fopen(pass_filepath, "w")) == NULL){
        printf("Error creating pass file.\n");
        return ERR;
    }

    if(fprintf(file, "%s", password) != PASS_SIZE - 1){
        printf("Error writing on pass file.\n");
        return ERR;
    }

    if (fclose(file) != 0) {
        printf("Error closing pass file.\n");
        return ERR;
    }

    if (create_login(user_uid, login_filepath) != STATUS_OK)
        return ERR;

    return LOGIN_REG;
}

int user_login(char *buffer) {
    int check_pass_result;
    char password[PASS_SIZE] = {0};                 // user password read from the socket
    char user_uid[USER_UID_SIZE] = {0};             // user uid read from the socket
    char login_filepath[MAX_FILE_LENGH] = {0};      // login file path
    char pass_filepath[MAX_FILE_LENGH] = {0};       // password file path

    // Getting the user uid, and his password
    sscanf(buffer + 3, "%s %s\n", user_uid, password);

    // Checks password format
    if(strlen(password) != PASS_SIZE - 1){
        printf("Wrong password format.");
        return ERR;
    }

    // Checks user_uid format
    if(strlen(user_uid) != USER_UID_SIZE - 1){
        printf("Wrong user id format.");
        return ERR;
    }

    // Creating the file path
    sprintf(login_filepath, "%s%s%s%s%s", USERS_DIR, user_uid, "/", user_uid, "_login.txt"); // Watch out ! Bad implemention
    sprintf(pass_filepath, "%s%s%s%s%s", USERS_DIR, user_uid, "/", user_uid, "_pass.txt"); // Watch out ! Bad implemention
    
    if(access(login_filepath, F_OK) != -1)
        return STATUS_OK;

    if(access(pass_filepath, F_OK) == -1)
        return user_create(login_filepath, pass_filepath, user_uid, password);

    if((check_pass_result = check_password(password, pass_filepath)) == STATUS_OK)
        return create_login(user_uid, login_filepath);
    
    return check_pass_result;
}

int user_delete(char *pass_filepath, char *login_filepath) { // deletes user's files and directory
    //char user_dir[USER_DIR_SIZE] = {0};
    //memcpy(user_dir, pass_filepath, USER_DIR_SIZE - 1);

    // Removes the password file
    if (remove(pass_filepath) != 0) {
        perror("Error removing password file");
        return ERR;
    }

    // Remove the login file
    if (remove(login_filepath) != 0) {
        perror("Error removing login file");
        return ERR;
    }

    /*
    if (remove(user_dir) != 0) {
        perror("Error removing user directory");
        return ERR;
    }
    */

    return STATUS_OK;
}

int user_unregister(char *buffer) {
    int check_pass_result;
    char password[PASS_SIZE] = {0};         // user password read from the socket
    char user_uid[USER_UID_SIZE] = {0};     // user uid read from the socket
    char pass_filepath[MAX_FILE_LENGH] = {0};    // password file path
    char login_filepath[MAX_FILE_LENGH] = {0};    // login file path

    // Getting the user uid, and his password
    sscanf(buffer + 3, "%s %s\n", user_uid, password);
    printf("|%s| |%s|\n", user_uid, password);

    // Creating the file paths
    sprintf(pass_filepath, "%s%s%s%s%s", USERS_DIR, user_uid, "/", user_uid, "_pass.txt"); // !
    sprintf(login_filepath, "%s%s%s%s%s", USERS_DIR, user_uid, "/", user_uid, "_login.txt"); // !

    if (access(login_filepath, F_OK) == -1)
        return STATUS_NOK;

    if (access(pass_filepath, F_OK) == -1)
        return UNREG_UNR;

    if((check_pass_result = check_password(password, pass_filepath)) == STATUS_OK)
        return user_delete(pass_filepath, login_filepath);

    return check_pass_result;
}

int user_logout(char *buffer) {
    int check_pass_result;
    char user_uid[USER_UID_SIZE] = {0};             // user uid read from the socket
    char password[PASS_SIZE] = {0};                 // password read from the buffer received
    char login_filepath[MAX_FILE_LENGH] = {0};      // login file path
    char pass_filepath[MAX_FILE_LENGH] = {0};       // password file path

    // Getting the user uid, and his password
    sscanf(buffer + 3, "%s %s\n", user_uid, password);

    // Creating the path's
    sprintf(login_filepath, "%s%s%s%s%s", USERS_DIR, user_uid, "/", user_uid, "_login.txt"); // Watch out ! Bad implemention
    sprintf(pass_filepath, "%s%s%s%s%s", USERS_DIR, user_uid, "/", user_uid, "_pass.txt"); // Watch out ! Bad implemention

    if(access(login_filepath, F_OK) == -1)
        return STATUS_NOK;

    if(access(pass_filepath, F_OK) == -1)
        return LOGOUT_UNR;

    if((check_pass_result = check_password(password, pass_filepath)) == STATUS_OK){
        // Remove the login file
        if (remove(login_filepath) != 0) {
            perror("Error removing login file");
            return ERR;
        }
        return STATUS_OK;
    }

    return check_pass_result;
}

int main() {
    char message_code[CODE_SIZE] = {0};
    char answer[MAX_MSG_LEN] = {0};
    // UDP setup
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if (udp_fd == -1)                        /*error*/
        exit(1);

    memset(&udp_hints, 0, sizeof udp_hints);
    udp_hints.ai_family = AF_INET;      // IPv4
    udp_hints.ai_socktype = SOCK_DGRAM; // UDP socket
    udp_hints.ai_flags = AI_PASSIVE;

    udp_errcode = getaddrinfo(NULL, PORT, &udp_hints, &udp_res);
    if (udp_errcode != 0) /*error*/
        exit(1);

    udp_n = bind(udp_fd, udp_res->ai_addr, udp_res->ai_addrlen);
    if (udp_n == -1) /*error*/
        exit(1);

    // TCP setup

    if ((tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) // TCP socket
        exit(1);                                          // error

    memset(&tcp_hints, 0, sizeof tcp_hints);
    tcp_hints.ai_family = AF_INET;       // IPv4
    tcp_hints.ai_socktype = SOCK_STREAM; // TCP socket
    tcp_hints.ai_flags = AI_PASSIVE;

    if ((tcp_errcode = getaddrinfo(NULL, PORT, &tcp_hints, &tcp_res)) != 0) /*error*/
        exit(1);

    if ((tcp_n = bind(tcp_fd, tcp_res->ai_addr, tcp_res->ai_addrlen)) == -1) /*error*/
        exit(1);

    if (listen(tcp_fd, 5) == -1) /*error*/
        exit(1);

    // Main Loop
    while (stop != 1)
    {
        //fd_set read_fds;
        //FD_ZERO(&read_fds);
        //FD_SET(udp_fd, &read_fds);
        //FD_SET(tcp_fd, &read_fds);
//
        //int max_fd = max(udp_fd, tcp_fd);
//
        //if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1)
        //    exit(1);
//
        //// Handle UDP activity
        //if (FD_ISSET(udp_fd, &read_fds))
        //{
        memset(answer, 0, MAX_MSG_LEN);
        udp_addrlen = sizeof(udp_addr);
        udp_n = recvfrom(udp_fd, buffer, 128, 0, (struct sockaddr *)&udp_addr, &udp_addrlen);
        if (udp_n == -1) /*error*/
            exit(1);

        memcpy(message_code, buffer, 3);
        write(1, "received: ", 10);
        write(1, buffer, udp_n);
        if(strcmp(message_code, "LIN") == 0){
            switch (user_login(buffer)){
                case STATUS_OK:
                    strcpy(answer, "RLI OK");
                    break;
                
                case STATUS_NOK:
                    strcpy(answer, "RLI NOK");
                    break;
                
                case LOGIN_REG:
                    strcpy(answer, "RLI REG");
                    break;

                default:
                    strcpy(answer, "RLI ERR");
                    break;
            }
        }

        else if(strcmp(message_code, "UNR") == 0){
            switch (user_unregister(buffer)){
                case STATUS_OK:
                    strcpy(answer, "RUR OK");
                    break;
                
                case STATUS_NOK:
                    strcpy(answer, "RUR NOK");
                    break;
                
                case UNREG_UNR:
                    strcpy(answer, "RUR UNR");
                    break;
                
                default:
                    strcpy(answer, "RUR ERR");
                    break;
            }
        }

        else if(strcmp(message_code, "LOU") == 0){
            switch (user_logout(buffer)){
                case STATUS_OK:
                    strcpy(answer, "RLO OK");
                    break;
                
                case STATUS_NOK:
                    strcpy(answer, "RLO NOK");
                    break;
                
                case LOGOUT_UNR:
                    strcpy(answer, "RLO UNR");
                    break;
                
                default:
                    strcpy(answer, "RLO ERR");
                    break;
            }
        }
        udp_n = sendto(udp_fd, answer, strlen(answer), 0,(struct sockaddr *)&udp_addr, udp_addrlen);
            if (udp_n == -1) /*error*/
                exit(1);
        
        //}

        // Handle TCP activity
//        if (FD_ISSET(tcp_fd, &read_fds))
//        {
//            // Code to handle TCP connection
//        }
    }

    freeaddrinfo(udp_res);
    close(udp_fd);

    return 0;
}