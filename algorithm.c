#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

// Standard structure for all algorithms
typedef struct {
    int pid;           // Process ID
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
}

// Helper function to calculate final metrics and averages
void calculate_metrics(Process p[], int n) {
    float total_wt = 0, total_tat = 0;
    for (int i = 0; i < n; i++) {
        p[i].turnaround_time = p[i].completion_time - p[i].arrival_time;
        p[i].waiting_time = p[i].turnaround_time - p[i].burst_time;
        // Response time is calculated dynamically during execution
        
        total_wt += p[i].waiting_time;
        total_tat += p[i].turnaround_time;
    }
    printf("\nAverage Waiting Time: %.2f\n", total_wt / n);
    printf("Average Turnaround Time: %.2f\n", total_tat / n);
}

// 1. First-Come-First-Served (FCFS)
void fcfs(Process p[], int n) {
    reset_processes(p, n);
    int current_time = 0;

    for (int i = 0; i < n; i++) {
        // Handle idle CPU time if process hasn't arrived yet
        if (current_time < p[i].arrival_time) {
            current_time = p[i].arrival_time;
        }
        
        p[i].start_time = current_time;
        p[i].response_time = p[i].start_time - p[i].arrival_time;
        
        current_time += p[i].burst_time;
        p[i].completion_time = current_time;
    }
    calculate_metrics(p, n);
}

// 2. Shortest Job First - Non-Preemptive[cite: 3]
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
            current_time += p[idx].burst_time;
            p[idx].completion_time = current_time;
            is_completed[idx] = true;
            completed++;
        } else {
            current_time++; // CPU is idle
        }
    }
    calculate_metrics(p, n);
}

// 3. Shortest Job First - Preemptive (Shortest Remaining Time First)[cite: 3]
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
            p[idx].remaining_time--;
            current_time++;

            if (p[idx].remaining_time == 0) {
                p[idx].completion_time = current_time;
                completed++;
            }
        } else {
            current_time++;
        }
    }
    calculate_metrics(p, n);
}

// 4. Priority Scheduling - Non-Preemptive[cite: 3]
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
            current_time += p[idx].burst_time;
            p[idx].completion_time = current_time;
            is_completed[idx] = true;
            completed++;
        } else {
            current_time++;
        }
    }
    calculate_metrics(p, n);
}

// 5. Priority Scheduling - Preemptive[cite: 3]
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
            p[idx].remaining_time--;
            current_time++;

            if (p[idx].remaining_time == 0) {
                p[idx].completion_time = current_time;
                completed++;
            }
        } else {
            current_time++;
        }
    }
    calculate_metrics(p, n);
}

// 6. Round Robin (RR) with configurable time quantum[cite: 3]
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