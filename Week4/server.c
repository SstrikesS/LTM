#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include "linklist.h"

#define MAX 100
#define MAX_ACC 100
typedef struct Data{
    int status; // 1 is active, 0 is block
    char username[MAX];
    char password[MAX];
    int loginfail; // count numbers of login failed
    struct sockaddr_in client;
}Data;

typedef struct Message{
    char username[MAX];
    char string[MAX];
    char number[MAX];
}Message;

struct sockaddr_in server_addr, client_addr_1 , client_addr_2;
struct hostent *host;
int sockfd;

int check_input(char *input, char *number, char *string){// check message from client
    int i;
    int j = 0;
    int k = 0;
    for(i = 0; i < strlen(input); i++){
        if(input[i] >= '0' && input[i] <= '9'){
            number[j++] = input[i];
        }else if((input[i] >= 'a' && input[i] <= 'z') || (input[i] >= 'A' && input[i] <= 'Z')){
            string[k++] = input[i];
        }else{
            return 1; // check failed
        }
    }
    number[j] ='\0';
    string[k] = '\0';
    return 0; // check succeed
}
void setupServer(int port){ // setup Server port and IP
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Socket creation failed!\n");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr_1, 0, sizeof(client_addr_1));
    memset(&client_addr_2, 0, sizeof(client_addr_2));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if(bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("Socket binding failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Server is now online\n");
}

char *getNameClient(llist *acc_list, struct sockaddr_in client_tmp){ // get client name when login succeed
    struct node *current = *acc_list;
    while (current != NULL){
        Data *data = (Data *)current->data;
        if(strcmp(inet_ntoa(data->client.sin_addr), inet_ntoa(client_tmp.sin_addr)) == 0 && client_tmp.sin_port == data->client.sin_port){
            return data->username;
        }
        current = current->next;
    }
    return NULL;
}

void mess_handle(llist *acc_list){ // manage message from 2 client
    struct sockaddr_in client_tmp;
    Message buffer;
    Message message;
    memset(&client_tmp, 0, sizeof(client_tmp));
    int length, check;
    Message error;
    strcpy(error.string, "Error");
    char *host_addr;
    int buff_size = 0;
    while(1){
        memset(&buffer, 0, sizeof(buffer));
        memset(&message, 0, sizeof(message));
        length = sizeof(client_tmp);
        buff_size = recvfrom(sockfd, (Message *)&buffer, sizeof(Message), MSG_WAITALL, (struct sockaddr *) &client_tmp, &length);
        if(buff_size != sizeof(Message)){
            printf("Error!\n");
        }
        check = check_input(buffer.string, message.number, message.string);
        host = gethostbyaddr((const char *)&client_tmp.sin_addr.s_addr, sizeof(client_tmp.sin_addr.s_addr), AF_INET);
        host_addr = inet_ntoa(client_tmp.sin_addr);
        char *client_name = getNameClient(acc_list, client_tmp);
        strcpy(message.username, client_name); 
        strcpy(error.username, client_name);
        printf("Message from %s [ip %s, port %d]: %s\n", client_name, host_addr, client_tmp.sin_port ,buffer.string);
        printf("-----------------------------------------------------\n");
        if(strcmp(buffer.string, "bye") == 0){
            printf("Goodbye %s\n", client_name);
        }
        if(strcmp(host_addr, inet_ntoa(client_addr_1.sin_addr)) == 0 && client_addr_1.sin_port == client_tmp.sin_port){
            if(check == 0){
                write(sockfd, &message, sizeof(Message));
                sendto(sockfd, (Message*)&message, sizeof(Message), MSG_CONFIRM, (struct sockaddr *) &client_addr_2 , sizeof(client_addr_2));
            }else{
                write(sockfd, &error, sizeof(Message));
                sendto(sockfd, (Message*)&error, sizeof(Message), MSG_CONFIRM, (struct sockaddr *) &client_addr_2 , sizeof(client_addr_2));
            }        
        }else{
            if(check == 0){
                write(sockfd, &message, sizeof(Message));
                sendto(sockfd, (Message*)&message, sizeof(Message), MSG_CONFIRM, (struct sockaddr *) &client_addr_1 , sizeof(client_addr_1));
            }else{
                write(sockfd, &error, sizeof(Message));
                sendto(sockfd, (Message*)&error, sizeof(Message), MSG_CONFIRM, (struct sockaddr *) &client_addr_1 , sizeof(client_addr_1));
            }
        }
    }
}

void print_data(void *data){ // print data of struct Data
    Data *acc_data = (Data *)data; 
    if(acc_data == NULL){
        perror("NULL account\n");
        exit(EXIT_FAILURE);
    }
    printf("%s %s %d\n", acc_data->username, acc_data->password, acc_data->status);
}

void updateFile(llist *acc_list){ // Update info to file account.txt
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

void readFlie(llist *acc_list){ // read info from file account.txt
    FILE *pt = fopen("account.txt", "r");
    if(pt == NULL){
        printf("File was not found\n");
        exit(1);
    }
    int i = 0;
    char c = fgetc(pt);
    Data *data = malloc(sizeof(Data) * MAX_ACC);
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

char* check_login(llist *acc_list, Data data){ // check username, password, status and return message for client
    struct node *current;
    current = *acc_list;
    while(current != NULL){
        Data *tmp = (Data *)current->data;
        if(strcmp(tmp->username, data.username) == 0){
            if(tmp->status == 1 && strcmp(tmp->password, data.password) == 0){
                tmp->client.sin_family = data.client.sin_family;
                tmp->client.sin_addr = data.client.sin_addr;
                tmp->client.sin_port = data.client.sin_port; 
                return "OK";
            }else if(tmp->status == 0){
                return "Account not ready";
            }else{
                tmp->loginfail++;
                if(tmp->loginfail == 3){
                    tmp->status = 0;
                    updateFile(acc_list);
                    return "Account not ready";
                }
                return "Invalid password";
            }
        }
        current = current->next;
    }
    return "Invalid account";
}

void changePass(llist *acc_list, Data data){
    struct node * current = *acc_list;
    while(current != NULL){
        Data *tmp = (Data *)current->data;
        if(strcmp(tmp->username, data.username) == 0){
            strcpy(tmp->password, data.password);
        }
        current = current->next;
    }
    updateFile(acc_list);
}

struct sockaddr_in Login(llist *acc_list){ // Receive username, password from client and check
    struct sockaddr_in client_tmp;
    memset(&client_tmp, 0, sizeof(client_tmp));
    int length;
    int buff_size = 0;
    char *message = (char *)malloc(sizeof(char) * MAX);
    Data data;
    length = sizeof(client_addr_1);
    memset(data.username, '\0', sizeof(data.username));
    memset(data.password, '\0', sizeof(data.password));

    buff_size = recvfrom(sockfd, (char *)data.username, MAX, MSG_WAITALL, (struct sockaddr *) &client_tmp, &length);
    data.username[buff_size] = '\0';

    buff_size = recvfrom(sockfd, (char *)data.password, MAX, MSG_WAITALL, (struct sockaddr *) &client_tmp, &length);
    data.password[buff_size] = '\0';
        
    
    data.client.sin_addr = client_tmp.sin_addr;
    data.client.sin_family = client_tmp.sin_family;
    data.client.sin_port = client_tmp.sin_port;

    message = check_login(acc_list, data);
    printf("%s\n", message);
    sendto(sockfd, (char *)message, strlen(message), MSG_CONFIRM, (struct sockaddr *) &client_tmp, length); 
    
    if(strcmp(message, "OK") != 0){
        client_tmp = Login(acc_list);
    }
    else{
        buff_size = recvfrom(sockfd, (char *)data.password, MAX, MSG_WAITALL, (struct sockaddr *) &client_tmp, &length);
        data.password[buff_size] = '\0';
        char *number = (char *)malloc(sizeof(char) * MAX);
        char *string = (char *)malloc(sizeof(char) * MAX);
        char *error = "Error";
        int check = check_input(data.password, number, string);
        if(check == 0){
            changePass(acc_list, data);
            printf("User %s changes password succeed!\n", data.username);
            sendto(sockfd, (char *)string, strlen(string), MSG_CONFIRM, (struct sockaddr *) &client_tmp, length);
            sendto(sockfd, (char *)number, strlen(number), MSG_CONFIRM, (struct sockaddr *) &client_tmp, length);
        }else{
            printf("User %s, changes password fail!\n", data.username);
            sendto(sockfd, (char *)error, strlen(error), MSG_CONFIRM, (struct sockaddr *) &client_tmp, length);
        }  
    }
    return client_tmp;
}

int main(int argc, char const *argv[]){
    
    if(argc < 2 ){
        printf("Server creation failed! Need port number! Usage: %s [port_number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    llist *acc_list =  create_llist(NULL);

    readFlie(acc_list);

    setupServer(port);

    client_addr_1 = Login(acc_list);
    client_addr_2 = Login(acc_list);
    
    mess_handle(acc_list);

    close(sockfd);
    return 0;
}
