#ifndef __PROCESS_H
#define __PROCESS_H

#include<sys/defs.h>
#include<sys/page.h>
#include<sys/gdt.h>
#include<sys/syscall.h>

#define KERNBASE 0xffffffff80000000ul
#define NPROC 30000				/* maximum number of processes */
#define KSTACKSIZE 4096			/* size of per-process kernel stack */
#define NOFILE 16				/* open files per process */
#define MAXARG 32				/* max exec arguments */
#define NFILE 100				/* maximum files open in entire system */
#define NCHARS 100				/* maximum inode childrene */
#define NLINKS 30
#define USTACK 0xfffffeff70000000ul /* maps to 509 entry of pml4, can be anything as long as not 511 entry */
#define STACK_THRESH 0x10000
#define ARGV_SIZE 200

#define FL_IF 0x0000000000000200 /* interrupt enable */

#define GDT_CS        (0x00180000000000)  /*** code segment descriptor ***/
#define GDT_DS        (0x00100000000000)  /*** data segment descriptor ***/

#define C             (0x00040000000000)  /*** conforming ***/
#define DPL0          (0x00000000000000)  /*** descriptor privilege level 0 ***/
#define DPL1          (0x00200000000000)  /*** descriptor privilege level 1 ***/
#define DPL2          (0x00400000000000)  /*** descriptor privilege level 2 ***/
#define DPL3          (0x00600000000000)  /*** descriptor privilege level 3 ***/
#define P             (0x00800000000000)  /*** present ***/
#define L             (0x20000000000000)  /*** long mode ***/
#define D             (0x40000000000000)  /*** default op size ***/
#define W             (0x00020000000000)  /*** writable data segment ***/


struct proc *initproc;
struct proc *proc;

uint64_t SEG_KCS();
uint64_t SEG_KDS();
uint64_t SEG_UCS();
uint64_t SEG_UDS();

uint32_t proc_count;


struct cpu{

	struct context *scheduler;
	/* struct taskstate ts; */
};


/* process states */
enum procstate{

	UNUSED,
	EMBRYO,
	SLEEPING,
	RUNNABLE,
	RUNNING,
	ZOMBIE
};

struct context{
	uint64_t rbx;
	uint64_t rbp;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rip;

};

struct trapframe{
/* trapframe generated by isr_common */
	uint64_t rbp;
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;

/* change in idt.c */
	/* uint64_t gs; */
	/* uint64_t fs; */
	/* uint64_t es; */
	/* uint64_t ds; */

	uint64_t intr_num;
	uint64_t error_code;

	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
};

/* PIPE */
#define PIPESIZE 0x1000

struct pipe{
	char data[PIPESIZE];
	uint64_t nread;				/* number of bytes read */
	uint64_t nwrite;			/* number of bytes written */
	int readopen;				/* read fd is still open */
	int writeopen;				/* write fd is still open */
};



/* FILE */

struct file{
	enum{FD_NONE,FD_PIPE,FD_INODE,FD_STDIN,FD_STDOUT,FD_STDERR,FD_DIR,FD_FILE} type; /* type of file */
	int ref;					/* refernce count */
	char readable;				/* file is readable?? */
	char writable;				/* file is writable?? */
	uint64_t *addr;				/* pointer to the first byte of content, directory->first directory tarfs hdr, file, exec->first content byte */
	struct pipe *pipe;			/* pointer to pipe struct */
	int64_t offset; 			/* stores the offset */
	int64_t size;				/* size of file, for do_read on file */
	char inode_name[NCHARS];	/* name of directory or file */
};
struct file * filealloc();
int pipealloc(struct file **f0,struct file **f1);
void pipeclose(struct pipe *p, int writable);
int pipewrite(struct pipe *p, char *addr, size_t n);
int piperead(struct pipe *p, char *addr, size_t n);
int fdalloc(struct file *f);
void fileclose(struct file *f);
struct file * filedup(struct file *f);

int add_root(struct file *fd,int mustBeEmpty,char *buf,size_t len);
int add_non_root(struct file *fd,char *buf,size_t len);
/* stdin */

struct proc *fgproc;

struct read_proc{
	struct proc *proc;
	size_t count;
	void *buf;
};

struct read_proc *_stdin;

#define TERMBUF_SIZE 1024		/* size of termbuf  */
char *termbuf;					/* _stdin's kernel buffer. allocated in userinit. size 4KB */
char *termbuf_head,*termbuf_tail;
int isBufFull;					/* flags wehther termbuf is full */
void add_buf(char c);
void do_copy();
void init_stdin_queue();
char *read_kstack;

/* PCB - process control block */
struct proc{
	uint64_t size;				/* process memory size in bytes */
	pml4 *pml4_t;				  /* pointer to pml4. */
	char *kstack;				  /* pointer to start of stack (i.e bottom of stack) */
	enum procstate state;		  /* process state */
	int pid;					  /* process identifiers */
	struct proc *parent;		  /* parent of process */
	struct trapframe *tf;		  /* stack trapframe */
	struct context *context;	  /* kernel stack */
	void *chan;					  /* if non-zero, sleeping on chan */
	int killed;					  /* if non-zero then killed */
	/* struct file *ofile[];		  /\* list of open files *\/ */
	char name[32];				  /* process name */
	struct vma *vma_head ;		  /* pointer to the first VMA */
	struct file *ofile[NOFILE];	  /* pointers to file structs. */
	char cwd[NCHARS];			  /* name of cwd */
	
};

void swtch(struct context **cpu,struct context *new );


/* inode */
struct inode{
    char name[NCHARS];
	struct inode *link[NLINKS];
};


/* VMA */

#define PF_GROWSUP 0x10			/* custom def, for heap */
#define PF_GROWSDOWN 0x8		/* custom def, for stack */
#define PF_R 0x4				/* from elf */
#define PF_W 0x2				/* from elf */
#define PF_X 0x1				/* from elf man page */


struct vma{
	uint64_t start;				/* start  virt address */
	uint64_t end;				/* one byte after the end , virt address*/
	uint32_t flags;				/* read, write, exec, grow */
	struct vma *next;			/* ptr to next vma. NULL is end */
};


void init_inodes();


uint64_t get_virt_addr(uint64_t x);

uint64_t get_phys_addr(uint64_t x);

void userinit();
struct proc * alloc_proc();
void forkret();

int valid_addr(uint64_t addr);
int valid_addr_range(uint64_t addr, uint64_t size);

pml4 *load_kern_vm();
void scheduler();
void sched();
void switchuvm(struct proc *p);
void wakeup1(void *chan);
void wakeup(void *chan);
void sleep(void *chan);
void set_wp_bit();
void clear_wp_bit();

/* VMA related */
void free_vma_list(struct vma **p);

/* free pcb */
void free_pcb(struct proc *p);

/* sleep */
struct sleep_entry{
	struct proc *proc;
	struct timespec rem;
	struct sleep_entry *next;
};

struct sleep_entry*  sleep_head;
struct sleep_entry*  sleep_tail;

/* waitpid */
struct waitpid_entry{
  struct proc *parent_proc;
  pid_t pid;
  struct waitpid_entry *next;
};

struct waitpid_entry*  waitpid_head;
struct waitpid_entry*  waitpid_tail;

void init_sleep_queue();
void update_sleep_queue();
int enqueue_sleep(struct proc *p,struct timespec *rem);
void dequeue_sleep(struct sleep_entry *p);

void init_waitpid_queue();
int enqueue_waitpid(struct proc *p, int pid);
void update_waitpid_queue(struct proc *p);
void dequeue_waitpid(struct waitpid_entry *p);

void cli();
void sti();
void ltr(uint16_t v);

void do_ps();
int do_kill(int pid);
#endif
