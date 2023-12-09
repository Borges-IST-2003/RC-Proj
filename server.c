#include "server.h"

// UDP functions

int valid_pass(char* pass){
    if(strlen(pass) != PASS_SIZE - 1) return ERR;
    while (*pass) {
        if (!isalnum(*pass))
            return ERR;                 // Not alphanumeric
        ++pass;                         // Move to the next character
    }
    return STATUS_OK;
}

int valid_uid(char* uid){
    if(strlen(uid) != USER_UID_SIZE - 1) return ERR;
    while (*uid) {
        if (!isdigit(*uid))
            return ERR;                 // Not digit
        ++uid;                          // Move to the next character
    }
    return STATUS_OK;
}

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

int user_login(const char *buffer){
    int check_pass_result;
    char password[PASS_SIZE] = {0};                 // user password read from the socket
    char user_uid[USER_UID_SIZE] = {0};             // user uid read from the socket
    char login_filepath[MAX_FILE_LENGTH] = {0};      // login file path
    char pass_filepath[MAX_FILE_LENGTH] = {0};       // password file path

    // Getting the user uid, and his password
    sscanf(buffer + 3, "%s %s\n", user_uid, password);

    // Checks password format
    if(valid_pass(password) == ERR){
        printf("Wrong password format.\n");
        return ERR;
    }

    // Checks user_uid format
    if(valid_uid(user_uid) == ERR){
        printf("Wrong user id format.\n");
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

int user_unregister(const char *buffer) {
    int check_pass_result;
    char password[PASS_SIZE] = {0};         // user password read from the socket
    char user_uid[USER_UID_SIZE] = {0};     // user uid read from the socket
    char pass_filepath[MAX_FILE_LENGTH] = {0};    // password file path
    char login_filepath[MAX_FILE_LENGTH] = {0};    // login file path

    // Getting the user uid, and his password
    sscanf(buffer + 3, "%s %s\n", user_uid, password);

    // Checks password format
    if(valid_pass(password) == ERR){
        printf("Wrong password format.\n");
        return ERR;
    }

    // Checks user_uid format
    if(valid_uid(user_uid) == ERR){
        printf("Wrong user id format.\n");
        return ERR;
    }

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

int user_logout(const char *buffer) {
    int check_pass_result;
    char user_uid[USER_UID_SIZE] = {0};             // user uid read from the socket
    char password[PASS_SIZE] = {0};                 // password read from the buffer received
    char login_filepath[MAX_FILE_LENGTH] = {0};      // login file path
    char pass_filepath[MAX_FILE_LENGTH] = {0};       // password file path

    // Getting the user uid, and his password
    sscanf(buffer + 3, "%s %s\n", user_uid, password);

    // Checks password format
    if(valid_pass(password) == ERR){
        printf("Wrong password format.\n");
        return ERR;
    }

    // Checks user_uid format
    if(valid_uid(user_uid) == ERR){
        printf("Wrong user id format.\n");
        return ERR;
    }

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

int list_user_auctions(const char *buffer, char *answer) {
    DIR *dir;
    struct dirent *entry;
    int is_open = 0;                                    // nº of files in a dir
    char user_uid[USER_UID_SIZE] = {0};                 // user uid read from the socket
    char login_filepath[MAX_FILE_LENGTH] = {0};         // login file path
    char hosted_dir[USER_SUB_DIR_LENGTH] = {0};         // hosted directory

    sscanf(buffer + 3, "%s\n", user_uid);

    // Checks user_uid format
    if(valid_uid(user_uid) == ERR){
        printf("Wrong user id format.\n");
        return ERR;
    }

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

int list_user_bids(const char *buffer, char *answer) {
    DIR *dir;
    struct dirent *entry;
    int is_open= 0;                                     // nº of files in a dir
    char user_uid[USER_UID_SIZE] = {0};                 // user uid read from the socket
    char login_filepath[MAX_FILE_LENGTH] = {0};         // login file path
    char bidded_dir[USER_SUB_DIR_LENGTH] = {0};         // bidded directory

    sscanf(buffer + 3, "%s\n", user_uid);

    // Checks user_uid format
    if(valid_uid(user_uid) == ERR){
        printf("Wrong user id format.\n");
        return ERR;
    }

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

// TCP functions

int tcp_read(char* buffer, int len){ //jorge = bytes read
    int jorge = 0;
    while(len > 0){
        jorge = read(tcp_newfd, buffer, len);
        if(jorge == -1)
            return ERR;

        if(jorge == 0) //closed by peer
            break;

        len -= jorge;
        buffer += jorge;
    }

    return STATUS_OK;
}

int tcp_write(char* buffer, int len){ //jorge = bytes written
    int jorge = 0;
    while(len > 0){
        jorge = write(tcp_newfd, buffer, len);
        if(jorge == -1){
            return ERR;
        }
        len -= jorge;
        buffer += jorge;
    }
    return STATUS_OK;
}

// Like the tcp_read but doesn't have a fix size
int read_next_word(char *buffer, int max){
    int i = 0;
    while (1) {
        if (tcp_read(&buffer[i], 1) == ERR)
            return ERR;

        if(i >= max-2 || buffer[i] == ' ')
            break;

        if(buffer[i] == '\n' || buffer[i] == '\0')
            return STATUS_NOK;

        i++;
    }

    buffer[i] = '\0';  // Null-terminate the string

    return STATUS_OK;
}

int open(const char* buffer){
    // #
    // # Verify everything
    char user_uid[USER_UID_SIZE] = {0};
    char password[PASS_SIZE] = {0};
    char name[AUCT_NAME] = {0};
    char start_value[START_VALUE] = {0};
    char time_active[TIME_ACTIVE] = {0};
    char img_name[DEFAULT] = {0};
    char file_size_str[5] = {0};
    int file_size = 0;
    char login_filepath[MAX_FILE_LENGTH] = {0};      // login file path
    char pass_filepath[MAX_FILE_LENGTH] = {0};       // password file path
    int check_pass_result, aux = STATUS_OK;             // Remove = STATUS_OK

    if(sscanf(buffer, "%s %s ", user_uid, password) != 2){
        printf("Wrong format in open() function.\n");
        return ERR;
    }

    if(read_next_word(name, AUCT_NAME) != STATUS_OK){
        printf("Error reading the name in open().\n");
        return ERR;
    }

    if(read_next_word(start_value, START_VALUE) != STATUS_OK){
        printf("Error reading the start_value in open().\n");
        return ERR;
    }

    if(read_next_word(time_active, TIME_ACTIVE) != STATUS_OK){
        printf("Error reading the time_active in open().\n");
        return ERR;
    }

    if(read_next_word(img_name, DEFAULT) != STATUS_OK){
        printf("Error reading the img_name in open().\n");
        return ERR;
    }

    if(read_next_word(file_size_str, 5) != STATUS_OK){
        printf("Error reading the img_name in open().\n");
        return ERR;
    }

    if((file_size = atoi(file_size_str)) == 0){
        printf("A non valid integer read in file_size_str.\n");
        return ERR;
    }
    file_size++; //'\0' space :)

    char *file_data = (char *)malloc(file_size * sizeof(char)); 
    if(tcp_read(file_data, file_size) == ERR){
        printf("Error reading image file from socket.\n");
        return ERR;
    }
    file_data[strlen(file_data)- 1] = '\0';         // Subs the '\n' with '\0';

    printf("|%s|\n", file_data);

    sprintf(login_filepath, "%s%s/%s_login.txt", USERS_DIR, user_uid, user_uid);
    sprintf(pass_filepath, "%s%s/%s_pass.txt", USERS_DIR, user_uid, user_uid);
    if(access(login_filepath, F_OK) != -1)
        return ROA_NLG;
    
    if((check_pass_result = check_password(password, pass_filepath)) == STATUS_OK){
        // # aux = create_auction(name, time_active, img_name, file_size, file_data);
        free(file_data);
        return aux;
    }

    free(file_data);

    return check_pass_result;
}

int max(int a, int b){
    return (a > b ? a : b);
}

void udp_handler(){
    char buffer[MAX_MSG_LEN] = {0};
    char message_code[CODE_SIZE + 1] = {0};
    char answer[MAX_MSG_LEN] = {0};
    udp_addrlen = sizeof(udp_addr);
    udp_n = recvfrom(udp_fd, buffer, MAX_MSG_LEN, 0, (struct sockaddr *)&udp_addr, &udp_addrlen);

    if (udp_n == -1)
        exit(1);
    memcpy(message_code, buffer, CODE_SIZE);
    write(1, "received: ", 10);
    write(1, buffer, udp_n);

    if(strcmp(message_code, "LIN ") == 0){
        if(udp_n != LIN_SIZE)
            strcpy(answer, "RLI ERR");

        else{
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
    }

    else if(strcmp(message_code, "LOU ") == 0){
        udp_n = recvfrom(udp_fd, buffer, MAX_MSG_LEN, 0, (struct sockaddr *)&udp_addr, &udp_addrlen);
        if(udp_n != LOU_SIZE)
            strcpy(answer, "RLO ERR");

        else{
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
    }

    else if(strcmp(message_code, "UNR ") == 0){
        udp_n = recvfrom(udp_fd, buffer, MAX_MSG_LEN, 0, (struct sockaddr *)&udp_addr, &udp_addrlen);
        if(udp_n != UNR_SIZE)
            strcpy(answer, "RUR ERR");

        else {
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
    }
    
    else if(strcmp(message_code, "LMA ") == 0){
        udp_n = recvfrom(udp_fd, buffer, MAX_MSG_LEN, 0, (struct sockaddr *)&udp_addr, &udp_addrlen);
        if(udp_n != LMA_SIZE)
            strcpy(answer, "RMA ERR");

        else {    
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
    }

    else if(strcmp(message_code, "LMB ") == 0){
        udp_n = recvfrom(udp_fd, buffer, MAX_MSG_LEN, 0, (struct sockaddr *)&udp_addr, &udp_addrlen);
        if(udp_n != LMB_SIZE)
            strcpy(answer, "RMB ERR");

        else {   
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
    }

    else if(strcmp(message_code, "LST ") == 0){
        udp_n = recvfrom(udp_fd, buffer, MAX_MSG_LEN, 0, (struct sockaddr *)&udp_addr, &udp_addrlen);
        if(udp_n != LST_SIZE)
            strcpy(answer, "RLS ERR");

        else {
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
    }

    else if(strcmp(message_code, "SRC ") == 0){
        udp_n = recvfrom(udp_fd, buffer, MAX_MSG_LEN, 0, (struct sockaddr *)&udp_addr, &udp_addrlen);
        if(udp_n != SRC_SIZE){
            strcpy(answer, "RRC ERR");
        }
        else{
            return;
        }
    }
    
    else {
        strcpy(answer, "ERR Wrong Format");
    }

    udp_n = sendto(udp_fd, answer, strlen(answer), 0,(struct sockaddr *)&udp_addr, udp_addrlen);
        if (udp_n == -1) /*error*/
            exit(1);

    return;
}

void tcp_handler(int tcp_newfd){
    char buffer[MAX_MSG_LEN] = {0};
    char answer[MAX_MSG_LEN] = {0};
    char message_code[CODE_SIZE+1] = {0};
    if((tcp_read(message_code, CODE_SIZE)) == ERR)
        exit(1);

    printf("Code received: |%s|\n", message_code);

    if(strcmp(message_code, "OPA ") == 0){
        if(tcp_read(buffer, OPEN_SIZE) == ERR)
            exit(1);
        switch (open(buffer)) {

        case STATUS_OK:
            strcpy(answer, "ROA OK");
            break;
        
        default:
            strcpy(answer, "ROA ERR");
            break;
        }
    }

    else if(strcmp(message_code, "CLS ") == 0){
        return;
    }
    
    else if(strcmp(message_code, "SAS ") == 0){
        return;
    }
    
    else if(strcmp(message_code, "BID ") == 0){
        return;
    }

    else{
        strcpy(answer, "Invalid operand.\n");
    }
    
    if(tcp_write(answer, strlen(answer)) == ERR){
        printf("Error sending client message TCP.\n");    
        exit(1);
    }
        

    close(tcp_newfd);
    return;
}

int main(){
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