/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _SYSCALL_H_
#define _SYSCALL_H_


#include <cdefs.h> /* for __DEAD */
struct trapframe; /* from <machine/trapframe.h> */

/*
 * The system call dispatcher.
 */

void syscall(struct trapframe *tf);

/*
 * Support functions.
 */

/* Helper for fork(). You write this. */   //do this after everything done, then we add concurrency stuff. basically just add semaphores to the fP and OP 
//and use them in crit regions where things are incremented like fp pos or reference numbers in OP
void enter_forked_process(struct trapframe *tf);

/* Enter user mode. Does not return. */
__DEAD void enter_new_process(int argc, userptr_t argv, userptr_t env,
		       vaddr_t stackptr, vaddr_t entrypoint);


/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);
int sys___time(userptr_t user_seconds, userptr_t user_nanoseconds);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////            MAKE FP and OP FUNCTIONS AND EDIT THEM                    /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//this is a file pointer. Just rope copied off some guy on gitlab
//the importaint thing is pos for the offset and flag

//when we do the concurrency stuff, gota chuck a semaphore in here too but can add later
typedef struct filePointer{
    unsigned int     read;         
    unsigned int     write;   
    off_t            pos;           
} FP;


//This openfile structure just got off gitlab too
//needs to hold the file pointer aswell as a vnote pointer
//needs ref count
//need to put semaphore in later when doing concurrency

typedef struct openfile {
    FP               *fp;          
    int              ref_count;    
    struct vnode     *vnode;       
} OP;


// headers for stuff in file.c
FP *newFP(int flag);
OP *newOP(FP *fp, struct vnode *vnode);

void freeFP(FP *fp);
void freeOP(OP *op);

void upRef(OP *op);
int  downRef(OP *op);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////      ALL OUR SYS_@@@@@ STUFF     //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//all the shit we are making in file.c
int sys_open(const char *filename, int flags, mode_t mode); 
int sys_close(int fd);
int sys_dup2(int oldfd, int newfd);
ssize_t sys_write(int fd, const void *buf, size_t nbytes);
ssize_t sys_read(int fd, void *buf, size_t buflen);
off_t sys_lseek(int fd, off_t pos, int whence); //whence is where you start





#endif /* _SYSCALL_H_ */
