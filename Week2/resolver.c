#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
int check_input(char *input){ // check parameter that it's IP address or hostname. return 1 if it's hostname, 0 if it's IP  
    int i;
    char c;
    for(i = 0; i < strlen(input); i++){
        c = input[i];
        if(c == 46){
            continue;
        }else if(c < 48 || c > 57){
            return 1; 
        }
    }
    return 0;
}
int main(int argc, char *argv[]){
    if(argc < 2){ // check parameter
        printf("Need parameter! Bad usage : %s [ip_address/host_name]! Example : %s [23.202.89.74/google.com]\n", argv[0], argv[0]);
        return -1;
    }
    struct hostent *host;
    struct in_addr addr;
    struct in_addr **addr_list; 
    char **allias_list;
    char *input = (char *)malloc(sizeof(char) * 50);
    int i;
    strcpy(input ,argv[1]);
    if(check_input(input) == 1){
        host = gethostbyname(input);
        if(host == NULL){
            printf("Not found information\n");
            return -1;
        }else{
            printf("Official IP: %s\n", inet_ntoa(*(struct in_addr*)host->h_addr));
            printf("Allias IP: ");
            addr_list = (struct in_addr **)host->h_addr_list;
            for(i = 0; addr_list[i] != NULL; i++){
                printf("%s\n", inet_ntoa(*addr_list[i]));
            }
        }
    }else{    
        inet_aton(input, &addr);
        host = gethostbyaddr(&addr, sizeof(struct in_addr), AF_INET);
        if(host == NULL){
            printf("Not found information\n");
            return -1;
        }else{
            printf("Official name: %s\n",host->h_name);
            allias_list = host->h_aliases;
            for(i = 0; allias_list[i] != NULL; i++){ // cant find case that have many alias name to test this function
                printf("Allias name: %s\n", allias_list[i]);
            }
        }
    }
    return 0;
}