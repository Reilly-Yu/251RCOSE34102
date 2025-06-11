#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define max_queue_size 100
#define max_io 2

typedef struct {
    int pid;
    int arrival_time;
    int cpu_burst;
    int priority;

    int io_request_times[max_io];
    int io_bursts[max_io];
    int io_count;
    int current_io_index;
    int in_io;
    int io_remaining_time;

    int remaining_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;
    int is_completed;

    int is_enqueued;
} process;

typedef struct 
{
    int front;
    int rear;
    process* items[max_queue_size];
}queue;

void init_queue(queue* q) {
    q->front = 0;
    q->rear = 0;
}

int is_empty(queue* q) {
    return q->front == q->rear;
}

int is_full(queue* q) {
    return q->rear == max_queue_size;
}

void enqueue(queue* q, process* p) {
    if (!is_full(q)) {
        q->items[q->rear++] = p;
    } else {
        printf("Queue is full\n");
    }
}

process* dequeue(queue* q) {
    if (!is_empty(q)) {
        return q->items[q->front++];
    }
    return NULL;
}

process* create_process(int n) {
    process* process_list = (process*)malloc(sizeof(process) * n);
    srand(time(NULL));

    for (int i = 0; i < n; i++) {
        process_list[i].pid = i + 1;
        process_list[i].arrival_time = rand() % 10;
        process_list[i].cpu_burst = rand() % 8 + 3;
        process_list[i].priority = rand() % 10 + 1;

        process_list[i].io_count = rand() % 3;
        for (int j = 0; j < process_list[i].io_count; j++) {
            process_list[i].io_request_times[j] = rand() % process_list[i].cpu_burst;
            process_list[i].io_bursts[j] = rand() % 5 + 1;
        }
        process_list[i].current_io_index = 0;
        process_list[i].in_io = 0;
        process_list[i].io_remaining_time = 0;

        process_list[i].remaining_time = process_list[i].cpu_burst;
        process_list[i].completion_time = 0;
        process_list[i].waiting_time = 0;
        process_list[i].turnaround_time = 0;
        process_list[i].is_completed = 0;
    }

    return process_list;
}

void print_processes(process* process_list, int n) {
    printf("PID\tAT\tCPU\tPri\tIO_Count\tIO_Times\tIO_Bursts\n");
    for (int i = 0; i < n; i++) {
        printf("%2d\t%2d\t%2d\t%2d\t%2d\t\t",
               process_list[i].pid,
               process_list[i].arrival_time,
               process_list[i].cpu_burst,
               process_list[i].priority,
               process_list[i].io_count);

        for (int j = 0; j < process_list[i].io_count; j++) {
            printf("%d ", process_list[i].io_request_times[j]);
        }

        printf("\t\t");

        for (int j = 0; j < process_list[i].io_count; j++) {
            printf("%d ", process_list[i].io_bursts[j]);
        }
        printf("\n");
    }
}


void config(process* process_list, int n , queue* ready_queue, queue* waiting_queue) {
    init_queue(ready_queue);
    init_queue(waiting_queue);

    for (int i = 0; i < n; i++) {
        enqueue(ready_queue, &process_list[i]);
    }  
}

void sort_ready_queue_by_arrival(queue* q) {
    for (int i = q->front; i < q->rear - 1; i++) {
        for (int j = i + 1; j < q->rear; j++) {
            if (q->items[i]->arrival_time > q->items[j]->arrival_time) {
                process* temp = q->items[i];
                q->items[i] = q->items[j];
                q->items[j] = temp;
            }
        }
    }
}


void FCFS(queue* ready_queue, int process_num) {
    int current_time = 0;
    int count = 0;
    int total_waiting = 0;
    int total_turnaround = 0;   

    sort_ready_queue_by_arrival(ready_queue);

    printf("Gantt Chart\n");

    while (count < process_num) {
        if (is_empty(ready_queue)) {
            printf("| IDLE ");
            current_time++;
            continue;
        }

        process* p = dequeue(ready_queue);

        if(p->arrival_time > current_time) {
            printf("| IDLE ");
            current_time++;
            enqueue(ready_queue, p);
            continue;
        }
        
        p->waiting_time = current_time - p->arrival_time;
        current_time = current_time + p->cpu_burst;
        p->completion_time = current_time;
        p->turnaround_time = p->completion_time - p->arrival_time;

        total_waiting += p->waiting_time;
        total_turnaround += p->turnaround_time;

        printf("| P%d",p->pid);
        count++;
    }

    printf("\nAverage Waiting Time: %.2f\n", (float)total_waiting / count);
    printf("Average Turnaround Time: %.2f\n", (float)total_turnaround / count);

}

void SJF_NONPREEM(queue* ready_queue, int process_num) {
    int current_time = 0;
    int count = 0;
    int total_waiting = 0;
    int total_turnaround = 0;

    printf("Gantt Chart\n");

    while (count < process_num) {
        if (is_empty(ready_queue)) {
            printf("| IDLE ");
            current_time++;
            continue;
        }

        int min_index = -1;
        int min_burst = __INT_MAX__;
        int size = ready_queue->rear - ready_queue->front;

        for (int i = ready_queue->front; i < ready_queue->rear; i++) {
            process* p = ready_queue->items[i];
            if (p->arrival_time <= current_time) {
                if (p->cpu_burst < min_burst || 
                    (p->cpu_burst == min_burst && p->arrival_time < ready_queue->items[min_index]->arrival_time)) {
                    min_burst = p->cpu_burst;
                    min_index = i;
                }
            }

        }

        if (min_index == -1) {
            printf("| IDLE ");
            current_time++;
            continue;
        }

        process* p = ready_queue->items[min_index];

        for (int i = min_index; i < ready_queue->rear - 1; i++) {
            ready_queue->items[i] = ready_queue->items[i + 1];
        }
        ready_queue->rear--;

        p->waiting_time = current_time - p->arrival_time;
        current_time += p->cpu_burst;
        p->completion_time = current_time;
        p->turnaround_time = p->completion_time - p->arrival_time;

        total_waiting += p->waiting_time;
        total_turnaround += p->turnaround_time;

        printf("| P%d", p->pid);
        count++;
    }

    printf("|\n");
    printf("\nAverage Waiting Time: %.2f\n", (float)total_waiting / process_num);
    printf("Average Turnaround Time: %.2f\n", (float)total_turnaround / process_num);
}

void SJF_PREEM(queue* ready_queue, int process_num) {
    int current_time = 0;
    int count = 0;
    int total_waiting = 0;
    int total_turnaround = 0;
    int prev_pid = -1;

    printf("Gantt Chart\n");

    while (count < process_num) {
        process* shortest = NULL;
        int min_remain = __INT_MAX__;

        for (int i = ready_queue->front; i < ready_queue->rear; i++) {
            process* p = ready_queue->items[i];
            if (p->arrival_time <= current_time && p->remaining_time > 0) {
                if (p->remaining_time < min_remain) {
                    min_remain = p->remaining_time;
                    shortest = p;
                }
            }
        }

        if (shortest == NULL) {
            if (prev_pid != -2) {
                printf("| IDLE ");
                prev_pid = -2;
            }
            current_time++;
            continue;
        }

        if (shortest->pid != prev_pid) {
            printf("| P%d ", shortest->pid);
            prev_pid = shortest->pid;
        }

        shortest->remaining_time--;
        current_time++;

        if (shortest->remaining_time == 0) {
            shortest->completion_time = current_time;
            shortest->turnaround_time = current_time - shortest->arrival_time;
            shortest->waiting_time = shortest->turnaround_time - shortest->cpu_burst;
            shortest->is_completed = 1;
            total_turnaround += shortest->turnaround_time;
            total_waiting += shortest->waiting_time;
            count++;
        }
    }

    printf("|\n\n");
    printf("Average Waiting Time: %.2f\n", (float)total_waiting / process_num);
    printf("Average Turnaround Time: %.2f\n", (float)total_turnaround / process_num);
}

void Priority_NONPREEM(queue* ready_queue, int process_num) {
    int current_time = 0;
    int count = 0;
    int total_waiting = 0;
    int total_turnaround = 0;

    printf("Gantt Chart\n");

    while (count < process_num) {
        if (is_empty(ready_queue)) {
            printf("| IDLE ");
            current_time++;
            continue;
        }

        int min_index = -1;
        int min_priority = __INT_MAX__;

        for (int i = ready_queue->front; i < ready_queue->rear; i++) {
            process* p = ready_queue->items[i];
            if (p->arrival_time <= current_time) {
                if (p->priority < min_priority ||
                    (p->priority == min_priority && min_index != -1 && p->arrival_time < ready_queue->items[min_index]->arrival_time)) {
                    min_priority = p->priority;
                    min_index = i;
                }
            }
        }

        if (min_index == -1) {
            printf("| IDLE ");
            current_time++;
            continue;
        }

        process* p = ready_queue->items[min_index];

        for (int i = min_index; i < ready_queue->rear - 1; i++) {
            ready_queue->items[i] = ready_queue->items[i + 1];
        }
        ready_queue->rear--;

        p->waiting_time = current_time - p->arrival_time;
        current_time += p->cpu_burst;
        p->completion_time = current_time;
        p->turnaround_time = p->completion_time - p->arrival_time;

        total_waiting += p->waiting_time;
        total_turnaround += p->turnaround_time;

        printf("| P%d", p->pid);
        count++;
    }

    printf("|\n\n");
    printf("Average Waiting Time: %.2f\n", (float)total_waiting / process_num);
    printf("Average Turnaround Time: %.2f\n", (float)total_turnaround / process_num);
}

void Priority_PREEM(queue* ready_queue, int process_num) {
    int current_time = 0;
    int count = 0;
    int total_waiting = 0;
    int total_turnaround = 0;
    int prev_pid = -1;

    printf("Gantt Chart\n");

    while (count < process_num) {
        process* highest = NULL;
        int min_priority = __INT_MAX__;

        for (int i = ready_queue->front; i < ready_queue->rear; i++) {
            process* p = ready_queue->items[i];

            if (p->arrival_time <= current_time && p->remaining_time > 0) {
                if (p->priority < min_priority) {
                    min_priority = p->priority;
                    highest = p;
                }
            }
        }

        if (highest == NULL) {
            if (prev_pid != -2) {
                printf("| IDLE ");
                prev_pid = -2;
            }
            current_time++;
            continue;
        }

        if (highest->pid != prev_pid) {
            printf("| P%d ", highest->pid);
            prev_pid = highest->pid;
        }

        highest->remaining_time--;
        current_time++;

        if (highest->remaining_time == 0) {
            highest->completion_time = current_time;
            highest->turnaround_time = current_time - highest->arrival_time;
            highest->waiting_time = highest->turnaround_time - highest->cpu_burst;
            highest->is_completed = 1;
            total_turnaround += highest->turnaround_time;
            total_waiting += highest->waiting_time;
            count++;
        }
    }

    printf("|\n\n");
    printf("Average Waiting Time: %.2f\n", (float)total_waiting / process_num);
    printf("Average Turnaround Time: %.2f\n", (float)total_turnaround / process_num);
}

void RoundRobin(queue* ready_queue, process* process_list, int process_num, int time_quantum) {
    int current_time = 0;
    int count = 0;
    int total_waiting = 0;
    int total_turnaround = 0;
    int prev_pid = -1;
    int quantum_tick = 0;

    printf("Gantt Chart\n");

    queue rr_queue;
    init_queue(&rr_queue);

    while (count < process_num) {
        for (int i = 0; i < process_num; i++) {
            if (process_list[i].arrival_time == current_time && process_list[i].is_enqueued == 0) {
                enqueue(&rr_queue, &process_list[i]);
                process_list[i].is_enqueued = 1;
            }
        }

        if (is_empty(&rr_queue)) {
            printf("| IDLE ");
            current_time++;
            continue;
        }

        process* p = dequeue(&rr_queue);

        int exec_time = (p->remaining_time < time_quantum) ? p->remaining_time : time_quantum;
        quantum_tick = 0;

        for (int t = 0; t < exec_time; t++) {
            if (p->pid != prev_pid || quantum_tick == 0) {
                printf("| P%d ", p->pid);
                prev_pid = p->pid;
            }

            current_time++;
            quantum_tick++;

            for (int i = ready_queue->front; i < ready_queue->rear; i++) {
                process* q = ready_queue->items[i];
                if (q->arrival_time <= current_time && q->is_enqueued == 0) {
                    enqueue(&rr_queue, q);
                    q->is_enqueued = 1;
                }
            }
        }

        p->remaining_time -= exec_time;

        if (p->remaining_time == 0) {
            p->completion_time = current_time;
            p->turnaround_time = current_time - p->arrival_time;
            p->waiting_time = p->turnaround_time - p->cpu_burst;
            p->is_completed = 1;
            total_turnaround += p->turnaround_time;
            total_waiting += p->waiting_time;
            count++;
        } else {
            enqueue(&rr_queue, p);
        }
    }

    printf("|\n\n");
    printf("Average Waiting Time: %.2f\n", (float)total_waiting / process_num);
    printf("Average Turnaround Time: %.2f\n", (float)total_turnaround / process_num);
}


void evaluate(const char* name, process* plist, int n, void (*scheduler)(queue*, int)) {
    process temp_list[n];
    for (int i = 0; i < n; i++) {
        temp_list[i] = plist[i];
        temp_list[i].remaining_time = temp_list[i].cpu_burst;
        temp_list[i].completion_time = 0;
        temp_list[i].waiting_time = 0;
        temp_list[i].turnaround_time = 0;
        temp_list[i].is_completed = 0;
    }

    queue q;
    init_queue(&q);
    for (int i = 0; i < n; i++) {
        enqueue(&q, &temp_list[i]);
    }

    scheduler(&q, n);

    float total_wt = 0, total_tt = 0;
    for (int i = 0; i < n; i++) {
        total_wt += temp_list[i].waiting_time;
        total_tt += temp_list[i].turnaround_time;
    }

    printf("[%s] Avg Waiting Time: %.2f | Avg Turnaround Time: %.2f\n",
           name, total_wt / n, total_tt / n);
}

void evaluate_rr(const char* name, process* plist, int n, int time_quantum) {
    process temp_list[n];
    for (int i = 0; i < n; i++) {
        temp_list[i] = plist[i];
        temp_list[i].remaining_time = temp_list[i].cpu_burst;
        temp_list[i].completion_time = 0;
        temp_list[i].waiting_time = 0;
        temp_list[i].turnaround_time = 0;
        temp_list[i].is_completed = 0;
        temp_list[i].is_enqueued = 0;
    }

    queue q;
    init_queue(&q);
    for (int i = 0; i < n; i++) {
        enqueue(&q, &temp_list[i]);
    }

    RoundRobin(&q, temp_list, n, time_quantum); 

    float total_wt = 0, total_tt = 0;
    for (int i = 0; i < n; i++) {
        total_wt += temp_list[i].waiting_time;
        total_tt += temp_list[i].turnaround_time;
    }

    printf("[%s] Avg Waiting Time: %.2f | Avg Turnaround Time: %.2f\n",
           name, total_wt / n, total_tt / n);
}


int main() {
    int n;
    
    printf("ENTER NUMBER OF PROCESS: ");
    scanf("%d", &n);

    process* process_list = create_process(n);

    printf("\n[📋] 생성된 프로세스 목록:\n");
    print_processes(process_list, n);

    process backup_list[n];  

    for (int i = 0; i < n; i++) {
        backup_list[i] = process_list[i];
    }

    queue ready_queue;
    queue waiting_queue;
    printf("FCFS\n");
    config(backup_list, n, &ready_queue, &waiting_queue);
    FCFS(&ready_queue, n);
    printf("\n\nSJF_NONPREEM\n");
    for (int i = 0; i < n; i++) {
        backup_list[i] = process_list[i];
    }
    config(backup_list, n, &ready_queue, &waiting_queue);  
    SJF_NONPREEM(&ready_queue, n);
    printf("\n\nSJF_PREEM\n");

    for (int i = 0; i < n; i++) {
        backup_list[i] = process_list[i];
    }
    config(backup_list, n, &ready_queue, &waiting_queue);
    SJF_PREEM(&ready_queue, n);
    printf("\n\nPRIORITY_NONPREEM\n");
    for (int i = 0; i < n; i++) {
        backup_list[i] = process_list[i];
    }
    config(backup_list, n, &ready_queue, &waiting_queue);
    Priority_NONPREEM(&ready_queue, n);
    printf("\n\nPRIORITY_PREEM\n");
    for (int i = 0; i < n; i++) {
        backup_list[i] = process_list[i];
    }
    config(backup_list, n, &ready_queue, &waiting_queue);
    Priority_PREEM(&ready_queue, n);
    printf("\n\nRR\n");
    for (int i = 0; i < n; i++) {
        backup_list[i] = process_list[i];
    }
    config(backup_list, n, &ready_queue, &waiting_queue);
    RoundRobin(&ready_queue, backup_list, n, 2);
    printf("\n\n");

    evaluate("FCFS", process_list, n, FCFS);
    evaluate("SJF_NONPREEM", process_list, n, SJF_NONPREEM);
    evaluate("SJF_PREEM", process_list, n, SJF_PREEM);
    evaluate("Priority_NONPREEM", process_list, n, Priority_NONPREEM);
    evaluate("Priority_PREEM", process_list, n, Priority_PREEM);
    evaluate_rr("RR", process_list, n, 3);

    return 0;
}