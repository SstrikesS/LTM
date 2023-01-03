#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>

#define MAX 10000
#define SUCCESS 2 //login success
#define LOCK 1 //account is lock
#define FAIL 0 //login fail

int sockfd;
struct sockaddr_in server_addr;
pthread_t ptd;

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

void *listenToServer(){// ham nghe tu server
    char *buffer = calloc(MAX, sizeof(char));
    int buffer_size;
    pthread_detach(pthread_self());
    do{
        bzero(buffer, MAX);
        buffer_size = recv(sockfd, buffer, MAX, 0);
        buffer[buffer_size] = '\0';
        if(strcmp(buffer, "Bye") == 0)
            break;
        printf("%s\n", buffer);
    }while(1);
    close(sockfd);

}

void sendToServer(){// ham gui cho server
    char *message = calloc(MAX, sizeof(char));
    int buffer_size;
    do{
        bzero(message, MAX);
        fflush(stdin);
        scanf("%[^\n]%*c", message);
        message[strlen(message)] = '\0';
        send(sockfd, message, strlen(message), 0);
    }while (strcmp(message, "Exit") != 0);
}

void Login(const char *username, const char *password){// gui username/password den server
    char *buffer = calloc(MAX, sizeof(char));
    bzero(buffer, MAX);
    int buffer_size, check;
    uint32_t un;

    send(sockfd, (char *)username, strlen(username), 0);
    sleep(1);
    send(sockfd, (char *)password, strlen(password), 0);

    buffer_size = recv(sockfd, (uint32_t *)&un, sizeof(uint32_t), 0);
    check = ntohl(un);
    if(check == FAIL){
        printf("Username is not exist or wrong password!\n");
        close(sockfd);
        exit(0);
    }else if(check == SUCCESS){
        buffer_size = recv(sockfd, (char *)buffer, MAX, 0);
        buffer[buffer_size] = '\0';
        if(strcmp(buffer, "NULL") != 0){
            printf("%s", buffer);
        }
        printf("Welcome to our chat room ! Press 'Exit' to log out\n");
    }else if(check == LOCK){
        printf("This account has been set to 0! Please try another account\n");
        close(sockfd);
        exit(0);
    }
}

int main(int argc, char const *argv[]){
    pthread_t pid;
    int stat;
    if(argc < 5){
        printf("Server connection failed! Usage: %s [host_ip_address] [port_number] [username] [password]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[2]);
    setupClient(argv[1], port);
    Login(argv[3], argv[4]);
    pthread_create(&pid, NULL, &listenToServer, NULL);
    sendToServer(); // client chinh se chi doc message tu server
    close(sockfd);
    return 0;
}