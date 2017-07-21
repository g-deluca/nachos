/// Routines for synchronizing threads.
///
/// Three kinds of synchronization routines are defined here: semaphores,
/// locks and condition variables (the implementation of the last two are
/// left to the reader).
///
/// Any implementation of a synchronization routine needs some primitive
/// atomic operation.  We assume Nachos is running on a uniprocessor, and
/// thus atomicity can be provided by turning off interrupts.  While
/// interrupts are disabled, no context switch can occur, and thus the
/// current thread is guaranteed to hold the CPU throughout, until interrupts
/// are reenabled.
///
/// Because some of these routines might be called with interrupts already
/// disabled (`Semaphore::V` for one), instead of turning on interrupts at
/// the end of the atomic operation, we always simply re-set the interrupt
/// state back to its original value (whether that be disabled or enabled).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "synch.hh"
#include "system.hh"


/// Initialize a semaphore, so that it can be used for synchronization.
///
/// * `debugName` is an arbitrary name, useful for debugging.
/// * `initialValue` is the initial value of the semaphore.
Semaphore::Semaphore(const char *debugName, int initialValue)
{
    name  = debugName;
    value = initialValue;
    queue = new List<Thread *>;
}

/// De-allocate semaphore, when no longer needed.
///
/// Assume no one is still waiting on the semaphore!
Semaphore::~Semaphore()
{
    delete queue;
}

/// Wait until semaphore `value > 0`, then decrement.
///
/// Checking the value and decrementing must be done atomically, so we need
/// to disable interrupts before checking the value.
///
/// Note that `Thread::Sleep` assumes that interrupts are disabled when it is
/// called.
void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);
      // Disable interrupts.

    while (value == 0) {  // Semaphore not available.
        queue->Append(currentThread);  // So go to sleep.
        currentThread->Sleep();
    }
    value--;  // Semaphore available, consume its value.

    interrupt->SetLevel(oldLevel);  // Re-enable interrupts.
}

/// Increment semaphore value, waking up a waiter if necessary.
///
/// As with `P`, this operation must be atomic, so we need to disable
/// interrupts.  `Scheduler::ReadyToRun` assumes that threads are disabled
/// when it is called.
void
Semaphore::V()
{
    Thread   *thread;
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);

    thread = queue->Remove();
    if (thread != NULL)  // Make thread ready, consuming the `V` immediately.
        scheduler->ReadyToRun(thread);
    value++;
    interrupt->SetLevel(oldLevel);
}

/// Dummy functions -- so we can compile our later assignments.
///
/// Note -- without a correct implementation of `Condition::Wait`, the test
/// case in the network assignment will not work!

Lock::Lock(const char *debugName)
{
    Semaphore *sem_ = new Semaphore(name, 1);
    name = debugName;
    sem = sem_;
}

Lock::~Lock()
{
    delete sem;
}

void
Lock::Acquire()
{
    ASSERT(!(IsHeldByCurrentThread()));
    if(lockThread != NULL  && currentThread->GetPriority() > lockThread->GetPriority()){
        lockThread->ModifyPriority(currentThread->GetPriority());
        scheduler->ChangePriority(lockThread);
    }
    sem->P();
    lockThread = currentThread;
}

void
Lock::Release()
{
    ASSERT(IsHeldByCurrentThread());
    if (currentThread->GetPriority() > currentThread->GetRealPriority())
        scheduler->RestorePriority(currentThread);
    sem->V();
    lockThread = NULL;
}

bool
Lock::IsHeldByCurrentThread()
{
    return currentThread == lockThread;
}

Condition::Condition(const char *debugName, Lock *conditionLock)
{
    name = debugName;
    internalLock = new Lock (name);
    internalSem = new Semaphore (name, 0);
    externalLock = conditionLock;
    waiters = 0;
}

Condition::~Condition()
{
    delete internalLock;
    delete internalSem;
}


void
Condition::Wait()
{
    ASSERT(externalLock -> IsHeldByCurrentThread());

    internalLock -> Acquire();
    waiters ++;
    internalLock -> Release();
    
    externalLock -> Release();
    internalSem -> P();
    externalLock -> Acquire();
}

void
Condition::Signal()
{
    internalLock -> Acquire();
        if (waiters > 0){
            waiters--;
            internalSem -> V();
        }
    internalLock -> Release();
}

void
Condition::Broadcast()
{
    internalLock -> Acquire();
        while (waiters > 0){
            waiters--;
            internalSem -> V();
        }
    internalLock -> Release();
}

Port::Port(const char * debugName)
{
    name = debugName;
    buffer = NULL;
    lock = new Lock(name);
    esperandoRecepcion = new Condition (name, lock); 
    esperandoEnvio = new Condition (name, lock); 
}

Port::~Port()
{
    delete lock;
    delete esperandoRecepcion;
    delete esperandoEnvio;
}

bool
Port::IsBuffEmpty()
{
    return buffer == NULL;
}

void
Port::Send(int n)
{
    lock -> Acquire();
    while (!(IsBuffEmpty()))
        esperandoRecepcion -> Wait();
    if (IsBuffEmpty()) {
        buffer = &n;
        esperandoEnvio -> Signal();
        esperandoRecepcion -> Wait();
    }
    lock -> Release();
}

void
Port::Receive(int *n)
{
    lock -> Acquire();
    while (IsBuffEmpty())
        esperandoEnvio -> Wait();
    if (!(IsBuffEmpty())) {
        *n = *buffer;
        buffer = NULL;
        esperandoRecepcion -> Signal();
    }
    lock -> Release();
}
