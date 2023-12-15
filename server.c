#include "server.h"

// Aux functions
void print(const char *format, ...) {
    // Check if command-line arguments are present
    if (v == 1) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

void get_aid(){
    DIR* dir;
    struct dirent* entry;
    char auction_id[AID] = {0};
    
    dir = opendir(AUCTIONS_DIR);
    while ((entry = readdir(dir)) != NULL){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        memset(auction_id, 0, strlen(auction_id));
        strncpy(auction_id, entry->d_name, AID-1);
    }

    if((aid = atoi(auction_id)) != 0 )
        return;
    aid = 0;

}

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

int CheckAssetFile(const char* fname) {
    struct stat filestat;
    if (stat(fname, &filestat) == -1) {
        perror("Error checking file status");
        return -1;
    }

    return filestat.st_size;
}


int create_file(const char* filepath){
    FILE* file;
    if((file = fopen(filepath, "w")) == NULL){
        print("Error creating file %s.\n", filepath);
        return ERR;
    }

    if(fclose(file) != 0){
        print("Error closing file %s.\n", filepath);
        return ERR;
    }

    return STATUS_OK;
}

int create_end_file(int _aid, const char* end_datetime, int end_sec_time){
    FILE* file;
    char auction_end_filepath[AUCTION_END_LENGHT] = {0};
    char end_buffer[DEFAULT] = {0};

    sprintf(auction_end_filepath, "%s%03d/END_%03d.txt", AUCTIONS_DIR, _aid, _aid);
    print("%s\n", auction_end_filepath);
    sprintf(end_buffer, "%s %d", end_datetime, end_sec_time);
    if((file = fopen(auction_end_filepath, "w")) == NULL){
        print("Error creating END.txt file.\n");
        return ERR;
    }
    if(fprintf(file, "%s", end_buffer) != strlen(end_buffer)){
        print("Error writing in END.txt file.\n");
        return ERR;
    }
    if(fclose(file) != 0){
        print("Error closing the END.txt file.\n");
        return ERR;
    }

    return STATUS_OK;
}

/*
STATUS_OK -> certo
STATUS_NOK -> não existe a auction
ERR -> Erro.
*/
int get_auction(int _aid, Auction* auct){
    DIR* bids;
    FILE* file;
    char auct_end_filepath[AUCTION_END_LENGHT] = {0};
    char start_filepath[AUCTION_START_LENGTH] = {0};
    char bids_dir[BIDS_DIR_SIZE] = {0};
    struct dirent *entry;
    char last_value[VALUE] = {0};
    char curr_time[CURR_TIME] = {0};
    time_t fulltime;
    char buffer[DEFAULT] = {0};
    
    sprintf(start_filepath, "%s%03d/START_%03d.txt", AUCTIONS_DIR, _aid, _aid);
    if(access(start_filepath, F_OK) == -1) return STATUS_NOK;

    auct->auction_id = _aid;
    auct->is_active = true;

    sprintf(auct_end_filepath, "%s%03d/END_%03d.txt", AUCTIONS_DIR, _aid, _aid);
    if(access(auct_end_filepath, F_OK) != -1) auct->is_active = false;

    if((file = fopen(start_filepath, "r")) == NULL){
        print("Error opening the start.txt file.\n");
        return ERR;
    }
    fread(buffer, sizeof(char), sizeof(buffer), file);
    if(fclose(file) != 0){
        print("Error closing the start.txt file.\n");
        return ERR;
    }

    if(sscanf(buffer, "%s %*s %*s %d %*s %*d-%*d-%*d %*d:%*d:%*d %d", auct->user_uid, &auct->time_active, &auct->fulltime) != 3){
        print("Error getting information from the start file.\n");
        return ERR;
    }

    get_time(&fulltime, curr_time);
    if(((fulltime - auct->fulltime) / 60) >= auct->time_active) {           // Seconds / 60 (to get minutes)
        if(create_end_file(_aid, curr_time, (fulltime - auct->fulltime)) != STATUS_OK)
            return ERR;
        auct->is_active = false;
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

// UDP functions

// Creates the login file
int create_login(const char* user_uid, const char* login_filepath){
    FILE *file;

    if((file = fopen(login_filepath, "w")) == NULL){
        print("Error creating login file.\n");
        return ERR;
    }

    if(fprintf(file, "%s", user_uid) != USER_UID_SIZE - 1){
        print("Error writing on login file.\n");
        return ERR;
    }

    if (fclose(file) != 0) {
        print("Error closing login file.\n");
        return ERR;
    }
    return STATUS_OK;                               // returns 0
}

// Checks if the password given is the same in the pass file
int check_password(const char* password, const char* pass_filepath){
    FILE *file;
    char pass[PASS_SIZE] = {0};                     // pass read from the file '_pass.txt'

    if((file = fopen(pass_filepath, "r")) == NULL){
        print("Error opening the pass file.\n");
        return ERR;
    }
    if(fread(pass, sizeof(char), PASS_SIZE-1, file) != PASS_SIZE-1){
        print("Error reading the pass file.\n");
        return ERR;
    }
    if(fclose(file) != 0){
        print("Error closing pass file.\n");
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
            print("Error creating user directory.\n");
            return ERR;
        }
    }
    
    // Creates the hosted sub-directory of the user (eg. USERS/103074/'HOSTED')
    if (mkdir(user_hosted_dir, 0777) != 0) {
        if (errno != EEXIST) {
            print("Error creating user directory.\n");
            return ERR;
        }
    }
    
    // Creates the bidded sub-directory of the user (eg. USERS/103074/'BIDDED')
    if (mkdir(user_bidded_dir, 0777) != 0) {
        if (errno != EEXIST) {
            print("Error creating user directory.\n");
            return ERR;
        }
    }

    // Creates password file. (eg. USERS/103074/'103074_pass.txt')
    if((file = fopen(pass_filepath, "w")) == NULL){
        print("Error creating pass file.\n");
        return ERR;
    }

    if(fprintf(file, "%s", password) != PASS_SIZE - 1){
        print("Error writing on pass file.\n");
        return ERR;
    }

    if (fclose(file) != 0) {
        print("Error closing pass file.\n");
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
        print("Error removing password file");
        return ERR;
    }

    // Remove the login file
    if (remove(login_filepath) != 0) {
        print("Error removing login file");
        return ERR;
    }

    return STATUS_OK;
}

// Function LIN
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
        print("Wrong password format.\n");
        return ERR;
    }
    // Checks user_uid format
    if(valid_uid(user_uid) == ERR){
        print("Wrong user id format.\n");
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

// Function LOU
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
        print("Wrong password format.\n");
        return ERR;
    }

    // Checks user_uid format
    if(valid_uid(user_uid) == ERR){
        print("Wrong user id format.\n");
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
            print("Error removing login file");
            return ERR;
        }
        return STATUS_OK;
    }

    return check_pass_result;
}

// Function UNR
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
        print("Wrong password format.\n");
        return ERR;
    }
    // Checks user_uid format
    if(valid_uid(user_uid) == ERR){
        print("Wrong user id format.\n");
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

// Function LMA
int list_user_auctions(const char *buffer, char *answer) {
    DIR *dir;
    struct dirent *entry;
    char user_uid[USER_UID_SIZE] = {0};                     // user uid read from the socket
    char login_filepath[MAX_FILE_LENGTH] = {0};             // login file path
    char hosted_dir[USER_SUB_DIR_LENGTH] = {0};             // hosted directory
    char _aid[AID] = {0};

    sscanf(buffer + 3, "%s\n", user_uid);

    // Checks user_uid format
    if(valid_uid(user_uid) == ERR){
        print("Wrong user id format.\n");
        return ERR;
    }

    sprintf(login_filepath, "%s%s/%s_login.txt", USERS_DIR, user_uid, user_uid);
    sprintf(hosted_dir, "%s%s/HOSTED", USERS_DIR, user_uid);

    if(access(login_filepath, F_OK) == -1)
        return RMA_NLG;

    // Open the directory
    if ((dir = opendir(hosted_dir)) == NULL) {
        print("Error opening directory");
        return ERR;
    }

    int is_open = 0;                                        // nº of files in a dir
    rewinddir(dir);                                         // Reset the directory stream
    strcat(answer, "RMA OK ");
    Auction auct;
    // Concatenate file names into the allocated memory
    while ((entry = readdir(dir)) != NULL){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        strncpy(_aid, entry->d_name, 3);
        if(get_auction(atoi(_aid), &auct) != STATUS_OK)
            return ERR;
        strcat(answer, _aid);
        strcat(answer, " ");
        is_open++;
        if(auct.is_active) strcat(answer, "1");
        else strcat(answer, "0");
        strcat(answer, " ");
    }

    // Close the directory
    closedir(dir);
    if (is_open == 0)       // If nº files == 0, dir is empty
        return STATUS_NOK;

    return STATUS_OK;
}

// Function LMB()
int list_user_bids(const char *buffer, char *answer) {
    DIR *dir;
    struct dirent *entry;
    char user_uid[USER_UID_SIZE] = {0};                 // user uid read from the socket
    char login_filepath[MAX_FILE_LENGTH] = {0};         // login file path
    char bidded_dir[USER_SUB_DIR_LENGTH] = {0};         // bidded directory
    char _aid[AID] = {0};

    sscanf(buffer + 3, "%s\n", user_uid);

    // Checks user_uid format
    if(valid_uid(user_uid) == ERR){
        print("Wrong user id format.\n");
        return ERR;
    }

    sprintf(login_filepath, "%s%s/%s_login.txt", USERS_DIR, user_uid, user_uid);
    sprintf(bidded_dir, "%s%s/BIDDED", USERS_DIR, user_uid);
    if(access(login_filepath, F_OK) == -1)
        return RMB_NLG;

    // Open the directory
    if ((dir = opendir(bidded_dir)) == NULL) {
        print("Error opening directory");
        return ERR;
    }

    int is_open= 0;                                     // nº of files in a dir
    rewinddir(dir);                                     // Reset the directory stream
    strcat(answer, "RMB OK ");
    Auction auct;
    // Concatenate file names into the allocated memory
    while ((entry = readdir(dir)) != NULL){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        strncpy(_aid, entry->d_name, 3);
        if(get_auction(atoi(_aid), &auct) != STATUS_OK)
            return ERR;
        strcat(answer, _aid);
        strcat(answer, " ");
        is_open++;
        if(auct.is_active) strcat(answer, "1");
        else strcat(answer, "0");
        strcat(answer, " ");
    }

    // Close the directory
    closedir(dir);

    if (is_open == 0)       // If nº files == 0, dir is empty
        return STATUS_NOK;

    return STATUS_OK;
}

// Function LST()
int list_auctions(char *answer) {
    DIR *dir;
    struct dirent *entry;
    char _aid[AID] = {0};

    // Open the directory
    if ((dir = opendir(AUCTIONS_DIR)) == NULL) {
        print("Error opening directory");
        return ERR;
    }

    int is_open = 0;                                        // nº of files in a dir
    rewinddir(dir);                                         // Reset the directory stream
    strcat(answer, "RLS OK ");
    Auction auct;
    // Concatenate file names into the allocated memory
    while ((entry = readdir(dir)) != NULL){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        strncpy(_aid, entry->d_name, 3);
        if(get_auction(atoi(_aid), &auct) != STATUS_OK)
            return ERR;
        strcat(answer, _aid);
        strcat(answer, " ");
        is_open++;
        if(auct.is_active) strcat(answer, "1");
        else strcat(answer, "0");
        strcat(answer, " ");
    }

    // Close the directory
    closedir(dir);

    if (is_open == 0)       // If nº files == 0, dir is empty
        return STATUS_NOK;
    return STATUS_OK;
}

// Function SRC()
int show_record(const char* buffer, char* answer) {
    int _aid;
    Auction auct;
    FILE* file;
    DIR* dir;
    struct dirent* entry;
    char bid[VALUE] = {0};
    char aux_buff[DEFAULT] = {0};

    if(sscanf(buffer, "%*s %d", &_aid) != 1) {
        print("Error reading the _aid from buffer.\n");
        return ERR;
    }
    if(_aid < 1 || _aid > 999) {
        print("Wrong format for the 'aid' given in SRC.\n");
        return ERR;
    }
    
    switch(get_auction(_aid, &auct)){
        case STATUS_NOK:
            return STATUS_NOK;
        case ERR:
            return ERR;
        default:
            break;
    }

    strcat(answer, "RRC OK ");
    // AUCTIONS/(aid)/START_(aid).txt
    char start_filepath[AUCTION_START_LENGTH] = {0};
    sprintf(start_filepath, "%s%03d/START_%03d.txt", AUCTIONS_DIR, auct.auction_id, auct.auction_id);
    
    // Start file information
    if((file = fopen(start_filepath, "r")) == NULL){
        print("Error opening the %s file in SRC.\n", start_filepath);
        return ERR;
    }

    if(fgets(aux_buff, DEFAULT, file) == NULL){
        print("Error reading from the %s file.\n", start_filepath);
        return ERR;
    }

    if(fclose(file) != 0){
        print("Error closing the %s file.\n", start_filepath);
        return ERR;
    }

    strcat(answer, aux_buff);

    // AUCTIONS/(aid)/BIDS
    char bids_dir[BIDS_DIR_SIZE] = {0};
    char bid_file[BID_FILEPATH] = {0};
    sprintf(bids_dir, "%s%03d/BIDS", AUCTIONS_DIR, auct.auction_id);

    // Getting all Bids
    if ((dir = opendir(bids_dir)) == NULL) {
        print("Error opening directory");
        return ERR;
    }
    rewinddir(dir);                                         // Reset the directory stream

    while ((entry = readdir(dir)) != NULL){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        memset(aux_buff, 0, strlen(aux_buff));
        memset(bid_file, 0, strlen(bid_file));
        memset(bid, 0, strlen(bid));

        strncpy(bid, entry->d_name, VALUE - 1);
        sprintf(bid_file, "%s/%s", bids_dir, bid);
        
        if((file = fopen(bid_file, "r")) == NULL){
            print("Error opening the %s file in SRC.\n", bid_file);
            return ERR;
        }
        if(fgets(aux_buff, DEFAULT, file) == NULL){
            print("Error reading from the %s file.\n", entry->d_name);
            return ERR;
        }
        if(fclose(file) != 0){
            print("Error closing the %s file.\n", entry->d_name);
            return ERR;
        }
        strcat(answer, "\nB ");
        strcat(answer, aux_buff);
    }

    // Close the directory
    closedir(dir);

    if(auct.is_active){
        return STATUS_OK; 
    }

    // AUCTIONS/(aid)/END_(aid).txt
    char end_filepath[AUCTION_END_LENGHT] = {0};
    sprintf(end_filepath, "%s%03d/END_%03d.txt", AUCTIONS_DIR, auct.auction_id, auct.auction_id);
    memset(aux_buff, 0, strlen(aux_buff));

    // End file information
    if((file = fopen(end_filepath, "r")) == NULL){
        print("Error opening the %s file in SRC.\n", end_filepath);
        return ERR;
    }
    if(fgets(aux_buff, DEFAULT, file) == NULL){
        print("Error reading from the %s file.\n", end_filepath);
        return ERR;
    }
    if(fclose(file) != 0){
        print("Error closing the %s file.\n", end_filepath);
        return ERR;
    }

    strcat(answer, "\nE ");
    strcat(answer, aux_buff);
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
    char auction_dir[AUCTIONS_DIR_SIZE] = {0};
    char asset_dir[ASSET_DIR_SIZE] = {0};
    char bids_dir[BIDS_DIR_SIZE] = {0};

    aid += 1;
    sprintf(auction_dir, "%s%03d", AUCTIONS_DIR, aid);
    sprintf(asset_dir, "%s/ASSET", auction_dir);
    sprintf(bids_dir, "%s/BIDS", auction_dir);

    if(mkdir(auction_dir, 0777) != 0){
        if (errno == EEXIST) {
            print("Directory '%s' already exists.\n", auction_dir);
        }
        print("Error creating the auction directory.\n");
        return ERR;
    }
    if(mkdir(asset_dir, 0777) != 0){
        print("Error creating the asset directory.\n");
        return ERR;
    }
    if(mkdir(bids_dir, 0777) != 0){
        print("Error creating the bids directory.\n");
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
    sprintf(start_data, "%s %s %s %s %s %s %ld", user_uid, name, asset_name, start_value, time_active, curr_time, fulltime);
    if((file = fopen(auction_start_filepath, "w")) == NULL){
        print("Error creating start.txt file.\n");
        return ERR;
    }
    if(fprintf(file, "%s", start_data) != strlen(start_data)){
        print("Error writing in start.txt file.\n");
        return ERR;
    }
    if(fclose(file) != 0){
        print("Error writing in start.txt file.\n");
        return ERR;
    }

    // Adding the auction to the user hosted directory
    sprintf(user_hosted_auction, "%s%s/HOSTED/%03d.txt", USERS_DIR, user_uid, aid);
    if((file = fopen(user_hosted_auction, "w")) == NULL){
        print("Error creating hosted auction file in USERS.\n");
        return ERR;
    }
    if(fclose(file) != 0){
        print("Error closing user hosted file.\n");
        return ERR;
    }

    // Creating the asset file
    sprintf(auction_asset_filepath, "%s/%s", asset_dir, asset_name);
    if((file = fopen(auction_asset_filepath, "w")) == NULL){
        print("Error creating asset file .\n");
        return ERR;
    }
    if((fprintf(file, "%s", asset_data)) != strlen(asset_data)){
        print("Error writing to the asset file.\n");
        return ERR;
    }
    if(fclose(file) != 0){
        print("Error closing asset file.\n");
        return ERR;
    }

    return STATUS_OK;
}

// Sub function from bid() (BID)
int create_bid(const char* user_uid, int _aid, int value){
    FILE* file;
    char bid_filepath[BID_FILEPATH] = {0};
    char user_bid_filepath[USER_HOSTED_AUCTION_LENGTH] = {0};
    char bid_buffer[DEFAULT] = {0};
    char curr_time[CURR_TIME] = {0};
    time_t fulltime;
    get_time(&fulltime, curr_time);

    sprintf(bid_filepath, "%s%03d/BIDS/%d.txt", AUCTIONS_DIR, _aid, value);
    sprintf(user_bid_filepath, "%s%s/BIDDED/%03d.txt", USERS_DIR, user_uid, _aid);
    sprintf(bid_buffer, "%s %d %s %ld", user_uid, value, curr_time, fulltime);

    if((file = fopen(bid_filepath, "w")) == NULL){
        print("Error creating file %s.\n", bid_filepath);
        return ERR;
    }

    if(fprintf(file, "%s", bid_buffer) != strlen(bid_buffer)){
        print("Error writing to the bid file.\n");
        return ERR;
    }

    if(fclose(file) != 0){
        print("Error closing file %s.\n", bid_filepath);
        return ERR;
    }

    if(create_file(user_bid_filepath) != STATUS_OK) return ERR;
    print("Ending create_bid.\n");
    return STATUS_OK;
}

// Function BID
int bid(const char* buffer) {
    char user_uid[USER_UID_SIZE] = {0};
    char password[PASS_SIZE] = {0};
    int _aid;
    char str_value[VALUE] = {0};
    int value;

    char login_filepath[MAX_FILE_LENGTH] = {0};
    char hosted_filepath[USER_HOSTED_AUCTION_LENGTH] = {0};

    print("buffer to bid: |%s|\n", buffer);
    if(sscanf(buffer, "%s %s %d\n", user_uid, password, &_aid) != 3){
        print("Wrong format sent to bid() function.\n");
        return ERR;
    }
    if(read_next_word(str_value, VALUE) == ERR){
        print("Error reading the value to bid.\n");
        return ERR;
    }
    value = atoi(str_value);

    if(valid_uid(user_uid) != STATUS_OK){
        print("Wrong format user_uid in bid() function.\n");
        return ERR;
    }
    if(valid_pass(password) != STATUS_OK){
        print("Wrong format password in bid() function.\n");
        return ERR;
    }
    if(_aid < 1 || _aid > 999){
        print("Wrong format _aid in bid() function.\n");
        return ERR;
    }
    if( value < 1 || value > 999999){
        print("Wrong format value in bid() function.\n");
        return ERR;
    }

    print("Getting auction.\n");
    Auction auct;
    get_auction(_aid, &auct);

    if(!(auct.is_active)) return STATUS_NOK;

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
        print("Wrong format sent to open().\n");
        return ERR;
    }

    if(valid_uid(user_uid) == ERR){
        print("Wrong format in user_uid on open().\n");
        return ERR;
    }
    if(valid_pass(password) == ERR){
        print("Wrong format in password on open().\n");
        return ERR;
    }

    if(read_next_word(name, AUCT_NAME) != STATUS_OK){
        print("Error reading the name in open().\n");
        return ERR;
    }
    if(read_next_word(start_value, VALUE) != STATUS_OK){
        print("Error on start_value in open().\n");
        return ERR;
    }
    if(read_next_word(time_active, TIME_ACTIVE) != STATUS_OK){
        print("Error on time_active in open().\n");
        return ERR;
    }
    if(read_next_word(asset_name, ASSET_NAME) != STATUS_OK){
        print("Error on asset_name in open().\n");
        return ERR;
    }
    if(read_next_word(file_size_str, 5) != STATUS_OK){
        print("Error reading the file_size in open().\n");
        return ERR;
    }

    if((file_size = atoi(file_size_str)) == 0){
        print("A non valid integer read in file_size_str.\n");
        return ERR;
    }
    file_size++;            // '\0'
    char *asset_data = (char *)malloc(file_size * sizeof(char)); 
    if(tcp_read(asset_data, file_size) == ERR){
        print("Wrong format sent to open().\n");
        free(asset_data);
        return ERR;
    }
    asset_data[strlen(asset_data)- 1] = '\0';         // Subs the expected "\n" or "" or " " with "\0";

    if(valid_pass(password) == ERR || valid_uid(user_uid) == ERR || is_numeric(start_value) == ERR
        || is_numeric(time_active) == ERR){
        print("Wrong format sent to open().\n");
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
            sprintf(answer, "ROA OK %03d", aid);
        return result;
    }
    free(asset_data);
    return check_pass_result;
}

// Function CLS
int close_auction(const char* buffer){
    char user_uid[USER_UID_SIZE] = {0};
    char password[PASS_SIZE] = {0};

    char pass_filepath[MAX_FILE_LENGTH] = {0};
    char login_filepath[MAX_FILE_LENGTH] = {0};
    int _aid;

    if(sscanf(buffer, "%s %s %d\n", user_uid, password, &_aid) != 3){
        print("Wrong format sent to close().\n");
        return ERR;
    }
    if(valid_uid(user_uid) != STATUS_OK){
        print("Wrong format user_uid in close().\n");
        return ERR;
    }
    if(valid_pass(password) != STATUS_OK){
        print("Wrong format password in close().\n");
        return ERR;
    }
    if(_aid < 1 || _aid > 999){
        print("Wrong format: AID in close().\n");
        return ERR;
    }

    sprintf(login_filepath, "%s%s/%s_login.txt", USERS_DIR, user_uid, user_uid);
    if(access(login_filepath, F_OK) == -1) return RCL_NLG;

    sprintf(pass_filepath, "%s%s/%s_pass.txt", USERS_DIR, user_uid, user_uid);
    if(check_password(password, pass_filepath) != STATUS_OK) return STATUS_NOK;

    Auction auct;
    switch (get_auction(_aid, &auct)) {
        case STATUS_NOK:
            return RCL_EAU;
        case ERR:
            return ERR;
        default:
            break;
    }

    if(strcmp(auct.user_uid, user_uid) != 0) return RCL_EOW;

    if(!auct.is_active) return RCL_END;

    char curr_time[CURR_TIME] = {0};
    time_t fulltime;
    get_time(&fulltime, curr_time);

    return create_end_file(_aid, curr_time, (fulltime - auct.fulltime));
}

// Function SAS
int show_asset(const char* buffer, char* answer){
    int _aid;
    char start_filepath[AUCTION_START_LENGTH] = {0};
    char asset_filepath[AUCTION_ASSET_LENGTH] = {0};
    char asset_name[AUCT_NAME] = {0};
    char asset_data[DATA] = {0};
    int fsize;
    FILE* file;

    if(sscanf(buffer, "%d", &_aid) != 1) return STATUS_NOK;
    if(_aid < 1 || _aid > 999) return STATUS_NOK;

    sprintf(start_filepath, "%s%03d/START_%03d.txt", AUCTIONS_DIR, _aid, _aid);
    if((file = fopen(start_filepath, "r")) == NULL) return STATUS_NOK;
    if(fscanf(file, "%*s %*s %s", asset_name) != 1) return STATUS_NOK;
    if(fclose(file) != 0) return STATUS_NOK;
    sprintf(asset_filepath, "%s%03d/ASSET/%s", AUCTIONS_DIR, _aid, asset_name);
    fsize = CheckAssetFile(asset_filepath);
    if((file = fopen(asset_filepath, "r")) == NULL) return STATUS_NOK;
    fread(asset_data, fsize, sizeof(char), file);
    if(fclose(file) != 0) return STATUS_NOK;
    print("|%s|, %d\n",asset_data, fsize);
    sprintf(answer, "RSA OK %s %d ", asset_name, fsize);
    if(tcp_write(answer, strlen(answer)) != STATUS_OK) return STATUS_NOK;
    if(tcp_write(asset_data, fsize) != STATUS_OK) return STATUS_NOK;
    return STATUS_OK;
}

// Main/Connection functions
int max(int a, int b){
    return (a > b ? a : b);
}

void udp_handler(){
    char buffer[MAX_MSG_LEN] = {0};
    char message_code[CODE_SIZE+1] = {0};
    char answer[MAX_MSG_LEN] = {0};
    udp_addrlen = sizeof(udp_addr);
    udp_n = recvfrom(udp_fd, buffer, MAX_MSG_LEN, 0, (struct sockaddr *)&udp_addr, &udp_addrlen);

    if (udp_n == -1)
        exit(1);
    memcpy(message_code, buffer, CODE_SIZE);
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

    else if(strcmp(message_code, "LST\n") == 0){
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
            switch (show_record(buffer, answer)) {
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

    else strcpy(answer, "Error wrong Format.\n");

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
        if(tcp_read(buffer, SAS_SIZE) == ERR)
            exit(1);
        switch (show_asset(buffer, answer)) {
            case STATUS_OK:
                return;
            default:
                sprintf(answer, "RSA ERR");
                break;
        }
    }
    else if(strcmp(message_code, "BID ") == 0){
        if(tcp_read(buffer, BID_SIZE) == ERR)
            exit(1);
        switch (bid(buffer)) {
            case STATUS_OK:
                strcpy(answer, "RBD ACC");
                break;
            case STATUS_NOK:
                strcpy(answer, "RBD NOK");
                break;
            case RBD_NLG:
                strcpy(answer, "RBD NLG");
                break;
            case RBD_REF:
                strcpy(answer, "RBD REF");
                break;
            case RBD_ILG:
                strcpy(answer, "RBD ILG");
                break;
            default:
                strcpy(answer, "RBD ERR");
                break;
        }
    }
    else{
        strcpy(answer, "Invalid operand.\n");
    }
    
    if(tcp_write(answer, strlen(answer)) == ERR){
        print("Error sending client message TCP.\n");    
        exit(1);
    }

    close(tcp_newfd);
    return;
}

int main(int argc, char *argv[]) {
    char port[PORT_SIZE] = {0};
    switch (argc){
        case 2:
            if(strcmp(argv[1], "-v") == 0) {
                v = 1;
                strcat(port, PORT);
            }
            else strcat(port, argv[1]);
            break;
        case 3:
            v = 1;
            strcat(port, argv[1]);
            break;
        default:
            v = 1;          // REMOVE ! #
            strcat(port, PORT);
            break;
    }

    // UDP setup
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd == -1)
        exit(1);

    memset(&udp_hints, 0, sizeof udp_hints);
    udp_hints.ai_family = AF_INET;
    udp_hints.ai_socktype = SOCK_DGRAM;
    udp_hints.ai_flags = AI_PASSIVE;

    if ((udp_errcode = getaddrinfo(NULL, port, &udp_hints, &udp_res)) != 0)
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

    if ((tcp_errcode = getaddrinfo(NULL, port, &tcp_hints, &tcp_res)) != 0)
        exit(1);

    if ((tcp_n = bind(tcp_fd, tcp_res->ai_addr, tcp_res->ai_addrlen)) == -1)
        exit(1);

    if (listen(tcp_fd, 5) == -1)
        exit(1);

    // Main Loop
    while (1) {
        get_aid();
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(tcp_fd, &read_fds);
        FD_SET(udp_fd, &read_fds);
        int max_fd = max(udp_fd, tcp_fd);

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1){
            freeaddrinfo(udp_res);
            close(udp_fd);
            freeaddrinfo(tcp_res);    
            close(tcp_fd);
            exit(1);
        }

        // Handle UDP activity
        if (FD_ISSET(udp_fd, &read_fds))
            udp_handler();

        // Handle TCP activity
        if (FD_ISSET(tcp_fd, &read_fds)){
            tcp_addrlen = sizeof(tcp_addr);
            if((tcp_newfd = accept(tcp_fd, (struct sockaddr*)&tcp_addr, &tcp_addrlen)) == -1){
                exit(1);
                freeaddrinfo(udp_res);
                close(udp_fd);
                freeaddrinfo(tcp_res);    
                close(tcp_fd);
            }
            if((child_pid = fork()) == -1){
                freeaddrinfo(udp_res);
                close(udp_fd);
                freeaddrinfo(tcp_res);    
                close(tcp_fd);
                exit(1);
            }
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