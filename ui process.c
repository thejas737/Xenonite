#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

#define QUEUE_UI_TO_CORE "/ui_to_core"
#define HALT_OPCODE 99

typedef struct {
    int opcode;
    int value;
} UI_Command;

int main(void) {
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = sizeof(UI_Command),
        .mq_curmsgs = 0
    };

    /* O_CREAT handles startup if the UI runs before Core. */
    mqd_t ui_mq = mq_open(QUEUE_UI_TO_CORE, O_CREAT | O_WRONLY, 0644, &attr);
    if (ui_mq == (mqd_t)-1) {
        perror("[UI] Failed to open message queue");
        return EXIT_FAILURE;
    }

    printf("[UI] Connected to the Core process.\n");

    for (;;) {
        UI_Command cmd;

        printf("\nEnter opcode (99 to halt): ");
        if (scanf("%d", &cmd.opcode) != 1) {
            fprintf(stderr, "[UI] Invalid opcode input.\n");
            break;
        }

        printf("Enter value: ");
        if (scanf("%d", &cmd.value) != 1) {
            fprintf(stderr, "[UI] Invalid value input.\n");
            break;
        }

        if (mq_send(ui_mq, (const char *)&cmd, sizeof(cmd), 0) == -1) {
            perror("[UI] Failed to send command");
            break;
        }

        printf("[UI] Sent opcode %d with value %d.\n", cmd.opcode, cmd.value);

        if (cmd.opcode == HALT_OPCODE) {
            printf("[UI] Halt command sent.\n");
            break;
        }
    }

    if (mq_close(ui_mq) == -1) {
        perror("[UI] Failed to close message queue");
        return EXIT_FAILURE;
    }

    /*
     * Core-Process.c already unlinks /ui_to_core when it exits.
     * Leave unlinking to Core so the UI does not remove the queue
     * while Core is still using it.
     */
    printf("[UI] Exiting.\n");
    return EXIT_SUCCESS;
}
