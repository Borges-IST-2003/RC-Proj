#include "user.h"

void end_user(){
    exit(1);
}

int is_udp(char *code){
    char *udp_commands[] = {"LIN", "LOU", "UNR", "LMA", "LMB", "LST"};
    for(int i = 0; i < 6; i++){
        if (strcmp(code, udp_commands[i]) == 0)
            return 1;
    }
    return 0;
}

int udp_connect(char *message, int message_len) {
    memset(buffer, 0, strlen(buffer));
    fd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if (fd == -1)                        /*error*/
        return -1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    errcode = getaddrinfo("193.136.138.142", PORT, &hints, &res);
    if (errcode != 0) /*error*/
        return -1;
    n = sendto(fd, message, message_len, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/
        return -1;
    addrlen = sizeof(addr);
    n = recvfrom(fd, buffer, MAX_MSG_LEN, 0, (struct sockaddr *)&addr, &addrlen);
    if (n == -1) /*error*/
        return -1;

    if(strcmp(buffer, LOGOUT_OK) == 0 || strcmp(buffer, LOGIN_NOK) == 0
        || strcmp(buffer, LOGIN_ERR) == 0) // If the login failed or had an error or the logout was successful
        logout = 1;

    printf("%s\n", buffer);

    freeaddrinfo(res);
    close(fd);
    return 0;
}

void long_long_to_string(long long fsize, char *fsize_string){
    long long ix = 1;
    int num_car = 0;
    while(ix <fsize){
        ix = ix * 10;
        num_car++;
    }

    while(fsize > 0){
        fsize_string[num_car] = '0' + fsize%10;
        fsize = fsize / 10;
        num_car--;
    }
}

//sends size of file in bytes and file data
int send_file(int fd, char *filepath){
    char data[BUFFER_SIZE + 1] = {0};
    char file_size_string[32] = {0};
    int fd_file, bytes_read = 0;
    char space = ' ';

    if((fd_file = open(filepath, O_RDONLY)) == -1){
        printf("opening error\n");
        return -1;
    }

    struct stat file_info;
    
    if(fstat(fd_file, &file_info) != 0){
        return -1;
    }

    long long file_size = file_info.st_size;
    long_long_to_string(file_size, file_size_string);

    if(write(fd, &space, sizeof(char)) == -1){
        return -1;
    }
    write(1, &space, sizeof(char));

    if(write(fd, file_size_string, sizeof(file_size_string)) == -1){
        return -1;
    }
    write(1, file_size_string, sizeof(file_size_string));


    if(write(fd, &space, sizeof(char)) == -1){
        return -1;
    }
    write(1, &space, sizeof(char));

    while((bytes_read = read(fd_file, data, BUFFER_SIZE)) != 0){
        n = write(fd, data, BUFFER_SIZE);
        write(1, data, BUFFER_SIZE);
        if (n == -1) /*error*/
            return -1;
    }

    close(fd_file);
    return 0;
}

//reads filename
int read_back_until(char *buffer, char stopchar, char *destination){
    int ix = strlen(buffer) - 1;
    int dest_size = 0;

    while(buffer[ix] != stopchar){
        if (ix < 0)
            return -1;
        dest_size++;
        ix--;
    }

    ix = strlen(buffer) - 1;


    while(dest_size){
        dest_size--;
        if (buffer[ix] != '\n')
            destination[dest_size] = buffer[ix];
        ix--;
    }

    return 0;
}

int tcp_connect(char *message, int message_len) {
    char filepath[100] = {0};
    char new_line = '\n';
    message[strlen(message)-1] = '\0';
    
    fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (fd == -1)
        return -1; // error

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    errcode = getaddrinfo("127.0.0.1", PORT, &hints, &res);
    if (errcode != 0) /*error*/
        return -1;

    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/
        return -1;

    n = write(fd, message, message_len);
    if (n == -1) /*error*/
        return -1;

    n = read(fd, buffer, 128);
    if (n == -1) /*error*/
        return -1;

    write(1, "sent: ", 6);
    write(1, message, message_len);

    if(message[0] == 'O' && message[1] == 'P' && message[2] == 'A'){
        char filename[90] = {0};
        if (read_back_until(message, ' ', filename) == -1)
            return -1;  //error

        strcpy(filepath, "ASSETS/");
        strcat(filepath, filename);
        
        if (send_file(fd, filepath) == -1)
            return -1;  //error
    }
    
    write(1, &new_line, sizeof(char));


    write(1, "ans: ", 5);
    write(1, buffer, n);
    write(1, &new_line, sizeof(char));

    freeaddrinfo(res);
    close(fd);
    return 0;
}


int main() {
    char message_code[CODE_SIZE] = {0};
    char message_buffer[MAX_MSG_LEN] = {0};

    while(logout != 1){
        memset(message_buffer, 0, strlen(message_buffer));
        memset(message_code, 0, strlen(message_code)); 
        
        fgets(message_buffer, MAX_MSG_LEN, stdin);              // Reads from stdin
        memcpy(message_code, message_buffer, 3);                //  Gets the first 3 digits (pseudo-code)

        if (is_udp(message_code)){
            if(udp_connect(message_buffer, strlen(message_buffer)) == -1)
                end_user();
        }

        else {
            if(tcp_connect(message_buffer, strlen(message_buffer)) == -1)
                end_user();
        }
    }

    return 0;
}