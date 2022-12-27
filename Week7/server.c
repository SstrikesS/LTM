#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>

#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <sys/inotify.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "linklist.h"

#define MAX_OF_FILE 10000 //max character in file
#define MAX_ACC_IN_SERVER 100  // max account in taikhoan.txt
#define MAX_CHAR_OF_MESSAGE 10000 //max character in message 
#define MAX_OF_USERS_ONLINE 100 // max users now are in server


#define MAX_EVENTS 1024  /* Maximum number of events to process*/
#define LEN_NAME 16  /* Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME ))


#define SUCCESS 2 //login success
#define LOCK 1 //account is lock
#define FAIL 0 //login fail

typedef struct _User{// Acc already exist in server
    int status; // 1 is active, 0 is block
    char username[MAX_CHAR_OF_MESSAGE];
    char password[MAX_CHAR_OF_MESSAGE];
    int loginfail; // count numbers of login failed
}User;

char *server_path; // path to server file (directory only)
struct sockaddr_in server_addr, client_addr; 
int listenfd, fd, wd; // listen file description, inotify instance, watch desciption
pid_t pid; // process id
llist *acc_list; // List account read by taikhoan.txt
User *CurrentUser; // Current user that fork server serves

void sig_handler(int sig){
    inotify_rm_watch(fd, wd);
    close(fd);
    exit(0);
}

void sig_child(int signo){
    pid_t pid;
    int stat;
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0){
        printf("Process %d terminated\n", pid);
    }
}

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

int checkLogin(char *username, char *password){ // Check valid login of income client, return FAIL if wrong password, return LOCK if account has been lock
    int i;
    struct node *current;
    current = *acc_list;
    while(current != NULL){
        User *tmp = (User *)current->data;
        if(strcmp(tmp->username, username) == 0){
            if(strcmp(tmp->password, password) == 0 && tmp->status != 0){
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

char * getLastMess(){ //get last message in file groupchat.txt
    FILE *pt = fopen("groupchat.txt", "r");
    char *str = calloc(MAX_OF_FILE, sizeof(char));
    char *buffer = calloc(MAX_OF_FILE, sizeof(char));
    bzero(str, MAX_OF_FILE);
    bzero(buffer, MAX_OF_FILE);
    do{
        bzero(str, MAX_OF_FILE);
        strcpy(str, buffer);
        bzero(buffer, MAX_OF_FILE);
        fgets(buffer, MAX_OF_FILE, pt);
    }while(strlen(buffer) != 0);
    str[strlen(str) - 1] = '\0';
    fclose(pt);
    return str;
}

void watchFile(int connfd){// watch file in folder Week7
    fd = inotify_init();// khoi tao inoti instance
    wd = inotify_add_watch(fd, server_path, IN_MODIFY | IN_CREATE | IN_DELETE); // tao watch descriptor
    if(wd == -1){ // tao loi neu folder ko ton tai (Neu chon folder sai dan den watchFile se khong chay dung)
        printf("Invalid path\n");
        exit(0);
    }
    char *sendMess = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    signal(SIGINT, sig_handler); // tin hieu ket thuc watchFile (debug)
    while(1){
        char buffer[BUF_LEN]; //string chua noi dung doc
        int i = 0, length;
        length = read(fd, buffer, BUF_LEN);// doc buffer
        while(i < length){ //
            struct inotify_event *event = (struct inotify_event *) &buffer[i];//gan event
            if(event->len){ //Neu co event
                if(event->mask & IN_MODIFY){ 
                    sendMess = getLastMess();// doc dong cuoi file groupchat.txt
                    printf("New mess: '%s' send to user %s\n", sendMess, CurrentUser->username);
                    send(connfd, sendMess, strlen(sendMess), 0);//gui cho client
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }
}

void handle_mess(int connfd){// handle new message of income client
    char *buffer = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    char *sendmess = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    bzero(buffer, MAX_CHAR_OF_MESSAGE);
    bzero(sendmess, MAX_CHAR_OF_MESSAGE);
    int buffer_size;
    signal(SIGCHLD, sig_child);
    if((pid = fork()) == 0){
        watchFile(connfd);
        close(connfd);
        exit(0);
    }else{
        do{
            bzero(buffer, MAX_CHAR_OF_MESSAGE);
            bzero(sendmess, MAX_CHAR_OF_MESSAGE);
            buffer_size = recv(connfd, buffer, MAX_CHAR_OF_MESSAGE, 0);
            buffer[buffer_size] = '\0';

            //printf("Server of %s connfd %d : %s\n" , CurrentUser->username, connfd ,buffer);
            
            strcat(sendmess, CurrentUser->username);
            strcat(sendmess, ": ");
            strcat(sendmess, buffer);
            SaveFile(sendmess); // save client message
            printf("%s\n", sendmess);
            if(strcmp(buffer, "Exit") == 0){// Client exit
                bzero(sendmess, MAX_CHAR_OF_MESSAGE);
                strcpy(sendmess, "Bye");
                send(connfd, sendmess, strlen(sendmess), 0);
                bzero(sendmess, MAX_CHAR_OF_MESSAGE);
                strcat(sendmess, CurrentUser->username);
                strcat(sendmess, " has left chat room!");
                SaveFile(sendmess); //save client exit message
                break;
            }
        }while(1);
    }
}

int Login(int connfd){ // Login new client to server
    char *buffer = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    char *sendMess = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    char *username = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    char *password = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    char *note = calloc(MAX_OF_FILE, sizeof(char));
    int check,buffer_size, i, string_length;
    bzero(buffer, MAX_CHAR_OF_MESSAGE);
    bzero(sendMess, MAX_CHAR_OF_MESSAGE);
    bzero(username, MAX_CHAR_OF_MESSAGE);
    bzero(password, MAX_CHAR_OF_MESSAGE);
    uint32_t un;

    buffer_size = recv(connfd, (uint32_t *)&un, sizeof(uint32_t), 0);
    string_length = ntohl(un); // return strlen(username)
    buffer_size = recv(connfd, (char *)buffer, MAX_CHAR_OF_MESSAGE, 0);
    buffer[buffer_size] = '\0';// return username + password

    if(string_length == -1){
        perror("Error packet return!");
        exit(EXIT_FAILURE);
    }else{
        for(i = 0; i < buffer_size; i++){ // split buffer to username and password
            if(i < string_length){
                username[i] = buffer[i]; 
            }else{
                password[i - string_length] = buffer[i]; 
            }
        }
        password[buffer_size - string_length] = '\0';
        username[string_length] = '\0';
    }

    check = checkLogin(username, password); // check valid account

    if(check == FAIL){
        uint32_t un = htonl(FAIL);
        send(connfd, (uint32_t *)&un, sizeof(uint32_t), 0); // send FAIL to client
        close(connfd);
        return FAIL;
    }else if(check == SUCCESS){
        uint32_t un = htonl(SUCCESS); // send SUCCESS to client
        send(connfd, (uint32_t *)&un, sizeof(uint32_t), 0);
        bzero(CurrentUser->username, MAX_CHAR_OF_MESSAGE);
        bzero(CurrentUser->password, MAX_CHAR_OF_MESSAGE);

        strcpy(CurrentUser->username, username); // set current user 
        strcpy(CurrentUser->password, password);
        printf("User %s connfd %d\n", CurrentUser->username, connfd);
        bzero(note, MAX_OF_FILE);
        note = getSaveFile(); // get previous groupchat
        note[strlen(note)] = '\0';
        send(connfd, note, strlen(note), 0);

        strcat(sendMess, username);
        strcat(sendMess, " joins the server!");
        SaveFile(sendMess);// notify new client to groupchat
        printf("%s\n", sendMess);

        return SUCCESS;
    }else{
        uint32_t un = htonl(LOCK);// send LOCK to client
        send(connfd, (uint32_t *)&un, sizeof(uint32_t), 0);
        close(connfd);
        return LOCK;
    }
}
void setupServer(int port){ //setup a server and handle fork server to client
    unsigned int length = sizeof(client_addr);
    int connfd, i, check;
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
    signal(SIGCHLD, sig_child);
    while(1){
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &length);
        if(connfd < 0){
            if(errno == EINTR){
                continue;
            }else{
                perror("Server accept failed\n");
                exit(EXIT_FAILURE);
            }
        }
        check = Login(connfd);// login client
        if(check != LOCK && check != FAIL){// when success
            if((pid = fork()) == 0){// create new fork server
                close(listenfd);
                handle_mess(connfd);
                close(connfd);
                exit(0);
            }
        }
    }
}
int main(int argc, char const *argv[]){
    int i;
    int port = 5500;
    server_path = calloc(MAX_CHAR_OF_MESSAGE, sizeof(char));
    bzero(server_path, MAX_CHAR_OF_MESSAGE);
    fd = inotify_init();
    if(argc == 2){
        port = atoi(argv[1]);
    }
    server_path = "../Week7"; // folder can de xem file 'groupchat.txt'
    acc_list =  create_llist(NULL);
    CurrentUser = calloc(1, sizeof(User));
    readUser(acc_list);
    setupServer(port);
    close(listenfd);
    return 0;
}