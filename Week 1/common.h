#ifndef COMMON_H
#define COMMON_H

#include <mqueue.h>
#include <stddef.h>

#define CMD_QUEUE "/osdsa_cmd_q"
#define RESP_QUEUE "/osdsa_resp_q"
#define LOG_QUEUE "/osdsa_log_q"
#define LOG_FILE "simulator.log"
#define TEXT_LEN 160
#define MEMORY_SIZE 10
#define STACK_SIZE 10
#define QUEUE_SIZE 10

typedef enum {
    CMD_ADD = 1,
    CMD_MEM_READ,
    CMD_PUSH,
    CMD_POP,
    CMD_ENQUEUE,
    CMD_DEQUEUE,
    CMD_SHOW,
    CMD_BENCHMARK,
    CMD_SHUTDOWN
} CommandType;

typedef struct {
    int type;
    int value;
    char text[TEXT_LEN];
} Message;

static inline void set_text(Message *m, const char *s) {
    size_t i = 0;
    if (!m) return;
    while (s && s[i] && i < TEXT_LEN - 1) {
        m->text[i] = s[i];
        ++i;
    }
    m->text[i] = '\0';
}

#endif
