#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
 
#define MAX_SIGNALS 100

volatile sig_atomic_t received_signals[MAX_SIGNALS];
volatile sig_atomic_t count = 0;

void my_handler(int nsig){
    if (count < MAX_SIGNALS) {
        received_signals[count] = nsig;
        count++;
    }
}

const char *sig_name(int nsig){
    switch(nsig){
        case SIGINT:  return "SIGINT";
        case SIGQUIT: return "SIGQUIT";
        case SIGTERM: return "SIGTERM";
        default:      return "UNKNOWN";
    }
}
 
int main(void){
    struct sigaction sa;
    sigset_t set;
    int i;
 
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = my_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
 
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGTERM);
 
    printf("PID of process: %d\n", getpid());
    fflush(stdout);
 
    printf("Blocking SIGINT, SIGQUIT, SIGTERM for a 5 seconds...\n");
    fflush(stdout);
    sigprocmask(SIG_BLOCK, &set, NULL);
    sleep(5);
 
    printf("Unblocked signals. Send them within 20 seconds...\n");
    fflush(stdout);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    sleep(20);
 
    printf("List of unprocessed signals (%d):\n", (int)count);
    for (i = 0; i < count; i++) {
        printf("-->%d) number=%d (%s)\n", i + 1,
               (int)received_signals[i], sig_name(received_signals[i]));
    }
 
    return 0;
}
 
