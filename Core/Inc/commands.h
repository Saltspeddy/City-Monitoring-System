#ifndef COMMANDS_H
#define COMMANDS_H

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define BASE_DIR "city_districts"
#define SEVERITY_THRESHOLD 2
#define MONITOR_PID_FILE "city_districts/.monitor_pid"

typedef enum role {Inspector, Manager} Role_t;

typedef struct {
    Role_t role;
    char *user;
    char *command;
    char *district;
    int report_id;
    int threshold;
    char **conditions;
    int condition_count;
} Args_t;

typedef struct {
    int report_id;
    char name[30];
    float latitude, longitude;
    char issue_category[20];
    int severity;
    time_t timestamp;
    char description[150];
} Report_t;

void logAction(Args_t *args, const char *action);
void cmdAdd(Args_t *args);
void permsToString(mode_t mode, char *permissions);
void checkSymlink(Args_t *args);
void cmdList(Args_t *args);
void cmdView(Args_t *args);
void cmdRemove(Args_t *args);
void cmdUpdateThreshold(Args_t *args);
int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(Report_t *r, const char *field, const char *op, const char *value);
void cmdFilter(Args_t *args);
void cmdRemoveDistrict(Args_t * args);

#endif