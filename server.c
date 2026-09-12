#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>

/* Визначення спільних констант та структури */
#define RESPONSE_OFFSET 1000
#define MIN_CLIENT_TYPE 1
#define MAX_CLIENT_TYPE 10
#define TEXT_LEN 128
#define TERMINATE_TEXT "SERVER_TERMINATE"

struct msg_buffer {
    long mtype;
    int  client_type;
    int  msg_num;
    char text[TEXT_LEN];
};

static int msqid = -1;

static void cleanup_and_exit(int signo)
{
    (void) signo;
    if (msqid != -1) {
        if (-1 == msgctl(msqid, IPC_RMID, NULL))
            perror("msgctl(IPC_RMID)");
        else
            printf("\nMessage queue (msqid = %d) deleted. Server was terminated.\n", msqid);
    }
    exit(0);
}

static void print_timestamp(void) {
    time_t now = time(NULL);
    char buf[26];
    ctime_r(&now, buf);
    buf[24] = '\0';
    printf("%s", buf);
}

int main(int argc, char **argv) {
    /* За замовчуванням генеруємо ключ через ftok */
    key_t key = ftok(argv[0], 'A');
    if (key == -1) {
        key = 0x1234ABCD;
    }

    int perms = 0666;
    int n_limit = 0;
    int c;

    while (-1 != (c = getopt(argc, argv, "k:n:p:"))) {
        switch (c)
        {
        case 'k':
            key = (key_t) strtoul(optarg, NULL, 0);
            break;
        case 'n':
            n_limit = atoi(optarg);
            break;
        case 'p':
            perms = (int) strtoul(optarg, NULL, 0);
            break;
        default:
            fprintf(stderr, "Incorrect use");
            return 1;
        }
    }

    if (n_limit <= 0) {
        fprintf(stderr, "Error: N > 0\n");
        return 1;
    }

    msqid = msgget(key, IPC_CREAT | perms);
    if (-1 == msqid) {
        perror("msgget()");
        return 1;
    }

    printf("Server started. msqid=%d, key=0x%x, N=%d\n", msqid, key, n_limit);
    printf("Waiting for clients messages...\n",
           MIN_CLIENT_TYPE, MAX_CLIENT_TYPE);

    signal(SIGINT, cleanup_and_exit);
    signal(SIGTERM, cleanup_and_exit);

    int counters[MAX_CLIENT_TYPE + 1] = {0};

    struct msg_buffer msg;
    struct msg_buffer resp;

    for (;;)
    {
        ssize_t ret = msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), -MAX_CLIENT_TYPE, 0);
        if (ret == -1) {
            if (errno == EINTR)
                continue;
            perror("msgrcv()");
            break;
        }

        int type = msg.client_type;
        if (type < MIN_CLIENT_TYPE || type > MAX_CLIENT_TYPE) {
            fprintf(stderr, "Ignored message with incorrect type\n");
            continue;
        }

        counters[type]++;

        printf("New message: type=%d, №=%d, text=\"%s\", time: ",
               type, msg.msg_num, msg.text);
        print_timestamp();
        printf("  (got %d/%d messages from client with type %d)\n",
               counters[type], n_limit, type);

        memset(&resp, 0, sizeof(resp));
        resp.mtype = type + RESPONSE_OFFSET;
        resp.client_type = type;
        resp.msg_num = msg.msg_num;

        if (counters[type] >= n_limit) {
            strncpy(resp.text, TERMINATE_TEXT, TEXT_LEN - 1);
            if (-1 == msgsnd(msqid, &resp, sizeof(resp) - sizeof(long), 0))
                perror("msgsnd() (termination)");
            else
                printf("-> Client with type %d was terminated.\n\n", type);

            counters[type] = 0;
        } else {
            snprintf(resp.text, TEXT_LEN, "ACK %d", msg.msg_num);
            if (-1 == msgsnd(msqid, &resp, sizeof(resp) - sizeof(long), 0))
                perror("msgsnd() (ack)");
        }
    }

    cleanup_and_exit(0);
    return 0;
}