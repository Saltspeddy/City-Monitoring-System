#include <sys/stat.h>   // stat(), mkdir()
#include <sys/types.h>  // time_t, mode_t
#include <sys/wait.h>   // waitpid()
#include <signal.h>     // signal
#include <time.h>       // time()
#include <unistd.h>     // write(), read(), lseek()
#include <fcntl.h>      // open()
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../Inc/commands.h"

#define BASE_DIR "city_districts"
#define SEVERITY_THRESHOLD 2

void readCommand(char command[], Args_t *args){
    if (strcmp(command, "--add") == 0)
        args->command = "add";
    else if (strcmp(command, "--list") == 0)
        args->command = "list";
    else if (strcmp(command, "--view") == 0)
        args->command = "view";
    else if (strcmp(command, "--remove_report") == 0)
        args->command = "remove_report";
    else if (strcmp(command, "--update_threshold") == 0)
        args->command = "update_threshold";
    else if (strcmp(command, "--filter") == 0)
        args->command = "filter";
    else if (strcmp(command, "--remove_district") == 0)
        args->command = "remove_district";
    else {
        printf("Error: unknown command '%s'\n", command);
        exit(1);
    }
}

void readFlags(int argc, char * argv[], Args_t *args){
    int i = 1;
    while(i < argc){
        if(strcmp(argv[i],"--role") == 0){
            if (strcmp(argv[++i], "manager") == 0)
                args->role = Manager;
            else if (strcmp(argv[i], "inspector") == 0)
                args->role = Inspector;
            else {
                printf("Error: unknown role '%s'\n", argv[i]);
                exit(1);
            }
        }
        else if(strcmp(argv[i],"--user") == 0){
            args->user = malloc(sizeof(char) * (strlen(argv[++i]) + 1));
            strcpy(args->user,argv[i]);
        }
                else {
            readCommand(argv[i], args);
            args->district = argv[++i];

            if (strcmp(args->command, "view") == 0 || 
                strcmp(args->command, "remove_report") == 0)
                args->report_id = atoi(argv[++i]);
            else if (strcmp(args->command, "update_threshold") == 0)
                args->threshold = atoi(argv[++i]);
            else if (strcmp(args->command, "filter") == 0) {
                args->conditions = &argv[++i];
                args->condition_count = argc - i;
                break;
            }
        }
        i++;
    }
}

void runCommand(Args_t *args){
    if (strcmp(args->command, "add") == 0)
        cmdAdd(args);
    else if (strcmp(args->command, "list") == 0)
        cmdList(args);
    else if (strcmp(args->command, "view") == 0)
        cmdView(args);
    else if (strcmp(args->command, "remove_report") == 0)
        cmdRemove(args);
    else if (strcmp(args->command, "update_threshold") == 0)
        cmdUpdateThreshold(args);
    else if (strcmp(args->command, "filter") == 0)
        cmdFilter(args);
    else if (strcmp(args->command, "remove_district") == 0)
        cmdRemoveDistrict(args);
    else {
        printf("Error: unknown command '%s'\n", args->command);
        exit(1);
    }
}

int main(int argc, char * argv[]){
//     int exit_manager = 0;
//     while(!exit_manager){ May be added later
//         read_command();
//     }
    Args_t args = {0};
    readFlags(argc, argv, &args);
    runCommand(&args);
} 