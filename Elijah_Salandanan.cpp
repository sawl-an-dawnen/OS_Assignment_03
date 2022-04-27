#include <iostream>
#include <fstream>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>

using namespace std;

//need semaphore for page fault handler to call on the page replacement algo
const char *PRA_SEMA_NAME = "PRA";
//need semaphore for page replace algo to call on the disk driver
const char *DD_SEMA_NAME = "DD";
//need semaphore to pause PFA
const char *PFH_SEMA_NAME = "PFH";

int main(int argc, char** argv) {
    //input handler
    {
        ifstream input(argv[1]);
        ofstream systemInfo("SystemInfo.txt");
        int system[7];
        //system array schema:
        //0 tp total number of page frames in main memory
        //1 ps page size in number of bytes
        //2 r number of page frames per process for FIFO, LRU, LRU-kth, LFU, OPT, or delta for WS
        //3 x lookahead window for OPT, X for LRU-xth or 0 for algorithms that don't use lookahead
        //4 min min free pool size;
        //5 max max free pool size;
        //6 k total number of processes
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

        while (getline(input, data)) {
            cout << data << endl;
        }

    }

    bool DD_active = true;
    bool PRA_active = true;
    int pnum = -1;
    int pid = -1;
    for (int i = 0; i < 3; i++) {
        pid = fork();
        if (pid == 0) {
            pnum = i;
            break;
        }
    }

    /*pageFaultHandler
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
    */
    if (pid == 0 && pnum == 0) {
        sem_t *PFH_SEMA = sem_open(PFH_SEMA_NAME, O_CREAT, 0600, 0);
        sem_t *PRA_SEMA = sem_open(PRA_SEMA_NAME, O_CREAT, 0600, 0);
        //cout << "PAGE FAULT HANDLER RUNNING\n";
        return 0;
    }

    /*diskDriver
        //get signal from PRA or PFH
        //if signaled with key to terminate GOTO terminate
        //else
            //empty process unless simulating disk
            //wait
            //loop to top
        //treminate
    */
    if (pid == 0 && pnum == 1) {
        sem_t *DD_SEMA = sem_open(DD_SEMA_NAME, O_CREAT, 0600, 0);
        sem_t *PRA_SEMA = sem_open(PRA_SEMA_NAME, O_CREAT, 0600, 0);
        sem_wait(DD_SEMA);
        return 0;
    }

    /*pageReplacementAlgorithm
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
    */
    if (pid == 0 && pnum == 2)  {
        sem_t *PFH_SEMA = sem_open(PFH_SEMA_NAME, O_CREAT, 0600, 0);
        sem_t *DD_SEMA = sem_open(DD_SEMA_NAME, O_CREAT, 0600, 0);
        sem_t *PRA_SEMA = sem_open(PRA_SEMA_NAME, O_CREAT, 0600, 0);
        sem_wait(PRA_SEMA);
        return 0;
    }


    return 0;
}

/*
References:
1.https://github.com/MagedSaeed/vertual_memroy_manager/blob/master/main.c
2.https://linuxize.com/post/how-to-install-gcc-compiler-on-ubuntu-18-04/ ---getting gcc working
*/