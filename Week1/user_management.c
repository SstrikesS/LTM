#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_OF_CHAR 50
typedef struct Data{
    int status; // 1 is active, 0 is block
    char username[MAX_OF_CHAR];
    char password[MAX_OF_CHAR];
    int loginfail; // count numbers of login failed
}Data;
typedef struct Node{ //setup node of linklist
    struct Node * next;
    Data data;

}*LL;
LL user; //username that login successfully, only one account can be login in one time

LL make_node(){ //make a none data node
    LL temp = (LL)malloc(sizeof(struct Node));
    temp->next = NULL;
    return temp;
}
LL add_node(LL head, Data data){ // add a node into linklist
    LL temp = make_node();
    temp->data = data;
    if(head == NULL){
        head = temp;
    }else{
        LL p = head;
        while (p->next != NULL){
            p = p->next;
        }
        p->next = temp;
    }
    return head;
}
LL readFile(LL head){ // read info from file to linklist
    FILE *pt = fopen("account.txt", "r");
    if(pt == NULL){
        printf("File was not found\n");
        exit(1);
    }
    char c = fgetc(pt);
    Data data;
    while (c != EOF){
        if(c != '\n' && c != '\t' && c != ' ' && c != EOF){
            fseek(pt, -1, SEEK_CUR);
            fscanf(pt, "%s", data.username);
            fscanf(pt, "%s", data.password);
            fscanf(pt, "%d", &data.status);
            data.loginfail = 0;
            head = add_node(head, data);
        }
        c = fgetc(pt);
    }
    fclose(pt);
    LL p = head;
    // for(p = head; p != NULL; p = p->next){
    //     printf("%s\n", p->data.username);
    // }
    return head;
}
void MENU(){ // display a menu 
    printf("-----------------------------------\nUSER MANAGEMENT PROGRAM\n-----------------------------------\n1. Register\n2. Sign in\n3. Search\n4. Sign out\nYour choice (1-4, other to quit): ");
}
LL check_acc(LL head, char *username){
    LL p;
    for(p = head; p != NULL; p = p->next){
        if(strcmp(p->data.username, username) == 0)
            return p;
    }
    return NULL;
}
LL update_file(LL head){ // update info to file txt
    FILE *pt = fopen("account.txt", "w+");
    if(pt == NULL){
        printf("File was not found\n");
        exit(1);
    }
    LL temp;
    for(temp = head; temp != NULL; temp = temp->next){
        fprintf(pt, "%s %s %d\n", temp->data.username, temp->data.password, temp->data.status);
    }
    fclose(pt);
    return head;
}
LL register_acc(LL head){ // register an account
    Data data;
    printf("Enter username: ");
    fflush(stdin);
    scanf("%s", data.username);
    LL temp = check_acc(head, data.username);
    if(temp == NULL){
        printf("Enter password: ");
        fflush(stdin);
        scanf("%s", data.password);
        data.status = 1;
        data.loginfail = 0;
        head = add_node(head, data);
        printf("Successful registration\n");
        head = update_file(head);
    }
    else{
       printf("Account existed\n"); 
    }
    return head;
}
LL Login(LL head){ // Sign in
    Data data;
    printf("Enter username: ");
    fflush(stdin);
    scanf("%s", data.username);
    LL temp = check_acc(head, data.username);
    if(temp == NULL){
        printf("Can't find account\n");
    }
    else if(temp != NULL && temp->data.status == 1){
        printf("Enter password: ");
        fflush(stdin);
        scanf("%s", data.password);
        if(strcmp(data.password, temp->data.password) == 0){
            printf("Hello %s\n", temp->data.username);
            user = temp;
        }else{
            printf("Password is incorrect\n");
            temp->data.loginfail++;
            if(temp->data.loginfail == 3){
                temp->data.status = 0;
                printf("Account is blocked\n");
                head = update_file(head);
            }
        }
    }
    else{
        printf("Account is blocked\n");
    }
    return head;
}
LL search(LL head){
    Data data;
    printf("Enter username: ");
    fflush(stdin);
    scanf("%s", data.username);
    LL temp = check_acc(head, data.username);
    if(user == NULL || temp == NULL){
        printf("Can't find account or you are not sign in yet\n");
    }
    else if(temp->data.status == 0){
        printf("Account is blocked\n");
    }
    else{
        printf("Account is actived\n");
    }
    return head;
}
LL Logout(LL head){
    Data data;
    printf("Enter username: ");
    fflush(stdin);
    scanf("%s", data.username);
    LL temp = check_acc(head, data.username);
    if(temp == NULL){
        printf("Can't not find account\n");
    }
    else if(user == NULL || strcmp(user->data.username, temp->data.username) != 0){
        printf("Account is not sign in\n");
    }
    else{
        printf("Goodbye %s\n", temp->data.username);
        user = NULL;
    }
    return head;
}
int main(){
    LL head = NULL;
    head = readFile(head);
    user = NULL;
    int choice = 1;
    do{
        MENU();
        scanf("%d", &choice);
        switch (choice){
        case 1:
            head = register_acc(head);
            break;
        case 2:
            if(user == NULL){
                head = Login(head);
            }else{
                printf("You must sign out to login an new account\n");
            }
            break;
        case 3:
            head = search(head);
            break;
        case 4:
            head = Logout(head);
            break;
        default:
            exit(1);
            break;
        }
    }while(1);
    return 0;
}