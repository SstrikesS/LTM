#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>
#include <pthread.h>

#define MAX 1000
struct sockaddr_in server_addr, client_addr; // MAX OF CHARACTER OF MESSAGE
int listenfd; // listen socket
int cur = 0;// number of client sended data to server

void *SaveClientInfo(void *arg){// handle client message
    pthread_detach(pthread_self());
    char *buffer = calloc(MAX, sizeof(char));
    char *message = calloc(MAX, sizeof(char));
    char *filename = calloc(MAX, sizeof(char));
    char *number = calloc(4, sizeof(char));
    char *port = calloc(10, sizeof(char));
    char *tmp = calloc(MAX, sizeof(char));
    char **token = calloc(4, sizeof(char *));
    int i;
    int current = cur; // get current client counter

    for(i = 0; i < 4; i++){
        token[i] = calloc(MAX, sizeof(char));
    }
    
    int buffer_size;
    int connfd = *((int *)arg); // get current client socket

    //get current filename to save 
    sprintf(number, "%d", current);
    strcat(filename, "Log");
    strcat(filename, number);
    strcat(filename, ".txt");
    sprintf(port, "%d", client_addr.sin_port);

    while(1){
        // get client ip and port
        bzero(buffer, MAX);
        bzero(message, MAX);
        strcat(buffer, "Client ");
        strcat(buffer, number);
        strcat(buffer, "\nIP Address is: ");
        strcat(buffer, inet_ntoa(client_addr.sin_addr));
        strcat(buffer, "\nPort is: ");
        strcat(buffer, port);
        buffer[strlen(buffer)] = '\0';
        
        //get client message
        buffer_size = recv(connfd, message, MAX, 0);
        buffer[buffer_size]= '\0';

        if(strcmp(message, "Exit") == 0){ // Exit message
            cur++; // serve next client request
            close(connfd); // close socket of current client
            break;
        }else{
            // get information from client message
            // split message into token string array and remove '|'
            tmp = strtok(message, "|");
            strcpy(token[0], tmp);
            i = 1;
            while(1){
                tmp = strtok(NULL, "|");
                if(tmp != NULL){
                    strcpy(token[i], tmp);
                    i++;
                }else{
                    break;
                }
            }

            //copy information to buffer
            strcat(buffer, "\nSystem is ");
            strcat(buffer, token[0]);
            strcat(buffer, " on ");
            strcat(buffer, token[1]);
            strcat(buffer, "\nOS Release is ");
            strcat(buffer, token[2]);
            strcat(buffer, "\nOS version is ");
            strcat(buffer, token[3]);
            buffer[strlen(buffer)] = '\0';

            FILE *pt = fopen(filename, "w+");// open save filename of current client
            fputs(buffer, pt); // write buffer to file log name
            printf("Save information of Client %s suceessfully!\n", number);
            fclose(pt);
        }
    }
    return NULL;
}

void setupServer(int port){ //setup a server
    unsigned int length = sizeof(client_addr);
    int connfd; // connect socket of each client
    pthread_t tid; // thread id
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
    if(listen(listenfd, 100) != 0){// listen request of client
        perror("Listen failed\n");
        exit(EXIT_FAILURE);
    }
    while(1){
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &length); // accept client request
        if(connfd < 0){
            perror("Server accept failed\n");
            exit(EXIT_FAILURE);
        }else{
            int *arg = &connfd; // get connect socket
            cur++; //increase client's counter
            pthread_create(&tid, NULL, &SaveClientInfo, (int *)arg);
        }
    }
}

int main(int argc, char const *argv[]){
    if(argc < 2 ){
        printf("Server creation failed! Need port number! Usage: %s [port_number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    setupServer(port);
    close(listenfd);
    return 0;
}