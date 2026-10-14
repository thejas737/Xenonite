#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

/* ======================================================================
 *  PROCESS SCHEDULING SIMULATOR
 *  Part 1: Core algorithms (original code + small logging hooks)
 *  Part 2: Visualization & reporting (Gantt chart, comparison table)
 *  Part 3: Test cases and main()
 * ====================================================================== */

// Standard structure for all algorithms
typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int start_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;
    int response_time;
} Process;

/* ----------------------------------------------------------------------
 *  EXECUTION LOG (needed for the Gantt chart)
 *  start_time/completion_time on a Process only store the FIRST start and
 *  the FINAL finish, which is not enough to draw preemptive algorithms
 *  (SRTF, preemptive Priority, Round Robin). So every algorithm records
 *  each stretch of CPU time it hands out: (pid, from, to).  pid = -1 -> idle.
 * -------------------------------------------------------------------- */
#define MAX_SEG 2000

typedef struct {
    int pid;     // -1 means CPU idle
    int start;
    int end;
} Segment;

static Segment executed[MAX_SEG];
static int seg_count = 0;

// Record a slice of CPU time; merges with the previous slice if contiguous & same pid
static void log_segment(int pid, int start, int end) {
    if (end <= start) return;
    if (seg_count > 0 &&
        executed[seg_count - 1].pid == pid &&
        executed[seg_count - 1].end == start) {
        executed[seg_count - 1].end = end;
        return;
    }
    if (seg_count < MAX_SEG) {
        executed[seg_count].pid = pid;
        executed[seg_count].start = start;
        executed[seg_count].end = end;
        seg_count++;
    }
}

// Averages of the most recent algorithm run (filled by calculate_metrics)
typedef struct {
    float avg_wt, avg_tat, avg_rt;
} Metrics;

static Metrics last_metrics;

/* ======================== PART 1: CORE ALGORITHMS ====================== */

// Helper function to reset process states before running a new algorithm
void reset_processes(Process p[], int n) {
    for (int i = 0; i < n; i++) {
        p[i].remaining_time = p[i].burst_time;
        p[i].start_time = -1;
        p[i].completion_time = 0;
        p[i].waiting_time = 0;
        p[i].turnaround_time = 0;
        p[i].response_time = 0;
    }
    seg_count = 0;   // new run -> new Gantt log
}

// Helper function to calculate final metrics and averages
// (printing moved to show_results() so the output order is: table, Gantt, averages)
void calculate_metrics(Process p[], int n) {
    float total_wt = 0, total_tat = 0, total_rt = 0;
    for (int i = 0; i < n; i++) {
        p[i].turnaround_time = p[i].completion_time - p[i].arrival_time;
        p[i].waiting_time = p[i].turnaround_time - p[i].burst_time;
        total_wt  += p[i].waiting_time;
        total_tat += p[i].turnaround_time;
        total_rt  += p[i].response_time;
    }
    last_metrics.avg_wt  = total_wt / n;
    last_metrics.avg_tat = total_tat / n;
    last_metrics.avg_rt  = total_rt / n;
}

// 1. First-Come-First-Served (FCFS)  (expects p[] sorted by arrival time)
void fcfs(Process p[], int n) {
    reset_processes(p, n);
    int current_time = 0;

    for (int i = 0; i < n; i++) {
        // Handle idle CPU time if process hasn't arrived yet
        if (current_time < p[i].arrival_time) {
            log_segment(-1, current_time, p[i].arrival_time);
            current_time = p[i].arrival_time;
        }

        p[i].start_time = current_time;
        p[i].response_time = p[i].start_time - p[i].arrival_time;

        log_segment(p[i].pid, current_time, current_time + p[i].burst_time);
        current_time += p[i].burst_time;
        p[i].completion_time = current_time;
    }
    calculate_metrics(p, n);
}

// 2. Shortest Job First - Non-Preemptive
void sjf_non_preemptive(Process p[], int n) {
    reset_processes(p, n);
    int current_time = 0, completed = 0;
    bool is_completed[100] = {false};

    while (completed != n) {
        int idx = -1;
        int min_burst = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= current_time && !is_completed[i]) {
                if (p[i].burst_time < min_burst) {
                    min_burst = p[i].burst_time;
                    idx = i;
                }
            }
        }

        if (idx != -1) {
            p[idx].start_time = current_time;
            p[idx].response_time = p[idx].start_time - p[idx].arrival_time;
            log_segment(p[idx].pid, current_time, current_time + p[idx].burst_time);
            current_time += p[idx].burst_time;
            p[idx].completion_time = current_time;
            is_completed[idx] = true;
            completed++;
        } else {
            log_segment(-1, current_time, current_time + 1);
            current_time++; // CPU is idle
        }
    }
    calculate_metrics(p, n);
}

// 3. Shortest Job First - Preemptive (Shortest Remaining Time First)
void sjf_preemptive(Process p[], int n) {
    reset_processes(p, n);
    int current_time = 0, completed = 0;

    while (completed != n) {
        int idx = -1;
        int min_remaining = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= current_time && p[i].remaining_time > 0) {
                if (p[i].remaining_time < min_remaining) {
                    min_remaining = p[i].remaining_time;
                    idx = i;
                }
            }
        }

        if (idx != -1) {
            if (p[idx].start_time == -1) {
                p[idx].start_time = current_time;
                p[idx].response_time = p[idx].start_time - p[idx].arrival_time;
            }
            log_segment(p[idx].pid, current_time, current_time + 1);
            p[idx].remaining_time--;
            current_time++;

            if (p[idx].remaining_time == 0) {
                p[idx].completion_time = current_time;
                completed++;
            }
        } else {
            log_segment(-1, current_time, current_time + 1);
            current_time++;
        }
    }
    calculate_metrics(p, n);
}

// 4. Priority Scheduling - Non-Preemptive
void priority_non_preemptive(Process p[], int n) {
    reset_processes(p, n);
    int current_time = 0, completed = 0;
    bool is_completed[100] = {false};

    while (completed != n) {
        int idx = -1;
        int highest_priority = INT_MAX; // Lower number = higher priority

        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= current_time && !is_completed[i]) {
                if (p[i].priority < highest_priority) {
                    highest_priority = p[i].priority;
                    idx = i;
                }
            }
        }

        if (idx != -1) {
            p[idx].start_time = current_time;
            p[idx].response_time = p[idx].start_time - p[idx].arrival_time;
            log_segment(p[idx].pid, current_time, current_time + p[idx].burst_time);
            current_time += p[idx].burst_time;
            p[idx].completion_time = current_time;
            is_completed[idx] = true;
            completed++;
        } else {
            log_segment(-1, current_time, current_time + 1);
            current_time++;
        }
    }
    calculate_metrics(p, n);
}

// 5. Priority Scheduling - Preemptive
void priority_preemptive(Process p[], int n) {
    reset_processes(p, n);
    int current_time = 0, completed = 0;

    while (completed != n) {
        int idx = -1;
        int highest_priority = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= current_time && p[i].remaining_time > 0) {
                if (p[i].priority < highest_priority) {
                    highest_priority = p[i].priority;
                    idx = i;
                }
            }
        }

        if (idx != -1) {
            if (p[idx].start_time == -1) {
                p[idx].start_time = current_time;
                p[idx].response_time = p[idx].start_time - p[idx].arrival_time;
            }
            log_segment(p[idx].pid, current_time, current_time + 1);
            p[idx].remaining_time--;
            current_time++;

            if (p[idx].remaining_time == 0) {
                p[idx].completion_time = current_time;
                completed++;
            }
        } else {
            log_segment(-1, current_time, current_time + 1);
            current_time++;
        }
    }
    calculate_metrics(p, n);
}

// 6. Round Robin (RR) with configurable time quantum
void round_robin(Process p[], int n, int time_quantum) {
    reset_processes(p, n);
    int current_time = 0, completed = 0;
    int queue[1000], front = 0, rear = 0;
    bool in_queue[100] = {false};

    // Push initially arrived processes to queue
    for (int i = 0; i < n; i++) {
        if (p[i].arrival_time <= current_time) {
            queue[rear++] = i;
            in_queue[i] = true;
        }
    }

    while (completed != n) {
        if (front != rear) {
            int idx = queue[front++]; // Pop process from queue

            if (p[idx].start_time == -1) {
                p[idx].start_time = current_time;
                p[idx].response_time = p[idx].start_time - p[idx].arrival_time;
            }

            int execution_time = (p[idx].remaining_time > time_quantum) ? time_quantum : p[idx].remaining_time;
            log_segment(p[idx].pid, current_time, current_time + execution_time);
            p[idx].remaining_time -= execution_time;
            current_time += execution_time;

            // Check for new arrivals while this process was executing
            for (int i = 0; i < n; i++) {
                if (p[i].arrival_time > (current_time - execution_time) &&
                    p[i].arrival_time <= current_time &&
                    !in_queue[i]) {
                    queue[rear++] = i;
                    in_queue[i] = true;
                }
            }

            // If process is not finished, put it back at the end of the queue
            if (p[idx].remaining_time > 0) {
                queue[rear++] = idx;
            } else {
                p[idx].completion_time = current_time;
                completed++;
            }
        } else {
            // CPU Idle, move time forward and check for arrivals
            log_segment(-1, current_time, current_time + 1);
            current_time++;
            for (int i = 0; i < n; i++) {
                if (p[i].arrival_time == current_time && !in_queue[i]) {
                    queue[rear++] = i;
                    in_queue[i] = true;
                }
            }
        }
    }
    calculate_metrics(p, n);
}

/* ================= PART 2: VISUALIZATION & REPORTING =================== */

// ---- Per-process result table ----
void print_process_table(Process p[], int n) {
    printf("\n+-----+----+----+----+-----+-----+----+-----+----+\n");
    printf("| PID | AT | BT | PR | Strt| Comp| WT | TAT | RT |\n");
    printf("+-----+----+----+----+-----+-----+----+-----+----+\n");
    for (int i = 0; i < n; i++) {
        printf("| P%-2d | %2d | %2d | %2d | %3d | %3d | %2d | %3d | %2d |\n",
               p[i].pid, p[i].arrival_time, p[i].burst_time, p[i].priority,
               p[i].start_time, p[i].completion_time,
               p[i].waiting_time, p[i].turnaround_time, p[i].response_time);
    }
    printf("+-----+----+----+----+-----+-----+----+-----+----+\n");
}

// ---- Gantt chart (reads pid / start / end from the executed[] log) ----
void print_gantt(void) {
    if (seg_count == 0) return;

    int w[MAX_SEG];
    for (int i = 0; i < seg_count; i++) {
        int dur = executed[i].end - executed[i].start;
        w[i] = dur * 2;          // 2 characters per time unit...
        if (w[i] < 4)  w[i] = 4; // ...but never narrower than a label
        if (w[i] > 16) w[i] = 16;
    }

    printf("\nGantt Chart:\n");

    // top border
    for (int i = 0; i < seg_count; i++) {
        putchar('+');
        for (int k = 0; k < w[i]; k++) putchar('-');
    }
    printf("+\n");

    // process labels (centered)
    for (int i = 0; i < seg_count; i++) {
        char label[16];
        if (executed[i].pid < 0) snprintf(label, sizeof label, "IDLE");
        else                     snprintf(label, sizeof label, "P%d", executed[i].pid);
        int len = (int)strlen(label);
        int left = (w[i] - len) / 2;
        int right = w[i] - len - left;
        putchar('|');
        printf("%*s%s%*s", left, "", label, right, "");
    }
    printf("|\n");

    // bottom border
    for (int i = 0; i < seg_count; i++) {
        putchar('+');
        for (int k = 0; k < w[i]; k++) putchar('-');
    }
    printf("+\n");

    // time markers, aligned under each boundary
    for (int i = 0; i < seg_count; i++) {
        printf("%-*d", w[i] + 1, executed[i].start);
    }
    printf("%d\n", executed[seg_count - 1].end);
}

// ---- Comparison table ----
#define MAX_ALGOS 10

typedef struct {
    char  name[32];
    float avg_wt, avg_tat, avg_rt;
    float cpu_util;      // % of time CPU was busy
    float throughput;    // processes per time unit
    int   ctx_switches;  // dispatches after the first one
    int   makespan;      // time at which the last process finished
} AlgoResult;

// Capture the metrics of the run that just finished
void record_result(AlgoResult *r, const char *name, Process p[], int n) {
    strncpy(r->name, name, sizeof r->name - 1);
    r->name[sizeof r->name - 1] = '\0';
    r->avg_wt  = last_metrics.avg_wt;
    r->avg_tat = last_metrics.avg_tat;
    r->avg_rt  = last_metrics.avg_rt;

    int makespan = 0, busy = 0, runs = 0;
    for (int i = 0; i < n; i++)
        if (p[i].completion_time > makespan) makespan = p[i].completion_time;
    for (int i = 0; i < seg_count; i++) {
        if (executed[i].pid >= 0) {
            busy += executed[i].end - executed[i].start;
            runs++;
        }
    }
    r->makespan = makespan;
    r->cpu_util = makespan ? 100.0f * busy / makespan : 0;
    r->throughput = makespan ? (float)n / makespan : 0;
    r->ctx_switches = runs > 0 ? runs - 1 : 0;
}

static const char *best_mark(float v, float best) {
    return (v - best < 0.0001f) ? "*" : " ";
}

void print_comparison_table(AlgoResult r[], int count) {
    float bw = r[0].avg_wt, bt = r[0].avg_tat, br = r[0].avg_rt;
    for (int i = 1; i < count; i++) {
        if (r[i].avg_wt  < bw) bw = r[i].avg_wt;
        if (r[i].avg_tat < bt) bt = r[i].avg_tat;
        if (r[i].avg_rt  < br) br = r[i].avg_rt;
    }

    printf("\n==================== ALGORITHM COMPARISON ====================\n");
    printf("+---------------------------+-----------+-----------+-----------+--------+---------+-----+\n");
    printf("| Algorithm                 | Avg WT    | Avg TAT   | Avg RT    | CPU %%  | Thruput | CS  |\n");
    printf("+---------------------------+-----------+-----------+-----------+--------+---------+-----+\n");
    for (int i = 0; i < count; i++) {
        printf("| %-25s | %7.2f %s | %7.2f %s | %7.2f %s | %5.1f%% | %7.3f | %3d |\n",
               r[i].name,
               r[i].avg_wt,  best_mark(r[i].avg_wt, bw),
               r[i].avg_tat, best_mark(r[i].avg_tat, bt),
               r[i].avg_rt,  best_mark(r[i].avg_rt, br),
               r[i].cpu_util, r[i].throughput, r[i].ctx_switches);
    }
    printf("+---------------------------+-----------+-----------+-----------+--------+---------+-----+\n");
    printf("  * = best (lowest) in column   WT/TAT/RT in time units\n");
    printf("  CS = context switches (dispatches after the first)\n");
}

// Print everything for one finished algorithm run
void show_results(const char *name, Process p[], int n, AlgoResult *slot) {
    printf("\n--------------------------------------------------------------\n");
    printf(" %s\n", name);
    printf("--------------------------------------------------------------");
    print_process_table(p, n);
    print_gantt();
    printf("\nAverage Waiting Time:    %.2f\n", last_metrics.avg_wt);
    printf("Average Turnaround Time: %.2f\n", last_metrics.avg_tat);
    printf("Average Response Time:   %.2f\n", last_metrics.avg_rt);
    record_result(slot, name, p, n);
}

/* ============ PART 3: TEST CASES AND DRIVER ============================ */

// Run all six algorithms on one workload and print the comparison table.
void run_test_case(const char *title, const Process base[], int n, int quantum) {
    Process w[100];
    AlgoResult results[MAX_ALGOS];
    int cnt = 0;
    char rr_name[32];

    // Work on a copy, sorted by arrival time (stable) so FCFS/RR order is correct
    for (int i = 0; i < n; i++) w[i] = base[i];
    for (int i = 1; i < n; i++) {
        Process key = w[i];
        int j = i - 1;
        while (j >= 0 && w[j].arrival_time > key.arrival_time) { w[j + 1] = w[j]; j--; }
        w[j + 1] = key;
    }

    printf("\n\n##############################################################\n");
    printf("# %s\n", title);
    printf("##############################################################\n");
    printf("Round Robin time quantum = %d   (lower priority number = higher priority)\n", quantum);

    fcfs(w, n);                 show_results("FCFS", w, n, &results[cnt++]);
    sjf_non_preemptive(w, n);   show_results("SJF (Non-Preemptive)", w, n, &results[cnt++]);
    sjf_preemptive(w, n);       show_results("SJF (Preemptive / SRTF)", w, n, &results[cnt++]);
    priority_non_preemptive(w, n); show_results("Priority (Non-Preemptive)", w, n, &results[cnt++]);
    priority_preemptive(w, n);  show_results("Priority (Preemptive)", w, n, &results[cnt++]);
    round_robin(w, n, quantum);
    snprintf(rr_name, sizeof rr_name, "Round Robin (q=%d)", quantum);
    show_results(rr_name, w, n, &results[cnt++]);

    print_comparison_table(results, cnt);
}

int main(void) {
    // {pid, arrival, burst, priority, remaining, start, completion, wt, tat, rt}

    // Test 1: all processes arrive together
    Process tc1[] = {
        {1, 0, 8, 2, 0, 0, 0, 0, 0, 0},
        {2, 0, 4, 4, 0, 0, 0, 0, 0, 0},
        {3, 0, 9, 1, 0, 0, 0, 0, 0, 0},
        {4, 0, 5, 3, 0, 0, 0, 0, 0, 0},
    };

    // Test 2: staggered arrivals (preemption matters)
    Process tc2[] = {
        {1, 0, 7, 3, 0, 0, 0, 0, 0, 0},
        {2, 2, 4, 1, 0, 0, 0, 0, 0, 0},
        {3, 4, 1, 4, 0, 0, 0, 0, 0, 0},
        {4, 5, 4, 2, 0, 0, 0, 0, 0, 0},
    };

    // Test 3: CPU sits idle between two bursts of activity
    Process tc3[] = {
        {1,  0, 2, 3, 0, 0, 0, 0, 0, 0},
        {2,  1, 1, 1, 0, 0, 0, 0, 0, 0},
        {3, 10, 6, 2, 0, 0, 0, 0, 0, 0},
        {4, 11, 2, 4, 0, 0, 0, 0, 0, 0},
        {5, 12, 4, 1, 0, 0, 0, 0, 0, 0},
    };

    run_test_case("TEST CASE 1: Simultaneous arrival (all at t=0)",
                  tc1, sizeof tc1 / sizeof tc1[0], 3);
    run_test_case("TEST CASE 2: Staggered arrivals",
                  tc2, sizeof tc2 / sizeof tc2[0], 2);
    run_test_case("TEST CASE 3: Idle gap between workloads",
                  tc3, sizeof tc3 / sizeof tc3[0], 4);

    return 0;
}