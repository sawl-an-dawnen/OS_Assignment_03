#include <iostream>
#include <fstream>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <vector>
#include <cmath>
#include <sys/ipc.h>
#include <sys/shm.h>

using namespace std;

struct process {
    public:
        bool active = true;
        int pid = -1;
        int pf = 0;//total page frames on disk
        void print(ofstream &output) {
            output << "PROCESS: " << pid << endl;
            output << "TOTAL PAGE FRAMES ON DISK: " << pf << endl;
        }      
};

struct request {
    public:
        int pid;
        string address;
        string binaryAddr;
        int pn_bits; //number of bits representing page number
        int dis_bits;//number of bits represeting displacement
        int pn_dec;
        int dis_dec;
        void print(ofstream &output) {
        }
};

const char* hex_char_to_bin(char c)
{
    // TODO handle default / error
    switch(toupper(c))
    {
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'A': return "1010";
        case 'B': return "1011";
        case 'C': return "1100";
        case 'D': return "1101";
        case 'E': return "1110";
        case 'F': return "1111";
        default: return "";
    }
}
string hex_str_to_bin_str(const string& hex)
{
    // TODO use a loop from <algorithm> or smth
    string bin;
    for(unsigned i = 0; i != hex.length(); ++i)
       bin += hex_char_to_bin(hex[i]);
    return bin;
}
int bin_to_dec(int n)
{
    int num = n;
    int dec_value = 0;
 
    // Initializing base value to 1, i.e 2^0
    int base = 1;
 
    int temp = num;
    while (temp) {
        int last_digit = temp % 10;
        temp = temp / 10;
 
        dec_value += last_digit * base;
 
        base = base * 2;
    }
 
    return dec_value;
}

//need semaphore for page fault handler to call on the page replacement algo
const char *PRA_SEMA_NAME = "PRA";
//need semaphore for page replace algo to call on the disk driver
const char *DD_SEMA_NAME = "DD";
//need semaphore to pause PFA
const char *PFH_SEMA_NAME = "PFH";

int main(int argc, char** argv) {
    //input handler
    ifstream input(argv[1]);
    ofstream systemInfo("SystemInfo.txt");
    int systemData[7];
    vector<process> processes;
    vector<request> requests;

    //system array schema:
    //0 tp total number of page frames in main memory - page frames you can have in main memory at a time
    //1 ps page size in number of bytes - ceillogbase2 gives dispalcement/offset 
    //2 r number of page frames per process for FIFO, LRU, LRU-kth, LFU, OPT, or delta for WS
    //3 x lookahead window for OPT, X for LRU-xth or 0 for algorithms that don't use lookahead
    //4 min min free pool size;
    //5 max max free pool size;
    //6 k total number of processes
    string data;
    for (int i = 0; i < 7; i++) {
        getline(input, data);
        systemData[i] = stoi(data);
    }
    systemInfo << "---SYSTEM INFORMATION---\n";
    systemInfo << "Total page frames in main memory: " << systemData[0] << endl;
    systemInfo << "Page size: " << systemData[1] << " byte(s)\n";
    systemInfo << "Page frames per process (or delta for WS): " << systemData[2] << endl;
    systemInfo << "Lookahead/X: " << systemData[3] << endl;
    systemInfo << "Minimum free pool size: " << systemData[4] << endl;
    systemInfo << "Maximum free pool size: " << systemData[5] << endl;
    systemInfo << "Number of processes: " << systemData[6] << endl;

    systemInfo << "\n---PROCESS INFO---\n";
    for (int i = 0; i < systemData[6]; i++) {
        process temp;
        getline(input, data);
        temp.pid = stoi(data.substr(0,data.find(" ")));
        temp.pf = stoi(data.substr(data.find(" ")));//take ceillogbase2 to get the page number
        processes.push_back(temp);
        temp.print(systemInfo);
    }

    systemInfo << "\n---REQUESTS---\n";
    while (getline(input, data)) {
        request temp;
        temp.pid = stoi(data.substr(0,data.find(" ")));
        temp.address = data.substr(data.find(" ")+1);
        temp.binaryAddr = hex_str_to_bin_str(temp.address.substr(2));
        if (temp.binaryAddr == "") {
                temp.address = "";
                temp.binaryAddr = "";
                temp.pn_bits = -1;
                temp.dis_bits = -1;
                temp.dis_dec = -1;
                temp.pn_dec = -1;
                requests.push_back(temp);
            continue;
        }
        for (int i = 0; i < processes.size(); i++) {
            if(temp.pid == processes[i].pid) {
                temp.pn_bits = ceil(log2(processes[i].pf));
                temp.dis_bits = ceil(log2(systemData[1]));
                temp.dis_dec = bin_to_dec(stoi(temp.binaryAddr.substr(temp.binaryAddr.size()-temp.dis_bits)));
                temp.pn_dec = bin_to_dec(stoi(temp.binaryAddr.substr(temp.binaryAddr.size()-temp.dis_bits-temp.pn_bits,temp.pn_bits)));
                requests.push_back(temp);
            }
        }
    }
    for (int i = 0; i < requests.size(); i ++) {
        if (requests[i].pn_dec == -1) {
            systemInfo << "---REQUEST #" << i << "---" << endl;
            systemInfo << "PROCESS " << requests[i].pid << " STAGED FOR TERMINATION" << endl;
            systemInfo << endl;
            continue; 
        }
        systemInfo << "---REQUEST #" << i << "---" << endl;
        systemInfo << "PID: " << requests[i].pid << endl;
        systemInfo << "PAGE NUMBER: " << requests[i].pn_dec << endl;
        systemInfo << "DISPLACEMENT: " << requests[i].dis_dec << endl;
        systemInfo << endl;
    }
    
    key_t key_ft = ftok("shmfile",65);
    int shmid_ft;
    int* frameTable;
    int count_ft = systemData[0]*2;
    shmid_ft = shmget(key_ft, count_ft*sizeof(int), 0644|IPC_CREAT);
    frameTable = (int*)shmat(shmid_ft,0,0);
    for (int i = 0; i < count_ft; i++) {
        frameTable[i] = -1;
    }

    key_t key_pgt;
    int shmid_pgt;
    int* pageTables;
    int count_pgt = systemData[6]*systemData[2]*2;
    shmid_pgt = shmget(key_pgt, count_pgt*sizeof(int), 0644|IPC_CREAT);
    pageTables = (int*)shmat(shmid_pgt,0,0);
    for (int i = 0; i < count_pgt; i++) {
        pageTables[i] = -1;
    }

    for (int i = 0; i < systemData[6]; i++) {
        for (int j = 0; j < systemData[2]*2; j = j+2) {
            pageTables[i+j] = processes[i].pid;
        }
    }

    for (int i = 0; i < processes.size(); i++) {
        cout << pageTables[i] << " ";
    }
    cout << endl;


    key_t key_flt;
    int shmid_flt;
    int* faultTracker;
    int count_flt = systemData[6]+1;
    shmid_flt = shmget(key_flt, count_flt*sizeof(int), 0644|IPC_CREAT);
    faultTracker = (int*)shmat(shmid_flt,0,0);
    for (int i = 0; i < count_pgt; i++) {
        faultTracker[i] = 0;
    }

    key_t key_instr;
    int shmid_instr;
    int* currentInstruction;
    shmid_instr = shmget(key_instr, sizeof(int), 0644|IPC_CREAT);
    currentInstruction = (int*)shmat(shmid_instr,0,0);
    *currentInstruction = 0;

    //create processes with fork()
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
        for (int i = 0; i < requests.size(); i++)   {
            *currentInstruction = i;
            requests[i];

        }
        sem_post(PRA_SEMA);
        sem_wait(PFH_SEMA);
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
        sem_post(PFH_SEMA);
        return 0;
    }
    return 0;
}

/*
References:
1.https://github.com/MagedSaeed/vertual_memroy_manager/blob/master/main.c
2.https://linuxize.com/post/how-to-install-gcc-compiler-on-ubuntu-18-04/ 
---getting gcc working
3.https://stackoverflow.com/questions/18310952/convert-strings-between-hex-format-and-binary-format#:~:text=You%20can%20use%20a%20combination%20of%20std%3A%3Astringstream%2C%20std%3A%3Ahex,hex%20and%20binary%20in%20C%2B%2B03.%20Here%27s%20an%20example%3A?msclkid=a74ff812c6a711eca18a767da44e484b
---to get hex to binary functions
4.https://www.geeksforgeeks.org/program-binary-decimal-conversion/
---to get binary ro decimal
https://www.geeksforgeeks.org/ipc-shared-memory/
---to get shared memory learned
5.https://stackoverflow.com/questions/21227270/read-write-integer-array-into-shared-memory#:~:text=array%20%3D%20%28int%20%2A%29shmat%20%28shmid%2C%200%2C%200%29%3B%20array,you%20cant%20allocate%20it%20later%20by%20other%20means.
---shared memory array
*/