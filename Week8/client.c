#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <sys/utsname.h>
#define MAX 1000
int sockfd;
struct sockaddr_in server_addr;
void sendInfo();
void MENU(){
    int choice = 0;
    char *bye = "Exit";
    do{
        printf("----------MENU-------------\n1.Send information to server\n2.Exit\nEnter number: ");
        scanf("%d", &choice);
        switch (choice){
        case 1:
            sendInfo();
            break;
        case 2:
            send(sockfd, bye, strlen(bye), 0);
            close(sockfd);
            exit(0);
            break;
        default:
            printf("Invalid choice\n");
            break;
        }
    }while(choice != 2);
}
void sendInfo(){
    char *buffer = calloc(MAX, sizeof(char));
    bzero(buffer, MAX);
    struct utsname uts;
    uname(&uts);
    strcat(buffer, uts.sysname);
    strcat(buffer, "|");
    strcat(buffer, uts.machine);
    strcat(buffer, "|");
    strcat(buffer, uts.release);
    strcat(buffer, "|");
    strcat(buffer, uts.version);
    buffer[strlen(buffer)] = '\0';
    printf("%s\n", buffer);
    send(sockfd, buffer, strlen(buffer), 0);
}

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
int main(int argc, char const *argv[])  {
    if(argc < 3){
        printf("Server connection failed! Usage: %s [host_ip_address] [port_number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[2]);
    setupClient(argv[1], port);
    MENU();
    return 0;
}