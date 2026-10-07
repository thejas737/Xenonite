#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

// Define the queue names (must start with '/')
#define QUEUE_UI_TO_CORE "/ui_to_core"
#define QUEUE_CORE_TO_LOG "/core_to_log"
#define MAX_MSG_SIZE 256

// Structured message for UI commands
typedef struct {
    int opcode;       // e.g., 1 for PUSH, 2 for POP, 3 for ADD
    int value;        // Data for memory/stack operations
} UI_Command;

int main() {
    mqd_t ui_mq, log_mq;
    struct mq_attr attr;
    UI_Command cmd;
    char log_buffer[MAX_MSG_SIZE];

    // Configure queue attributes
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(UI_Command);
    attr.mq_curmsgs = 0;

    printf("[CORE] Starting Core Process (CPU, Memory, Stack, Queue)...\n");

    // Open Queue to receive from UI (Create if it doesn't exist)
    ui_mq = mq_open(QUEUE_UI_TO_CORE, O_CREAT | O_RDONLY, 0644, &attr);
    if (ui_mq == (mqd_t)-1) {
        perror("[CORE] Failed to open UI queue");
        exit(1);
    }

    // Open Queue to send to Logger (Logger must create its own queue attributes, so we just open for write)
    struct mq_attr log_attr;
    log_attr.mq_flags = 0;
    log_attr.mq_maxmsg = 10;
    log_attr.mq_msgsize = MAX_MSG_SIZE;
    log_attr.mq_curmsgs = 0;
    
    log_mq = mq_open(QUEUE_CORE_TO_LOG, O_CREAT | O_WRONLY, 0644, &log_attr);
    if (log_mq == (mqd_t)-1) {
        perror("[CORE] Failed to open Logger queue");
        exit(1);
    }

    printf("[CORE] Waiting for instructions from UI...\n");

    // Core Execution Loop
    while (1) {
        // 1. Receive Instruction from UI (This blocks until a message arrives)
        ssize_t bytes_read = mq_receive(ui_mq, (char *)&cmd, sizeof(UI_Command), NULL);
        if (bytes_read >= 0) {
            
            // 2. Execute CPU/Memory/Stack Logic
            if (cmd.opcode == 99) { // Example termination code
                sprintf(log_buffer, "HALT instruction received. Shutting down.");
                mq_send(log_mq, log_buffer, strlen(log_buffer) + 1, 0);
                break;
            }

            printf("[CORE] Executing Opcode: %d with Value: %d\n", cmd.opcode, cmd.value);
            
            // Format execution result for the logger
            sprintf(log_buffer, "SUCCESS: Executed opcode %d (Value: %d) on CPU/Stack.", cmd.opcode, cmd.value);

            // 3. Send log to Logger Process
            if (mq_send(log_mq, log_buffer, strlen(log_buffer) + 1, 0) == -1) {
                perror("[CORE] Failed to send log");
            }
        }
    }

    // Cleanup IPC resources
    mq_close(ui_mq);
    mq_close(log_mq);
    mq_unlink(QUEUE_UI_TO_CORE); // Only one process needs to unlink the queue
    printf("[CORE] Process terminated safely.\n");

    return 0;
}