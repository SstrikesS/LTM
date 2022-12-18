#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>

#define MAX 100
int sockfd;
struct sockaddr_in server_addr;

void setupClient(const char *ip, int port){
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0){
        perror("Connection with the server failed\n");
        exit(EXIT_FAILURE);
    }
}
void chat(){
    while(1);
}
void Login(){
    char *buffer = calloc(MAX, sizeof(char));
    char *username = calloc(MAX, sizeof(char));
    char *password = calloc(MAX, sizeof(char));
    do{
        bzero(buffer, MAX);
        bzero(username, MAX);
        bzero(password, MAX);
        printf("Username: ");
        fflush(stdin);
        scanf("%[^\n]%*c", username);
        username[strlen(username)] = '\0';
        printf("Password: ");
        fflush(stdin);
        scanf("%[^\n]%*c", password);
        password[strlen(password)] = '\0';
        int buffer_size;
        int string_length = strlen(username);
        uint32_t un = htonl(string_length);
        send(sockfd, (uint32_t *)&un, sizeof(uint32_t), 0);
        send(sockfd, (char *)username, strlen(username), 0);
        send(sockfd, (char *)password, strlen(password), 0);
        buffer_size = recv(sockfd, (char *)buffer, MAX, 0);
        buffer[buffer_size] = '\0';
        
        printf("%s\n", buffer);
        printf("\n");
        if(strcmp(buffer, "Welcome to our server!") != 0){
            printf("Please try again!\n");
        }
    }while(strcmp(buffer, "Welcome to our server!") != 0);
}
int main(int argc, char const *argv[])  {
    if(argc < 3){
        printf("Server connection failed! Usage: %s [host_ip_address] [port_number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[2]);
    setupClient(argv[1], port);
    Login();
    chat();
    close(sockfd);
    return 0;
}