used commands:
build
$ gcc -pthread -lrt Elijah_Salandanan.cpp -lstdc++ -o Elijah_Salandanan
run
$ ./Elijah_Salandanan Input/Assignment3_Input.txt

notes 4/25/2022
-program framework and shape worked out. fault handler calls page replacement algo, and then the page replacer calls disk to modify the shared memory space. For now I intend to
leave the disk manager process as an empty process

notes 4/26/2022
-learned how to use semaphores in a local space. I know what to study to get them to share between processes. Using the semaphore.h helped alot. I knew the concept, just not how to
get them into my code

notes 4/27/2022
-finally completley worked out how to share and use semaphores between processes 5:41PM, bruh i just want to design games why do i need semaphores

---HW INSTRUCTIONS---
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