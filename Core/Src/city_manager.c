#include <sys/stat.h>   // stat(), mkdir()
#include <sys/types.h>  // time_t, mode_t
#include <sys/wait.h>  // waitpid()
#include <time.h>       // time()
#include <unistd.h>     // write(), read(), lseek()
#include <fcntl.h>      // open()
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum role {Inspector, Manager} Role_t; //Inspector - group, Manager - owner

#define BASE_DIR "city_districts"
#define SEVERITY_THRESHOLD 2

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

void logAction(Args_t *args, const char *action) {
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s/%s/logged_district", BASE_DIR, args->district);
    
    int log_fd = open(log_path, O_WRONLY | O_APPEND);
    if (log_fd == -1) return;
    
    char log_entry[256];
    snprintf(log_entry, sizeof(log_entry), "%ld\t%s\t%s\t%s\n",
             time(NULL), args->user,
             args->role == Manager ? "manager" : "inspector",
             action);
    write(log_fd, log_entry, strlen(log_entry));
    close(log_fd);
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
        chmod(district_path, 0750);
        int fd;
        fd = open(dat_path, O_RDWR | O_CREAT | O_APPEND, 0664);
        chmod(dat_path, 0664);
        close(fd);
        fd = open(cfg_path, O_CREAT | O_WRONLY, 0640);
        chmod(cfg_path, 0640);
        close(fd);
        fd = open(log_path, O_CREAT | O_WRONLY, 0644);
        chmod(log_path, 0644);
        close(fd);
        char relative_target[256];
        snprintf(relative_target, sizeof(relative_target), "%s/reports.dat", args->district);
        symlink(relative_target, link_name);
        fd = open(cfg_path, O_RDWR);
        char config_text[40];
        snprintf(config_text, sizeof(config_text), "threshold=%d\n", SEVERITY_THRESHOLD);
        write(fd, config_text, strlen(config_text));
        close(fd);
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

    logAction(args, "add");

    printf("Report added successfully with ID %d\n", report.report_id);
}

void permsToString(mode_t mode, char *permissions) {
    permissions[0] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[1] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[2] = (mode & S_IXUSR) ? 'x' : '-';
    permissions[3] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[4] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[5] = (mode & S_IXGRP) ? 'x' : '-';
    permissions[6] = (mode & S_IROTH) ? 'r' : '-';
    permissions[7] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[8] = (mode & S_IXOTH) ? 'x' : '-';
    permissions[9] = '\0';
}

void checkSymlink(Args_t *args) {
    char link_name[256], dat_path[256];
    snprintf(link_name, sizeof(link_name), "%s/active_reports-%s", BASE_DIR, args->district);
    snprintf(dat_path, sizeof(dat_path), "%s/%s/reports.dat", BASE_DIR, args->district);

    struct stat lst;
    if (lstat(link_name, &lst) == -1) {
        printf("Warning: symlink %s does not exist\n", link_name);
        return;
    }
    if (!S_ISLNK(lst.st_mode)) {
        printf("Warning: %s is not a symlink\n", link_name);
        return;
    }
    struct stat st;
    if (stat(link_name, &st) == -1) {
        printf("Warning: dangling symlink detected — %s points to nonexistent file\n", link_name);
    }
}

void cmdList(Args_t *args){
    checkSymlink(args);
    char dist_report_path[256];
    snprintf(dist_report_path,sizeof(dist_report_path),"%s/%s/reports.dat", BASE_DIR, args->district);

    int fd = open(dist_report_path, O_RDONLY);


    struct stat st;
    if(stat(dist_report_path,&st) == -1){
        printf("Unknown district: %s", args->district);
        exit(1);
    }

    char permissions[30];
    permsToString(st.st_mode, permissions);
    printf("\nFile: %s\n", dist_report_path);
    printf("Permissions: %s\n", permissions);
    printf("Size: %ld bytes\n", st.st_size);
     printf("Last modified: %s", ctime(&st.st_mtime));
    printf("-----------------------------------\n");

    if(fd == -1){
        printf("Unknown district: %s", args->district);
    }

    Report_t report;
    while(read(fd, &report, sizeof(Report_t)) == sizeof(Report_t)){
        printf("Report ID: %d, ", report.report_id);
        printf("Name: %s, ",report.name);
        printf("Latitude: %f| Longitude: %f, ", report.latitude, report.longitude);
        printf("Issue category: %s, ", report.issue_category);
        printf("Severity: %d, ", report.severity);
        printf("Description: %s, ", report.description);
        printf("Timestamp: %ld\n", report.timestamp);
        printf("-----------------------------------\n");
    }
    logAction(args, "list");

    close(fd);
}

void cmdView(Args_t *args){
    char dist_report_path[256];
    snprintf(dist_report_path, sizeof(dist_report_path), "%s/%s/reports.dat", BASE_DIR, args->district);
    
    int fd = open(dist_report_path, O_RDONLY);

    struct stat st;
    if(stat(dist_report_path,&st) == -1){
        printf("Unknown district: %s", args->district);
        exit(1);
    }

    lseek(fd, args->report_id * sizeof(Report_t), SEEK_SET);
    Report_t report;

    if(read(fd, &report, sizeof(Report_t)) != sizeof(Report_t)){
        printf("Error: report ID %d not found\n", args->report_id);
        close(fd);
        exit(1);
    }

    printf("Report ID: %d\n", report.report_id);
    printf("Name: %s\n", report.name);
    printf("Latitude: %f | Longitude: %f\n", report.latitude, report.longitude);
    printf("Issue category: %s\n", report.issue_category);
    printf("Severity: %d\n", report.severity);
    printf("Description: %s\n", report.description);
    printf("Timestamp: %s", ctime(&report.timestamp));

    logAction(args, "view");

    close(fd);
}

int checkPermission(mode_t mode, Role_t role) {
    if (role == Manager) {
        if (!(mode & S_IWUSR)) {
            printf("Permission denied!\n");
            return 0;
        }
    }
    if (role == Inspector) {
        if (!(mode & S_IWGRP)) {
            printf("Permission denied!\n");
            return 0;
        }
    }
    return 1;
}

void cmdRemove(Args_t *args){
    char dist_report_path[256];
    snprintf(dist_report_path, sizeof(dist_report_path), "%s/%s/reports.dat", BASE_DIR, args->district);
    
    int fd = open(dist_report_path, O_RDWR);

    struct stat st;
    if(stat(dist_report_path,&st) == -1){
        printf("Unknown district: %s", args->district);
    }
    if(!checkPermission(st.st_mode, args->role)){
        exit(1);
    }

    Report_t report;
    lseek(fd, args->report_id * sizeof(Report_t), SEEK_SET);

    int total = st.st_size / sizeof(Report_t);

    if (args->report_id < 0 || args->report_id >= total) {
        printf("Error: report ID %d does not exist\n", args->report_id);
        close(fd);
        exit(1);
    }

    for (int i = args->report_id; i < total - 1; i++) {
        lseek(fd, (i + 1) * sizeof(Report_t), SEEK_SET);
        read(fd, &report, sizeof(Report_t));

        report.report_id = i;

        lseek(fd, i * sizeof(Report_t), SEEK_SET);
        write(fd, &report, sizeof(Report_t));
    }
    ftruncate(fd, (total - 1) * sizeof(Report_t));

    logAction(args, "remove_report");

    close(fd);
    printf("Report %d removed successfully\n", args->report_id);
}

void cmdUpdateThreshold(Args_t *args){
    char cfg_path[256];
    snprintf(cfg_path, sizeof(cfg_path), "%s/%s/district.cfg", BASE_DIR, args->district);

    struct stat st;
    stat(cfg_path, &st);
    if (!checkPermission(st.st_mode, args->role)) {
        exit(1);
    }
    if((st.st_mode & 0777) != 0640){
        printf("Error: district.cfg permissions have been tampered with!\n");
        exit(1);
    }

    int fd = open(cfg_path, O_WRONLY | O_TRUNC);
    char threshold_str[32];
    snprintf(threshold_str, sizeof(threshold_str), "threshold=%d\n", args->threshold);
    write(fd, threshold_str, strlen(threshold_str));
    close(fd);
    logAction(args, "update_threshold");
    printf("Threshold updated to %d\n", args->threshold);
}

int parse_condition(const char *input, char *field, char *op, char *value) {
    const char *first_colon = strchr(input, ':');
    if (!first_colon) return 0;

    strncpy(field, input, first_colon - input);
    field[first_colon - input] = '\0';

    const char *second_colon = strchr(first_colon + 1, ':');
    if (!second_colon) return 0;

    strncpy(op, first_colon + 1, second_colon - first_colon - 1);
    op[second_colon - first_colon - 1] = '\0';

    strcpy(value, second_colon + 1);
    return 1;
}

int match_condition(Report_t *r, const char *field, const char *op, const char *value) {
    if (strcmp(field, "severity") == 0) {
        int val = atoi(value);
        if (strcmp(op, "==") == 0) return r->severity == val;
        if (strcmp(op, "!=") == 0) return r->severity != val;
        if (strcmp(op, "<")  == 0) return r->severity <  val;
        if (strcmp(op, "<=") == 0) return r->severity <= val;
        if (strcmp(op, ">")  == 0) return r->severity >  val;
        if (strcmp(op, ">=") == 0) return r->severity >= val;
    }
    if (strcmp(field, "category") == 0) {
        int cmp = strcmp(r->issue_category, value);
        if (strcmp(op, "==") == 0) return cmp == 0;
        if (strcmp(op, "!=") == 0) return cmp != 0;
    }
    if (strcmp(field, "inspector") == 0) {
        int cmp = strcmp(r->name, value);
        if (strcmp(op, "==") == 0) return cmp == 0;
        if (strcmp(op, "!=") == 0) return cmp != 0;
    }
    if (strcmp(field, "timestamp") == 0) {
        time_t val = (time_t)atol(value);
        if (strcmp(op, "==") == 0) return r->timestamp == val;
        if (strcmp(op, "!=") == 0) return r->timestamp != val;
        if (strcmp(op, "<")  == 0) return r->timestamp <  val;
        if (strcmp(op, "<=") == 0) return r->timestamp <= val;
        if (strcmp(op, ">")  == 0) return r->timestamp >  val;
        if (strcmp(op, ">=") == 0) return r->timestamp >= val;
    }
    return 0;
}

void cmdFilter(Args_t *args) {
    char dat_path[256];
    snprintf(dat_path, sizeof(dat_path), "%s/%s/reports.dat", BASE_DIR, args->district);

    int fd = open(dat_path, O_RDONLY);
    if (fd == -1) {
        printf("Error: district '%s' not found\n", args->district);
        exit(1);
    }

    char field[32], op[4], value[64];
    Report_t report;

    while (read(fd, &report, sizeof(Report_t)) == sizeof(Report_t)) {
        int match = 1;
        for (int i = 0; i < args->condition_count; i++) {
            if (!parse_condition(args->conditions[i], field, op, value)) {
                printf("Error: invalid condition '%s'\n", args->conditions[i]);
                match = 0;
                break;
            }
            if (!match_condition(&report, field, op, value)) {
                match = 0;
                break;
            }
        }
        if (match) {
            printf("Report ID: %d\n", report.report_id);
            printf("Name: %s\n", report.name);
            printf("Category: %s\n", report.issue_category);
            printf("Severity: %d\n", report.severity);
            printf("Description: %s\n", report.description);
            printf("Timestamp: %s", ctime(&report.timestamp));
            printf("-----------------------------------\n");
        }
    }
    close(fd);
    logAction(args, "filter");
}

void cmdRemoveDistrict(Args_t * args){
    if(args->role != Manager){
        printf("Error: only managers can remove districts\n");
        exit(1);
    }

    char district_path[256], link_name[256];
    snprintf(district_path, sizeof(district_path), "%s/%s", BASE_DIR, args->district);
    snprintf(link_name, sizeof(link_name), "%s/active_reports-%s", BASE_DIR, args->district);

    struct stat st;
    if(stat(district_path, &st) == -1){
        printf("Error: district '%s' not found\n", args->district);
        exit(1);
    }

    pid_t pid = fork();

    if(pid == 0){
        execl("/bin/rm", "rm", "-rf", district_path, NULL);
        printf("Error: execl failed\n");
        exit(1);
    }
    else{
        int status;
        waitpid(pid, &status, 0);
        unlink(link_name);
        printf("District '%s' removed successfully\n", args->district);
        logAction(args, "remove_district");
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