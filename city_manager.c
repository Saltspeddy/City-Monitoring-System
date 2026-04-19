#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum role {Inspector, Manager} Role_t; //Inspector - group, Manager - owner

// void read_command(){

// }

Role_t role;
char* user;

void readFlags(int argc, char * argv[]){
    int i = 1;
    while(i < argc){
        if(strcmp(argv[i],"--role") == 0){
            if (strcmp(argv[++i], "manager") == 0)
                role = Manager;
            else if (strcmp(argv[i], "inspector") == 0)
                role = Inspector;
            else {
                printf("Error: unknown role '%s'\n", argv[i]);
                exit(1);
            }
        }
        if(strcmp(argv[i],"--user") == 0){
            user = malloc(sizeof(char) * (strlen(argv[++i]) + 1));
            strcpy(user,argv[i]);
        }
        i++;
    }
}

int main(int argc, char * argv[]){
//     int exit_manager = 0;
//     while(!exit_manager){ May be added later
//         read_command();
//     }

    readFlags(argc, argv);
    printf("%d %s", role, user);

    free(user);
}