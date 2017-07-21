/// Simple test case for the threads assignment.
///
/// Create several threads, and have them context switch back and forth
/// between themselves by calling `Thread::Yield`, to illustrate the inner
/// workings of the thread system.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#define NUM_THREADS 5

#include "system.hh"
#include "synch.hh"
#include <unistd.h>
 
#ifdef DEADLOCK_TEST
Semaphore *blisto = new Semaphore("blisto", 0);
Semaphore *blisto2 = new Semaphore("blisto2", 0);
Lock *lock = new Lock("lock"); 
#endif

#ifdef LOCK_TEST
Lock *lock;
#endif

#ifdef COND_TEST
Port *port; 
#endif

/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.
#ifdef LOCK_TEST
void
SimpleThread(void *name_)
{
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.

    lock->Acquire();
    DEBUG('s', "\" %s \" taking lock\n", name);

    for (unsigned num = 0; num < 10; num++) {
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
        printf("*** Thread `%s` is running: iteration %d\n", name, num);
        //interrupt->SetLevel(oldLevel);
        currentThread->Yield();
    }

    DEBUG('s', "\"%s\" releasing lock\n", name);
    lock->Release();
    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf("!!! Thread `%s` has finished\n", name);
    //interrupt->SetLevel(oldLevel);
}
#endif

#ifdef DEADLOCK_TEST
void A(void *name_) {
    blisto -> P();
    lock -> Acquire();
    printf("A: si solucionaste el problema de las prioridades verias esto.\n");
    lock -> Release();
}

void M(void *name_) {
    blisto2 -> P();
    while(1);
}
 
void B(void *name_) {
    lock -> Acquire();
    blisto -> V();
    blisto2 -> V();
    blisto2 -> V();
    currentThread -> Yield ();
    printf("B: si solucionaste el problema de las prioridades verias esto.\n");
    lock -> Release();
}                    
#endif


/// Set up a ping-pong between several threads.
///
/// Do it by launching ten threads which call `SimpleThread`, and finally
/// calling `SimpleThread` ourselves.

#ifdef COND_TEST
void
Prod(void *name_)
{
    int i=0;
    char *name = (char *) name_;
    while(i<10) {
        printf("%s: Llenando el buffer con 42 \n", name);
        port -> Send(42);
        i++;
    }
}

void 
Cons(void *name_)
{
    int i=0;
    int *buf;
    buf = (int *) malloc(sizeof(int));
    char *name = (char *) name_;
    while(i<10) {
        port -> Receive(buf);
        printf("%s: Recibiendo %d \n", name, *buf);
        i++;
    }
    free(buf);
}
#endif

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

#ifdef COND_TEST
    Thread *firstThread, *secondThread, *thirdThread;

    firstThread = new Thread("Productor",true,3); 
    secondThread = new Thread("Consumidor1",false,3); 
    thirdThread = new Thread("Consumidor2",false,3); 
    port = new Port("8000");

    firstThread -> Fork(Prod, (void *) "Productor");
    secondThread -> Fork(Cons, (void *) "Consumidor1");
    thirdThread -> Fork(Cons, (void *) "Consumidor2");
    printf("Imprimiendo antes de Join()...\n"); 
    firstThread -> Join();
    printf("Imprimiendo después de Join()...\n"); 
#endif

#ifdef LOCK_TEST
    lock = new Lock("lock1");

    Thread *newThread;
    int i;
    for (i=0; i<NUM_THREADS; i++)
    {
        char *name = new char[64];
        sprintf(name, "thread_%d", i);
        newThread = new Thread(name, false,1);
        newThread->Fork(SimpleThread, (void *) name);
    }
#endif

#ifdef DEADLOCK_TEST
    Thread *firstThread, *secondThread, *thirdThread, *fourthThread;

    firstThread = new Thread("A",true,8); 
    secondThread = new Thread("B",false,2); 
    thirdThread = new Thread("M",false,5); 
    fourthThread = new Thread("M",false,5); 

    firstThread -> Fork(A, (void *) "A");
    secondThread -> Fork(B, (void *) "B");
    thirdThread -> Fork(M, (void *) "M");
    fourthThread -> Fork(M, (void *) "M");

#endif
}
