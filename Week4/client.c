#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX 100

int sockfd;
struct sockaddr_in server_addr;

void mess_handle(){
    char *buffer = (char*)malloc(sizeof(char) * MAX);
    char *message = (char *)malloc(sizeof(char) * MAX);
    int buff_size = 0;
    int length;
    do{
        memset(buffer, '\0', sizeof(buffer));
        memset(message, '\0', sizeof(message));
        printf("Input message: ");
        fflush(stdin);
        scanf("%s", buffer);
        printf("------------------------------------\n");

        sendto(sockfd, (char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *) &server_addr, sizeof(server_addr));
        length = sizeof(server_addr);
        if(strcmp(buffer, "bye") == 0){
            exit(1);
        }
        buff_size = recvfrom(sockfd, (char *)message, MAX, MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
        message[buff_size] = '\0';
        if(strlen(message) > 0){
            printf("Message from other client: %s\n", message);
        }
        if(strcmp(message, "Error") != 0){
            memset(message, '\0', sizeof(message));
            buff_size = recvfrom(sockfd, (char *)message, MAX, MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
            message[buff_size] = '\0';
            if(strlen(message) > 0){
                printf("Message from other client: %s\n", message);
            }
        }
        
    }while(buffer[0] != '\n');
}

void setupClient(const char *ip, int port){
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);
}

void Login(){
    char *username = (char *)malloc(sizeof(char) * MAX);
    char *password = (char *)malloc(sizeof(char) * MAX);
    char *buffer = (char *)malloc(sizeof(char) * MAX);
    int length = sizeof(server_addr);
    int buff_size = 0;
    printf("User: ");
    fflush(stdin);
    scanf("%s",username);
    printf("Password: ");
    fflush(stdin);
    scanf("%s" ,password);

    sendto(sockfd, (char *)username, strlen(username), MSG_CONFIRM, (const struct sockaddr *) &server_addr, length);
    
    sendto(sockfd, (char *)password, strlen(password), MSG_CONFIRM, (const struct sockaddr *) &server_addr, length);
    
    buff_size = recvfrom(sockfd, (char *)buffer, MAX, MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
    buffer[buff_size] = '\0';
    printf("%s\n", buffer);
    if(strcmp(buffer, "OK") == 0){
        // memset(buffer, '\0', sizeof(buffer));
        // printf("Enter new password (Must be different 'bye'): ");
        // fflush(stdin);
        // scanf("%s", buffer);
        // if(strcmp(buffer , "bye") == 0){
        //     exit(1);
        // }
        // sendto(sockfd, (char *)buffer, strlen(buffer), MSG_CONFIRM, (struct sockaddr *) &server_addr, length);
        // memset(buffer, '\0', sizeof(buffer));
        // buff_size = recvfrom(sockfd, (char *)buffer, MAX, MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
        // buffer[buff_size] = '\0';
        // if(strcmp(buffer, "ERROR") == 0){
        //     printf("Change password fail\n");
        // }else{
        //     printf("Receive from server : %s", buffer);
        //     memset(buffer, '\0', sizeof(buffer));
        //     buff_size = recvfrom(sockfd, (char *)buffer, MAX, MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
        //     buffer[buff_size] = '\0';
        //     printf(" and %s\n", buffer);
        // }
    }else{
        Login();
    }
}

int main(int argc, char const *argv[]){
    if(argc < 3){
        printf("Server connection failed! Usage: %s [host_ip_address] [port_number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);
   
    setupClient(argv[1], port);
    Login();
    mess_handle();
    close(sockfd);
    return 0;
}
