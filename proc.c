#include "os.h"

/* defined in entry.S */
extern void switch_to(struct context *next);

#define MAX_TASKS 10
#define STACK_SIZE 1024
// /*
//  * In the standard RISC-V calling convention, the stack pointer sp
//  * is always 16-byte aligned.
//  */
uint8_t __attribute__((aligned(16))) task_stack[MAX_TASKS][STACK_SIZE];
// struct context ctx_tasks[MAX_TASKS];

// /*
//  * _top is used to mark the max available position of ctx_tasks
//  * _current is used to point to the context of current task
//  */
// static int _top = 0;
// static int _current = -1;

// void sched_init()
// {
// 	w_mscratch(0);

// 	/* enable machine-mode software interrupts. */
// 	w_mie(r_mie() | MIE_MSIE);
// }

// /*
//  * implment a simple cycle FIFO schedular
//  */
// void schedule()
// {
// 	if (_top <= 0) {
// 		panic("Num of task should be greater than zero!");
// 		return;
// 	}

// 	_current = (_current + 1) % _top;
// 	struct context *next = &(ctx_tasks[_current]);
// 	switch_to(next);
// }

// /*
//  * DESCRIPTION
//  * 	Create a task.
//  * 	- start_routin: task routine entry
//  * RETURN VALUE
//  * 	0: success
//  * 	-1: if error occured
//  */
// int task_create(void (*start_routin)(void))
// {
// 	if (_top < MAX_TASKS) {
// 		ctx_tasks[_top].sp = (reg_t) &task_stack[_top][STACK_SIZE];
// 		ctx_tasks[_top].pc = (reg_t) start_routin;
// 		_top++;
// 		return 0;
// 	} else {
// 		return -1;
// 	}
// }

// /*
//  * DESCRIPTION
//  * 	task_yield()  causes the calling task to relinquish the CPU and a new 
//  * 	task gets to run.
//  */
// void task_yield()
// {
// 	/* trigger a machine-level software interrupt */
// 	int id = r_mhartid();
// 	*(uint32_t*)CLINT_MSIP(id) = 1;
// }

// /*
//  * a very rough implementaion, just to consume the cpu
//  */
// void task_delay(volatile int count)
// {
// 	count *= 50000;
// 	while (count--);
// }


//============================================================

#include "proc.h"

struct cpu cpu;

struct proc proc[MAX_TASKS];

int nextpid = 1;

void sched_init()
{
	w_mscratch(0);

	/* enable machine-mode software interrupts. */
	w_mie(r_mie() | MIE_MSIE);
}

/*
 * implment a simple cycle FIFO schedular
 */
void schedule()
{

	struct proc *p;
  	struct cpu *c = &cpu;

  	c->proc = 0;
	int t = 0;
 	for(;;){

		printf("%d\n", t++);
    	int found = 0;
    	for(p = proc; p < &proc[MAX_TASKS]; p++) {
      		if(p->state == RUNNABLE ) {
      			p->state = RUNNING;
      			c->proc = p;

				struct context *next = &(p->context);
      			switch_to(next);
				
      			c->proc = 0;
      			found = 1;
      		}
    	}
    	if(found == 0) {
      		// nothing to run; stop running on this core until an interrupt.
      		asm volatile("wfi");
    	}
  	}
}

void proc_init()
{
	struct proc *p;
	for (p = proc; p < &proc[MAX_TASKS]; p++) {
		p->state = UNUSED;
	}
}

struct proc* proc_alloc()
{
	struct proc *p;
	for(p = proc; p < &proc[MAX_TASKS]; p++) {
		if (p->state == UNUSED) {
			p->state = USED;
			p->pid = nextpid++;
			// if( (p->stack = page_alloc(1)) == NULL ) {
			// 	p->state = UNUSED;
			// 	return NULL;
			// }
			// p->stack += STACK_SIZE;
			return p;
		}
	}
	return NULL;
}

int proc_free(struct proc *p)
{
	if (p == NULL) {
		return -1;
	}
	page_free((void *)p->stack);
	p->state = UNUSED;
	return 0;
}

int task_create(void (*start_routine)(void))
{
	struct proc *p;

	p = proc_alloc();
	if (p == NULL) {
		return -1;
	}

	p->context.sp = (reg_t) &p->stack+STACK_SIZE;
	p->context.pc = (reg_t) start_routine;
	p->state = RUNNABLE;

	return 0;
}

void task_exit()
{
	proc_free(cpu.proc);
	cpu.proc = NULL;

}

/*
 * DESCRIPTION
 * 	task_yield()  causes the calling task to relinquish the CPU and a new 
 * 	task gets to run.
 */
 void task_yield()
 {
	 /* trigger a machine-level software interrupt */
	 int id = r_mhartid();
	 *(uint32_t*)CLINT_MSIP(id) = 1;
	 struct cpu *c = &cpu;
	 c->proc->state = RUNNABLE;
 }
 
 /*
  * a very rough implementaion, just to consume the cpu
  */
 void task_delay(volatile int count)
 {
	 count *= 50000;
	 while (count--);
 }