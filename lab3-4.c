#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define FIFO_0_TO_1 "/tmp/fifo_0_to_1"
#define FIFO_1_TO_0 "/tmp/fifo_1_to_0"
#define BUF_SIZE 256
#define EXIT_WORD "exit"

void create_fifo(const char *name) {
    if (mkfifo(name, 0666) < 0) {
        if (errno != EEXIST) {
            perror("mkfifo");
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp(argv[1], "0") != 0 && strcmp(argv[1], "1") != 0)) {
        fprintf(stderr, "There must be only one argument: 1 or 0.");
        exit(1);
    }

    int terminal = atoi(argv[1]);
    char buf[BUF_SIZE];
    int fd_write, fd_read;

    create_fifo(FIFO_0_TO_1);
    create_fifo(FIFO_1_TO_0);

    if (terminal == 0) {
        printf("Terminal 0. Waiting for another terminal\n");
        fd_write = open(FIFO_0_TO_1, O_WRONLY);
        fd_read  = open(FIFO_1_TO_0, O_RDONLY);
    } else {
        printf("Terminal 1. Waiting for another terminal\n");
        fd_read  = open(FIFO_0_TO_1, O_RDONLY);
        fd_write = open(FIFO_1_TO_0, O_WRONLY);
    }

    if (fd_write < 0 || fd_read < 0) {
        perror("open");
        exit(1);
    }

    printf("Connected! To finish enter \"%s\"\n", EXIT_WORD);
    int my_turn_to_write = (terminal == 0);

    while (1) {
        if (my_turn_to_write) {
            printf("[%d] > ", terminal);
            fflush(stdout);

            if (fgets(buf, BUF_SIZE, stdin) == NULL) {
                break;
            }
			
			if (strchr(buf, '\n') == NULL) {
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                printf("Error: Message is too long! Maximum of %d characters\n", BUF_SIZE - 1);
                continue;
            }
			
            buf[strcspn(buf, "\n")] = '\0';
			
			if (strlen(buf) == 0) {
                printf("Error: Empty message\n");
                continue;
            }
			
            ssize_t n = write(fd_write, buf, strlen(buf) + 1);
            if (n < 0) {
                perror("write");
                break;
            }

            if (strcmp(buf, EXIT_WORD) == 0) {
                break;
            }
        } else {
            ssize_t n = read(fd_read, buf, BUF_SIZE);
            if (n <= 0) {
                printf("Channel closed by another terminal\n");
                break;
            }
            printf("[%d] %s\n", terminal, buf);

            if (strcmp(buf, EXIT_WORD) == 0) {
                printf("Finished\n");
                break;
            }
        }

        my_turn_to_write = !my_turn_to_write;
    }

    close(fd_write);
    close(fd_read);

    if (terminal == 1) {
        unlink(FIFO_0_TO_1);
        unlink(FIFO_1_TO_0);
    }

    return 0;
}
