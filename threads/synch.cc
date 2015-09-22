// synch.cc
//  Routines for synchronizing threads.  Three kinds of
//  synchronization routines are defined here: semaphores, locks
//      and condition variables (the implementation of the last two
//  are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include <iostream>
#include "copyright.h"
#include "synch.h"
#include "system.h"

using namespace std;

//----------------------------------------------------------------------
// Semaphore::Semaphore
//  Initialize a semaphore, so that it can be used for synchronization.
//
//  "debugName" is an arbitrary name, useful for debugging.
//  "initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
//  De-allocate semaphore, when no longer needed.  Assume no one
//  is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
//  Wait until semaphore value > 0, then decrement.  Checking the
//  value and decrementing must be done atomically, so we
//  need to disable interrupts before checking the value.
//
//  Note that Thread::Sleep assumes that interrupts are disabled
//  when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts

    while (value == 0) {            // semaphore not available
        queue->Append((void *)currentThread);   // so go to sleep
        currentThread->Sleep();
    }
    value--;                    // semaphore available,
                        // consume its value

    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
//  Increment semaphore value, waking up a waiter if necessary.
//  As with P(), this operation must be atomic, so we need to disable
//  interrupts.  Scheduler::ReadyToRun() assumes that threads
//  are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)    // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) // constructor
{
    name = debugName; //
    lockStatus = FREE;
    lockOwner = NULL;
    waitQueue = new List;
}

Lock::~Lock() // deconstructor
{
    delete waitQueue;
}

void Lock::Acquire()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
    if(currentThread == lockOwner) //current thread is lock owner
    {
        (void) interrupt->SetLevel(oldLevel); //restore interrupts
        return;
    }

    if(lockStatus == FREE) //lock is available
    {
        //I can have the lock
        lockStatus = BUSY; //make state BUSY
        lockOwner = currentThread; //make myself the owner
    }
    else //lock is busy
    {
        waitQueue->Append(currentThread); //Put current thread on the lock’s waitQueue
        currentThread->Sleep(); //make the current thread sleep
    }

    (void) interrupt->SetLevel(oldLevel); //restore interrupts
}

void Lock::Release()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
    if(currentThread != lockOwner) //current thread is not lock owner
    {
        (void) interrupt->SetLevel(oldLevel); //restore interrupts
        return;
    }

    if(!waitQueue->IsEmpty()) //lock waitQueue is not empty
    {
        Thread* thread = (Thread*)waitQueue->Remove(); //remove 1 waiting thread
        lockOwner = thread; //make them lock owner
        scheduler->ReadyToRun(thread); //puts a thread at the back of the
                             //readyQueue in the ready state
    }
    else
    {
        lockStatus = FREE; //make lock available
        lockOwner = NULL; //unset ownership
    }

    (void) interrupt->SetLevel(oldLevel); //restore interrupts
}

void Lock::Print() // print out all of the threads waiting for this lock
{
    waitQueue->Mapcar((VoidFunctionPtr) ThreadPrint);
}

Condition::Condition(char* debugName) // constructor
{
    name = debugName;
    waitingLock = NULL;
    waitQueue = new List;
}

Condition::~Condition() // deconstructor
{
    delete waitQueue;
}

void Condition::Wait(Lock* conditionLock)
{
    //ASSERT(FALSE);

    IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
    if(conditionLock == NULL) // if the lock is non existent
    {
        (void) interrupt->SetLevel(oldLevel); //restore interrupts
        return;
    }
    if(waitingLock == NULL) // if the waitingLock has not been set yet, set which lock to wait on
    {
        //no one waiting
        waitingLock = conditionLock;
    }
    if(waitingLock != conditionLock)
    {
        (void) interrupt->SetLevel(oldLevel); //restore interrupts
        return;
    }
    //OK to wait
    waitQueue->Append(currentThread);//add myself to Condition Variable waitQueue
    conditionLock->Release();
    currentThread->Sleep(); //currentThread is put on the waitQueue
    conditionLock->Acquire();
    (void) interrupt->SetLevel(oldLevel); //restore interrupts
}

void Condition::Signal(Lock* conditionLock)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
    if(waitQueue->IsEmpty()) //no thread waiting
    {
        (void) interrupt->SetLevel(oldLevel); //restore interrupts
        return;
    }

    if(waitingLock != conditionLock) // if the locks are different, the thread can't wake up
    {
        (void) interrupt->SetLevel(oldLevel); //restore interrupts
        return;
    }

    //Wake up one waiting thread
    Thread* thread = (Thread*)waitQueue->Remove(); //Remove one thread from waitQueue
    scheduler->ReadyToRun(thread); //Put on readyQueue

    if(waitQueue->IsEmpty()) //waitQueue is empty //TODO don't know why
    {
        waitingLock = NULL;
    }

    (void) interrupt->SetLevel(oldLevel); //restore interrupts
}

void Condition::Broadcast(Lock* conditionLock)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
    if(conditionLock == NULL) //if there is no lock to signal...
    {
        (void) interrupt->SetLevel(oldLevel); //restore interrupts
        return;
    }

    if(conditionLock != waitingLock)
    {
        (void) interrupt->SetLevel(oldLevel); //restore interrupts
        return;
    }

    (void) interrupt->SetLevel(oldLevel); //restore interrupts
    while(!waitQueue->IsEmpty()) //waitQueue is not empty
    {
        Signal(conditionLock); //signal every thread to try and Acquire() the lock and ultimately get into the lock's waitQueue
    }
}
