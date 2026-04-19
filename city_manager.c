#include <sys/stat.h>   // stat(), mkdir()
#include <sys/types.h>  // time_t, mode_t
#include <time.h>       // time()
#include <unistd.h>     // write(), read(), lseek()
#include <fcntl.h>      // open()
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum role {Inspector, Manager} Role_t; //Inspector - group, Manager - owner

#define BASE_DIR "city_districts"

typedef struct {
    Role_t role;
    char *user;
    char *command;    // "add", "list", "view", "remove_report", "update_threshold", "filter"
    char *district;   // e.g. "downtown"
    int report_id;    // for view and remove_report
    int threshold;    // for update_threshold
    char **conditions; // for filter e.g. "severity:>=:2"
    int condition_count;
} Args_t;

typedef struct{
    int report_id;
    char name[30];
    float latitude, longitude;
    char issue_category[20];
    int severity;
    time_t timestamp;
    char description[150];
} Report_t;

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

void cmdAdd(Args_t *args) {
    char dat_path[256], cfg_path[256], log_path[256], link_name[256], district_path[256];

    snprintf(district_path, sizeof(district_path), "%s/%s", BASE_DIR, args->district);
    snprintf(dat_path, sizeof(dat_path), "%s/%s/reports.dat", BASE_DIR, args->district);
    snprintf(cfg_path, sizeof(cfg_path), "%s/%s/district.cfg", BASE_DIR, args->district);
    snprintf(log_path, sizeof(log_path), "%s/%s/logged_district", BASE_DIR, args->district);
    snprintf(link_name, sizeof(link_name), "%s/active_reports-%s", BASE_DIR, args->district);

    struct stat st;
    if(stat(district_path, &st) == -1){
        mkdir(district_path, 0750);
        int fd;
        fd = open(dat_path, O_RDWR | O_CREAT | O_APPEND, 0664);
        close(fd);
        fd = open(cfg_path, O_CREAT | O_WRONLY, 0640);
        close(fd);
        fd = open(log_path, O_CREAT | O_WRONLY, 0644);
        close(fd);
        symlink(dat_path, link_name);
    }

    Report_t report = {0};

    printf("X: "); 
    scanf("%f", &report.latitude);
    printf("Y: "); 
    scanf("%f", &report.longitude);
    printf("Category (road/lighting/flooding/other): "); 
    scanf("%19s", report.issue_category);
    printf("Severity level (1/2/3): "); 
    scanf("%d", &report.severity);
    printf("Description: "); 
    scanf(" %149[^\n]", report.description);

    strncpy(report.name, args->user, 29);
    report.timestamp = time(NULL);

    int fd = open(dat_path, O_RDWR | O_APPEND);
    off_t size = lseek(fd, 0, SEEK_END);
    report.report_id = size / sizeof(Report_t);

    write(fd, &report, sizeof(Report_t));
    close(fd);

    int log_fd = open(log_path, O_WRONLY | O_APPEND);
    char log_entry[256];
    snprintf(log_entry, sizeof(log_entry), "%ld\t%s\t%s\tadd\n",
             time(NULL), args->user, 
             args->role == Manager ? "manager" : "inspector");
    write(log_fd, log_entry, strlen(log_entry));
    close(log_fd);

    printf("Report added successfully with ID %d\n", report.report_id);
}

void runCommand(Args_t *args){
    if (strcmp(args->command, "add") == 0)
       cmdAdd(args);
    // else if (strcmp(command, "list") == 0)
      
    // else if (strcmp(command, "view") == 0)
        
    // else if (strcmp(command, "remove_report") == 0)
        
    // else if (strcmp(command, "update_threshold") == 0)
        
    // else if (strcmp(command, "filter") == 0)
       
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
    printf("%d %s", args.role, args.user);

    
}