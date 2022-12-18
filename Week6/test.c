#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
int main()
{
    char *username = calloc(100, sizeof(char));
    printf("Enter: ");
    scanf("%[^\n]%*c", username);
    username[strlen(username)] = '\0';
    printf("%s\n", username);
    return 0;
}
