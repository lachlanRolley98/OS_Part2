Introduction
In this assignment you will be implementing a software bridge between a set of file-related system calls inside the OS/161 kernel and their implementation within the VFS (obviously also inside the kernel). Upon completion, your operating system will be able to run a single application at user-level and perform some basic file I/O.

A substantial part of this assignment is understanding how OS/161 works and determining what code is required to implement the required functionality. Expect to spend at least as long browsing and digesting OS/161 code as actually writing and debugging your own code.

If you attempt the advanced part, you will add process related system calls and the ability to run multiple applications.

Your current OS/161 system has minimal support for running executables, nothing that could be considered a true process. Assignment 2 starts the transformation of OS/161 into something closer to a true operating system. After this assignment, OS/161 will be capable of running a process from actual compiled programs stored in your account. The program will be loaded into OS/161 and executed in user mode by System/161; this will occur under the control of your kernel. First, however, you must implement part of the interface between user-mode programs ("userland") and the kernel. As usual, we provide part of the code you will need. Your job is to design and build the missing pieces.

The code can run one user-level C program at a time as long as it doesn't want to do anything but shut the system down. We have provided sample user programs that do this (reboot, halt, poweroff), as well as others that make use of features you might be adding in this and future assignments. So far, all the code you have written for OS/161 has only been run within, and only been used by, the operating system kernel itself. In a real operating system, the kernel's main function is to provide support for user-level programs. Most such support is accessed via "system calls". We give you two system call implementations: sys_reboot() in main/main.c and sys___time() in syscall/time_syscalls.c. In GDB, if you put a breakpoint on sys_reboot() and run the "reboot" program, you can use "backtrace" (or "where") to see how it got there.

User-level programs
Our System/161 simulator can run normal C programs if they are compiled with a cross-compiler, os161-gcc. A cross compiler runs on a host (e.g., a Linux x86 machine) and produces MIPS executables; it is the same compiler used to compile the OS/161 kernel. Various user-level programs already exist in userland/bin, userland/testbin, and userland/sbin. Note: that only a small subset these programs will execute successfully due to OS/161 only supporting a small subset of the system call interface.

To create new user programs (for testing purposes), you need to edit the Makefile in bin, sbin, or testbin (depending on where you put your programs) and then create a directory similar to those that already exist. Use an existing program and its Makefile as a template.

Design
In the beginning, you should tackle this assignment by producing a DESIGN. The design should clearly reflect the development of your solution. The design should not merely be what you programmed. If you try to code first and design later, or even if you design hastily and rush into coding, you will most certainly end up in a software "tar pit". Don't do it! Plan everything you will do. Don't even think about coding until you can precisely explain to your partner what problems you need to solve and how the pieces relate to each other. Note that it can often be hard to write (or talk) about new software design, you are facing problems that you have not seen before, and therefore even finding terminology to describe your ideas can be difficult. There is no magic solution to this problem; but it gets easier with practice. The important thing is to go ahead and try. Always try to describe your ideas and designs to someone else. In order to reach an understanding, you may have to invent terminology and notation, this is fine. If you do this, by the time you have completed your design, you will find that you have the ability to efficiently discuss problems that you have never seen before. Why do you think that CS is filled with so much jargon? To help you get started, we have provided the following questions as a guide for reading through the code to comprehend what is already provided.

To get a feel for what problems you need to solve, review the design questions and design document section later in this specification.

Existing Code Walkthrough
A guided walkthrough of the relevant code base is available here.

This walkthrough complements the existing ASST2 video . There are answers available on the course wiki . Additionally, if you have any further queries, use one of the week 6 consultations.

Basic Assignment
Setup
We assume after ASST0 and ASST1 that you now have some familiarity with setting up for OS/161 development. If you need more detail, refer back to ASST0.

Clone the ASST2 source repository from gitlab.cse.unsw.edu.au, replacing the XXX with your 3 digit group number:

% cd ~/cs3231
% git clone https://zNNNNNNN@gitlab.cse.unsw.edu.au/COMP3231/20T2/grpXXX-asst2.git asst2-src
Note: The gitlab repository is shared between you and your partner. You can both push and pull changes to and from the repository to cooperate on the assignment. If you are not familiar with cooperative software development and git you should consider spending a little time familiarising yourself with git.

Building and Testing the Provided Code
Configure OS/161 for Assignment 2
Before proceeding further, configure your new sources:

% cd ~/cs3231/asst2-src
% ./configure
Unlike previous the previous assignment, you will need to build and install the user-level programs that will be run by your kernel in this assignment:

% cd ~/cs3231/asst2-src
% bmake
% bmake install
For your kernel development, again we have provided you with a framework for you to run your solutions for ASST2. You have to reconfigure your kernel before you can use this framework. The procedure for configuring a kernel is the same as in ASST0 and ASST1, except you will use the ASST2 configuration file:

% cd ~/cs3231/asst2-src/kern/conf
% ./config ASST2
You should now see an ASST2 directory in the compile directory.

Building for ASST2
When you built OS/161 for ASST1, you ran make from compile/ASST1. In ASST2, you run make from (you guessed it) compile/ASST2:

% cd ../compile/ASST2
% bmake depend
% bmake
% bmake install
If you are told that the compile/ASST2 directory does not exist, make sure you ran config for ASST2.

Command Line Arguments to OS/161
Your solutions to ASST2 will be tested by running OS/161 with command line arguments that correspond to the menu options in the OS/161 boot menu. IMPORTANT: Please DO NOT change these menu option strings!

Running "asst2"
For this assignment, we have supplied a user-level OS/161 program that you can use for testing. It is called asst2, and its sources live in src/testbin/asst2. You can test your assignment by typing p /testbin/asst2 at the OS/161 menu prompt. As a shortcut, you can also specify menu arguments on the command line, example: sys161 kernel "p /testbin/asst2".

Note: If you don't have a sys161.conf file, you can use the one from ASST1.

The simplest way to install it is as follows:

% cd ~/cs3231/root
% wget http://cgi.cse.unsw.edu.au/~cs3231/21T1/assignments/asst2/sys161.conf
Running the program produces output similar to the following prior to starting the assignment:

Unknown syscall 55
Unknown syscall 55
Unknown syscall 55
Unknown syscall 55
:
:
Unknown syscall 55
Unknown syscall 55
Unknown syscall 3
exit() was called, but it's unimplemented.
This is expected if your user-level program has finished.
panic: Can't continue further until sys_exit() is implemented
asst2 produces the following output on a (maybe partially) working assignment:

OS/161 kernel [? for menu]: p /testbin/asst2
Operation took 0.000212160 seconds
OS/161 kernel [? for menu]:

**********
* File Tester
**********
* write() works for stdout
**********
* write() works for stderr
**********
* opening new file "test.file"
* open() got fd 3
* writing test string
* wrote 45 bytes
* writing test string again
* wrote 45 bytes
* closing file
**********
* opening old file "test.file"
* open() got fd 3
* reading entire file into buffer
* attempting read of 500 bytes
* read 90 bytes
* attempting read of 410 bytes
* read 0 bytes
* reading complete
* file content okay
**********
* testing lseek
* reading 10 bytes of file into buffer
* attempting read of 10 bytes
* read 10 bytes
* reading complete
* file lseek  okay
* closing file
exit() was called, but it's unimplemented.
This is expected if your user-level program has finished.
panic: Can't continue further until sys_exit() is implemented
Note that the final panic is expected, and is due to exit() (system call 3) not being implemented completely by OS/161. Implementing exit() is part of the advanced assignment.

The Assignment Task: File System Calls
Of the full range of system calls that is listed in kern/include/kern/syscall.h, your task is to implement the following file-based system calls: open, read, write, lseek, close, dup2, and document your design. Note: You will be writing the kernel code that implements part of the system call functionality within the kernel. You are not writing the C stubs that user-level applications call to invoke the system calls. The userland stubs are automatically generated when you build OS/161 in build/userland/lib/libc/syscalls.S which you should not modify.

It's crucial that your syscalls handle all error conditions gracefully (i.e., without crashing OS/161.) You should consult the OS/161 man pages (also included in the distribution) and understand the system calls that you must implement. Your system calls must return the correct value (in case of success) or error code (in case of failure) as specified in the man pages. Some of the auto-marking scripts rely on the return of error codes, however, we are lenient as to the specific code in the case of potential ambiguity as to which error code would be most appropriate. It's also not necessary to generate all error codes listed in the man pages.

The file userland/include/unistd.h contains the user-level interface definition of the system calls. This interface is different from that of the kernel functions that you will define to implement these calls. You need to design the kernel side of this interface. The function prototype for your interface can be put it in kern/include/syscall.h. The integer codes for the calls are defined in kern/include/kern/syscall.h.

Notes on the file system system calls
open(), read(), write(), lseek(), close(), and dup2()

While this assignment requires you to implement file-system-related system calls, you actually have to write virtually no low-level file system code in this assignment. You will use the existing VFS layer to do most of the work. Your job is to construct the subsystem that implements the interface expected by userland programs by invoking the appropriate VFS and vnode operations.

Although these system calls may seem to be tied to the filesystem, in fact, these system calls are really about manipulation of file descriptors, or filesystem state. A large part of this assignment is designing and implementing a system to track this state.

Some of this state is specific to a process and file descriptor (i.e. one can expect to extend OS/161 with a per-process file descriptor data structure), and some information is shared across multiple processes (e.g. an open file table). Don't rush this design. Think carefully about the state you need to maintain, how to organise it, and when and how it has to change.

You need to think about a variety of issues associated with implementing system calls. Perhaps, the most obvious one is: can two different user-level processes find themselves running a system call at the same time? If so, what are data structures are private to each process, what are shared (and thus have concurrency issues in a complete system).

Note that the basic assignment does not involve implementing fork() (that's part of the advanced assignment). So in regard to concurrency, you can assume only a single process runs at a time. You should NOT (need to) synchronise any data structures you add for the basic assignment.

However, the design and implementation of your system calls should not assume only a single process will ever exist at a time. It should be possible to add a fork() implementation to your system call implementation, and then only synchronise your existing design to handle the concurrency.

While you are not restricted to only modifying these files, please place most of your implementation in the following files: function prototypes and data types for your file subsystem in kern/include/syscall.h or kern/include/file.h , and the function implementations and variable instantiations in kern/syscall/file.c .

Notes on standard file descriptors
For any given process, the first file descriptors (0, 1, and 2) are considered to be standard input (stdin), standard output (stdout), and standard error (stderr) respectively. For this basic assignment, the file descriptors 1 (stdout) and 2 (stderr) must start out attached to the console device ("con:"), 0 (stdin) can be left unattached. You will probably modify runprogram() to achieve this. Your implementation must allow programs to use dup2() to change stdin, stdout, stderr to point elsewhere.

Some Design Questions
Here are some additional questions and issues to aid you in developing your design. They are by no means comprehensive, but they are a reasonable place to start developing your solution. What primitive operations exist to support the transfer of data to and from kernel space? You will need to "bullet-proof" the OS/161 kernel from user program errors. There should be nothing a user program can do to crash the operating system when invoking the file system calls. It is okay in the basic assignment for the kernel to perform a controlled panic for an unimplemented system call (e.g. exit()), or a user-level program error. It is not okay for the kernel to crash due to user-program invoking your system calls with erroneous arguments. Decide which functions you need to change and which structures you may need to create to implement the system calls. How you will keep track of open files? For which system calls is this useful? For additional background, consult one or more of the following texts for details how similar existing operating systems structure their file system management:

Section 10.6.3 and "NFS implementation" in Section 10.6.4, Tannenbaum, Modern Operating Systems.
Section 6.4 and Section 6.5, McKusick et al., The Design and Implementation of the 4.4 BSD Operating System.
Chapter 8, Vahalia, Unix Internals: the new frontiers.
The original VFS paper is available here
Documenting your solution
This is a compulsory component of this assignment. You must submit a small design document identifying the basic issues in this assignment and describes your solution to the problems you have identified. The design document could be based on what developed in the planning phase. The document must be plain ASCII text. We expect such a document to be roughly 500 to 1000 words, i.e. clear and to the point. The document will be used to guide our markers in their evaluation of your solution to the assignment. If you can't describe your own solution clearly, you can't expect us to reverse engineer the code to a poor and complex solution to the assignment. Place your design document in asst2-design.txt (which we have created for you) at the top of the source tree to OS/161 (i.e. in ~/cs3231/asst2-src/asst2-design.txt).

When you later commit your changes into your repository, your design doc will be included in the commit, and later in your submission. Also, please word wrap you design doc if your have not already done so. You can use the unix fmt command to achieve this if your editor cannot.

A marker should be able to answer the following questions from your design document.

What data structures have you added and what function do they perform?
What are any significant issues surround managing the data structures and state do they contain?
What data structures are per-process and what structures are shared between processes?
Are there any issues related to transferring data to and from applications?
If fork() was implemented, what concurrency issues would be introduced to your implementation?
FAQ, Gotchas, and Video
See https://wiki.cse.unsw.edu.au/cs3231cgi/2021t1/Asst2 for an up to date list of potential issues you might encounter.

There is also an overview video on the assignment available on the lectures page in the course account http://cgi.cse.unsw.edu.au/~cs3231/lectures.php.
