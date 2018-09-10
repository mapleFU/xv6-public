#ifndef PROCESS_H
#define PROCESS_H

// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct proc *proc;           // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int ncpu;

// self defined users
#define MAX_USERINFO_LENGTH 3


#define CurrentUsers 3
extern struct user system_users[CurrentUsers];
extern void initize_sys_users();
// current user in the program
struct user {
  char *username;
  char *password;
};

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

#include "process.h"

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap

// Per-process state
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)


  // self defined fields
  int alarmticks;
  int alarmstartticks;
  void (*alarmhandler)();
  
  int current_level;

};

struct user* current_user;

// data structure for mlfq schedule

#define QUNUE_NUM 4
typedef struct {
  uint que_datas[QUNUE_NUM][NPROC];
  int  tick_remainings[QUNUE_NUM][NPROC];
  // struct spinlock lock;
} Mlfq_queue;

void enqueue(Mlfq_queue*, uint pid);
int level(Mlfq_queue*, uint pid);
int downgrade(Mlfq_queue* queue, uint pid);
void initgrade(Mlfq_queue*, uint pid);
void add_proc(Mlfq_queue*, uint pid);
void remove(Mlfq_queue*, uint);

extern Mlfq_queue mlfq_queue;
// // tic tac time for mlfq.
// const int tick_times[QUNUE_NUM];

#define USE_QUEUE 1
#define USE_OLD_XV6 (!USE_QUEUE) 


struct proc_table {
  struct spinlock lock;
  struct proc proc[NPROC];
};


extern struct proc_table ptable;



#endif // !PROCESS_H
