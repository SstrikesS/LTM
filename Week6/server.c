#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "linklist.h"

#define MAX_ACC 100 
#define MAX 100
#define MAX_OF_USERS 100
#define SUCCESS 1
#define LOCK 0
#define FAIL -1
typedef struct Data{
    int status; // 1 is active, 0 is block
    char username[MAX];
    char password[MAX];
    int loginfail; // count numbers of login failed
    struct sockaddr_in client;
}Data;
struct sockaddr_in server_addr, client_addr;
struct hostent *host;
int listenfd;
pid_t pid;
llist *acc_list;


void updateFile(){ // Update info to file account.txt
    FILE *pt = fopen("account.txt", "w+");
    if(pt == NULL){
        printf("File was not found\n");
        exit(1);
    }
    struct node * current = *acc_list;
    while(current != NULL){
        Data *data = (Data *)current->data;
        fprintf(pt, "%s %s %d\n", data->username, data->password, data->status);
        current = current->next;
    }
    fclose(pt);
}

void readFlie(){ // read info from file account.txt
    FILE *pt = fopen("account.txt", "r");
    if(pt == NULL){
        printf("File was not found\n");
        exit(1);
    }
    int i = 0;
    char c = fgetc(pt);
    Data *data = calloc(MAX_ACC, sizeof(Data));
    while (c != EOF){
        if(c != '\n' && c != '\t' && c != ' ' && c != EOF){
            fseek(pt, -1, SEEK_CUR);
            fscanf(pt, "%s", data[i].username);
            fscanf(pt, "%s", data[i].password);
            fscanf(pt, "%d", &data[i].status);
            data[i].loginfail = 0;
            memset(&data->client, 0, sizeof(data->client));
            push_llist(acc_list, &data[i]);
            i++;
        }
        c = fgetc(pt);
    }
    fclose(pt);
}
int checkLogin(char *username, char *password){
    struct node *current;
    current = *acc_list;
    while(current != NULL){
        Data *tmp = (Data *)current->data;
        if(strcmp(tmp->username, username) == 0){
            if(strcmp(tmp->password, password) == 0 && tmp->status != 0){
                return SUCCESS;
            }else{
                if(tmp->status == 0){
                    return LOCK;
                }
                if(tmp->loginfail < 3){
                    tmp->loginfail++;
                }
                if(tmp->loginfail == 3){
                    tmp->status = 0;
                    printf("Account %s has been set to 0\n",tmp->username);
                    updateFile(acc_list);
                    return LOCK;
                }
                return FAIL;
            }   
        }
        current = current->next;
    }
    return FAIL;
}
void chat(){
    while(1);
}
void Login(int connfd){
    char *buffer = calloc(MAX, sizeof(char));
    char *sendMess = calloc(MAX, sizeof(char));
    char *username = calloc(MAX, sizeof(char));
    char *password = calloc(MAX, sizeof(char));
    int check;
    do{
        bzero(buffer, MAX);
        bzero(sendMess, MAX);
        bzero(username, MAX);
        bzero(password, MAX);
        int buffer_size, i;
        int string_length;
        uint32_t un;
        buffer_size = recv(connfd, (uint32_t *)&un, sizeof(uint32_t), 0);
        string_length = ntohl(un);
        buffer_size = recv(connfd, (char *)buffer, MAX, 0);
        buffer[buffer_size] = '\0';
        if(string_length == -1){
            perror("Error packet return!");
            exit(EXIT_FAILURE);
        }else{
            for(i = 0; i < buffer_size; i++){
                if(i < string_length){
                    username[i] = buffer[i]; 
                }else{
                    password[i - string_length] = buffer[i]; 
                }
            }
            password[buffer_size - string_length] = '\0';
            username[string_length] = '\0';
        }
        check = checkLogin(username, password);
        if(check == FAIL){
            bzero(sendMess, MAX);
            strcpy(sendMess, "Username is not exist or wrong password!");
            send(connfd, (char *)sendMess ,strlen(sendMess), 0);
        }else if(check == SUCCESS){
            bzero(sendMess, MAX);
            strcpy(sendMess, "Welcome to our server!");
            printf("%s joined the server!\n", username);
            send(connfd, (char *)sendMess ,strlen(sendMess), 0);
        }else{
            bzero(sendMess, MAX);
            strcpy(sendMess, "This account has been set to 0! Please try another account\n");
            send(connfd, (char *)sendMess ,strlen(sendMess), 0);
        }
    }while(check != SUCCESS);
}

void serve(int connfd){
   Login(connfd);
   //chat();
}
void setupServer(int port){ //setup a server
    unsigned int length = sizeof(client_addr);
    int connfd;
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if((bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) != 0){
        perror("Socket bind failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Server is now online...\n");
    if(listen(listenfd, MAX_OF_USERS    ) != 0){
        perror("Listen failed\n");
        exit(EXIT_FAILURE);
    }
    while(1){
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &length);
        if(connfd < 0){
            perror("Server accept failed\n");
            exit(EXIT_FAILURE);
        }
        if((pid = fork()) == 0){
            close(listenfd);
            serve(connfd);
            //printf("Client exit\n");
            close(connfd);
            exit(0);
        }
    }
    
}

int main(int argc, char const *argv[]){
    if(argc < 2 ){
        printf("Server creation failed! Need port number! Usage: %s [port_number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    acc_list =  create_llist(NULL);
    readFlie(acc_list);
    setupServer(port);
    close(listenfd);
    return 0;
}