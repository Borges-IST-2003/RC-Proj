#include "server.h"

int user_create(char *filepath, char *user_uid, char *password) { // Creates user
    FILE *fp;
    int registred = 0;
    char user_dir[USER_DIR_SIZE] = {0};
    memcpy(user_dir, filepath, USER_DIR_SIZE - 1);

    if (mkdir(user_dir, 0777) != 0) {
        // Check if the directory already exists
        if (errno != EEXIST) {
            perror("Error creating user directory");
            return ERR;
        }
    }

    if ((fp = fopen(filepath, "w")) == NULL) {
        printf("Error creating file.\n");
        printf("Error number: %d\n", errno); 
        return ERR;
    }

    if (fprintf(fp, "%s", user_uid) != strlen(user_uid)) {
        printf("Error writing on file.\n");
        return ERR;
    }

    if (fclose(fp) != 0) {
        printf("Error closing file.\n");
        return ERR;
    }

    // File path to the "login.txt"
    sprintf(filepath, "%s%s%s%s%s", USERS_DIR, user_uid, "/", user_uid, "_pass.txt"); // Watch out ! Bad implemention

    if ((fp = fopen(filepath, "r")) == NULL) {
        if(errno == ENOENT){
            if ((fp = fopen(filepath, "r")) == NULL)
                //!!!!
        }
        else {
            perror("Error creating file.\n");
            printf("Error number: %d\n", errno); 
            return ERR;
        }
    }

    if (fprintf(fp, "%s", password) != PASS_SIZE - 1) {
        perror("Error writing on file.\n");
        return ERR;
    }

    if (fclose(fp) != 0) {
        printf("Error closing file.\n");
        return ERR;
    }

    return LOGIN_REG;
}

int user_delete(char *pass_filepath, char *login_filepath) { // deletes user's files and directory
    char user_dir[USER_DIR_SIZE] = {0};

    memcpy(user_dir, pass_filepath, USER_DIR_SIZE - 1);

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

    if (remove(user_dir) != 0) {
        perror("Error removing user directory");
        return ERR;
    }

    return STATUS_OK;
}

int user_login(char *buffer) {
    FILE *file;
    char password[PASS_SIZE] = {0};         // user password read from the socket
    char user_uid[USER_UID_SIZE] = {0};     // user uid read from the socket
    char filepath[MAX_FILE_LENGH] = {0};    // password file path
    char pass[PASS_SIZE] = {0};             // the pass read from the file

    // Getting the user uid, and his password
    sscanf(buffer + 3, "%s %s\n", user_uid, password);
    printf("|%s| |%s|\n", user_uid, password);

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
    sprintf(filepath, "%s%s%s%s%s", USERS_DIR, user_uid, "/", user_uid, "_login.txt"); // Watch out ! Bad implemention

    if ((file = fopen(filepath, "r")) == NULL){ // If has error opening the file
        if(errno == ENOENT)
            return user_create(filepath, user_uid, password);

        else{
            printf("Error opening the login file");
            return ERR;
        }
    }

    if (fclose(file) != 0) {
        printf("Error closing file.\n");
        return ERR;
    }

    sprintf(filepath, "%s%s%s%s%s", USERS_DIR, user_uid, "/", user_uid, "_pass.txt"); // Watch out ! Bad implemention
    if ((file = fopen(filepath, "r")) == NULL){ // If has error opening the file
        printf("Error opening the pass file");
        return ERR;
    }


    fseek(file, 0, SEEK_SET);

    if (fread(pass, sizeof(char), PASS_SIZE - 1, file) != PASS_SIZE - 1){
        printf("Error reading the file.\n");
        return ERR;
    }

    if (strcmp(pass, password) != 0)
        return STATUS_NOK;

    if (fclose(file) != 0) {
        printf("Error closing file.\n");
        return ERR;
    }

    return STATUS_OK;
}

int user_unregister(char *buffer) {
    FILE *fp;
    char password[PASS_SIZE] = {0};         // user password read from the socket
    char user_uid[USER_UID_SIZE] = {0};     // user uid read from the socket
    char pass_filepath[MAX_FILE_LENGH] = {0};    // password file path
    char login_filepath[MAX_FILE_LENGH] = {0};    // login file path
    char pass[PASS_SIZE] = {0};             // the pass read from the file

    // Getting the user uid, and his password
    sscanf(buffer + 3, "%s %s\n", user_uid, password);
    printf("|%s| |%s|\n", user_uid, password);

    // Creating the file paths
    sprintf(pass_filepath, "%s%s%s%s%s", USERS_DIR, user_uid, "/", user_uid, "_pass.txt"); // !
    sprintf(login_filepath, "%s%s%s%s%s", USERS_DIR, user_uid, "/", user_uid, "_login.txt"); // !

    if ((fp = fopen(pass_filepath, "r")) == NULL){ // If has error opening the file w/ password
        if(errno == ENOENT)
            return UNREG_UNR;

        else{
            printf("Error looking for pass file");
            return ERR;
        }
    }

    if (fread(pass, sizeof(char), PASS_SIZE-1, fp) != PASS_SIZE-1){
        printf("Error reading the file.\n");
        return ERR;
    }

    if (fclose(fp) != 0) {
        printf("Error closing file.\n");
        return ERR;
    }

    // Checks if user is logged in
    if ((fp = fopen(login_filepath, "r")) == NULL){ // If doesn't find the login file
        if(errno == ENOENT) {
            printf("Error looking for login file.");
            return STATUS_NOK;
        }

        else {
            printf("Error opening the login file.");
            return ERR;
        }
    }

    if (fclose(fp) != 0) {
        printf("Error closing file.\n");
        return ERR;
    }

    if(strcmp(pass, password) != 0){
        printf("Passwords doesn't meet.");
        return STATUS_NOK;
    }

    return user_delete(pass_filepath, login_filepath);
}

int user_logout(char *buffer) {
    FILE *file;
    char user_uid[USER_UID_SIZE] = {0};     // user uid read from the socket
    char login_filepath[MAX_FILE_LENGH] = {0};    // password file path
    char pass_filepath[MAX_FILE_LENGH] = {0};
    
    // Getting the user uid, and his password
    strncpy(user_uid, buffer + CODE_SIZE, USER_UID_SIZE - 1);

    sprintf(login_filepath, "%s%s%s%s%s", USERS_DIR, user_uid, "/", user_uid, "_login.txt"); // Watch out ! Bad implemention
    sprintf(pass_filepath, "%s%s%s%s%s", USERS_DIR, user_uid, "/", user_uid, "_pass.txt"); // Watch out ! Bad implemention
    
    if((file = fopen(pass_filepath, "r")) == NULL){
        if(errno == ENOENT)
            return LOGOUT_UNR;
        else {
            printf("Error opening login file.\n");
            return ERR;
        }
    }

    if(fclose(file) != 0){
        printf("Error closing login file.\n");
        return ERR;
    }

    if(remove(login_filepath) != 0){
        if(errno == ENOENT)
            return STATUS_NOK;
        else {
            printf("Error removing login file.\n");
            return ERR;
        }
    }

    return STATUS_OK;
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