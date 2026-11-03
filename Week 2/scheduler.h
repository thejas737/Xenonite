#ifndef SCHEDULER_H
#define SCHEDULER_H

#define MAX_PROCESSES 100
#define MAX_SEGMENTS 100000
#define NAME_LEN 16

typedef struct {
    char id[NAME_LEN];
    int arrival, burst, priority;
} Process;

typedef struct {
    int pid;       /* index into original process array; -1 means IDLE */
    int start, end;
} Segment;

typedef struct {
    char name[40];
    double avg_waiting, avg_turnaround, avg_response;
    double cpu_utilization;
    int total_time, idle_time;
    int count;
    Segment segments[MAX_SEGMENTS];
    int completion[MAX_PROCESSES];
    int waiting[MAX_PROCESSES];
    int turnaround[MAX_PROCESSES];
    int response[MAX_PROCESSES];
} ScheduleResult;

enum Algorithm {
    FCFS = 1, SJF_NP, SJF_P, PRIORITY_NP, PRIORITY_P, ROUND_ROBIN
};

int run_schedule(const Process p[], int n, enum Algorithm alg, int quantum,
                 ScheduleResult *out);
void print_report(const Process p[], int n, const ScheduleResult *r);
const char *algorithm_name(enum Algorithm alg);

#endif
