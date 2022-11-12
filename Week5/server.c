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
#define MAX_CHAR 100
#define MAX_CHAR_OF_FILE 8888
int sockfd, conndf;
struct sockaddr_in server_addr, client_addr;
struct hostent *client;
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
    return k; // check succeed
}

void send_recv_message(){
    int buffer_size;
    char *number = (char *)malloc(sizeof(char) * MAX_CHAR);
    char *string = (char *)malloc(sizeof(char) * MAX_CHAR);
    char *error = "Error";
    int check = 0;
    char *buffer = (char *)malloc(sizeof(char) * MAX_CHAR);
    char *client_ip;
    do{
        bzero(buffer, MAX_CHAR);
        bzero(number, MAX_CHAR);
        bzero(string, MAX_CHAR);
        buffer_size = recv(conndf, buffer, MAX_CHAR, 0);
        buffer[buffer_size = '\0'];
        if(strcmp(buffer, "Exit") == 0){
            break;
        }
        client_ip = inet_ntoa(client_addr.sin_addr);
        printf("Message from %s [ip %s, port %d]: %s\n", client->h_name, client_ip, client_addr.sin_port, buffer);
        check = check_input(buffer, number, string);
        if(check != 1){
            send(conndf, (int *)&check, sizeof(check), 0);
            send(conndf, string, strlen(string), 0);
            send(conndf, number, strlen(number), 0);
        }else{
            printf("Message contains invalid character\n");
            send(conndf, error, strlen(error), 0);
        }
        if(strlen(buffer) == 0){
            printf("Goodbye %s\n", client->h_name);
            close(conndf);
            close(sockfd);
            exit(0);
        }
    }while(1);
}
void recv_file_content(){
    char *buffer = (char *)malloc(sizeof(char) * MAX_CHAR_OF_FILE);
    char *client_ip;
    int buffer_size;
    do{
        bzero(buffer, MAX_CHAR_OF_FILE);
        buffer_size = recv(conndf, buffer, MAX_CHAR_OF_FILE, 0);
        buffer[buffer_size] = '\0';
        if(strcmp(buffer, "Exit") == 0){
            break;
        }
        client_ip = inet_ntoa(client_addr.sin_addr);
        printf("Succeed receive file content from client %s [ip %s, port %d]:\n",client->h_name, client_ip, client_addr.sin_port);
        printf("%s\n", buffer);
        printf("---------------------------------\n");
    }while(1);
}
void serve(){
    int choice;
    do{
        printf("-----------------------------------\n");
        printf("Waiting client...\n");
        recv(conndf, (int *)&choice, sizeof(choice), 0);
        switch (choice){
        case 1:
            printf("Client choose option sending string message\n");
            send_recv_message();
            break;
        case 2:
            printf("Client choose option sending file text\n");
            recv_file_content();
            break;
        default:
            printf("Client is disconnected\n");
            printf("Goodbye %s\n", client->h_name);
            close(sockfd);
            close(conndf);
            exit(0);
            break;
        }
    }while(1);
}

void setupServer(int port){ //setup a server
    unsigned int length = sizeof(client_addr);
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if((bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) != 0){
        perror("Socket bind failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Server is now online...\n");
    if(listen(sockfd, 5) != 0){
        perror("Listen failed\n");
        exit(EXIT_FAILURE);
    }
    if((conndf = accept(sockfd, (struct sockaddr *)&client_addr, &length)) < 0){
        perror("Server accept failed\n");
        exit(EXIT_FAILURE);
    }
    client = gethostbyaddr((const char *)&client_addr.sin_addr.s_addr, sizeof(client_addr.sin_addr), AF_INET);
}
int main(int argc, char const *argv[]){
    int port = atoi(argv[1]);
    setupServer(port);
    serve();
    close(sockfd);
    close(conndf);
    return 0;
}
