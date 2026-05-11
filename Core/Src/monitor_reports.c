#include <sys/stat.h>   // stat(), mkdir()
#include <sys/types.h>  // time_t, mode_t
#include <sys/wait.h>  // waitpid()
#include <time.h>       // time()
#include <unistd.h>     // write(), read(), lseek()
#include <fcntl.h>      // open()
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MONITOR_PID_FILE "city_districts/.monitor_pid"

void handleUSR1(int sig) {
    (void)sig;
    printf("Monitor: new report has been added!\n");
}

void handleINT(int sig) {
    (void)sig;
    printf("\nMonitor: shutting down...\n");
    unlink(MONITOR_PID_FILE);
    exit(0);
}

int main(void){

    int fd;
    fd = open(MONITOR_PID_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if ( fd == -1 ){
        printf("Error: could not create .monitor_pid\n");
        exit(1);
    }

    pid_t pid = getpid();
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d\n", pid);
    write(fd, pid_str, strlen(pid_str));
    close(fd);

    signal(SIGUSR1, handleUSR1);
    signal(SIGINT, handleINT);

    printf("Monitor started with PID %d\n", pid);
    
    while(1){
        pause();
    }

    return 0;
}