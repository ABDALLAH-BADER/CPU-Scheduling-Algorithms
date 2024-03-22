/**************************************************************/
//                       Base Building                        //
/**************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the process structure
typedef struct
{
    char pid;
    int burst_time;
    int io_time;
    int io_duration;
    int priority;
    int arrive_time;
    int no_of_executions;
    int is_waiting;
    int start_waiting_time;
    int pass;
    int stride;
    int start;
    int end;
    int t;
} Process;

typedef struct
{
    char pid;
    int start_time;
    int end_time;

} ProcessGantt;

// Define the scheduling queue structure
typedef struct
{
    Process *queue; // Array to hold the processes
    int front;
    int rear;
    int size;
} Queue;

// Function to create a process
Process createProcess(char pid, int burst_time, int io_time, int io_duration, int priority, int arrive_time)
{
    Process process;
    process.pid = pid;
    process.burst_time = burst_time;
    process.io_time = io_time;
    process.io_duration = io_duration;
    process.priority = priority;
    process.arrive_time = arrive_time;
    process.no_of_executions = 0;
    process.is_waiting = 0;
    process.start_waiting_time = -1;
    process.start = __INT_MAX__;
    process.end = 0;
    return process;
}

// Function to initialize a scheduling queue
Queue *createQueue(int size)
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->queue = (Process *)malloc(size * sizeof(Process));
    queue->front = -1;
    queue->rear = -1;
    queue->size = size;
    return queue;
}

// Function to check if a queue is empty
int isEmpty(Queue *queue)
{
    return queue->front == -1;
}

// Function to check if a queue is full
int isFull(Queue *queue)
{
    return (queue->rear + 1) % queue->size == queue->front;
}

// Function to enqueue a process
void enqueue(Queue *queue, Process process)
{
    if (isFull(queue))
    {
        printf("Queue is full. Cannot enqueue process.\n");
        return;
    }
    if (isEmpty(queue))
    {
        queue->front = 0;
    }
    queue->rear = (queue->rear + 1) % queue->size;
    queue->queue[queue->rear] = process;
}

// Function to enqueue a process in the front
void enqueue_front(Queue *queue, Process process)
{
    if (isFull(queue))
    {
        printf("Queue is full. Cannot enqueue process.\n");
        return;
    }
    if (isEmpty(queue))
    {
        enqueue(queue, process);
        return;
    }

    if (queue->front == 0)
        queue->front = queue->size - 1;
    else
        queue->front = queue->front - 1;
    queue->queue[queue->front] = process;
}

// Function to dequeue a process
Process dequeue(Queue *queue)
{
    if (isEmpty(queue))
    {
        printf("Queue is empty. Cannot dequeue process.\n");
        Process emptyProcess;
        emptyProcess.pid = -1;
        return emptyProcess;
    }
    Process process = queue->queue[queue->front];
    if (queue->front == queue->rear)
    {
        queue->front = -1;
        queue->rear = -1;
    }
    else
    {
        queue->front = (queue->front + 1) % queue->size;
    }
    return process;
}

void sortQeue(Queue *queue)
{
    if (isEmpty(queue))
    {
        printf("Queue is empty. Cannot sort.\n");
        return;
    }

    Process *arr = (Process *)malloc(queue->size * sizeof(Process));
    int siz = 0;
    for (int i = 0;; i++)
    {
        arr[i] = queue->queue[queue->front];
        siz++;
        if (queue->front == queue->rear)
        {
            queue->front = 0;
            queue->rear = siz - 1;
            break;
        }
        queue->front = (queue->front + 1) % queue->size;
    }

    for (int i = 0; i < siz; i++)
    {
        for (int j = 0; j < siz - i - 1; j++)
        {
            if (arr[j].arrive_time > arr[j + 1].arrive_time)
            {
                Process temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }

    queue->queue = arr;
}

Queue *inputQueueFromFile(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }

    int size;
    fscanf(file, "number of processes: %d\n", &size);
    Queue *queue = createQueue(size);

    char pid;
    int burst_time, io_time, priority, arrive_time, io_duration;
    for (int i = 0; i < size; ++i)
    {

        fscanf(file, "process: %c, start: %d, take: %d, I/O burst: %d, I/O duration: %d, Priority: %d\n", &pid, &arrive_time, &burst_time, &io_time, &io_duration, &priority);
        Process process = createProcess(pid, burst_time, io_time, io_duration, priority, arrive_time);
        enqueue(queue, process);
    }

    fclose(file);

    return queue;
}

int processCount(Queue *queue)
{
    if (isEmpty(queue))
        return 0;
    int start = queue->front, end = queue->rear, count = 0;
    if (start == end && start != -1)
        return 1;
    while (start != end)
    {
        count++;
        start = (start + 1) % queue->size;
    }
    return count + 1;
}

void get_ready(Queue *ready, Queue *queue, int time)
{
    while (queue->front != queue->rear)
    {
        if (queue->queue[queue->front].arrive_time == time)
            enqueue(ready, dequeue(queue));
        else
            return;
    }
    if (queue->front == queue->rear && queue->front != -1)
    {
        if (queue->queue[queue->front].arrive_time == time)
            enqueue(ready, dequeue(queue));
        return;
    }
    // if (queue->queue[start].arrive_time == time)
    //     enqueue(ready, dequeue(queue));
}

void run_another(Queue *queue, int time, int *total_turnaround, int *last_execution, int core, char arr[][1000])
{
    if (isEmpty(queue))
    {
        printf("This core is empty from time %d to %d\n", time, time + 1);
        return;
    }
    Process process = dequeue(queue);
    if (process.start_waiting_time != -1 || last_execution[process.pid - 'A'] == time)
    {
        if (time + 1 - process.start_waiting_time >= process.io_duration)
        {
            process.start_waiting_time = -1;
        }
        run_another(queue, time, total_turnaround, last_execution, core, arr);
        enqueue_front(queue, process);
    }
    else
    {
        if (process.no_of_executions == 0)
            printf("Process %c has respose time %d \n", process.pid, time - process.arrive_time);
        printf("Process %c is executing in core %d from time %d to %d\n", process.pid, core + 1, time, time + 1);
        arr[core][time] = process.pid;
        last_execution[process.pid - 'A'] = time;
        process.no_of_executions++;
        process.burst_time--;
        if ((process.io_time != 0) && (process.no_of_executions % process.io_time == 0))
        {
            process.start_waiting_time = time + 1;
        }

        if (process.burst_time <= 0 && process.start_waiting_time == -1)
        {
            printf("Process %c is finished at time %d\n", process.pid, time + 1);
            *total_turnaround += (time + 1 - process.arrive_time);
        }
        else
            enqueue(queue, process);
    }
}

void move_processes(Queue *destination, Queue *source)
{
    while (!isEmpty(source))
    {
        Process process = dequeue(source);
        enqueue(destination, process);
    }
}

void zero(int *arr, int siz)
{
    for (int i = 0; i < siz; i++)
        arr[i] = 0;
}

int allLevelsEmpty(Queue *queues[], int no_of_levels)
{
    for (int i = 0; i < no_of_levels; i++)
    {
        if (!isEmpty(queues[i]))
            return 0;
    }
    return 1;
}

void boost_queue(Queue *queues[], int no_of_levels)
{
    for (int i = 1; i < no_of_levels; i++)
    {
        while (!isEmpty(queues[i]))
        {
            Process process = dequeue(queues[i]);
            enqueue(queues[0], process);
        }
    }
}

void displayProcessesGantt(int n, int time, char arr[n][1000])
{

    for (int i = 0; i < n; i++)
    {
        printf("\nThe Processes Gantt Chart of core %d .\n", i + 1);
        for (int j = 0; j < time; j++)
            printf("| %c ", arr[i][j]);
        printf("|\n");
    }
}

int min(int num1, int num2)
{
    return (num1 < num2) ? num1 : num2;
}

int max(int num1, int num2)
{
    return (num1 > num2) ? num1 : num2;
}

int leastremainingtime(Process *processes, int numProcesses, char *state, int current_time, int current_cpu, int *cpu_used)
{
    int mintime = __INT_MAX__;
    int minindex = -1;

    for (int i = 0; i < numProcesses; ++i)
    {
        if (cpu_used[i] != -1)
        {
            if (processes[i].burst_time > 0 && processes[i].burst_time < mintime && state[i] == 'r' && processes[i].arrive_time <= current_time && current_cpu == cpu_used[i])
            {
                mintime = processes[i].burst_time;
                minindex = i;
            }
            else if(processes[i].burst_time > 0 && processes[i].burst_time < mintime && state[i] == 'r' && processes[i].arrive_time <= current_time && cpu_used[i]<current_cpu){
                mintime = processes[i].burst_time;
                minindex = i;
            }
        }
        else
        {
            if (processes[i].burst_time > 0 && processes[i].burst_time < mintime && state[i] == 'r' && processes[i].arrive_time <= current_time)
            {

                mintime = processes[i].burst_time;
                minindex = i;
            }
        }
    }
    return minindex;
}


void decrementtime(Process *processes, int numProcesses, char *state, int *remaining, int *completed, int current_time)
{
    for (int i = 0; i < numProcesses; ++i)
    {
        if (remaining[i] > 0)
        {
            remaining[i]--;
            completed[i] = 0;
            if (remaining[i] == 0)
            {
                processes[i].arrive_time = current_time;
            }
        }
        else
        {
            if (processes[i].burst_time > 0 && processes[i].arrive_time <= current_time)
            {
                state[i] = 'r';
            }
        }
    }
}

int cores_reading(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening file: %s\n", filename);
        return 0;
    }
    int cores;
    fscanf(file, "cores: %d\n", &cores);
    fclose(file);
    return cores;
}

int leastarrive(Process *processes, int numProcesses, char *state, int current_time, int current_cpu, int *cpu_used)
{
    int mintime = __INT_MAX__;
    int minindex = -1;

    for (int i = 0; i < numProcesses; ++i)
    {
        if (cpu_used[i] != -1)
        {
            if (processes[i].burst_time > 0 && processes[i].arrive_time < mintime && state[i] == 'r' && processes[i].arrive_time <= current_time && current_cpu == cpu_used[i])
            {
                mintime = processes[i].arrive_time;
                minindex = i;
            }
            else if(processes[i].burst_time > 0 && processes[i].arrive_time < mintime && state[i] == 'r' && processes[i].arrive_time <= current_time && cpu_used[i]<current_cpu){
                mintime = processes[i].arrive_time;
                minindex = i;
            }
        }
        else
        {
            if (processes[i].burst_time > 0 && processes[i].arrive_time < mintime && state[i] == 'r' && processes[i].arrive_time <= current_time)
            {
                mintime = processes[i].arrive_time;
                minindex = i;
            }
        }
    }

    return minindex;
}

int shortest_job(Process *processes, int *current_cpu, int numProcesses, char *state, int current_time)
{
    int mintime = __INT_MAX__;
    int minindex = -1;

    for (int i = 0; i < numProcesses; ++i)
    {
        if (processes[i].burst_time > 0 && processes[i].burst_time < mintime && current_cpu[i] == -1 && state[i] == 'r' && processes[i].arrive_time <= current_time)
        {
            minindex = i;
            mintime = processes[i].burst_time;
        }
    }

    return minindex;
}

// Function to simulate process execution with priority scheduling
int highest_priority_job(Process *processes, int *current_cpu, int size, char *status, int current_time, int core)
{
    int highest_priority = -1;
    int highest_priority_index = -1;

    for (int i = 0; i < size; i++)
    {
        if (status[i] == 'r')
        {
            if (processes[i].arrive_time <= current_time)
            {
                if (highest_priority == -1 || processes[i].priority > highest_priority)
                {
                    if (current_cpu[i] == -1 || current_cpu[i] == core)
                    {
                        highest_priority = processes[i].priority;
                        highest_priority_index = i;
                    }
                }
            }
        }
    }

    return highest_priority_index;
}

int shortest_pass(Process *processes, int *current_cpu, int numProcesses, char *state, int current_time)
{
    int min_pass = __INT_MAX__;
    int minindex = -1;

    for (int i = 0; i < numProcesses; ++i)
    {
        if (processes[i].burst_time > 0 && processes[i].pass < min_pass && current_cpu[i] == -1 && state[i] == 'r' && processes[i].arrive_time <= current_time)
        {
            minindex = i;
            min_pass = processes[i].pass;
        }
    }

    return minindex;
}


/**************************************************************/
//                 Possible scheduling policies               //
/**************************************************************/


// Function to simulate process execution with stride scheduling
void _stride(Queue *queue, int cores)
{

    char arr[cores][1000];
    Process *processes = (Process *)malloc(queue->size * sizeof(Process));
    processes = queue->queue;
    char *status = (char *)malloc(queue->size * sizeof(char));
    int current_time = 0;
    int *current_cpu = (int *)malloc(queue->size * sizeof(int));
    int *current_core = (int *)malloc(cores * sizeof(int));
    int *arrive = (int *)malloc(cores * sizeof(int));
    for (int i = 0; i < cores; i++)
    {
        for (int j = 0; j < 1000; j++)
            arr[i][j] = 'i';
    }
    for (int i = 0; i < queue->size; i++)
    {
        processes[i].stride = (i + 1) * 10;
        processes[i].arrive_time = 0;
        processes[i].pass = 0;
        current_cpu[i] = -1;
        status[i] = 'r';
        arrive[i] = processes[i].arrive_time;
        // printf(" %d   %d   %d   %d   %d\n",processes[i].burst_time,processes[i].stride,processes[i].arrive_time,processes[i].io_time,processes[i].io_duration);
    }
    for (int i = 0; i < cores; i++)
        current_core[i] = -1;

    int *completed = (int *)calloc(queue->size, sizeof(int));
    int *remaining_io = (int *)calloc(queue->size, sizeof(int));

    int completed_processes = 0;

    while (completed_processes < queue->size)
    {
        decrementtime(processes, queue->size, status, remaining_io, completed, current_time);
        for (int i = 0; i < cores; i++)
        {
            int j = shortest_pass(processes, current_cpu, queue->size, status, current_time);
            if (current_core[i] != -1)
                j = current_core[i];
            if (j != -1)
            {
                processes[j].pass += processes[j].stride;
                current_core[i] = j;
                current_cpu[j] = i;
                processes[j].burst_time--;
                processes[j].start = min(processes[j].start, current_time);
                completed[j]++;
                status[j] = 'e';
                arr[i][current_time] = processes[j].pid;
                for (int k = 0; k < 4; k++)
                {
                    printf(" %d  ", processes[k].pass);
                }
                puts("");

                printf("Time %d~%d: Process %c executes on core %d", current_time, current_time + 1, processes[j].pid, i + 1);
                if (processes[j].burst_time == 0)
                {
                    status[j] = 'c';
                    completed_processes++;
                    completed[j] = current_time + 1;
                    processes[j].end = current_time + 1;
                    printf("\tTime %d: Process %c completed", completed[j], processes[j].pid);
                    current_core[i] = -1;
                    current_cpu[j] = -1;
                }
                if (completed[j] > 0 && processes[j].burst_time > 0 && processes[j].io_time > 0 && (completed[j]) % (processes[j].io_time) == 0)
                {
                    status[j] = 'i';
                    printf("\tTime %d: Process %c made I/O request", current_time + 1, processes[j].pid);
                    remaining_io[j] += processes[j].io_duration;
                    current_core[i] = -1;
                    current_cpu[j] = -1;
                }
                printf("\n");
            }
            else
            {
                printf("Time %d~%d: Process - executes on core %d\n", current_time, current_time + 1, i + 1);
            }
        }
        current_time++;

        printf("\n");
    }
    float average = 0;
    for (int k = 0; k < queue->size; k++)
    {
        average += (processes[k].end - arrive[k]);
        printf("\nTurnaround time for process %c: %d\n", processes[k].pid, (processes[k].end - arrive[k]));
        printf("Response time for process %c: %d\n", processes[k].pid, (processes[k].start - arrive[k]));
    }
    printf("\nAverage Turnaround time: %.2f\n\n", (average / queue->size));
    displayProcessesGantt(cores, current_time, arr);
    //  for (int i = 0; i <cores; i++)
    //     {
    //         printf("\nThe Processes Gantt Chart of core %d .\n", i + 1);
    //         for(int j=0;j<current_time;j++) printf("| %c ",arr[i][j]);
    //         printf("|\n");
    //     }
    free(status);
    free(completed);
    free(remaining_io);
    free(current_core);
    free(current_cpu);
}

// Function to simulate process execution with FCFS scheduling
void fcfs(Queue *queue)
{

    FILE *sysconf = fopen("system.txt", "r");
    if (sysconf == NULL)
    {
        printf("Error opening system file\n");
        exit(1);
    }
    int cores;
    if (fscanf(sysconf, "cores: %d", &cores) != 1)
    {
        printf("Error reading number of cores\n");
        exit(1);
    }
    fclose(sysconf);
    Process *processes = (Process *)malloc(queue->size * sizeof(Process));
    processes = queue->queue;
    char id;
    int startTime, executionTime, ioTime, duration, priority;

    char *status = (char *)malloc(queue->size * sizeof(char));
    int *start = (int *)calloc(queue->size, sizeof(int));
    int current_time = 0;
    int *current_cpu = (int *)malloc(queue->size * sizeof(int));
    char arr[cores][1000];
    for (int i = 0; i < cores; i++)
    {
        for (int j = 0; j < 1000; j++)
            arr[i][j] = 'i';
    }
    for (int i = 0; i < queue->size; i++)
    {
        start[i] = -1;
        current_cpu[i] = -1;
    }
    int *arrive = (int *)malloc(queue->size * sizeof(char));
    for (int i = 0; i < queue->size; i++)
    {
        arrive[i] = processes[i].arrive_time;
    }
    int *completed = (int *)calloc(queue->size, sizeof(int));
    int *remaining_io = (int *)calloc(queue->size, sizeof(int));

    int completed_processes = 0;

    while (completed_processes < queue->size)
    {

        decrementtime(processes, queue->size, status, remaining_io, completed, current_time);

        for (int i = 0; i < cores; i++)
        {
            int j = leastarrive(processes, queue->size, status, current_time, i, current_cpu);

            if (j != -1)
            {
                current_cpu[j] = i;
                processes[j].burst_time--;
                completed[j]++;
                status[j] = 'e';
                arr[i][current_time] = processes[j].pid;
                if (start[j] == -1)
                {
                    start[j] = current_time - arrive[j];
                }
                printf("Time %d~%d: Process %c executes on core %d", current_time, current_time + 1, processes[j].pid, i + 1);
                if (processes[j].burst_time == 0)
                {
                    status[j] = 'c';
                    completed_processes++;
                    completed[j] = current_time + 1;
                    printf("\tTime %d: Process %c completed", completed[j], processes[j].pid);
                }
                if (completed[j] > 0 && processes[j].burst_time > 0 && processes[j].io_time > 0 && (completed[j]) % (processes[j].io_time) == 0)
                {
                    status[j] = 'i';
                    printf("\tTime %d: Process %c made I/O request", current_time + 1, processes[j].pid);
                    remaining_io[j] += processes[j].io_duration;
                }
                printf("\n");
            }
            else
            {
                printf("Time %d~%d: Process - executes on core %d\n", current_time, current_time + 1, i + 1);
            }
        }
        current_time++;

        printf("\n");
    }
    float average = 0;
    for (int k = 0; k < queue->size; k++)
    {
        average += (completed[k] - arrive[k]);
        printf("\nTurnaround time for process %c: %d\n", processes[k].pid, (completed[k] - arrive[k]));
        printf("Response time for process %c: %d\n", processes[k].pid, (start[k]));
    }
    printf("\nAverage Turnaround time: %.2f\n\n", (average / queue->size));
     displayProcessesGantt(cores,current_time,arr);
    free(status);
    free(completed);
    free(remaining_io);
    free(start);
    free(arrive);
    free(current_cpu);

}
// Function to simulate process execution with SJF scheduling
void sjf(Queue *queue)
{
    FILE *sysconf = fopen("system.txt", "r");
    if (sysconf == NULL)
    {
        printf("Error opening system file\n");
        exit(1);
    }
    int cores;
    if (fscanf(sysconf, "cores: %d", &cores) != 1)
    {
        printf("Error reading number of cores\n");
        exit(1);
    }
    fclose(sysconf);
    char arr[cores][1000];
    Process *processes = (Process *)malloc(queue->size * sizeof(Process));
    processes = queue->queue;
    char *status = (char *)malloc(queue->size * sizeof(char));
    int current_time = 0;
    int *current_cpu = (int *)malloc(queue->size * sizeof(int));
    int *current_core = (int *)malloc(cores * sizeof(int));
    int *arrive = (int *)malloc(cores * sizeof(int));
    for (int i = 0; i < cores; i++)
    {
        for (int j = 0; j < 1000; j++)
            arr[i][j] = 'i';
    }
    for (int i = 0; i < queue->size; i++)
    {
        current_cpu[i] = -1;
        status[i] = 'r';
        arrive[i] = processes[i].arrive_time;
    }
    for (int i = 0; i < cores; i++)
        current_core[i] = -1;

    int *completed = (int *)calloc(queue->size, sizeof(int));
    int *remaining_io = (int *)calloc(queue->size, sizeof(int));

    int completed_processes = 0;

    while (completed_processes < queue->size)
    {
        decrementtime(processes, queue->size, status, remaining_io, completed, current_time);
        for (int i = 0; i < cores; i++)
        {
            int j = shortest_job(processes, current_cpu, queue->size, status, current_time);
            if (current_core[i] != -1)
                j = current_core[i];
            if (j != -1)
            {
                current_core[i] = j;
                current_cpu[j] = i;
                processes[j].burst_time--;
                processes[j].start = min(processes[j].start, current_time);
                completed[j]++;
                status[j] = 'e';
                arr[i][current_time] = processes[j].pid;
                printf("Time %d~%d: Process %c executes on core %d", current_time, current_time + 1, processes[j].pid, i + 1);
                if (processes[j].burst_time == 0)
                {
                    status[j] = 'c';
                    completed_processes++;
                    completed[j] = current_time + 1;
                    processes[j].end = current_time + 1;
                    printf("\tTime %d: Process %c completed", completed[j], processes[j].pid);
                    current_core[i] = -1;
                    current_cpu[j] = -1;
                }
                if (completed[j] > 0 && processes[j].burst_time > 0 && processes[j].io_time > 0 && (completed[j]) % (processes[j].io_time) == 0)
                {
                    status[j] = 'i';
                    printf("\tTime %d: Process %c made I/O request", current_time + 1, processes[j].pid);
                    remaining_io[j] += processes[j].io_duration;
                    current_core[i] = -1;
                    current_cpu[j] = -1;
                }
                printf("\n");
            }
            else
            {
                printf("Time %d~%d: Process - executes on core %d\n", current_time, current_time + 1, i + 1);
            }
        }
        current_time++;

        printf("\n");
    }
    float average = 0;
    for (int k = 0; k < queue->size; k++)
    {
        average += (processes[k].end - arrive[k]);
        printf("\nTurnaround time for process %c: %d\n", processes[k].pid, (processes[k].end - arrive[k]));
        printf("Response time for process %c: %d\n", processes[k].pid, (processes[k].start - arrive[k]));
    }
    printf("\nAverage Turnaround time: %.2f\n\n", (average / queue->size));
    displayProcessesGantt(cores, current_time, arr);
    //  for (int i = 0; i <cores; i++)
    //     {
    //         printf("\nThe Processes Gantt Chart of core %d .\n", i + 1);
    //         for(int j=0;j<current_time;j++) printf("| %c ",arr[i][j]);
    //         printf("|\n");
    //     }
    free(status);
    free(completed);
    free(remaining_io);
    free(current_core);
    free(current_cpu);
}

// Function to simulate process execution with STCF scheduling
void stcf(Queue *queue)
{


    FILE *sysconf = fopen("system.txt", "r");
    if (sysconf == NULL)
    {
        printf("Error opening system file\n");
        exit(1);
    }
    int cores;
    if (fscanf(sysconf, "cores: %d", &cores) != 1)
    {
        printf("Error reading number of cores\n");
        exit(1);
    }
    fclose(sysconf);

    Process *processes = (Process *)malloc(queue->size * sizeof(Process));
    processes = queue->queue;
    char id;
    int startTime, executionTime, ioTime, duration, priority;
    // beginning of the stcf
    char *status = (char *)malloc(queue->size * sizeof(char));
    int *start = (int *)calloc(queue->size, sizeof(int));
    int current_time = 0;
    int *current_cpu = (int *)malloc(queue->size * sizeof(int));
    char arr[cores][1000];
    for (int i = 0; i < cores; i++)
    {
        for (int j = 0; j < 1000; j++)
            arr[i][j] = 'i';
    }
    for (int i = 0; i < queue->size; i++)
    {
        start[i] = -1;
        current_cpu[i] = -1;
    }
    int *arrive = (int *)malloc(queue->size * sizeof(char));
    for (int i = 0; i < queue->size; i++)
    {
        arrive[i] = processes[i].arrive_time;
    }
    int *completed = (int *)calloc(queue->size, sizeof(int));
    int *remaining_io = (int *)calloc(queue->size, sizeof(int));
    int completed_processes = 0;

    while (completed_processes < queue->size)
    {

        decrementtime(processes, queue->size, status, remaining_io, completed, current_time);

        for (int i = 0; i < cores; i++)
        {
            int j = leastremainingtime(processes, queue->size, status, current_time, i, current_cpu);

            if (j != -1)
            {
                processes[j].burst_time--;
                current_cpu[j] = i;
                completed[j]++;
                status[j] = 'e';
                arr[i][current_time] = processes[j].pid;
                if (start[j] == -1)
                {
                    start[j] = current_time - arrive[j];
                }
                printf("Time %d~%d: Process %c executes on core %d", current_time, current_time + 1, processes[j].pid, i + 1);
                if (processes[j].burst_time == 0)
                {
                    status[j] = 'c';
                    completed_processes++;
                    completed[j] = current_time + 1;
                    printf("\tTime %d: Process %c completed", completed[j], processes[j].pid);
                }
                if (completed[j] > 0 && processes[j].burst_time > 0 && processes[j].io_time > 0 && (completed[j]) % (processes[j].io_time) == 0)
                {
                    status[j] = 'i';
                    printf("\tTime %d: Process %c made I/O request", current_time + 1, processes[j].pid);
                    remaining_io[j] += processes[j].io_duration;
                }
                printf("\n");
            }
            else
            {
                printf("Time %d~%d: Process - executes on core %d\n", current_time, current_time + 1, i + 1);
            }
        }
        current_time++;

        printf("\n");
        
    }
    float average = 0;
    for (int k = 0; k < queue->size; k++)
    {
        average += (completed[k] - arrive[k]);
        printf("\nTurnaround time for process %c: %d\n", processes[k].pid, (completed[k] - arrive[k]));
        printf("Response time for process %c: %d\n", processes[k].pid, (start[k]));
    }
    printf("\nAverage Turnaround time: %.2f\n\n", (average / queue->size));
    displayProcessesGantt(cores,current_time,arr);
    free(status);
    free(completed);
    free(remaining_io);
    free(start);
    free(arrive);
    free(current_cpu);
}
// Function to simulate process execution with priority scheduling
void priority(Queue *queue)
{
    FILE *sysconf = fopen("system.txt", "r");
    if (sysconf == NULL)
    {
        printf("Error opening system file\n");
        exit(1);
    }
    int cores;
    if (fscanf(sysconf, "cores: %d", &cores) != 1)
    {
        printf("Error reading number of cores\n");
        exit(1);
    }
    fclose(sysconf);

    char arr[cores][1000];
    Process *processes = (Process *)malloc(queue->size * sizeof(Process));
    processes = queue->queue;
    char *status = (char *)malloc(queue->size * sizeof(char));
    int current_time = 0;
    int *current_cpu = (int *)malloc(queue->size * sizeof(int));
    int *current_core = (int *)malloc(cores * sizeof(int));
    int *arrive = (int *)malloc(cores * sizeof(int));
    for (int i = 0; i < cores; i++)
    {
        for (int j = 0; j < 1000; j++)
            arr[i][j] = 'i';
    }
    for (int i = 0; i < queue->size; i++)
    {
        current_cpu[i] = -1;
        status[i] = 'r';
        arrive[i] = processes[i].arrive_time;
    }
    for (int i = 0; i < cores; i++)
        current_core[i] = -1;

    int *completed = (int *)calloc(queue->size, sizeof(int));
    int *remaining_io = (int *)calloc(queue->size, sizeof(int));

    int completed_processes = 0;

    while (completed_processes < queue->size)
    {
        decrementtime(processes, queue->size, status, remaining_io, completed, current_time);
        for (int i = 0; i < cores; i++)
        {
            int j = highest_priority_job(processes, current_cpu, queue->size, status, current_time, i);
            if (current_core[i] != -1)
                j = current_core[i];
            if (j != -1)
            {
                current_core[i] = j;
                current_cpu[j] = i;
                processes[j].burst_time--;
                processes[j].start = min(processes[j].start, current_time);
                completed[j]++;
                status[j] = 'e';
                arr[i][current_time] = processes[j].pid;
                printf("Time %d~%d: Process %c executes on core %d", current_time, current_time + 1, processes[j].pid, i + 1);
                if (processes[j].burst_time == 0)
                {
                    status[j] = 'c';
                    completed_processes++;
                    completed[j] = current_time + 1;
                    processes[j].end = current_time + 1;
                    printf("\tTime %d: Process %c completed", completed[j], processes[j].pid);
                    current_core[i] = -1;
                    current_cpu[j] = -1;
                }
                if (completed[j] > 0 && processes[j].burst_time > 0 && processes[j].io_time > 0 && (completed[j]) % (processes[j].io_time) == 0)
                {
                    status[j] = 'i';
                    printf("\tTime %d: Process %c made I/O request", current_time + 1, processes[j].pid);
                    remaining_io[j] += processes[j].io_duration;
                    current_core[i] = -1;
                    current_cpu[j] = -1;
                }
                printf("\n");
            }
            else
            {
                printf("Time %d~%d: Process - executes on core %d\n", current_time, current_time + 1, i + 1);
            }
        }
        current_time++;

        printf("\n");
    }
    float average = 0;
    for (int k = 0; k < queue->size; k++)
    {
        average += (processes[k].end - arrive[k]);
        printf("\nTurnaround time for process %c: %d\n", processes[k].pid, (processes[k].end - arrive[k]));
        printf("Response time for process %c: %d\n", processes[k].pid, (processes[k].start - arrive[k]));
    }
    printf("\nAverage Turnaround time: %.2f\n\n", (average / queue->size));
    displayProcessesGantt(cores, current_time, arr);

    free(status);
    free(completed);
    free(remaining_io);
    free(current_core);
    free(current_cpu);
}

// Function to simulate process execution with RR scheduling
void rr(Queue *queue, int time_quantum)
{
    int cores_number = cores_reading("system.txt");
    char arr[cores_number][1000];
    printf("\n");
    int time = 0, total_turnaround = 0, no_of_processes = processCount(queue);
    sortQeue(queue);
    Queue *ready = createQueue(queue->size);
    Queue *running = createQueue(cores_number);
    int last_execution[queue->size];
    zero(last_execution, queue->size);
    for (int i = 0; i < cores_number; i++)
    {
        for (int j = 0; j < 1000; j++)
            arr[i][j] = 'i';
    }
    while (!isEmpty(queue) || !isEmpty(ready) || !isEmpty(running))
    {
        get_ready(ready, queue, time);
        if (!isEmpty(ready))
        {
            for (int i = 0; i < cores_number; i++)
            {
                if (isEmpty(ready))
                    break;
                enqueue(running, dequeue(ready));
            }
            int time2 = time;
            for (time2; time2 < time + time_quantum; time2++)
            {
                if (isEmpty(running))
                    break;
                get_ready(ready, queue, time2);
                int loop = processCount(running);
                for (int k = 0; k < loop; k++)
                {
                    if (isEmpty(running))
                        break;
                    Process process = dequeue(running);
                    if (process.start_waiting_time != -1)
                    {
                        if (isEmpty(ready))
                            printf("This core %d is empty from time %d to %d\n", k + 1, time2, time2 + 1);
                        else
                        {
                            run_another(ready, time2, &total_turnaround, last_execution, k, arr);
                        }
                        if (time2 + 1 - process.start_waiting_time >= process.io_duration)
                        {
                            process.start_waiting_time = -1;
                        }
                    }
                    else
                    {
                        if (process.no_of_executions == 0)
                            printf("Process %c has respose time %d \n", process.pid, time2 - process.arrive_time);
                        printf("Process %c is executing in core %d from time %d to %d\n", process.pid, k + 1,time2, time2 + 1);
                        arr[k][time2] = process.pid;
                        process.no_of_executions++;
                        process.burst_time--;
                        if ((process.io_time != 0) && (process.no_of_executions % process.io_time == 0))
                        {
                            process.start_waiting_time = time2 + 1;
                        }
                    }

                    if (process.burst_time <= 0 && process.start_waiting_time == -1)
                    {
                        printf("Process %c is finished at time %d\n", process.pid, time2 + 1);
                        total_turnaround += (time2 + 1 - process.arrive_time);
                    }
                    else
                        enqueue(running, process);
                }
                if (!isEmpty(running))
                    printf("\n");
            }
            if (!isEmpty(running))
                move_processes(ready, running);
            time = time2;
        }
        else
        {
            printf("No process execution at time %d\n\n", time);
            time++;
        }
    }
    printf("\nAverage turnaround time is %f\n\n", (float)total_turnaround / no_of_processes);
    displayProcessesGantt(cores_number, time, arr);
    printf("\n");
    free(running);
    free(ready);
    free(queue);
}

// Function to simulate process execution with MLFQ scheduling
void mlfq(Queue *queue, int no_of_levels, int time_quantum, int boost_time)
{
    int cores_number = cores_reading("system.txt");
    char arr[cores_number][1000];
    printf("\n");
    int time = 0, total_turnaround = 0, no_of_processes = processCount(queue), last_boost_time = -1;
    sortQeue(queue);
    Queue *running = createQueue(cores_number);
    Queue *queues[no_of_levels];
    for (int i = 0; i < no_of_levels; i++)
        queues[i] = createQueue(1000);
    int last_execution[queue->size];
    zero(last_execution, queue->size);
    for (int i = 0; i < cores_number; i++)
    {
        for (int j = 0; j < 1000; j++)
            arr[i][j] = 'i';
    }
    while (!isEmpty(queue) || !allLevelsEmpty(queues, no_of_levels))
    {
        get_ready(queues[0], queue, time);
        int isExecuted = 0;
        for (int i = 0; i < no_of_levels; i++)
        {
            int new_quantum = time_quantum + i * 1;
            while (!isEmpty(queues[i]))
            {
                isExecuted = 1;
                for (int j = 0; j < cores_number; j++)
                {
                    if (isEmpty(queues[i]))
                        break;
                    enqueue(running, dequeue(queues[i]));
                }
                int time2 = time;
                for (time2; time2 < time + new_quantum; time2++)
                {
                    if (time2 != 0 && time2 % boost_time == 0 && last_boost_time != time2)
                    {
                        last_boost_time = time2;
                        move_processes(queues[i], running);
                        boost_queue(queues, no_of_levels);
                        break;
                    }
                    if (isEmpty(running))
                        break;
                    get_ready(queues[0], queue, time2);
                    int loop = processCount(running);
                    for (int k = 0; k < loop; k++)
                    {
                        if (isEmpty(running))
                            break;
                        Process process = dequeue(running);
                        if (process.start_waiting_time != -1)
                        {
                            if (isEmpty(queues[i]))
                                printf("This core %d is empty from time %d to %d\n", k + 1, time2, time2 + 1);
                            else
                            {
                                run_another(queues[i], time2, &total_turnaround, last_execution, k, arr);
                            }
                            if (time2 + 1 - process.start_waiting_time >= process.io_duration)
                            {
                                process.start_waiting_time = -1;
                            }
                        }
                        else
                        {
                            if (process.no_of_executions == 0)
                                printf("Process %c has respose time %d \n", process.pid, time2 - process.arrive_time);
                            printf("Process %c is executing in core %d from time %d to %d at level %d\n", process.pid, k + 1, time2, time2 + 1, i + 1);
                            arr[k][time2] = process.pid;
                            process.no_of_executions++;
                            process.burst_time--;
                            if ((process.io_time != 0) && (process.no_of_executions % process.io_time == 0))
                            {
                                process.start_waiting_time = time2 + 1;
                            }
                        }

                        if (process.burst_time <= 0 && process.start_waiting_time == -1)
                        {
                            printf("Process %c is finished at time %d\n", process.pid, time2 + 1);
                            total_turnaround += (time2 + 1 - process.arrive_time);
                        }
                        else
                            enqueue(running, process);
                    }
                    printf("\n");
                }
                if (!isEmpty(running) && i < no_of_levels - 1)
                    move_processes(queues[i + 1], running);
                else if (!isEmpty(running) && i == no_of_levels - 1)
                    move_processes(queues[i], running);
                time = time2;
            }
            if (isExecuted)
                break;
        }
        if (!isExecuted)
        {
            printf("No process execution at time %d\n\n", time);
            time++;
        }
    }
    printf("\nAverage turnaround time is %f\n\n", (float)total_turnaround / no_of_processes);
    displayProcessesGantt(cores_number, time, arr);
    printf("\n");
    free(running);
    free(queue);
}

/**************************************************************/
//                        Main Function                       //
/**************************************************************/

int main()
{
    system("cls");
    system("color A");
    printf("Start of simulation.\n");
    int cores_no = cores_reading("system.txt");
    while (1)
    {
        Queue *inputQueue = inputQueueFromFile("DataSet.txt");
        int choose, time_quantum, no_of_levels, boost_time, num_iterations;
        printf("The available scheduler ploicies:\n1-First Come First Served\n2-Shortest Job First\n3-Shortest To Complete First\n4-Round Robin\n5-Priority (non-preemptive)\n6-Multilevel Feedback Queue\n7-stride_scheduling");
        printf("\nEnter a choice:\t");
        scanf("%d", &choose);
        if (choose < 1 || choose > 7)
        {
            printf("Invalid number!\n\n");
            continue;
        }
        switch (choose)
        {
        case 1:
            fcfs(inputQueue);
            break;
        case 2:
            sjf(inputQueue);
            break;
        case 3:
            stcf(inputQueue);
            break;
        case 4:
            printf("Enter the time quantum:\t");
            scanf("%d", &time_quantum);
            rr(inputQueue, time_quantum);
            break;
        case 5:
            priority(inputQueue);
            break;
        case 6:
            printf("Enter the number of levels:\t");
            scanf("%d", &no_of_levels);
            printf("Enter the time quantum:\t");
            scanf("%d", &time_quantum);
            printf("Enter the boost time:\t");
            scanf("%d", &boost_time);
            mlfq(inputQueue, no_of_levels, time_quantum, boost_time);
            break;
        case 7:
            _stride(inputQueue, cores_no);
            break;
        }
    }

    return 0;
}

/*

// Function to simulate process execution with MLFQ scheduling
void mlfq(Queue *queue, int no_of_levels, int level_size, int time_quantum, int boost_time)
{
    int time = 0, old_time = 0, total_turnaround = 0, no_of_processes = processCount(queue), count = 0;
    ProcessGantt processes_time[1000];
    sortQeue(queue);
    Queue *queues[no_of_levels];
    for (int i = 0; i < no_of_levels; i++)
        queues[i] = createQueue(level_size);
    while (1)
    {
        if (!isEmpty(queue))
        {
            Process process = queue->queue[queue->front];
            if (process.arrive_time > time && allLevelsEmpty(queues, no_of_levels))
            {
                process = dequeue(queue);
                int time2 = time;
                while (time2 < process.arrive_time)
                    time2 += time_quantum;
                printf("No process execution from time %d to %d\n", time, time2);
                time = time2;
                enqueue(queues[0], process);
            }
            else if (process.arrive_time <= time)
            {
                process = dequeue(queue);
                enqueue(queues[0], process);
            }
        }

        for (int i = 0; i < no_of_levels; i++)
        {
            int isExecuted = 0;
            if (!isEmpty((queues[i])))
            {
                isExecuted = 1;
                int process_count = processCount(queues[i]);
                int new_quantum = time_quantum + i * 1;
                while (process_count--)
                {
                    sortQeue(queues[i]);
                    Process process = dequeue(queues[i]);
                    int required_time = process.burst_time + process.io_time;
                    if (required_time >= new_quantum || (time + required_time - old_time > boost_time))
                    {
                        if (process.no_of_executions == 0)
                            printf("Process %c has respose time %d \n", process.pid, time - process.arrive_time);
                        int end_time = (time + new_quantum - old_time) >= boost_time ? old_time + boost_time : time + new_quantum;
                        printf("Process %c is executing from time %d to %d\n\n", process.pid, time, end_time);
                        processes_time[count].start_time = time;
                        processes_time[count].end_time = end_time;
                        processes_time[count++].pid = process.pid;
                        total_turnaround += (end_time - process.arrive_time);
                        process.no_of_executions++;
                        process.burst_time -= (end_time - time);
                        if (process.burst_time < 0)
                            process.io_time += process.burst_time, process.burst_time = 0;
                        process.arrive_time = end_time;
                        if ((i + 1) < no_of_levels && (process.burst_time + process.io_time) > 0)
                            enqueue(queues[i + 1], process);
                        else if ((process.burst_time + process.io_time) > 0)
                            enqueue(queues[i], process);
                        time = end_time;
                        if (end_time - old_time >= boost_time)
                        {
                            boost_queue(queues, no_of_levels);
                            old_time = end_time;
                            break;
                        }
                    }
                    else
                    {
                        if (process.no_of_executions == 0)
                            printf("Process %c has respose time %d \n", process.pid, time - process.arrive_time);
                        printf("Process %c is executing from time %d to %d\n\n", process.pid, time, time + required_time);
                        processes_time[count].start_time = time;
                        processes_time[count].end_time = time + required_time;
                        processes_time[count++].pid = process.pid;
                        total_turnaround += (time + required_time - process.arrive_time);
                        process.no_of_executions++;
                        time += required_time;
                    }
                }
            }
            if (isExecuted)
                break;
        }

        // if ((time - old_time) >= boost_time)
        // {
        //     for (int i = 1; i < no_of_levels; i++)
        //     {
        //         while (!isEmpty(queues[i]))
        //         {
        //             Process process = dequeue(queues[i]);
        //             enqueue(queues[0], process);
        //         }
        //     }
        //     old_time = time;
        // }

        if (isEmpty(queue) && allLevelsEmpty(queues, no_of_levels))
            break;
    }
    printf("Average turnaround time is %f\n", (float)total_turnaround / no_of_processes);
    displayProcessesGantt(processes_time, count, time);
}






Function to simulate process execution with RR scheduling
void rr(Queue *queue, int time_quantum)
{
    int time = 0, total_turnaround = 0, no_of_processes = processCount(queue), count = 0;
    ProcessGantt processes_time[1000];
    // int mx[26];
    // int mn[26];
    // for (int i = 0; i < 26; i++)
    //     mn[i] = 1e7, mx[i] = 0;
    while (!isEmpty(queue))
    {
        sortQeue(queue);
        Process process = dequeue(queue);
        if (process.arrive_time > time)
        {
            int time2 = time;
            while (time2 < process.arrive_time)
                time2 += time_quantum;
            printf("No process execution from time %d to %d\n", time, time2);
            time = time2;
        }
        int required_time = process.burst_time + process.io_time;
        if (required_time > time_quantum)
        {
            if (process.no_of_executions == 0)
                printf("Process %c has respose time %d \n", process.pid, time - process.arrive_time);
            printf("Process %c is executing from time %d to %d\n\n", process.pid, time, time + time_quantum);
            total_turnaround += (time_quantum + time - process.arrive_time);
            processes_time[count].start_time = time;
            processes_time[count].end_time = time + time_quantum;
            processes_time[count].pid = process.pid;
            // mn[process.pid - 'A'] = min(mn[process.pid - 'A'], process.arrive_time);
            // mx[process.pid - 'A'] = max(mn[process.pid - 'A'], time + time_quantum);
            count++;
            process.no_of_executions++;
            process.burst_time -= time_quantum;
            if (process.burst_time < 0)
                process.io_time += process.burst_time, process.burst_time = 0;
            time += time_quantum;
            process.arrive_time = time;
            enqueue(queue, process);
        }
        else
        {
            if (process.no_of_executions == 0)
                printf("Process %c has respose time %d \n", process.pid, time - process.arrive_time);
            printf("Process %c is executing from time %d to %d\n\n", process.pid, time, time + min(time_quantum, required_time));
            total_turnaround += (required_time + time - process.arrive_time);
            processes_time[count].start_time = time;
            processes_time[count].end_time = time + min(time_quantum, required_time);
            processes_time[count].pid = process.pid;
            // mn[process.pid - 'A'] = min(mn[process.pid - 'A'], process.arrive_time);
            // mx[process.pid - 'A'] = max(mn[process.pid - 'A'], time + min(time_quantum, required_time));
            count++;
            process.no_of_executions++;
            time += required_time;
        }
    }
    // for (int i = 0; i < no_of_processes; i++)
    // total_turnaround += mx[i] - mn[i];

    printf("Average turnaround time is %f\n", (float)total_turnaround / no_of_processes);
    displayProcessesGantt(processes_time, count, time);
}




*/