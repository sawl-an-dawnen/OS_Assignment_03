#include <iostream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <semaphore.h>


using namespace std;

//there is one page table per address space (process)
//one table entry per page

//the address can be divided into 2 parts:
//page number
//displacement (offset)

//a frame table consists of:
//address space
//page number of the page cuurrently occupying each page frame

//FRAMES[f]: address_space page forward_link backwrad_link
//frames table is a linked list

//Disk Page Table
//DPT[address_space,page]

/*
1. Write a page fault handler process that can be invoked by the interrupt dispatcher when a
page fault occurs. The address space and page number of the missing page are made available
to the fault handler by the addressing hardware. The fault handler requests a page transfer
by placing an entry in the disk driver work queue and signaling the associated semaphore.

2. Design a disk driver process which schedules all I/O to the paging disk. disk command

STARTDISK(read/write, memory_addr, disk_addr)

initiates an I/O operation. The paging disk is wired to the semaphore of the disk driver
process, which it signals when a command is complete. The disk has a work queue containing
entries of the form

(process_id, read/write, frame_index, disk_addr).

3. Assume that the page replacement algorithm runs as a separate process which is signaled each
time a page frame is removed from the pool. the replacement process attempts to maintain a
free pool size between min and max frames. To accomplish this, one or more processes may
need to be ”deactivated” and ”activated” later.
Note: In this assignment, you need not consider the actual creation and deletion of address
spaces. Be sure to initialize the semaphores you use with correct initial counts.
*/

/*
Page Replacement Stratigies:
1.FIFO - first in first out
2.LRU - least recently used
3.LRU-K - least recently used whose kth most recent access is replaced
4.LFU - least frequently used
5.OPT-lookahead-X
6.WS (working set)
*/

//need semaphore for page fault handler to call on the page replacement algo
sem_t PRA_SEMA;
//need smaphore for page replace algo to call on the disk driver
sem_t DD_SEMA;

int main(int argc, char** argv) {

    ifstream input(argv[1]);
    ofstream systemInfo("SystemInfo.txt");
    {
        int system[7];
        //systemInfo data schema:
        //0 tp//total number of page frames in main memory
        //1 ps//page size in number of bytes
        //2 r//number of page frames per process for FIFO, LRU, LRU-kth, LFU, OPT, or delta for WS
        //3 x//lookahead window for OPT, X for LRU-xth or 0 for algorithms that don't use lookahead
        //4 min//min free pool size;
        //5 max//max free pool size;
        //6 k//total number of processes
        string data;
        for (int i = 0; i < 7; i++) {
            getline(input, data);
            system[i] = stoi(data);
        }
        systemInfo << "---SYSTEM INFORMATION---\n";
        systemInfo << "Total page frames in main memory: " << system[0] << endl;
        systemInfo << "Page size: " << system[1] << " byte(s)\n";
        systemInfo << "Page frames per process (or delta for WS): " << system[2] << endl;
        systemInfo << "Lookahead/X: " << system[3] << endl;
        systemInfo << "Minimum free pool size: " << system[4] << endl;
        systemInfo << "Maximum free pool size: " << system[5] << endl;
        systemInfo << "Number of processes: " << system[6] << endl;
        //keep a list of imaginary processes and the number of faults they incur  {

    }
    int pnum = -1;
    int pid = -1;
    sem_init(&PRA_SEMA, 1, 0);
    sem_init(&DD_SEMA, 1, 0);
    
    for (int i = 0; i < 3; i++) {
        pid = fork();
        if (pid == 0) {
            pnum = i;
            break;
        }
    }

    bool DD_active = true;
    bool PRA_active = true;

    if (pid == 0 && pnum == 0) {
        cout << "PAGE FAULT HANDLER RUNNING\n";
        cout << "\nPFH signaling PRA\n\n";
        sem_post(&PRA_SEMA);
        sem_post(&PRA_SEMA);
        sem_post(&PRA_SEMA);
        cout << "\nPFH signal for DD\n\n" << endl;
        sem_post(&DD_SEMA);
        return 0;
    }
    //pageFaultHandler
        //if there is process to read
            //read in process
            //convert address to binary
            //determine what page needs to be used
            //check main memory for required page
            //if(exists) complete process request
            //else 
                //record fault for process i
                //signal page replacement
                //verify page is in main memory
                //complete process request
            //loop back to top
        //else signal DD and PRA to terminate
        //terminate this process

    if (pid == 0 && pnum == 1) {
        sem_wait(&DD_SEMA);
        cout << "DD process PRIMED" << endl;
        cout << "DISK DRIVER ACTIVE\n";
        cout << "DD TERMINATE IMMINENT\n";
        sem_destroy(&DD_SEMA);
        return 0;
    }
    //diskDriver
        //get signal from PRA or PFH
        //if signaled with key to terminate GOTO terminate
        //else
            //empty process unless simulating disk
            //wait
            //loop to top
        //treminate
    
    if (pid == 0 && pnum == 2)  {
        sem_wait(&PRA_SEMA);
        cout << "PRA process PRIMED" << endl;
        cout << "PRA FIRING\n";
        cout << "PRA TERMINATNE IMMINENT\n";
        sem_destroy(&PRA_SEMA);
        return 0;
    }
    //pageReplacementAlgorithm
        //get signal from PRA
        //if signaled with key to terminate GOTO terminate
        //else
            //determine what algorithm is being implemented
            //determine what page frame needs to be replaced in main memory
            //signal disk driver to provide address location of new page
            //read in new address to correct page frame location
            //wait
            //loop back to top
        //terminate

    return 0;
}

/*
References:
1.https://github.com/MagedSaeed/vertual_memroy_manager/blob/master/main.c
2.https://linuxize.com/post/how-to-install-gcc-compiler-on-ubuntu-18-04/ ---getting gcc working
*/