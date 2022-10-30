#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#define MAX 100
int main(int argc, char const *argv[]){
    if(argc < 3){
        printf("Server connection failed! Usage: %s [host_ip_address] [port_number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[2]);
    int sockfd, length;
    struct sockaddr_in server_addr;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    char *buffer = (char*)malloc(sizeof(char) * MAX);
    char *tmp = "Hello server";
    sendto(sockfd, (char *)tmp, strlen(tmp), MSG_CONFIRM, (const struct sockaddr *) &server_addr, sizeof(server_addr));
    printf("Succeed connect to server\n");

    do{
        buffer[0] = '\0';
        printf("Input message: ");
        fflush(stdin);
        fgets(buffer, MAX, stdin);
        printf("------------------------------------\n");
        sendto(sockfd, (char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *) &server_addr, sizeof(server_addr));
        length = sizeof(server_addr);
        recvfrom(sockfd, (char *)buffer, MAX, MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
        printf("Message from other client: %s", buffer);
    }while(buffer[0] != '\n');
    close(sockfd);
    return 0;
}
