#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#define MAX_CHAR 100
#define MAX_CHAR_OF_FILE 8888
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

void send_recv_mess(){
    char *buffer = (char *)malloc(sizeof(char) * MAX_CHAR);
    char *message = (char *)malloc(sizeof(char) * MAX_CHAR);
    char *string = (char *)malloc(sizeof(char) * MAX_CHAR);
    char *number = (char *)malloc(sizeof(char) * MAX_CHAR);
    char *exit = "Exit";
    int buffer_size, string_length, i;
    do{
        bzero(message, MAX_CHAR);
        bzero(buffer, MAX_CHAR);
        bzero(string, MAX_CHAR);
        bzero(number, MAX_CHAR);
        printf("Input message: ");
        fflush(stdin);
        scanf("%[^\n]%*c", message);
        printf("-----------------------------------\n");
        if(strlen(message) == 0){
            send(sockfd, exit, strlen(exit), 0);
            break;
        }
        send(sockfd, message, strlen(message), 0);
        recv(sockfd, (int *)&string_length, sizeof(string_length), 0);
        buffer_size = recv(sockfd, buffer, MAX_CHAR, 0);
        buffer[buffer_size] = '\0';
        if(strcmp(buffer, "Error") == 0 || string_length == -1){
            printf("Message from server: %s\n", buffer);
        }else{
            for(i = 0; i < buffer_size; i++){
                if(i < string_length){
                    string[i] = buffer[i];
                }else{
                    number[i - string_length] = buffer[i];
                }
            }
            number[buffer_size - string_length] ='\0';
            string[string_length] = '\0';
            if(strlen(buffer) > 0){
                printf("Message from server: %s\n", string);
            }
            if(strlen(number) > 0){
                printf("Message from server: %s\n", number);
            }
        }
    }while(1);
}
char* readFile(char *file_path){
    FILE *pt = fopen(file_path, "r");
    int file_size;
    char *file_content = (char *)malloc(sizeof(char) * MAX_CHAR_OF_FILE);
    if(pt == NULL){
        printf("File was not found\n");
        return NULL;
    }else{
        file_size = fread(file_content, MAX_CHAR_OF_FILE, 1, pt);
        //printf("%s\n", file_content);
    }
    return file_content;
}
void send_file_txt(){
    char *buffer = (char *)malloc(sizeof(char) * MAX_CHAR);
    char *message = (char *)malloc(sizeof(char) * MAX_CHAR_OF_FILE);
    char *file_path = (char *)malloc(sizeof(char) * MAX_CHAR *10);
    char *error = "Exit";
    do{
        bzero(buffer, MAX_CHAR);
        bzero(message, MAX_CHAR);
        do{
            bzero(file_path, MAX_CHAR);
            printf("Input file path: ");
            fflush(stdin);
            scanf("%[^\n]%*c", file_path);
            printf("-----------------------------------\n");
            if(strlen(file_path) == 0){
                getchar();
                send(sockfd, error, strlen(error), 0);
                break;
            }
            message = readFile(file_path);
        }while(message == NULL);
        if(strlen(file_path) == 0){
            break;
        }
        printf("File was sent\n");
        printf("-----------------------------------\n");
        send(sockfd, message, strlen(message), 0);

    }while(1);
}
void MENU(){
    printf("MENU\n-----------------------------------\n");
    printf("1. Gửi xâu bất kỳ\n");
    printf("2. Gửi nội dung một file\n");
    printf("Your choice: ");
}

void menu_precess(){
    int choice = 0;
    do{
        MENU();
        fflush(stdin);
        scanf("%d", &choice);
        getchar();
        send(sockfd, (int *)&choice, sizeof(choice), 0);
        switch (choice){
            case 1:
                send_recv_mess();
                break;
            case 2:
                send_file_txt();
                break;
            default:
                close(sockfd);
                exit(0);
                break;
            }
    }while(1);
}
int main(int argc, char const *argv[])  {
    if(argc < 3){
        printf("Server connection failed! Usage: %s [host_ip_address] [port_number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[2]);
    setupClient(argv[1], port);
    menu_precess();
    close(sockfd);
    return 0;
}
