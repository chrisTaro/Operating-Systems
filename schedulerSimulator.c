/*
    Scheduler Simulator
    CS421 - Operating Systems
    cmscheduler.c
    By: Christian Magpantay
    Code/Book Reference:
        C++ Plus Data Structures, 6th Edition
            by: Dale, Weems, Richards
    Program: 
        This program will perform all of the schedulers, SJF, SRTF, & RR
        and display the stats of each scheduler. It may only read in an inputfile 
        that is name "inputfile.txt" and assumed that the inputfile is sorted with 
        arrival time in descending order, from 0 to infinity. 
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
enum RelationType {LESS, GREATER, EQUAL};
#define MAX_QUANTUM 4
#define MAX_PROCESS 7

/* 
    structs used for implementation
        stats Array - used for printing out statistics
        NodeType    - used for node in queue
        QueType     - used to implement queues
*/

struct statsArray {
    char procID;
    int burstTime;
    int arrivalTime;
    int waitingTime;
    int turnaroundTime;
};

typedef struct NodeType
{
    char processID;
    int burst;
    int arrival;
    struct NodeType* next;
} NodeType;

typedef struct QueType 
{
    int count;
    struct NodeType* front;
    struct NodeType* rear;
} QueType;

/* 
    queue functions
        initQueType     - initialize queue with front & rear == NULL
        enqeueue        - insert the node at the end or front if empty
                            have front and rear point respectively
        dequeue         - remove the front of queue
        displayQueue    - display the entire queue using front node
        queueCount      - return queue length
        loadJobQueue    - load the job queue from a file 
*/
void initQueType(QueType* qt){
    qt->count = 0;
    qt->front = NULL;
    qt->rear = NULL;
}

void enqueue(QueType* qt, char pid, int b, int a) {
    NodeType* newNode;
    newNode = (struct NodeType*)malloc(sizeof(struct NodeType)) ;
    newNode->processID = pid;
    newNode->burst = b;
    newNode->arrival = a;
    newNode->next = NULL;
    if (qt->rear == NULL)
        qt->front = newNode;
    else
        qt->rear->next = newNode;
    qt->rear = newNode;

    qt->count++;
}

void dequeue(QueType* qt) {
    NodeType* tempPtr;
    if(qt->front == NULL) {
        printf("\nEmpty queue") ;
    } else {
        tempPtr = qt->front ;
        qt->front = qt->front->next ;
        free(tempPtr) ;
        qt->count--;
    }
}

void displayQueue(NodeType* f) {
    if(f == NULL) {
    } else {
        printf("%c\t%d\t%d\n", f->processID, f->burst, f->arrival);
        displayQueue(f->next);
    }
}

int queueCount(QueType* qt){
    return qt->count;
}

int loadJobQueue(QueType* jq, FILE* ms){
    char procID;
    int burstTime;
    int arrivTime;
    int burstTot = 0;

    ms = fopen ("inputfile.txt", "r");
    while( fscanf(ms, " %c %d %d", &procID, &burstTime, &arrivTime) != EOF ) {
        enqueue(jq, procID, burstTime, arrivTime);
        burstTot += burstTime;
    }
    fclose (ms);
    return burstTot;
}

/* 
    stats array building functions
        loadStats       - load array with pid, burst time and arrival time
        findProcID      - helper function to find proc id in array
        updateStats     - updates the array for turnaroundtime and waiting time
        displayStats    - display the array
*/

void loadStats(struct statsArray sa[], char pid, int b, int a, int i) {
    sa[i].procID = pid;
    sa[i].burstTime = b;
    sa[i].arrivalTime = a;
}

int findProcID(struct statsArray sa[], char pid) {
    int x;
    for(x = 0; x < MAX_PROCESS + 1; x++){
        if(sa[x].procID == pid)
            return x;
    }
}

void updateStats(struct statsArray sa[], char pid, int t) {
    int pos;
    int bt;
    int at;
    pos = findProcID(sa, pid);
    bt = sa[pos].burstTime;
    //printf("%c\t\t%d\t\t%d\n", sa[pos].procID, sa[pos].jobSubmittal, t);

    sa[pos].turnaroundTime = t - sa[pos].arrivalTime;
    sa[pos].waitingTime = t - bt;
}

void displayStats(struct statsArray sa[]){
    int j;
    int avrgTAT = 0;
    int avrgWT = 0;
    int total = 0;
    printf("Process ID\tTurnaround Time\tWaiting Time\n");
    for(j = 0; j < MAX_PROCESS + 1; j++){
        if(sa[j].procID != 0)
            total++;
        avrgTAT += sa[j].turnaroundTime;
        avrgWT += sa[j].waitingTime;
        printf("%c\t\t%d\t\t%d\n", sa[j].procID, sa[j].turnaroundTime, sa[j].waitingTime);
    }
    printf("Average\t\t%d/%d = %d\t%d/%d = %d\t\n", 
                avrgTAT, total, avrgTAT/total, avrgWT, total, avrgWT/total);
}

/* 
    scheduling algorithms with helper functions
        all algorithms perform same behavior but helper functions
        cater to each algorithm

        runSJF          - run Shortest job function
        comparedTo      - used to compare to insert with node inside of list
        enqueueSort     - used for SRTF algorithm by inserting from job queue into ready
                        with shortest burst time in the front of ready queue
        runSRTF         - run shortest remaining time first algorithm
        rearrangeQueue  - used for round robin by dequeue front when quantum time is expired
                        - and enqueue to the back
        runRR           - run round robin algorithm
*/

// parameters: job queue, ready queue, time, burst total, stat array sjf and array index
void runSJF(QueType* jq, QueType* rq, int t, int bt, struct statsArray sjf[], int i) {
    while(queueCount(jq) != 0) {    // enqueue into ready until job is empty
        if(t == jq->front->arrival) {   
            enqueue(rq, jq->front->processID, jq->front->burst, jq->front->arrival);
            loadStats(sjf, jq->front->processID, jq->front->burst, jq->front->arrival, i);
            i++; // increment index
            dequeue(jq); 
            runSJF(jq, rq, t, bt, sjf, i); // recursive call; find all other same arrival times
        } else {
            if(t == 0 && rq->front->burst == 1){
                printf("%d %c\t Process Terminated\n", t, rq->front->processID);
            }
            t++;    // increment time
            rq->front->burst--; // perform task for process
            if(rq->front->burst == 0) {
                updateStats(sjf, rq->front->processID, t);  // process finish, update stats
                dequeue(rq);
                printf("%d %c\t Process Terminated\n", t, rq->front->processID);
            }
        }
    }
    while(queueCount(rq) != 0) {
        t++;
        rq->front->burst--; // perform task
        if(rq->front->burst == 0) {
            updateStats(sjf, rq->front->processID, t);
            dequeue(rq);
            if(queueCount(rq) == 0)
                break;
            printf("%d %c\t Process Terminated\n", t, rq->front->processID);
        }
    }
    if(t == bt)
        printf("%d Completed\n", t);
}

// comparison of two burst times to return LESS, GREATER, EQUAL
// between to be inserted node and node in queue
enum RelationType comparedTo(int x, int y) {
    if(x < y)
        return LESS;
    else if (x > y)
        return GREATER;
    else return EQUAL;
}

// sort by inserting shortest time in the front of the queue
void enqueueSort(QueType* qt, char pid, int b, int a, int t) {
    NodeType* newNode;  // pointer to node
    NodeType* predLoc;  // trailing pointer
    NodeType* location; // traveling pointer
    bool moreToSearch;

    location = qt->front;
    predLoc = NULL;
    moreToSearch = (location != NULL);

    while(moreToSearch) {   // find spot to insert by comparing
        switch(comparedTo(b, location->burst)) {
            case GREATER:   predLoc = location;
                            location = location->next;
                            moreToSearch = (location != NULL);
                            break;
            case EQUAL: {
                            if(pid > location->processID){
                                predLoc = location;
                                location = location->next;
                                moreToSearch = (location != NULL);
                            }
                            else 
                                moreToSearch = false;
                            break;
                        }
            case LESS:      moreToSearch = false;
                            break;
        }
    }

    // set up for insertion
    newNode = (struct NodeType*)malloc(sizeof(struct NodeType)) ;
    newNode->processID = pid;
    newNode->burst = b;
    newNode->arrival = a;

    if (predLoc == NULL){
        if( t > 0)  // used when a process prempts in front of queue, only if not empty
            printf("%d %c\t Process prempted by process with shorter burst time\n", t, pid);
        newNode->next = qt->front;
        qt->front = newNode;
    } else {
        newNode->next = location;
        predLoc->next = newNode;
    }
    qt->count++; // increment length of queue
}


void runSRTF(QueType* jq, QueType* rq, int t, int bt, struct statsArray srtf[], int i){
    while(queueCount(jq) != 0) {    // enqueue into ready until job is empty
        if(t == jq->front->arrival) {
            enqueueSort(rq, jq->front->processID, jq->front->burst, jq->front->arrival, t);
            loadStats(srtf, jq->front->processID, jq->front->burst, jq->front->arrival, i);
            i++;
            dequeue(jq);
            runSRTF(jq, rq, t, bt, srtf, i);
        } else {
            if(t == 0 && rq->front->burst == 1){
                printf("%d %c\t Process Terminated\n", t, rq->front->processID);
            }
            t++;
            rq->front->burst--; // perform task
            if(rq->front->burst == 0) {
                updateStats(srtf, rq->front->processID, t);
                dequeue(rq);
                printf("%d %c\t Process Terminated\n", t, rq->front->processID);
            }
        }
    }
    while(queueCount(rq) != 0) {
        //displayQueue(rq->front);
        t++;
        rq->front->burst--; // perform task
        if(rq->front->burst == 0) {
            updateStats(srtf, rq->front->processID, t);
            dequeue(rq);
            if(queueCount(rq) == 0)
                break;
            printf("%d %c\t Process Terminated\n", t, rq->front->processID);
        }
    }
    if(t == bt)
        printf("%d Completed\n", t);
}

void rearrangeQueue(QueType* rq){
    NodeType* tempNode;
    tempNode = (struct NodeType*)malloc(sizeof(struct NodeType)) ;

    tempNode->processID = rq->front->processID;
    tempNode->burst = rq->front->burst;
    tempNode->arrival = rq->front->arrival;

    dequeue(rq);
    enqueue(rq, tempNode->processID, tempNode->burst, tempNode->arrival);
}

void runRR(QueType* jq, QueType* rq, int t, int bt, int qc, struct statsArray rr[], int i){
    while(queueCount(jq) != 0) {
        if(t == jq->front->arrival) {
            enqueue(rq, jq->front->processID, jq->front->burst, jq->front->arrival);
            loadStats(rr, jq->front->processID, jq->front->burst, jq->front->arrival, i);
            i++;
            dequeue(jq);
            runRR(jq, rq, t, bt, qc, rr, i);
        } else {
            if(t == 0 && rq->front->burst == 1){
                printf("%d %c\t Process Terminated\n", t, rq->front->processID);
            }
            t++;
            qc++;
            rq->front->burst--; // perform task
            if(qc == MAX_QUANTUM){
                qc = 0;
                if(rq->front->burst == 0){
                    updateStats(rr, rq->front->processID, t);
                    dequeue(rq);
                    printf("%d %c\t Process Terminated\n", t, rq->front->processID);
                } else {
                    printf("%d %c\t Quantum Expired\n", t, rq->front->processID);
                    if(rq->count > 1)
                        rearrangeQueue(rq);
                }
            } else if(rq->front->burst == 0 && qc < 5) {
                qc = 0;
                updateStats(rr, rq->front->processID, t);
                dequeue(rq);
                printf("%d %c\t Process Terminated\n", t, rq->front->processID);
            }
        }
    }
    while(queueCount(rq) != 0) {
        t++;
        qc++;
        rq->front->burst--; // perform task
        if(rq->front->burst == 0 && qc < 5) {
            updateStats(rr, rq->front->processID, t);
            qc = 0;
            dequeue(rq);
            if(queueCount(rq) == 0)
                break;
            printf("%d %c\t Process Terminated\n", t, rq->front->processID);
        } else if(qc == MAX_QUANTUM) {
            qc = 0;
            if(rq->front->burst == 0){
                updateStats(rr, rq->front->processID, t);
                dequeue(rq);
                if(queueCount(rq) == 0)
                    break;
                printf("%d %c\t Process Terminated\n", t, rq->front->processID);
            } else {
                printf("%d %c\t Quantum Expired\n", t, rq->front->processID);
                if(rq->count > 1)
                    rearrangeQueue(rq);
            }
        } else if(rq->count == 1) {
            qc = 0;
        }
        //displayQueue(rq->front);
    }
    if(t == bt)
        printf("%d Completed\n", t);
}

/*
    main function
        init linked list queues, job and ready
        init file handling
        init time for simuation
        init burst total for all schedulers
        init quant counter for round robin
        init i for array indexing 
        init all stat arrays for SJF, SRTF, & RR
        init queue with initQuetype

        call loadJobQueue for first SJF
            return the total burst time
        print scheduler name
        run SJF
        display stats of SJF

        reinit for SRTF
        call loadJobQueue for first SRTF
        reset time and i to 0
        run SRTF
        display stats of SRTF

        reinit for RR
        call loadJobQueue for first RR
        reset time and i to 0
        run RR
        display stats of RR

        end
*/
void main()
{
    QueType *jobQueue;                                  // initialize queues
    QueType *readyQueue;
    FILE *my_stream;                                    // file stream
    char fileName[] = "inputfile.txt";
    int time = 0;
    int burstTotal = 0;
    int quantCounter = 0;
    int i = 0;
    struct statsArray sjf[MAX_PROCESS];
    struct statsArray srtf[MAX_PROCESS];
    struct statsArray rr[MAX_PROCESS];

    jobQueue = malloc(sizeof(QueType));
    readyQueue = malloc(sizeof(QueType));
    initQueType(jobQueue);
    initQueType(readyQueue);

    burstTotal = loadJobQueue(jobQueue, my_stream);
    printf("\nSJF Scheduling\n");
    runSJF(jobQueue, readyQueue, time, burstTotal, sjf, i);
    printf("\n");
    displayStats(sjf);

    initQueType(jobQueue);
    initQueType(readyQueue);
    loadJobQueue(jobQueue, my_stream);
    time = 0;
    i = 0;
    printf("\n\nSRTF Scheduling\n");
    runSRTF(jobQueue, readyQueue, time, burstTotal, srtf, i);
    printf("\n");
    displayStats(srtf);

    initQueType(jobQueue);
    initQueType(readyQueue);
    loadJobQueue(jobQueue, my_stream);
    time = 0;
    printf("\n\nRound Robin Scheduling\n");
    runRR(jobQueue, readyQueue, time, burstTotal, quantCounter, rr, i);
    printf("\n");
    displayStats(rr);
}


