#include "os.h"

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct proc {
    enum procstate state;          // Process state
    int pid;                       // Process ID
    uint8_t __attribute__((aligned(16))) stack[1024];                // stack for this process
    // uint64_t sz;                   // Size of process memory (bytes)
    struct context context;        // Switch here to run process
    char name[16];                 // Process name (debugging)
};

// Per-CPU state
struct cpu {
    struct proc *proc;          // The process running on this cpu, or null.
    // struct context context;     // Switch here to enter scheduler()
    // int noff;                   // Depth of push_off() nesting.
    // int intena;                 // Were interrupts enabled before push_off()?
};

extern struct cpu cpu;