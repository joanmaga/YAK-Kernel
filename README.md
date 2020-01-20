# YAK Kernel

Real-Time Kernel for Real-Time Operating Systems Class.

## Content:

* yak* Files - Kernel Code Functionality. Includes:
    1. yaks.s - Dispatcher, Context-Switching, and Mutex Functionality
    2. yaku.h - Constants used in Kernel
    3. yakc.c - Main Kernel File. Includes Scheduling, Dispatching, Event Handling, Semaphore Handling, and Queues
* isr.s - Interrupt Service Routines.
* clib.* Files - Contains functionality provided by Instructor, as well as the Interrupt Vector Table.
* lab7app.c - User Code provided by Instructor for testing.