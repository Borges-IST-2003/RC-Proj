#include "server.h"

// Creates the login file
int create_login(const char* user_uid, const char* login_filepath){
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

// Checks if the password given is the same in the pass file
int check_password(const char* password, const char* pass_filepath){
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

// Creates user's pass and login files & (if didn't previously exist) USER_UID, HOSTED, BIDDED dirs
int user_create(const char *login_filepath, const char *pass_filepath, const char *user_uid, const char *password) { // Creates user
    FILE *file;
    char user_dir[USER_DIR_SIZE] = {0};
    char user_hosted_dir[USER_SUB_DIR_LENGTH] = {0};
    char user_bidded_dir[USER_SUB_DIR_LENGTH] = {0};

    // Get's the directory to be created (eg. user_dir = 'USERS/103074/')
    memcpy(user_dir, login_filepath, USER_DIR_SIZE - 1);
    sprintf(user_hosted_dir, "%s/HOSTED", user_dir);
    sprintf(user_bidded_dir, "%s/BIDDED", user_dir);

    // Creates the directory of the user (eg. USERS/'103074'/)
    if (mkdir(user_dir, 0777) != 0) {
        if (errno != EEXIST) {
            printf("Error creating user directory.\n");
            return ERR;
        }
    }

    // Creates the hosted sub-directory of the user (eg. USERS/103074/'HOSTED')
    if (mkdir(user_hosted_dir, 0777) != 0) {
        if (errno != EEXIST) {
            printf("Error creating user directory.\n");
            return ERR;
        }
    }

    // Creates the bidded sub-directory of the user (eg. USERS/103074/'BIDDED')
    if (mkdir(user_bidded_dir, 0777) != 0) {
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

// Deletes user's pass and login files
int user_delete(const char *pass_filepath, const char *login_filepath) {
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

int user_login(char *buffer) {
    int check_pass_result;
    char password[PASS_SIZE] = {0};                 // user password read from the socket
    char user_uid[USER_UID_SIZE] = {0};             // user uid read from the socket
    char login_filepath[MAX_FILE_LENGTH] = {0};      // login file path
    char pass_filepath[MAX_FILE_LENGTH] = {0};       // password file path

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
    sprintf(login_filepath, "%s%s/%s_login.txt", USERS_DIR, user_uid, user_uid); 
    sprintf(pass_filepath, "%s%s/%s_pass.txt", USERS_DIR, user_uid, user_uid); 
    
    if(access(login_filepath, F_OK) != -1)
        return STATUS_OK;

    if(access(pass_filepath, F_OK) == -1)
        return user_create(login_filepath, pass_filepath, user_uid, password);

    if((check_pass_result = check_password(password, pass_filepath)) == STATUS_OK)
        return create_login(user_uid, login_filepath);

    return check_pass_result;
}

int user_unregister(char *buffer) {
    int check_pass_result;
    char password[PASS_SIZE] = {0};         // user password read from the socket
    char user_uid[USER_UID_SIZE] = {0};     // user uid read from the socket
    char pass_filepath[MAX_FILE_LENGTH] = {0};    // password file path
    char login_filepath[MAX_FILE_LENGTH] = {0};    // login file path

    // Getting the user uid, and his password
    sscanf(buffer + 3, "%s %s\n", user_uid, password);
    printf("|%s| |%s|\n", user_uid, password);

    // Creating the file paths
    sprintf(login_filepath, "%s%s/%s_login.txt", USERS_DIR, user_uid, user_uid); 
    sprintf(pass_filepath, "%s%s/%s_pass.txt", USERS_DIR, user_uid, user_uid); 

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
    char login_filepath[MAX_FILE_LENGTH] = {0};      // login file path
    char pass_filepath[MAX_FILE_LENGTH] = {0};       // password file path

    // Getting the user uid, and his password
    sscanf(buffer + 3, "%s %s\n", user_uid, password);

    // Creating the paths
    sprintf(login_filepath, "%s%s/%s_login.txt", USERS_DIR, user_uid, user_uid); 
    sprintf(pass_filepath, "%s%s/%s_pass.txt", USERS_DIR, user_uid, user_uid); 

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

int list_user_auctions(char *buffer, char *answer) {
    DIR *dir;
    struct dirent *entry;
    int is_open = 0;                                    // nº of files in a dir
    char user_uid[USER_UID_SIZE] = {0};                 // user uid read from the socket
    char login_filepath[MAX_FILE_LENGTH] = {0};         // login file path
    char hosted_dir[USER_SUB_DIR_LENGTH] = {0};         // hosted directory

    sscanf(buffer + 3, "%s\n", user_uid);

    sprintf(login_filepath, "%s%s/%s_login.txt", USERS_DIR, user_uid, user_uid);
    sprintf(hosted_dir, "%s%s/HOSTED", USERS_DIR, user_uid);

    if(access(login_filepath, F_OK) == -1)
        return RMA_NLG;

    // Open the directory
    dir = opendir(hosted_dir);
    if (dir == NULL) {
        perror("Error opening directory");
        return ERR;
    }

    // Reset the directory stream
    rewinddir(dir);

    strcat(answer, "RMA OK ");
    // Concatenate file names into the allocated memory
    while ((entry = readdir(dir)) != NULL){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        is_open++;
        strncat(answer, entry->d_name, 3);
        strcat(answer, " ");
    }

    // # Missing states

    // Close the directory
    closedir(dir);

    if (is_open == 0)       // If nº files == 0, dir is empty
        return STATUS_NOK;

    return STATUS_OK;
}

int list_user_bids(char *buffer, char *answer) {
    DIR *dir;
    struct dirent *entry;
    int is_open= 0;                                     // nº of files in a dir
    char user_uid[USER_UID_SIZE] = {0};                 // user uid read from the socket
    char login_filepath[MAX_FILE_LENGTH] = {0};         // login file path
    char bidded_dir[USER_SUB_DIR_LENGTH] = {0};         // bidded directory

    sscanf(buffer + 3, "%s\n", user_uid);

    sprintf(login_filepath, "%s%s/%s_login.txt", USERS_DIR, user_uid, user_uid);
    sprintf(bidded_dir, "%s%s/BIDDED", USERS_DIR, user_uid);

    if(access(login_filepath, F_OK) == -1)
        return RMB_NLG;

    // Open the directory
    dir = opendir(bidded_dir);
    if (dir == NULL) {
        perror("Error opening directory");
        return ERR;
    }

    // Reset the directory stream
    rewinddir(dir);

    strcat(answer, "RMB OK ");
    // Concatenate file names into the allocated memory
    while ((entry = readdir(dir)) != NULL){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        is_open++;
        strncat(answer, entry->d_name, 3);
        strcat(answer, " ");
    }

    // # Missing states

    // Close the directory
    closedir(dir);

    if (is_open == 0)       // If nº files == 0, dir is empty
        return STATUS_NOK;

    return STATUS_OK;
}

int list_auctions(char *answer) {
    DIR *dir;
    struct dirent *entry;
    int is_open = 0;

    // Open the directory
    dir = opendir(AUCTIONS_DIR);
    if (dir == NULL) {
        perror("Error opening directory");
        return ERR;
    }

    // Reset the directory stream
    rewinddir(dir);

    strcat(answer, "RLS OK ");
    // Concatenate file names into the allocated memory
    while ((entry = readdir(dir)) != NULL){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // There will be sub directories #
        is_open++;
        strncat(answer, entry->d_name, 3);
        strcat(answer, " ");
    }

    // # Missing states

    // Close the directory
    closedir(dir);

    if (is_open == 0)       // If nº files == 0, dir is empty
        return STATUS_NOK;

    return STATUS_OK;
}



int max(int a, int b){
    return (a > b ? a : b);
}

void udp_handler(){
    char message_code[CODE_SIZE] = {0};
    char answer[MAX_MSG_LEN] = {0};
    udp_addrlen = sizeof(udp_addr);
    udp_n = recvfrom(udp_fd, buffer, MAX_MSG_LEN, 0, (struct sockaddr *)&udp_addr, &udp_addrlen);
    if (udp_n == -1)
        exit(1);

    memcpy(message_code, buffer, 3);
    write(1, "received: ", 10);         // #
    write(1, buffer, udp_n);            // #
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
    
    else if(strcmp(message_code, "LMA") == 0){
        switch (list_user_auctions(buffer, answer)) {
            case STATUS_OK:
                break;
            
            case STATUS_NOK:
                strcpy(answer, "RMA NOK");
                break;
            
            case RMA_NLG:
                strcpy(answer, "RMA UNR");
                break;
            
            default:
                strcpy(answer, "RMA ERR");
                break;
        }
    }

    else if(strcmp(message_code, "LMB") == 0){
        switch (list_user_bids(buffer, answer)) {
            case STATUS_OK:
                break;
            
            case STATUS_NOK:
                strcpy(answer, "RMB NOK");
                break;
            
            case RMB_NLG:
                strcpy(answer, "RMB UNR");
                break;
            
            default:
                strcpy(answer, "RMB ERR");
                break;
        }
    }

    else if(strcmp(message_code, "LST") == 0){
        switch (list_auctions(answer)) {
            case STATUS_OK:
                break;
            
            case STATUS_NOK:
                strcpy(answer, "RLS NOK");
                break;
            
            default:
                strcpy(answer, "RLS ERR");
                break;
        }
    }

    udp_n = sendto(udp_fd, answer, strlen(answer), 0,(struct sockaddr *)&udp_addr, udp_addrlen);
        if (udp_n == -1) /*error*/
            exit(1);
    
    return;
}

void tcp_handler(int tcp_newfd){
    
    tcp_n = read(tcp_newfd, buffer, MAX_MSG_LEN);
    
    if(tcp_n == -1)
        exit(1);
    
    write(1, "received: ", 10);
    write(1, buffer, tcp_n);

    // Code

    tcp_n = write(tcp_newfd, buffer, tcp_n);
    if(tcp_n == -1)
        exit(1);
    
    close(tcp_newfd);
    return;
}

int main() {
    // UDP setup
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd == -1)
        exit(1);

    memset(&udp_hints, 0, sizeof udp_hints);
    udp_hints.ai_family = AF_INET;
    udp_hints.ai_socktype = SOCK_DGRAM;
    udp_hints.ai_flags = AI_PASSIVE;

    if ((udp_errcode = getaddrinfo(NULL, PORT, &udp_hints, &udp_res)) != 0)
        exit(1);

    if ((udp_n = bind(udp_fd, udp_res->ai_addr, udp_res->ai_addrlen)) == -1)
        exit(1);

    // TCP setup
    if ((tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        exit(1);

    memset(&tcp_hints, 0, sizeof tcp_hints);
    tcp_hints.ai_family = AF_INET;
    tcp_hints.ai_socktype = SOCK_STREAM;
    tcp_hints.ai_flags = AI_PASSIVE;

    if ((tcp_errcode = getaddrinfo(NULL, PORT, &tcp_hints, &tcp_res)) != 0)
        exit(1);

    if ((tcp_n = bind(tcp_fd, tcp_res->ai_addr, tcp_res->ai_addrlen)) == -1)
        exit(1);

    if (listen(tcp_fd, 5) == -1)
        exit(1);

    // Main Loop
    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(tcp_fd, &read_fds);
        FD_SET(udp_fd, &read_fds);
        int max_fd = max(udp_fd, tcp_fd);
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1)
            exit(1);

        // Handle UDP activity
        if (FD_ISSET(udp_fd, &read_fds))
            udp_handler();

        // Handle TCP activity
        if (FD_ISSET(tcp_fd, &read_fds)){
            tcp_addrlen = sizeof(tcp_addr);
            if((tcp_newfd = accept(tcp_fd, (struct sockaddr*)&tcp_addr, &tcp_addrlen)) == -1)
                exit(1);

            if((child_pid = fork()) == -1)
                exit(1);

            if(child_pid == 0)
                tcp_handler(tcp_newfd);
        }
    }

    freeaddrinfo(udp_res);
    close(udp_fd);
    freeaddrinfo(tcp_res);    
    close(tcp_fd);

    return 0;
}