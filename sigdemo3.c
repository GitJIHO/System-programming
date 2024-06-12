#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

time_t time_t1;
time_t time_t2;

void f(int signum);

int main() {
    
    signal(SIGINT, f);
    time_t2 = time(NULL);

    printf("you can't stop me!\n");

    while(1) {
        sleep(1);
        printf("haha\n");
    }
    return 0;
}

void f(int signum){
    time_t1 = time(NULL);
    long int curtime = time_t1-time_t2;
    printf("Currently elapsed time: %ld sec(s)", curtime);
}

