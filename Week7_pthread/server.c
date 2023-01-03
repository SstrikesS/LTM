#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "linklist.h"
#include <pthread.h>

#define MAX_OF_FILE 10000 //max character in file
#define MAX_ACC_IN_SERVER 100  // max account in taikhoan.txt
#define MAX_CHAR_OF_MESSAGE 10000 //max character in message 
#define MAX_OF_USERS_ONLINE 100 // max users now are in server

#define SUCCESS 2 //login success
#define LOCK 1 //account is lock
#define FAIL 0 //login fail

typedef struct _User{// Acc already exist in server
    int status; // 1 is active, 0 is block
    char username[MAX_CHAR_OF_MESSAGE];
    char password[MAX_CHAR_OF_MESSAGE];
    int loginfail; // count numbers of login failed
}User;

typedef struct _Client{
    int connfd;
    User *user;
}Client;

struct sockaddr_in server_addr, client_addr; 
int listenfd; //listen socket file description
pthread_t pid; // thread id
llist *acc_list; // List account read by taikhoan.txt
Client *cli_list;

void SaveFile(char *string){ // Save message into file groupchat.txt
    FILE *pt = fopen("groupchat.txt", "a+");
    fprintf(pt, "%s\n", string);
    fclose(pt);
}

char *getSaveFile(){ // Read message in file groupchat.txt
    FILE *pt = fopen("groupchat.txt", "r");
    char *str = calloc(MAX_OF_FILE, sizeof(char));
    bzero(str, MAX_OF_FILE);
    int length = 0;
    char c = fgetc(pt);
    while(c != EOF){
        str[length++] = c; 
        c = fgetc(pt);
    }
    str[strlen(str)] = '\0';
    fclose(pt);
    return str;
}

void readUser(){ // Read account from file account.txt
    FILE *pt = fopen("taikhoan.txt", "r");
    if(pt == NULL){
        printf("File was not found\n");
        exit(1);
    }
    int i = 0;
    char c = fgetc(pt);
    User *data = calloc(MAX_ACC_IN_SERVER, sizeof(User));
    while (c != EOF){
        if(c != '\n' && c != '\t' && c != ' ' && c != EOF){
            fseek(pt, -1, SEEK_CUR);
            fscanf(pt, "%s", data[i].username);
            fscanf(pt, "%s", data[i].password);
            fscanf(pt, "%d", &data[i].status);
            data[i].loginfail = 0;
            push_llist(acc_list, &data[i]);// add account into linklist
            i++;
        }
        c = fgetc(pt);
    }
    fclose(pt);
}

int checkLogin(char *username, char *password, User *current_user){ // Check valid login of income client, return FAIL if wrong password, return LOCK if account has been lock
    int i;
    struct node *current;
    current = *acc_list;
    while(current != NULL){
        User *tmp = (User *)current->data;
        if(strcmp(tmp->username, username) == 0){
            if(strcmp(tmp->password, password) == 0 && tmp->status != 0){
                strcpy(current_user->username, tmp->username);
                strcpy(current_user->password, tmp->password);
                return SUCCESS;
            }else{
                if(tmp->status == 0){
                    return LOCK;
                }
                return FAIL;
            }   
        }
        current = current->next;
    }
    return FAIL;
}


int Login(int connfd){ // Login new client to server
    char *buffer = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    char *sendMess = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    char *username = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    char *password = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    char *note = calloc(MAX_OF_FILE, sizeof(char));
    User *current_user = calloc(1, sizeof(User));
    int check,buffer_size, i;
    bzero(buffer, MAX_CHAR_OF_MESSAGE);
    bzero(sendMess, MAX_CHAR_OF_MESSAGE);
    bzero(username, MAX_CHAR_OF_MESSAGE);
    bzero(password, MAX_CHAR_OF_MESSAGE);
    uint32_t un;

    buffer_size = recv(connfd, (char *)username, MAX_CHAR_OF_MESSAGE, 0);
    buffer[buffer_size] = '\0';// return username + password
    buffer_size = recv(connfd, (char *)password, MAX_CHAR_OF_MESSAGE, 0);
    buffer[buffer_size] = '\0';// return username + password

    check = checkLogin(username, password, current_user); // check valid account
    if(check == FAIL){
        uint32_t un = htonl(FAIL);
        send(connfd, (uint32_t *)&un, sizeof(uint32_t), 0); // send FAIL to client
        close(connfd);
        return FAIL;
    }else if(check == SUCCESS){
        uint32_t un = htonl(SUCCESS); // send SUCCESS to client
        send(connfd, (uint32_t *)&un, sizeof(uint32_t), 0);
        for(i = 0; i < MAX_OF_USERS_ONLINE; i++){
            if(cli_list[i].connfd < 0){
                cli_list[i].connfd = connfd;
                strcpy(cli_list[i].user->username, current_user->username);
                strcpy(cli_list[i].user->password, current_user->password);
                break;
            }
        }
        bzero(note, MAX_OF_FILE);
        note = getSaveFile(); // get previous groupchat
        note[strlen(note)] = '\0';
        if(strlen(note) != 0){
            send(connfd, note, strlen(note), 0);
        }else{
            strcpy(note, "NULL");
            send(connfd, note, strlen(note), 0);
        }

        strcat(sendMess, username);
        strcat(sendMess, " joins the server!");
        SaveFile(sendMess);// notify new client to groupchat
        for(i = 0; i < MAX_OF_USERS_ONLINE; i++){
            if(cli_list[i].connfd > 0 && cli_list[i].connfd != connfd){
                send(cli_list[i].connfd, sendMess, strlen(sendMess), 0);
            }
        }
        printf("%s\n", sendMess);

        return SUCCESS;
    }else{
        uint32_t un = htonl(LOCK);// send LOCK to client
        send(connfd, (uint32_t *)&un, sizeof(uint32_t), 0);
        close(connfd);
        return LOCK;
    }
}

void *handle_chat(void * id){
    char *buffer = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    char *message = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    int buffer_size, i, list_id;
    list_id = *((int *)id);
    pthread_detach(pthread_self());
    do{
        bzero(buffer, MAX_CHAR_OF_MESSAGE);
        buffer_size = recv(cli_list[list_id].connfd, buffer, MAX_CHAR_OF_MESSAGE, 0);
        if(strcmp(buffer, "Exit") == 0){

            bzero(message, MAX_CHAR_OF_MESSAGE);
            strcpy(message, "Bye");
            message[strlen(message)] = '\0';
            send(cli_list[list_id].connfd, message, strlen(buffer), 0);

            bzero(message, MAX_CHAR_OF_MESSAGE);
            strcat(message, cli_list[list_id].user->username);
            strcat(message, " has left the server!");
            message[strlen(message)] = '\0';
            printf("%s\n", message);
            SaveFile(message);

            cli_list[list_id].connfd = -1;
            bzero(cli_list[list_id].user->password, MAX_CHAR_OF_MESSAGE);
            bzero(cli_list[list_id].user->username, MAX_CHAR_OF_MESSAGE);
            for(i = 0; i < MAX_OF_USERS_ONLINE; i++){
                if(cli_list[i].connfd > 0 && cli_list[i].connfd != cli_list[list_id].connfd){
                    send(cli_list[i].connfd, message, strlen(message), 0);
                }
            }
            break;
        }else{
            bzero(message, MAX_CHAR_OF_MESSAGE);
            strcat(message, cli_list[list_id].user->username);
            strcat(message, ": ");
            strcat(message, buffer);
            message[strlen(message)] = '\0';
            printf("%s\n", message);
            SaveFile(message);
            for(i = 0; i < MAX_OF_USERS_ONLINE; i++){
                if(cli_list[i].connfd > 0 && cli_list[i].connfd != cli_list[list_id].connfd){
                    send(cli_list[i].connfd, message, strlen(message), 0);
                }
                
            }
        }
    }while(1);
    close(cli_list[list_id].connfd);

}

void setupServer(int port){ //setup a server and handle fork server to client
    unsigned int length = sizeof(client_addr);
    int connfd, i, check;
    int *arg;
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
    printf("Server is now online at port %d ...\n", port);
    if(listen(listenfd, MAX_OF_USERS_ONLINE) != 0){
        perror("Listen failed\n");
        exit(EXIT_FAILURE);
    }
    while(1){
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &length);
        if(connfd < 0){
            perror("Server accept failed\n");
            exit(EXIT_FAILURE);
        }
        check = Login(connfd);// login client
        if(check == SUCCESS){// when success
            for(i = 0; i < MAX_OF_USERS_ONLINE; i++){
                if(cli_list[i].connfd == connfd){
                    arg = &i;
                    break;
                }
            }
            pthread_create(&pid, NULL, &handle_chat, (int *)arg);
        }
    }
}
int main(int argc, char const *argv[]){
    int i;
    int port = 5500;
    char *line = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    if(argc == 2){
        port = atoi(argv[1]);
    }
    acc_list =  create_llist(NULL);
    cli_list = calloc(MAX_OF_USERS_ONLINE, sizeof(Client));
    for(i = 0 ; i < MAX_OF_USERS_ONLINE; i++){
        cli_list[i].connfd = -1;
        cli_list[i].user = calloc(1, sizeof(User));
    }
    readUser(acc_list);
    setupServer(port);
    close(listenfd);
    return 0;
}