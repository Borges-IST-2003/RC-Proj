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

    errcode = getaddrinfo("127.0.0.1", PORT, &hints, &res);
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

int tcp_connect(char *message, int message_len) {
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
    write(1, "ans: ", 5);
    write(1, buffer, n);

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