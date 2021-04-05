#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <kern/stat.h>
#include <kern/seek.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <synch.h>
#include <vfs.h>
#include <vnode.h>
#include <file.h>
#include <syscall.h>
#include <copyinout.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define OPEN_MAX __OPEN_MAX  //cbs to remember to write __OPEN_MAX
#define NAME_MAX __NAME_MAX

static int checkFD(int fd) {
    int i = 0;
    if(fd < 0 ){
        i = 1;
    }
    if(curproc->FileTable[fd] == NULL){
        i = 1;
    }
    if(fd >= OPEN_MAX){
        i = 1;
    } 
    return i;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////            MAKE FP and OP FUNCTIONS AND EDIT THEM                    /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//Create the filepointer. Just copied off gitlab
//DONT FORGET WHEN WE ADD SEMAPHORE TO FP TO CREATE THE SEMAPHORE HERE
FP *newFP(int flags) {      //
    FP *fp = kmalloc(sizeof *fp); // this needs to be kmalloced so in kernel
    fp->pos = 0;   // always start from 0
    flags = flags & O_ACCMODE;

    //flag stuff
    if (flags == O_RDONLY) {
        fp->write = 0;
        fp->read = 1;
    }
     else if(flags == O_WRONLY) {
        fp->read = 0;
        fp->write = 1;
    }
    else{
        fp->read = 1;
        fp->write = 1;
    }

    return fp;
}


//Make OP
//DONT FORGET WHEN WE ADD SEMAPHORE TO OP TO CREATE THE SEMAPHORE HERE
OP *newOP(FP *fp, struct vnode *vnode) {
    OP *op = kmalloc(sizeof *op);
    op->ref_count = 1; //when we make theres only gona be 1. When we dup, gota increse this
    op->fp = fp;
    op->vnode = vnode;
    return op;
}

//Free FP 
// probs check if valid fP Later
//good op to make function that checks is a fp is valid later
void freeFP(FP *fp) {
    kfree(fp);
}

//Free OP
//Add in checks later to make sure its valid
void freeOP(OP *op) {
    freeFP(op->fp);
    kfree(op);
}

// inc ref count to OP
//YO When we add semaphores, gona have to use here cos gona be crit reg
// like 2 proccess can inc and will both go   (1 + 1 = 2 / 1 + 1 = 2) = 2 instead of 1 + 1 + 1 = 3;
void upRef(OP *op) { 
    op->ref_count++; 
}

// dec ref count to OP
//YO When we add semaphores, gona have to use here cos gona be crit reg
//Also we wana know if there is anything looking at this OP anymore so return int of how many left
//This matters for close cos if this return 0, we also free OP
int downRef(OP *op) {
    op->ref_count--;
    int x = op->ref_count;
    return x;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////      ALL OUR SYS_@@@@@ STUFF     //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//this bad boys open, 
//basically let the vfs do everything but we gota use that copy instruc they said in lectures for security with buffer locations/size
//we gona open or make a file and return a fd. 
//if open is successful, gota put all the info in the fdTable ->find lowest fd too
//if result from vfs_open isnt 0 we got a problem so return that error code
//basically follow guidlines to a tee and should be sweet, worried about differentitaing error messages thou
//use names shown in lectures, IDK if somthings gona get mad if we dont follow convention or its just human convention for other to read

int sys_open(const char *filename, int flags, mode_t mode) {


    struct vnode *vnode;   //vfs open makes the vnode, we just wana grab this reference to it so we know
    char path[NAME_MAX];   //needed in copyinst
    size_t size;           //needed in copyinst
    
    int res = copyinstr((userptr_t)filename, path, NAME_MAX, &size);      //tested
    if (res != 0) {     // if this isnt 0 we got a problem, copyinstr failed
        return -res;    // idk why return neg, copied off friend
    }
    
    //copyinstr worked
    
    res = vfs_open(path, flags, mode, &vnode);    //tested
    if(res != 0) {          //returns < 0 if fail, cant actually open file, can be for heaps of reasons. Mby return the legit res number ?
        return -ENODEV; // check this later, think its file path doesnt exist cos if file doesnt exist it just makes new one
    }

    //didnt return so we chilling
    //need to make fp first to pass into OP
    //then we find a index on the fdTable to put this op
    FP *Fp = newFP(flags);
    OP *Op = newOP(Fp, vnode); // chuck her in here ( They are already initilised with offsets and stuff)

    //since 1,2 are always linked, can skip checking
    int Fd = 0;   // This is like a check to make sure Fd gets set to somthing bigger than 2
    int j = 3;
    while(j < OPEN_MAX){
        if(curproc->FileTable[j] == NULL){
            break;
        }
        j++;
    }   
    Fd = j;
    //only problem we care about is too many open or cant open. Cant open is handled above
    if (Fd == OPEN_MAX){
        return -EMFILE;
    } 

    //put in fdTable and return
    curproc->FileTable[Fd] = Op; 
    
    return Fd;
}



//close just has to close a file -> dont forget about decrementing reference count in the OP
// if reference count is 0 (cos mby diff fd in diff processes looking at it) also gota free the OP
int sys_close(int fd) {

    /* check if the fd is valid */
    if(checkFD(fd) != 0){
        return EBADF;
    } 
    
    
    //basically we got this fd which is a reference in the fdTable (make sure we using the fdTable in curproc)
    //once we got this op we need the vnode it refers too to close in vfs_close so grab
    OP *op = curproc->FileTable[fd];
    struct vnode* vnode = op->vnode; 
    
    //first u wana dec ref count to see if anyone else is looking at the op
    int a = downRef(op);
    //now u get to close it
    vfs_close(vnode);

    //clear it from the fd table
    curproc->FileTable[fd] = NULL;

    //Check if we clear from OP we got this ref_count from downRef. Only free it when no one is referencing it
    if (a == 0){
        freeOP(op);
    } 
    
    return 0;   // this means we g
}

//heaps easy, just grab reference to op pointed to by a existing fd and make a new fd that contains it too.
// couple checks you gota go through but still. 
//dont forget reference count
int sys_dup2(int oldfd, int newfd) {
    
    //make sure the Fds are good
    
    if(checkFD(oldfd) > 0){ // checks its a real fd
        return -EBADF;
    }
    if(newfd < 0 ){   // Do we have to check if its not NULL ? mby can just strait override current fd
        return -EBADF;   // if we are overriding, gona have to decrement OP stuff cos its loosing a fd
    }
    if(newfd > OPEN_MAX ){   // Do we have to check if its not NULL ? mby can just strait override current fd
        return -EBADF;   // if we are overriding, gona have to decrement OP stuff cos its loosing a fd
    }
    

    // lets get this bread
    OP *newop = curproc->FileTable[newfd];
    OP *oldop = curproc->FileTable[oldfd];
    struct vnode *vnode = curproc->FileTable[oldfd]->vnode;
    

    //This is making sure we arnt just overwritting existing stuff. Gota cleanly close it
    if (newop != NULL) {
        
        int result = sys_close(newfd);   // this will do all the decrements for us. Just a strait up close call
        if (result) {
            return -result;  // closing failed
        }    
    } 

    /* Increase both ref_counts */
    upRef(oldop);
    VOP_INCREF(vnode);
    
    /* Make them point to the same file */
    curproc->FileTable[newfd] = oldop;
    return newfd;
}



//Read has problems with concurrency so gota use semaphores, crit region is basically when two process wana read same thing
//they will update the fd offset and they other wont know so semaphore basically before VOP_read and V() after fp->pos is updated
// Also it will return how much it read or a negative error
ssize_t sys_read(int fd, void *buf, size_t buflen) {
   
    if(checkFD(fd)) return -EBADF;
    
    //grab the importaint shit needed
	
    OP *op = curproc->FileTable[fd];
    struct vnode* vnode = op->vnode; 
    FP *fp = op->fp;

    //might not be alowed to read
    if(fp->read == 0) return -EBADF;

   
    //FROM ASST2 LECTURE, we need this stuff to safly handle buffers cos kern expects perfection baby
    struct iovec iov;
    struct uio uio;
    uio_uinit(&iov, &uio, (userptr_t)buf, buflen, fp->pos, UIO_READ); //copied from lecture
    

    //do it
    int res = VOP_READ(vnode, &uio);  // basically if it cant read anything, its gona spike an error. 0 if all g
    if(res) {
        
        return -res;
    }

    
    //figure out how much has been read
    size_t amountRead = buflen - getRemaining(&uio);

    //need to change the offset in fp
    fp->pos += amountRead;

    
    return amountRead;
}


//write basically grabs a fd and writes to it. This is also dangerous for conncurrency as you can write over stuff
//so same as read, we care about the fd->offset stuff so use semaphores P() before the VOP_write and V() after the offsets been mofified
// it should return neg error or amount written
ssize_t sys_write(int fd, const void *buf, size_t nbytes) {
    
    if(checkFD(fd)) return -EBADF;

    

    OP *op = curproc->FileTable[fd];
    FP *fp = op->fp;

    //need to be alowed to write
    if(fp->write == 0) return -EBADF;

    
    struct iovec iov;
    struct uio uio;
    uio_uinit(&iov, &uio, (userptr_t)buf, nbytes, fp->pos, UIO_WRITE);
    
    /* Write to the file */
    struct vnode* vnode = op->vnode; 
    int result = VOP_WRITE(vnode, &uio);  
    if(result) {
        return -result;
    }

    
   
    size_t bytes_written = nbytes - getRemaining(&uio); // this how much written

    
    fp->pos += bytes_written;

  
    return bytes_written;
}






//This guy will have huuuge conncurrency problems so we would use semaphores again. 
//Chuck it before u change fp->pos and give back pre much at the end
off_t sys_lseek(int fd, off_t pos, int whence) {

    //standard checks by now
    if(checkFD(fd)) return -EBADF;
    if(pos < 0) return -EINVAL;

    //grab OP for the final time !!!!!!!
    OP *op = curproc->FileTable[fd];
    FP *fp = op->fp;
    struct vnode* vnode = op->vnode; //need for VOP_STAT

   

   
    struct stat Vstat;
    int result = VOP_STAT(vnode, &Vstat); 
    if(result){
        return -result;
    } 

    struct stat *statpt = &Vstat;
    off_t eof = statpt->st_size;  


    // importain thing baout lseek is whence cos it changes type of seek like end or start
    //This is where ud chuck the semaphore P()
    
    if(whence == SEEK_SET){
        fp->pos = pos;
        result = fp->pos;
    }
    else if(whence == SEEK_CUR){
        fp->pos += pos;
        result = fp->pos;
    } 
    else if(whence == SEEK_END){
        fp->pos = eof + pos;
        result = fp->pos;
    }
    else {
        result = -EINVAL;
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
