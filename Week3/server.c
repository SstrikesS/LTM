#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX 100
int check_input(char *input, char *number, char *string){
    int i;
    int j = 0;
    int k = 0;
    for(i = 0; i < strlen(input) - 1; i++){
        if(input[i] >= '0' && input[i] <= '9'){
            number[j++] = input[i];
        }else if((input[i] >= 'a' && input[i] <= 'z') || (input[i] >= 'A' && input[i] <= 'Z')){
            string[k++] = input[i];
        }else{
            return 1; // check failed
        }
    }
    return 0; // check succeed
}

int main(int argc, char const *argv[]){
    
    if(argc < 2 ){
        printf("Server creation failed! Need port number! Usage: %s [port_number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int sockfd;
    struct sockaddr_in server_addr, client_addr_1 , client_addr_2, client_tmp;
    int buff_size = 0;
    int length;
    char *buffer = (char *)malloc(sizeof(char) * MAX);
    struct hostent *host;
    char *host_addr;
    char *number = (char *)malloc(sizeof(char) * MAX);
    char *string = (char *)malloc(sizeof(char) * MAX);
    char *error = "Error"; 
    int check;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Socket creation failed!\n");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr_1, 0, sizeof(client_addr_1));
    memset(&client_addr_2, 0, sizeof(client_addr_2));
    memset(&client_tmp, 0, sizeof(client_tmp));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if(bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("Socket binding failed\n");
        exit(EXIT_FAILURE);
    }

    printf("Server is now online\n");

    length = sizeof(client_addr_1);
    buff_size = recvfrom(sockfd, (char *)buffer, MAX, MSG_WAITALL, (struct sockaddr *) &client_addr_1, &length);
    length = sizeof(client_addr_2);
    buff_size = recvfrom(sockfd, (char *)buffer, MAX, MSG_WAITALL, (struct sockaddr *) &client_addr_2, &length);
    while (1){
        memset(buffer, '\0', sizeof(buffer));
        memset(number, '\0', sizeof(number));
        memset(string, '\0', sizeof(string));
        length = sizeof(client_tmp);
        buff_size = recvfrom(sockfd, (char *)buffer, MAX, MSG_WAITALL, (struct sockaddr *) &client_tmp, &length);
        buffer[buff_size] = '\0';
        check = check_input(buffer, number, string);
        host = gethostbyaddr((const char *)&client_tmp.sin_addr.s_addr, sizeof(client_tmp.sin_addr.s_addr), AF_INET);
        host_addr = inet_ntoa(client_tmp.sin_addr);
        printf("Succeed recieved message from %s [ip %s, port %d]: %s\n", host->h_name, host_addr, client_tmp.sin_port ,buffer);
        printf("-----------------------------------------------------\n");
        printf("%s\n", buffer);
        printf("%d\n", check);
        printf("%ld %ld\n", strlen(number), strlen(string));
        if(strcmp(host_addr, inet_ntoa(client_addr_1.sin_addr)) == 0 && client_addr_1.sin_port == client_tmp.sin_port){
            if(check == 0){
                sendto(sockfd, (char *)number, strlen(number), MSG_CONFIRM, (const struct sockaddr *) &client_addr_2, sizeof(client_addr_2));
                sendto(sockfd, (char *)string, strlen(string), MSG_CONFIRM, (const struct sockaddr *) &client_addr_2, sizeof(client_addr_2));
            }else{
                sendto(sockfd, (char *)error, strlen(error), MSG_CONFIRM, (const struct sockaddr *) &client_addr_2, sizeof(client_addr_2));
            }        
        }else{
            if(check == 0){
                sendto(sockfd, (char *)number, strlen(number), MSG_CONFIRM, (const struct sockaddr *) &client_addr_1, sizeof(client_addr_1));
                sendto(sockfd, (char *)string, strlen(string), MSG_CONFIRM, (const struct sockaddr *) &client_addr_1, sizeof(client_addr_1));
            }else{
                sendto(sockfd, (char *)error, strlen(error), MSG_CONFIRM, (const struct sockaddr *) &client_addr_1, sizeof(client_addr_1));
            }
        }
    }
    close(sockfd);
    return 0;
}
