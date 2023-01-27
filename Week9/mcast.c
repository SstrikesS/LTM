#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#define TIMEOUT 5 // timeout to send/recv 'Hello' 
#define MAX 1000

typedef struct _Node{
    int connfd;
    int port;
    char *ip;
}Node;
struct sockaddr_in server_addr, client_addr; // MAX OF CHARACTER OF MESSAGE
int listenfd; // listen socket
Node *client;
int ret, nEvents;
int isQuit = -1;
fd_set checkfds;

void printfTable(){
    printf("List of neighbours: \n");
    int i;
    for(i = 0; i < FD_SETSIZE; i++){
        if(client[i].connfd > 0){
            printf("%d %s %d\n", client[i].connfd,client[i].ip, client[i].port);
        }
    }
}
int recevieData(int connfd){
    char *buffer = calloc(MAX, sizeof(char));
    bzero(buffer, MAX);
    int buffer_size;
    buffer_size = recv(connfd, buffer, MAX, 0);
    buffer[buffer_size] = '\0';
    printf("Client %d says hello\n", connfd);
    if(buffer_size < 0){
        return -1;
    }
    return buffer_size;
}

void *sendData(){
    pthread_detach(pthread_self());
    int i;
    char *message = calloc(MAX, sizeof(char));
    bzero(message, MAX);
    strcpy(message, "Hello");
    while(1){
        for(i = 0; i < FD_SETSIZE; i++){
            if(client[i].connfd > 0){
                send(client[i].connfd, message, strlen(message), 0);
            }
        }
        sleep(1);
    }
}


void *checkTime(void *argument){
    fd_set readfd;
    pthread_detach(pthread_self());
    int arg = (*(int *)argument);
    int ret, j, length;
    FD_ZERO(&readfd);
    FD_SET(arg, &readfd);
    int max_fd = arg;
    while(arg > 0){
        struct timeval t = {TIMEOUT, 0};
        ret = select(max_fd + 1, &readfd, NULL, NULL, &t);
        if(ret == -1){
            perror("Select error\n");
            exit(EXIT_FAILURE);
        }else if(ret == 0){
            FD_CLR(arg, &readfd);
            char *buffer = "Exit";
            send(arg, buffer, strlen(buffer), 0);
            printf("Client %d is timeout after %d seconds\n", arg, TIMEOUT);
            close(arg);
            for(j = 0; j < FD_SETSIZE; j++){
                if(client[j].connfd == arg){
                    client[j].connfd = 0;
                    break;
                }
            }
            break;
        }else{
            length = recevieData(arg);
            if(length <= 0){
                FD_CLR(arg, &readfd);
                for(j = 0; j < FD_SETSIZE; j++){
                    if(client[j].connfd == arg){
                        client[j].connfd = 0;
                        break;
                    }
                }
                close(arg);
            }
        }
    }
    return NULL;
}
void *consoleCommand(){
    pthread_detach(pthread_self());
    char *buffer = calloc(MAX, sizeof(char));
    while(isQuit != 1){
        memset(buffer, 0 ,sizeof(*buffer));
        int buffer_size = read(STDIN_FILENO, buffer, MAX);
        buffer[buffer_size - 1] = '\0';
        if(strcmp(buffer, "print mtable") == 0){
            printfTable();
        }else if(strcmp(buffer, "quit") == 0){
            isQuit = 1;
            close(listenfd);
            exit(0);
        }
    }
    return NULL;
}

void setupServer(int port){ //setup a server
    pthread_t tid, tid2[FD_SETSIZE], tid3;
    unsigned int length = sizeof(client_addr);
    int connfd, j; // connect socket of each client
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){// listen socket
        perror("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    //bind server
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if((bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) != 0){
        perror("Socket bind failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Server is now online...\n");
    if(listen(listenfd, FD_SETSIZE) != 0){// listen request of client
        perror("Listen failed\n");
        exit(EXIT_FAILURE);
    }
    pthread_create(&tid, NULL, sendData, NULL);
    pthread_create(&tid3, NULL, consoleCommand, NULL);
    FD_ZERO(&checkfds);
    FD_SET(STDIN_FILENO, &checkfds);
    FD_SET(listenfd, &checkfds);
    int max_fd = listenfd;
    while(isQuit != 1){
        nEvents = select(max_fd + 1, &checkfds, NULL, NULL, NULL);
        if(nEvents < 0){
            perror("Select error\n");
            break;
        }
        if(FD_ISSET(listenfd, &checkfds)){
            connfd = accept(listenfd, (struct sockaddr *)&client_addr, &length); // accept client request
            if(connfd == FD_SETSIZE){
                printf("Too many client!\n");
                break;
            }
            FD_SET(connfd, &checkfds);
            for(j = 0; j < FD_SETSIZE; j++){
                if(client[j].connfd <= 0){
                    client[j].connfd = connfd;
                    printf("New connection detected! Connfd = %d\n", client[j].connfd);
                    bzero(client[j].ip, 100);
                    strcpy(client[j].ip, inet_ntoa(client_addr.sin_addr));
                    client[j].port = client_addr.sin_port;
                    int *arg = &client[j].connfd;
                    pthread_create(&tid2[j], NULL, checkTime, (void *)arg);
                    break;
                }
            }
        }
    }
}


int main(int argc, char const *argv[]){
    if(argc < 2 ){
        printf("Server creation failed! Need port number! Usage: %s [port_number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    client = calloc(FD_SETSIZE, sizeof(Node));
    int i;
    for(i = 0; i < FD_SETSIZE; i++){
        client[i].connfd = -1;
        client[i].port = 0;
        client[i].ip = calloc(100, sizeof(char));
    }
    setupServer(port);
    for(i = 0; i < FD_SETSIZE; i++){
        if(client[i].connfd > 0){
            char *buffer = "Exit";
            send(client[i].connfd, buffer, strlen(buffer), 0);
            close(client[i].connfd);
        }
    }
    close(listenfd);
    return 0;
}