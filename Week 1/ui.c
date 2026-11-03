#include "common.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static mqd_t open_queue(const char *name, long msg_size) {
    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = msg_size;
    mqd_t q = mq_open(name, O_CREAT | O_RDWR, 0660, &attr);
    if (q == (mqd_t)-1) perror(name);
    return q;
}

static int send_and_receive(mqd_t cmdq, mqd_t respq, Message *request) {
    if (mq_send(cmdq, (const char *)request, sizeof(*request), 0) == -1) {
        perror("mq_send command");
        return -1;
    }
    Message response;
    if (mq_receive(respq, (char *)&response, sizeof(response), NULL) == -1) {
        perror("mq_receive response");
        return -1;
    }
    printf("\n%s\n", response.text);
    return 0;
}

int main(void) {
    mqd_t cmdq = open_queue(CMD_QUEUE, sizeof(Message));
    mqd_t respq = open_queue(RESP_QUEUE, sizeof(Message));
    if (cmdq == (mqd_t)-1 || respq == (mqd_t)-1) return 1;

    puts("========================================");
    puts(" MULTI-PROCESS OS + DSA SIMULATOR");
    puts(" UI process | Core process | Logger process");
    puts("========================================");

    int running = 1;
    while (running) {
        puts("\n1. CPU: add two integers");
        puts("2. Read simulated memory slot");
        puts("3. Push onto stack");
        puts("4. Pop from stack");
        puts("5. Enqueue value");
        puts("6. Dequeue value");
        puts("7. Show memory, stack and queue");
        puts("8. Benchmark CPU addition loop");
        puts("0. Exit");
        printf("Choose: ");
        fflush(stdout);

        int choice;
        if (scanf("%d", &choice) != 1) {
            int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
            puts("Invalid input. Enter a menu number.");
            continue;
        }

        Message request;
        memset(&request, 0, sizeof(request));
        int a, b;
        switch (choice) {
            case 1:
                printf("Enter two integers: ");
                if (scanf("%d %d", &a, &b) != 2) { puts("Invalid integers."); break; }
                request.type = CMD_ADD; request.value = a; snprintf(request.text, sizeof(request.text), "%d", b);
                if (send_and_receive(cmdq, respq, &request)) running = 0;
                break;
            case 2:
                printf("Memory slot (0-%d): ", MEMORY_SIZE - 1);
                if (scanf("%d", &a) != 1) { puts("Invalid slot."); break; }
                request.type = CMD_MEM_READ; request.value = a;
                if (send_and_receive(cmdq, respq, &request)) running = 0;
                break;
            case 3:
                printf("Value to push: ");
                if (scanf("%d", &a) != 1) { puts("Invalid value."); break; }
                request.type = CMD_PUSH; request.value = a;
                if (send_and_receive(cmdq, respq, &request)) running = 0;
                break;
            case 4:
                request.type = CMD_POP;
                if (send_and_receive(cmdq, respq, &request)) running = 0;
                break;
            case 5:
                printf("Value to enqueue: ");
                if (scanf("%d", &a) != 1) { puts("Invalid value."); break; }
                request.type = CMD_ENQUEUE; request.value = a;
                if (send_and_receive(cmdq, respq, &request)) running = 0;
                break;
            case 6:
                request.type = CMD_DEQUEUE;
                if (send_and_receive(cmdq, respq, &request)) running = 0;
                break;
            case 7:
                request.type = CMD_SHOW;
                if (send_and_receive(cmdq, respq, &request)) running = 0;
                break;
            case 8:
                printf("Number of additions (e.g. 1000000): ");
                if (scanf("%d", &a) != 1 || a < 1) { puts("Enter a positive integer."); break; }
                request.type = CMD_BENCHMARK; request.value = a;
                if (send_and_receive(cmdq, respq, &request)) running = 0;
                break;
            case 0:
                request.type = CMD_SHUTDOWN;
                if (send_and_receive(cmdq, respq, &request) == 0) running = 0;
                else running = 0;
                break;
            default: puts("Choose one of the listed options."); break;
        }
    }

    mq_close(cmdq); mq_close(respq);
    mq_unlink(CMD_QUEUE); mq_unlink(RESP_QUEUE); mq_unlink(LOG_QUEUE);
    puts("UI process stopped. Check simulator.log for execution events.");
    return 0;
}
