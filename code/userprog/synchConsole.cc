#include "synchConsole.hh"

SynchConsole::SynchConsole(char *in, char *out) 
{
    myConsole = new Console(in, out, SynchConsole::ReadAvail, SynchConsole::WriteDone, 0);
    readAvail = new Semaphore("readAvail", 0);   
    writeDone = new Semaphore("writeDOne", 0);
    readers = new Lock ("readersLock");
    writers = new Lock ("writersLock");
} 

SynchConsole::~SynchConsole()
{
    myConsole -> ~Console();
    readAvail -> ~Semaphore();
    writeDone -> ~Semaphore();
    readers -> ~Lock();
    writers -> ~Lock();
}

void
SynchConsole::ReadAvail(void *arg)
{
    readAvail -> V();
}

void
SynchConsole::WriteDone(void *arg)
{
    writeDone -> V();
}

void
SynchConsole::PutChar(char c)
{
    writers -> Acquire();
    myConsole -> PutChar(c);
    writeDone -> P();
    writers -> Release();
}

char
SynchConsole::GetChar()
{
    char ch;
    readers -> Acquire();
    readAvail -> P();
    ch = myConsole -> GetChar();
    readers -> Release();
    return ch;
}
