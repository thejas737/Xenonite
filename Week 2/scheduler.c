#include "scheduler.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>

const char *algorithm_name(enum Algorithm a) {
    switch (a) {
        case FCFS: return "FCFS";
        case SJF_NP: return "SJF (Non-preemptive)";
        case SJF_P: return "SJF (Preemptive / SRTF)";
        case PRIORITY_NP: return "Priority (Non-preemptive)";
        case PRIORITY_P: return "Priority (Preemptive)";
        case ROUND_ROBIN: return "Round Robin";
        default: return "Unknown";
    }
}

static void add_segment(ScheduleResult *r, int pid, int t) {
    if (r->count > 0 && r->segments[r->count-1].pid == pid &&
        r->segments[r->count-1].end == t) {
        r->segments[r->count-1].end++;
        return;
    }
    if (r->count < MAX_SEGMENTS) {
        r->segments[r->count].pid = pid;
        r->segments[r->count].start = t;
        r->segments[r->count].end = t + 1;
        r->count++;
    }
}

/* Lower numeric priority value means higher priority. Ties use input order. */
static int choose_ready(const Process p[], int n, const int done[],
                        const int remaining[], int t, enum Algorithm alg) {
    int best = -1;
    for (int i = 0; i < n; ++i) {
        if (done[i] || remaining[i] <= 0 || p[i].arrival > t) continue;
        if (best == -1) { best = i; continue; }
        int better = 0;
        if (alg == FCFS) {
            better = p[i].arrival < p[best].arrival;
        } else if (alg == SJF_NP || alg == SJF_P) {
            better = remaining[i] < remaining[best] ||
                (remaining[i] == remaining[best] && p[i].arrival < p[best].arrival);
        } else if (alg == PRIORITY_NP || alg == PRIORITY_P) {
            better = p[i].priority < p[best].priority ||
                (p[i].priority == p[best].priority && p[i].arrival < p[best].arrival);
        }
        if (better) best = i;
    }
    return best;
}

int run_schedule(const Process p[], int n, enum Algorithm alg, int quantum,
                 ScheduleResult *out) {
    if (!p || !out || n < 1 || n > MAX_PROCESSES) return 0;
    if (alg == ROUND_ROBIN && quantum < 1) return 0;

    memset(out, 0, sizeof(*out));
    snprintf(out->name, sizeof(out->name), "%s", algorithm_name(alg));

    int remaining[MAX_PROCESSES], done[MAX_PROCESSES] = {0};
    int first_start[MAX_PROCESSES], completed = 0;
    for (int i = 0; i < n; ++i) {
        remaining[i] = p[i].burst;
        first_start[i] = -1;
    }

    /* RR uses a bounded circular FIFO ready queue. */
    int q[MAX_PROCESSES + 1], qhead = 0, qtail = 0, qsize = 0;
    int arrived[MAX_PROCESSES] = {0};
    int current = -1, slice = 0, t = 0;
    int nonpreemptive = (alg == FCFS || alg == SJF_NP || alg == PRIORITY_NP);

    while (completed < n && t < 10000000) {
        if (alg == ROUND_ROBIN) {
            /* New arrivals join the tail before an expired time slice is requeued. */
            for (int i = 0; i < n; ++i) {
                if (!arrived[i] && p[i].arrival <= t) {
                    q[qtail] = i;
                    qtail = (qtail + 1) % (MAX_PROCESSES + 1);
                    qsize++;
                    arrived[i] = 1;
                }
            }
            if (current == -1 && qsize > 0) {
                current = q[qhead];
                qhead = (qhead + 1) % (MAX_PROCESSES + 1);
                qsize--;
                slice = 0;
            }
            if (current == -1) {
                add_segment(out, -1, t++);
                continue;
            }
        } else {
            if (!(nonpreemptive && current != -1 && !done[current]))
                current = choose_ready(p, n, done, remaining, t, alg);
            if (current == -1) {
                add_segment(out, -1, t++);
                continue;
            }
        }

        if (first_start[current] == -1) first_start[current] = t;
        add_segment(out, current, t);
        remaining[current]--;
        t++;

        if (remaining[current] == 0) {
            done[current] = 1;
            out->completion[current] = t;
            completed++;
            current = -1;
            slice = 0;
        } else if (alg == ROUND_ROBIN) {
            slice++;
            if (slice == quantum) {
                q[qtail] = current;
                qtail = (qtail + 1) % (MAX_PROCESSES + 1);
                qsize++;
                current = -1;
                slice = 0;
            }
        }
    }
    if (completed != n) return 0;

    out->total_time = t;
    int total_burst = 0;
    double sum_w = 0, sum_tat = 0, sum_r = 0;
    for (int i = 0; i < n; ++i) {
        out->turnaround[i] = out->completion[i] - p[i].arrival;
        out->waiting[i] = out->turnaround[i] - p[i].burst;
        out->response[i] = first_start[i] - p[i].arrival;
        total_burst += p[i].burst;
        sum_w += out->waiting[i];
        sum_tat += out->turnaround[i];
        sum_r += out->response[i];
    }
    out->avg_waiting = sum_w / n;
    out->avg_turnaround = sum_tat / n;
    out->avg_response = sum_r / n;
    out->idle_time = t - total_burst;
    out->cpu_utilization = t ? (100.0 * total_burst / t) : 0.0;
    return 1;
}
