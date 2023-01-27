#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>

#include <sys/utsname.h>// lib to get system information

#define MAX 1000 // MAX OF CHARACTER OF MESSAGE

int sockfd; // connect socket

struct sockaddr_in server_addr; // server infomation

void sendInfo(); // send client information to server

void MENU(){ // menu display
    int choice = 0;
    char *bye = "Exit";
    do{
        printf("----------MENU-------------\n1.Send information to server\n2.Exit\nEnter number: ");
        scanf("%d", &choice);
        switch (choice){
        case 1:// send data to server
            sendInfo();
            break;
        case 2:// Exit
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
    uname(&uts); // get client information
    strcat(buffer, uts.sysname); // copy system name
    strcat(buffer, "|");
    strcat(buffer, uts.machine); // copy os information
    strcat(buffer, "|");
    strcat(buffer, uts.release); // copy relesase time
    strcat(buffer, "|");
    strcat(buffer, uts.version); // copy os version 
    buffer[strlen(buffer)] = '\0';
    printf("%s\n", buffer);
    send(sockfd, buffer, strlen(buffer), 0); // send to server
    //Example: buffer = "Linux|x86_64|5.15.0-56-generic|#62~20.04.1-Ubuntu SMP Tue Nov 22 21:24:20 UTC 2022";
}

void setupClient(const char *ip, int port){ // setup client
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){ // setup socket
        perror("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    bzero(&server_addr, sizeof(server_addr));
    // bind server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    //connect to server
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