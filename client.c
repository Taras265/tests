#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
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

int main(int argc, char **argv)
{
    key_t key = 0;
    int type = -1;
    int delay_ms = 500;
    int c;

    while (-1 != (c = getopt(argc, argv, "k:t:d:")))
    {
        switch (c)
        {
        case 'k':
            key = (key_t) strtoul(optarg, NULL, 0);
            break;
        case 't':
            type = atoi(optarg);
            break;
        case 'd':
            delay_ms = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Incorrect use");
            return 1;
        }
    }

    if (type < MIN_CLIENT_TYPE || type > MAX_CLIENT_TYPE) {
        fprintf(stderr, "Error: -t type must be from %d to %d.\n",
                MIN_CLIENT_TYPE, MAX_CLIENT_TYPE);
        return 1;
    }

    if (key == 0) {
        key = 0x1234ABCD;
    }

    int msqid = msgget(key, 0);
    if (-1 == msqid) {
        perror("msgget()");
        return 1;
    }

    printf("Client started. type=%d, msqid=%d\n\n", type, msqid);

    struct msg_buffer msg;
    struct msg_buffer resp;
    int msg_num = 1;

    for (;;)
    {
        memset(&msg, 0, sizeof(msg));
        msg.mtype = type;
        msg.client_type = type;
        msg.msg_num = msg_num;
        snprintf(msg.text, TEXT_LEN, "Message №%d from client with type %d", msg_num, type);

        if (-1 == msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0)) {
            perror("msgsnd()");
            break;
        }
        printf("Sent: \"%s\"\n", msg.text);

        ssize_t ret = msgrcv(msqid, &resp, sizeof(resp) - sizeof(long), type + RESPONSE_OFFSET, 0);
        if (ret == -1) {
            perror("msgrcv()");
            break;
        }

        if (strcmp(resp.text, TERMINATE_TEXT) == 0) {
            printf("Got message to terminate.\n");
            break;
        }

        printf("Got from server: \"%s\"\n\n", resp.text);

        msg_num++;
        if (delay_ms > 0)
            usleep((useconds_t) delay_ms * 1000);
    }

    return 0;
}