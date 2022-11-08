#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX 100

typedef struct Message{
    char username[MAX];
    char string[MAX];
    char number[MAX];
}Message;

int sockfd;
struct sockaddr_in server_addr;

void mess_handle(){
    Message message; 
    Message buffer;
    int buff_size = 0;
    int length;
    int c;
    do{
        memset(&buffer, 0, sizeof(buffer));
        memset(&message, 0, sizeof(message));
        printf("Input message: ");
        fflush(stdin);
        scanf("%[^\n]%*c", message.string);
        printf("------------------------------------\n");
        write(sockfd, &message, sizeof(Message));
        sendto(sockfd, (Message*) &message, sizeof(Message), MSG_CONFIRM, (struct sockaddr *) &server_addr , sizeof(server_addr));
        length = sizeof(server_addr);
        buff_size = recvfrom(sockfd, (Message *)&buffer, sizeof(Message), MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
        if(strlen(buffer.string) > 0){
            printf("Message from %s: %s\n", buffer.username, buffer.string);
        }
        if(strcmp(buffer.string, "Error") != 0 && strlen(buffer.number) > 0){
            printf("Message from %s: %s\n", buffer.username, buffer.number);
        }
    }while(strcmp(buffer.string, "bye") != 0 && strlen(message.string) != 0);
}

void setupClient(const char *ip, int port){
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);
}

void Login(){
    char *username = (char *)malloc(sizeof(char) * MAX);
    char *password = (char *)malloc(sizeof(char) * MAX);
    char *buffer = (char *)malloc(sizeof(char) * MAX);
    char *new_password = (char *)malloc(sizeof(char) * MAX);
    int length = sizeof(server_addr);
    int buff_size = 0;
    int c;
    printf("User: ");
    fflush(stdin);
    scanf("%[^\n]%*c", username);
    printf("Password: ");
    fflush(stdin);
    scanf("%[^\n]%*c", password);

    sendto(sockfd, (char *)username, strlen(username), MSG_CONFIRM, (const struct sockaddr *) &server_addr, length);
    
    sendto(sockfd, (char *)password, strlen(password), MSG_CONFIRM, (const struct sockaddr *) &server_addr, length);
    
    buff_size = recvfrom(sockfd, (char *)buffer, MAX, MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
    buffer[buff_size] = '\0';
    printf("%s\n", buffer);
    if(strcmp(buffer, "OK") == 0){
        do{
            memset(new_password, '\0', sizeof(buffer));
            printf("Enter new password (Must be different 'bye'): ");
            fflush(stdin);
            scanf("%[^\n]%*c", new_password);
        }while(strcmp(new_password, "bye") == 0);
        sendto(sockfd, (char *)new_password, strlen(new_password), MSG_CONFIRM, (struct sockaddr *) &server_addr, length);
        memset(buffer, '\0', sizeof(buffer));
        buff_size = recvfrom(sockfd, (char *)buffer, MAX, MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
        buffer[buff_size] = '\0';
        if(strlen(buffer) > 0){
             printf("Receive from server :%s", buffer);
        }
        if(strcmp("Error", buffer) != 0){
            memset(buffer, '\0', sizeof(buffer));
            buff_size = recvfrom(sockfd, (char *)buffer, MAX, MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
            buffer[buff_size] = '\0';
            if(strlen(buffer) > 0){
                printf(" and %s", buffer);
            }
        }
        printf("\n");
    }else{
        Login();
    }
}

int main(int argc, char const *argv[]){
    if(argc < 3){
        printf("Server connection failed! Usage: %s [host_ip_address] [port_number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);
   
    setupClient(argv[1], port);
    Login();
    mess_handle();
    close(sockfd);
    return 0;
}
