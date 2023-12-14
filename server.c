#include "server.h"

int is_alphanum(char* str){
    while (*str) {
        if (!isalnum(*str))
            return ERR;                 // Not alphanumeric
        ++str;                         // Move to the next character
    }
    return STATUS_OK;
}

int is_numeric(char* str){
    while (*str) {
        if (!isdigit(*str))
            return ERR;                 // Not digit
        ++str;                          // Move to the next character
    }
    return STATUS_OK;
}

int valid_pass(char* pass){
    if(strlen(pass) != PASS_SIZE - 1) return ERR;
    return is_alphanum(pass);
}

int valid_uid(char* uid){
    if(strlen(uid) != USER_UID_SIZE - 1) return ERR;
    return is_numeric(uid);
}

void get_time(time_t* fulltime, char* time_str){
    struct tm *current_time;

    time(fulltime);
    if(time_str != NULL) {
        current_time = gmtime(fulltime);
        sprintf(time_str, "%4d-%02d-%02d %02d:%02d:%02d",
                current_time->tm_year + 1900, current_time->tm_mon + 1, current_time->tm_mday,
                current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
    }
}

// UDP functions

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

    return STATUS_OK;
}

int user_login(const char *buffer){
    int check_pass_result;
    char password[PASS_SIZE] = {0};                 // user password read from the socket
    char user_uid[USER_UID_SIZE] = {0};             // user uid read from the socket
    char login_filepath[MAX_FILE_LENGTH] = {0};      // login file path
    char pass_filepath[MAX_FILE_LENGTH] = {0};       // password file path

    // Getting the user uid, and his password
    sscanf(buffer + CODE_SIZE, "%s %s\n", user_uid, password);
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
    char user_uid[USER_UID_SIZE] = {0};                     // user uid read from the socket
    char login_filepath[MAX_FILE_LENGTH] = {0};             // login file path
    char hosted_dir[USER_SUB_DIR_LENGTH] = {0};             // hosted directory
    char auction_end_filepath[AUCTION_END_LENGHT] = {0};
    char _aid[AID] = {0};

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

    int is_open = 0;                                        // nº of files in a dir
    rewinddir(dir);                                         // Reset the directory stream
    strcat(answer, "RMA OK ");
    // Concatenate file names into the allocated memory
    while ((entry = readdir(dir)) != NULL){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        memset(auction_end_filepath, 0, strlen(auction_end_filepath));
        strncpy(_aid, entry->d_name, 3);
        strcat(answer, _aid);
        strcat(answer, " ");
        is_open++;
        sprintf(auction_end_filepath, "%s%s/END_%s.txt", AUCTIONS_DIR, _aid, _aid);
        switch(access(auction_end_filepath, F_OK)){
            case 0:
                strcat(answer, "0");
                break;
            default:
                strcat(answer, "1");
                break;
        }
        strcat(answer, " ");
    }

    // Close the directory
    closedir(dir);
    if (is_open == 0)       // If nº files == 0, dir is empty
        return STATUS_NOK;

    return STATUS_OK;
}

int list_user_bids(const char *buffer, char *answer) {
    DIR *dir;
    struct dirent *entry;
    char user_uid[USER_UID_SIZE] = {0};                 // user uid read from the socket
    char login_filepath[MAX_FILE_LENGTH] = {0};         // login file path
    char bidded_dir[USER_SUB_DIR_LENGTH] = {0};         // bidded directory
    char auction_end_filepath[AUCTION_END_LENGHT] = {0};
    char _aid[AID] = {0};

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

    int is_open= 0;                                     // nº of files in a dir
    rewinddir(dir);                                     // Reset the directory stream
    strcat(answer, "RMB OK ");
    // Concatenate file names into the allocated memory
    while ((entry = readdir(dir)) != NULL){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        memset(auction_end_filepath, 0, strlen(auction_end_filepath));
        strncpy(_aid, entry->d_name, 3);
        strcat(answer, _aid);
        strcat(answer, " ");
        is_open++;
        sprintf(auction_end_filepath, "%s%s/END_%s.txt", AUCTIONS_DIR, _aid, _aid);
        switch(access(auction_end_filepath, F_OK)){
            case 0:
                strcat(answer, "0");
                break;
            default:
                strcat(answer, "1");
                break;
        }
        strcat(answer, " ");
    }

    // Close the directory
    closedir(dir);

    if (is_open == 0)       // If nº files == 0, dir is empty
        return STATUS_NOK;

    return STATUS_OK;
}

int list_auctions(char *answer) {
    DIR *dir;
    struct dirent *entry;
    char auction_end_filepath[AUCTION_END_LENGHT] = {0};
    char _aid[AID] = {0};

    // Open the directory
    dir = opendir(AUCTIONS_DIR);
    if (dir == NULL) {
        perror("Error opening directory");
        return ERR;
    }

    int is_open = 0;                                        // nº of files in a dir
    rewinddir(dir);                                         // Reset the directory stream
    strcat(answer, "RLS OK ");
    // Concatenate file names into the allocated memory
    while ((entry = readdir(dir)) != NULL){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        memset(auction_end_filepath, 0, strlen(auction_end_filepath));
        strncpy(_aid, entry->d_name, 3);
        strcat(answer, _aid);
        strcat(answer, " ");
        is_open++;
        sprintf(auction_end_filepath, "%s%s/END_%s.txt", AUCTIONS_DIR, _aid, _aid);
        switch(access(auction_end_filepath, F_OK)){
            case 0:                     // found the file
                strcat(answer, "0");
                break;
            default:                    // didn't find it
                strcat(answer, "1");
                break;
        }
        strcat(answer, " ");
    }

    // Close the directory
    closedir(dir);

    if (is_open == 0)       // If nº files == 0, dir is empty
        return STATUS_NOK;
    return STATUS_OK;
}

// TCP functions

int tcp_read(char* buffer, int len){ //b_read = bytes read
    int b_read = 0;
    while(len > 0){
        b_read = read(tcp_newfd, buffer, len);
        if(b_read == -1)
            return ERR;

        if(b_read == 0) //closed by peer
            break;

        len -= b_read;
        buffer += b_read;
    }

    return STATUS_OK;
}

int tcp_write(char* buffer, int len){ //b_written = bytes written
    int b_written = 0;
    while(len > 0){
        b_written = write(tcp_newfd, buffer, len);
        if(b_written == -1){
            return ERR;
        }
        len -= b_written;
        buffer += b_written;
    }
    return STATUS_OK;
}

// Like the tcp_read but doesn't have a fix size
int read_next_word(char *buffer, int max){
    int i = 0;
    while (1) {
        if (tcp_read(&buffer[i], 1) == ERR)
            return ERR;

        if(i >= max-1 || buffer[i] == ' ')
            break;

        if(buffer[i] == '\n' || buffer[i] == '\0')
            return STATUS_NOK;

        i++;
    }
    buffer[i] = '\0';  // Null-terminate the string
    return STATUS_OK;
}

// Sub function from open() (OPA)
int create_auction(char* user_uid, char* name, char* start_value, char* time_active, char* asset_name, char* asset_data){
    // Creating the directories
    // Dir's to be created
    char auction_dir[AUCTIONS_DIR_SIZE] = {0};
    char asset_dir[ASSET_DIR_SIZE] = {0};
    char bids_dir[BIDS_DIR_SIZE] = {0};

    aid++;
    sprintf(auction_dir, "%s%03d", AUCTIONS_DIR, aid);
    sprintf(asset_dir, "%s/ASSET", auction_dir);
    sprintf(bids_dir, "%s/BIDS", auction_dir);

    if(mkdir(auction_dir, 0777) != 0){
        printf("Error creating the auction directory.\n");
        return ERR;
    }
    if(mkdir(asset_dir, 0777) != 0){
        printf("Error creating the asset directory.\n");
        return ERR;
    }
    if(mkdir(bids_dir, 0777) != 0){
        printf("Error creating the bids directory.\n");
        return ERR;
    }

    // Creating the files
    FILE *file;
    char auction_start_filepath[AUCTION_START_LENGTH] = {0};
    char user_hosted_auction[USER_HOSTED_AUCTION_LENGTH] = {0};
    char auction_asset_filepath[AUCTION_ASSET_LENGTH] = {0};

    // Creating the start.txt file
    sprintf(auction_start_filepath, "%s/START_%03d.txt", auction_dir, aid);
    char start_data[START_MAX_SIZE] = {0};
    char curr_time[CURR_TIME] = {0};
    time_t fulltime;
    get_time(&fulltime, curr_time);
    sprintf(start_data, "%s %s %s %s %s %s %ld", user_uid, name, start_value, time_active, asset_name, curr_time, fulltime);
    if((file = fopen(auction_start_filepath, "w")) == NULL){
        printf("Error creating start.txt file.\n");
        return ERR;
    }
    if(fprintf(file, "%s", start_data) != strlen(start_data)){
        printf("Error writing in start.txt file.\n");
        return ERR;
    }
    if(fclose(file) != 0){
        printf("Error writing in start.txt file.\n");
        return ERR;
    }

    // Adding the auction to the user hosted directory
    sprintf(user_hosted_auction, "%s%s/HOSTED/%03d.txt", USERS_DIR, user_uid, aid);
    if((file = fopen(user_hosted_auction, "w")) == NULL){
        printf("Error creating hosted auction file in USERS.\n");
        return ERR;
    }
    if(fclose(file) != 0){
        printf("Error closing user hosted file.\n");
        return ERR;
    }

    // Creating the asset file
    sprintf(auction_asset_filepath, "%s/%s", asset_dir, asset_name);
    if((file = fopen(auction_asset_filepath, "w")) == NULL){
        printf("Error creating asset file .\n");
        return ERR;
    }
    if((fprintf(file, "%s", asset_data)) != strlen(asset_data)){
        printf("Error writing to the asset file.\n");
        return ERR;
    }
    if(fclose(file) != 0){
        printf("Error closing asset file.\n");
        return ERR;
    }

    return STATUS_OK;
}

// Sub function from bid() (BID)
int create_bid(const char* user_uid, int _aid, int value){
    FILE* file;
    char bid_filepath[BID_FILEPATH] = {0};
    char user_bid_filepath[USER_HOSTED_AUCTION_LENGTH] = {0};

    sprintf(bid_filepath, "%s%03d/BIDS/%d.txt", AUCTIONS_DIR, _aid, value);
    sprintf(user_bid_filepath, "%s%s/BIDDED/%03d.txt", USERS_DIR, user_uid, _aid);

    if((file = fopen(bid_filepath, "w")) == NULL){
        printf("Error creating bid_filepath in create_bid().\n");
        return ERR;
    }

    if(fclose(file) != 0){
        printf("Error closing bid_file in AUCTIONS.\n");
        return ERR;
    }

    if((file = fopen(user_bid_filepath, "w")) == NULL){
        printf("Error creating user_bid_filepath in create_bid().\n");
        return ERR;
    }

    if(fclose(file) != 0){
        printf("Error closing user_bid_file.\n");
        return ERR;
    }

    return STATUS_OK;
}


int get_auction(int _aid, Auction* auct){
    DIR* bids;
    FILE* file;
    char auct_end_filepath[AUCTION_END_LENGHT] = {0};
    char start_filepath[AUCTION_START_LENGTH] = {0};
    char bids_dir[BIDS_DIR_SIZE] = {0};
    struct dirent *entry;
    char last_value[VALUE] = {0};
    
    time_t fulltime;
    char buffer[DEFAULT] = {0};

    auct->auction_id = _aid;
    auct->is_active = true;

    sprintf(auct_end_filepath, "%s%03d/END_%03d.txt", AUCTIONS_DIR, _aid, _aid);
    if(access(auct_end_filepath, F_OK) != -1) auct->is_active = false;

    sprintf(start_filepath, "%s%03d/START_%03d.txt", AUCTIONS_DIR, _aid, _aid);
    if((file = fopen(start_filepath, "r")) == NULL)
        return ERR;
    while(fread(buffer, sizeof(char), sizeof(buffer), file) > 0);
    if(fclose(file) != 0)
        return ERR;

    if(sscanf(buffer, "%s %*s %*s %*s %d %*s %d", auct->user_uid, &auct->time_active, &auct->fulltime) != 3)
        return ERR;

    get_time(&fulltime, NULL);
    if(((fulltime - auct->fulltime) / 60) >= auct->time_active) {           // Seconds / 60 (to get minutes)
        auct->is_active = false; 
        create_end_file(_aid);
    }

    sprintf(bids_dir, "%s%03d/BIDS", AUCTIONS_DIR, _aid);
    bids = opendir(bids_dir);
    while((entry = readdir(bids)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        strcpy(last_value, entry->d_name);
    }
    auct->last_bid = atoi(last_value);

    return STATUS_OK;
}

// Function BID
int bid(const char* buffer){
    char user_uid[USER_UID_SIZE] = {0};
    char password[PASS_SIZE] = {0};
    int _aid;
    int value;

    char login_filepath[MAX_FILE_LENGTH] = {0};
    char hosted_filepath[USER_HOSTED_AUCTION_LENGTH] = {0};

    if(sscanf(buffer, "%s %s %d %d\n", user_uid, password, &_aid, &value) != 4){
        printf("Wrong format sent to bid() function.\n");
        return ERR;
    }

    if(valid_uid(user_uid) != STATUS_OK){
        printf("Wrong format user_uid in bid() function.\n");
        return ERR;
    }
    if(valid_pass(password) != STATUS_OK){
        printf("Wrong format password in bid() function.\n");
        return ERR;
    }
    if(_aid < 1 || _aid > 999){
        printf("Wrong format _aid in bid() function.\n");
        return ERR;
    }
    if( value < 1 || value > 999999){
        printf("Wrong format value in bid() function.\n");
        return ERR;
    }

    Auction auct;
    get_auction(_aid, &auct);

    if(auct.is_active)

    sprintf(login_filepath, "%s%s/%s_login.txt", USERS_DIR, user_uid, user_uid);
    if(access(login_filepath, F_OK) == -1) return RBD_NLG;

    if(auct.last_bid >= value)
        return RBD_REF;

    sprintf(hosted_filepath, "%s%s/HOSTED/%03d.txt", USERS_DIR, user_uid, _aid);
    if(access(hosted_filepath, F_OK) != -1) return RBD_ILG;    

    return create_bid(user_uid, _aid, value);
}

//Function OPA
int open_auction(const char* buffer, char* answer){
    char user_uid[USER_UID_SIZE] = {0};
    char password[PASS_SIZE] = {0};
    char name[AUCT_NAME] = {0};
    char start_value[VALUE] = {0};
    char time_active[TIME_ACTIVE] = {0};
    char asset_name[ASSET_NAME] = {0};
    char file_size_str[5] = {0};
    int file_size = 0;
    char login_filepath[MAX_FILE_LENGTH] = {0};      // login file path
    char pass_filepath[MAX_FILE_LENGTH] = {0};       // password file path
    int check_pass_result, result;

    if(sscanf(buffer, "%s %s ", user_uid, password) != 2){
        printf("Wrong format sent to open().\n");
        return ERR;
    }
    // #
    if((read_next_word(name, AUCT_NAME)) != STATUS_OK){
        printf("Error reading the name in open().\n");
        return ERR;
    }
    if(read_next_word(start_value, VALUE) != STATUS_OK){
        printf("Error on start_value in open().\n");
        return ERR;
    }
    if(read_next_word(time_active, TIME_ACTIVE) != STATUS_OK){
        printf("Error on time_active in open().\n");
        return ERR;
    }
    // #
    if(read_next_word(asset_name, ASSET_NAME) != STATUS_OK){
        printf("Error on asset_name in open().\n");
        return ERR;
    }
    if(read_next_word(file_size_str, 5) != STATUS_OK){
        printf("Error reading the file_size in open().\n");
        return ERR;
    }
    printf("%s %s %s %s %s\n", name, start_value, time_active, asset_name, file_size_str);
    if((file_size = atoi(file_size_str)) == 0){
        printf("A non valid integer read in file_size_str.\n");
        return ERR;
    }
    file_size++; // '\0'
    char *asset_data = (char *)malloc(file_size * sizeof(char)); 
    if(tcp_read(asset_data, file_size) == ERR){
        printf("Wrong format sent to open().\n");
        free(asset_data);
        return ERR;
    }
    asset_data[strlen(asset_data)- 1] = '\0';         // Subs the expected '\n' or '' with '\0';

    if(valid_pass(password) == ERR || valid_uid(user_uid) == ERR || is_numeric(start_value) == ERR
        || is_numeric(time_active) == ERR){
        printf("Wrong format sent to open().\n");
        free(asset_data);
        return ERR;
    }

    sprintf(login_filepath, "%s%s/%s_login.txt", USERS_DIR, user_uid, user_uid);
    sprintf(pass_filepath, "%s%s/%s_pass.txt", USERS_DIR, user_uid, user_uid);

    if(access(login_filepath, F_OK) == -1){
        free(asset_data);
        return ROA_NLG;
    }

    if ((check_pass_result = check_password(password, pass_filepath)) == STATUS_OK) {
        result = create_auction(user_uid, name, start_value, time_active, asset_name, asset_data);
        free(asset_data);
        if(result == STATUS_OK)
            sprintf(answer, "ROA OK %d", aid);
        return result;
    }
    free(asset_data);
    return check_pass_result;
}

// Function CLS
int close_auction(const char* buffer){
    FILE *file;
    DIR *dir;
    char user_uid[USER_UID_SIZE] = {0};
    char password[PASS_SIZE] = {0};
    char pass_filepath[MAX_FILE_LENGTH] = {0};
    char login_filepath[MAX_FILE_LENGTH] = {0};
    char auction_dir[AUCTIONS_DIR_SIZE] = {0};
    char hosted_filepath[USER_HOSTED_AUCTION_LENGTH] = {0};
    char auct_end_filepath[AUCTION_END_LENGHT] = {0};
    int _aid;

    if(sscanf(buffer, "%s %s %d\n", user_uid, password, &_aid) != 3){
        printf("Wrong format sent to close().\n");
        return ERR;
    }

    if(valid_uid(user_uid) != STATUS_OK){
        printf("Wrong format user_uid in close().\n");
        return ERR;
    }

    if(valid_pass(password) != STATUS_OK){
        printf("Wrong format password in close().\n");
        return ERR;
    }

    if(_aid < 1 || _aid > 999){
        printf("Wrong format: AID in close().\n");
        return ERR;
    }

    sprintf(login_filepath, "%s%s/%s_login.txt", USERS_DIR, user_uid, user_uid);
    if(access(login_filepath, F_OK) == -1) return RCL_NLG;
    sprintf(pass_filepath, "%s%s/%s_pass.txt", USERS_DIR, user_uid, user_uid);
    if(check_password(password, pass_filepath) != STATUS_OK) return STATUS_NOK;
    sprintf(auction_dir, "%s%03d", AUCTIONS_DIR, _aid);
    if((dir = opendir(auction_dir)) == NULL) return RCL_EAU;
    closedir(dir);
    sprintf(hosted_filepath, "%s%s/HOSTED/%03d.txt", USERS_DIR, user_uid, _aid);
    if(access(hosted_filepath, F_OK) == -1) return RCL_EOW;
    sprintf(auct_end_filepath, "%s%03d/END_%03d.txt", AUCTIONS_DIR, _aid, _aid);
    if(access(auct_end_filepath, F_OK) != -1) return RCL_END;

    if((file = fopen(auct_end_filepath, "w")) == NULL){
        printf("Error creating end.txt file in close().\n");
        return ERR;
    }
    
    // # "fprintf" conteudo end.txt

    if(fclose(file) != 0){
        printf("Error closing end.txt file in close().\n");
        return ERR;
    }

    return STATUS_OK;
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
        if(udp_n != SRC_SIZE)
            strcpy(answer, "RRC ERR");
        else{
            return;
        }
    }
    
    else {
        strcpy(answer, "ERR Wrong Format.\n");
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
        switch (open_auction(buffer, answer)) {
            case STATUS_OK:
                break;
            case STATUS_NOK:
                strcpy(answer, "ROA NOK");
                break;
            case ROA_NLG:
                strcpy(answer, "ROA NOK");
                break;
            default:
                strcpy(answer, "ROA ERR");
                break;
        }
    }
    else if(strcmp(message_code, "CLS ") == 0){
        if(tcp_read(buffer, CLOSE_SIZE) == ERR)
            exit(1);
        switch (close_auction(buffer)) {
            case STATUS_OK:
                strcpy(answer, "RCL OK");
                break;
            case STATUS_NOK:
                strcpy(answer, "RCL NOK");
                break;
            case RCL_NLG:
                strcpy(answer, "RCL NLG");
                break;
            case RCL_EAU:
                strcpy(answer, "RCL EAU");
                break;
            case RCL_EOW:
                strcpy(answer, "RCL EOW");
                break;
            case RCL_END:
                strcpy(answer, "RCL END");
                break;
            default:
                strcpy(answer, "RCL ERR");
                break;
        }
    }
    else if(strcmp(message_code, "SAS ") == 0){
        return;
    }
    else if(strcmp(message_code, "BID ") == 0){
        if(tcp_read(buffer, BID_SIZE) == ERR)
            exit(1);
        switch (bid(buffer)) {
            case STATUS_OK:
                strcpy(answer, "RBD OK");
                break;


            default:
                strcpy(answer, "RBD ERR");
                break;
        }
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