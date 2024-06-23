#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "queue.h"

void outprint(int time_x, int time_y, int pid, int arrival_time, int remaining_time);
int min(int x, int y);

Process MinProcID(Process x, Process y){
    Process min_proc;
    if (x.process_id < y.process_id) min_proc = x;
    else min_proc = y;
    return min_proc;
}

void SortProcessID(Process* p, int num){
    Process* a = p;
    Process* b = (Process*)malloc(num * sizeof(Process));
    int seg, start;
    for (seg = 1; seg < num; seg += seg) {
        for (start = 0; start < num; start += seg + seg) {
            int low = start, mid = min(start + seg, num), high = min(start + seg + seg, num);
            int k = low;
            int start1 = low, end1 = mid;
            int start2 = mid, end2 = high;
            while (start1 < end1 && start2 < end2){
                Process minproc = MinProcID(a[start1], a[start2]);
                if (minproc.process_id == a[start1].process_id)
                    b[k++] = a[start1++];
                else 
                    b[k++] = a[start2++];
            }
            while (start1 < end1)
                b[k++] = a[start1++];
            while (start2 < end2)
                b[k++] = a[start2++];
        }
        Process* tmp = a;
        a = b;
        b = tmp;
    }
    if (a != p) {
        int i;
        for (i = 0; i < num; i++)
            b[i] = a[i];
        b = a;
    }
    free(b);
}

bool allQueuesEmpty(LinkedQueue** ProcessQueue, int queue_num){
    for(int i = 0; i < queue_num; i++){
        if (!IsEmptyQueue(ProcessQueue[i])){
            return false;
        }
    }
    return true;
}


void BoostProcess(LinkedQueue** ProcessQueue, int proc_run, int queue_num){

    //get all processes
    Process* runningProcs = (Process*) malloc(proc_run * sizeof(Process));
    int counter = 0;
    for(int i = 0; i < queue_num; i++){
        while(!IsEmptyQueue(ProcessQueue[i])){
            runningProcs[counter] = DeQueue(ProcessQueue[i]);
            counter++;
        }
    }

    //Sort process
    SortProcessID(runningProcs, proc_run);

    //add all Process to highest queue again
    for(int i = 0; i < proc_run; i++){
        runningProcs[i].service_time = ProcessQueue[queue_num-1]->allotment_time;
        EnQueue(ProcessQueue[queue_num-1], runningProcs[i]);
    }

    free(runningProcs);
}

// Implement by students
void scheduler(Process* proc, LinkedQueue** ProcessQueue, int proc_num, int queue_num, int period){
    printf("Process number: %d\n", proc_num);
    for (int i = 0;i < proc_num; i++)
        printf("%d %d %d\n", proc[i].process_id, proc[i].arrival_time, proc[i].execution_time);

    printf("\nQueue number: %d\n", queue_num);
    printf("Period: %d\n", period);
    for (int i = 0;i < queue_num; i++){
        printf("%d %d %d\n", i, ProcessQueue[i]->time_slice, ProcessQueue[i]->allotment_time);
    }

    if (proc_num == 0 || queue_num == 0) return;
    
    int currTime = 0; //current time
    int proc_sched = 0; //how many procs have been scheduled
    int proc_run = 0; //how many procs are running currently
    Process currP;
    int nextBoost = period; //which period is next

    while(proc_sched < proc_num){

        //add Batch of Process if Queue is empty
        currTime = proc[proc_sched].arrival_time;
        while(proc_sched < proc_num && currTime >= proc[proc_sched].arrival_time){
            proc[proc_sched].service_time = ProcessQueue[queue_num-1]->allotment_time;
            EnQueue(ProcessQueue[queue_num-1], proc[proc_sched]);
            proc_sched++;
            proc_run++;
        }

        //check if there are sprocesses to process
        while(!allQueuesEmpty(ProcessQueue, queue_num)){
            for(int i = queue_num-1; i >=  0; i--){
                if(!IsEmptyQueue(ProcessQueue[i])){
                    
                    currP = DeQueue(ProcessQueue[i]);
                    
                    //Process may need to be interrupted due to Boost
                    if(currTime + ProcessQueue[i]->time_slice >= nextBoost){
                        currP.service_time -= nextBoost - currTime;
                        currP.execution_time -= nextBoost - currTime;

                        //Process finished
                        if(currP.execution_time <= 0){
                            outprint(currTime, nextBoost + currP.execution_time, currP.process_id, currP.arrival_time, 0);
                            proc_run--;
                            currTime = nextBoost + currP.execution_time;
                        }
                        else{
                            outprint(currTime, nextBoost, currP.process_id, currP.arrival_time, currP.execution_time);
                            currTime = nextBoost;
                            if(currP.service_time > 0 || i == 0){
                                EnQueue(ProcessQueue[i], currP);
                            }
                            else{
                                currP.service_time = ProcessQueue[i-1]->allotment_time;
                                EnQueue(ProcessQueue[i-1], currP);
                            }
                        }
                        
                    }
                    else{ 
                        currP.service_time -= ProcessQueue[i]->time_slice;
                        currP.execution_time -= ProcessQueue[i]->time_slice;

                        //Process finished
                        if(currP.execution_time <= 0){
                            outprint(currTime, currTime + ProcessQueue[i]->time_slice + currP.execution_time, currP.process_id, currP.arrival_time, 0);
                            proc_run--;
                            currTime += ProcessQueue[i]->time_slice + currP.execution_time;
                        }
                        else{
                            outprint(currTime, currTime + ProcessQueue[i]->time_slice, currP.process_id, currP.arrival_time, currP.execution_time);
                            currTime += ProcessQueue[i]->time_slice;

                            if(currP.service_time > 0 || i == 0){
                                EnQueue(ProcessQueue[i], currP);
                            }
                            else{
                                currP.service_time = ProcessQueue[i-1]->allotment_time;
                                EnQueue(ProcessQueue[i-1], currP);
                            }
                        }
                    }

                    //check for new Processes
                    while(proc_sched < proc_num && currTime >= proc[proc_sched].arrival_time){
                        proc[proc_sched].service_time = ProcessQueue[queue_num-1]->allotment_time;
                        EnQueue(ProcessQueue[queue_num-1], proc[proc_sched]);
                        proc_sched++;
                        proc_run++;
                    }
                    
                    //Boost Process
                    if(currTime == nextBoost){
                        nextBoost += period;
                        BoostProcess(ProcessQueue, proc_run, queue_num);
                    }

                    break;
                }
            }   
        }
    }
}