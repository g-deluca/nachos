#include "machine/console.hh"
#include "threads/synch.hh"

class SynchConsole {
private:
    Console *myConsole;
    Lock *readers, *writers;
    Semaphore *readAvail; 
    Semaphore *writeDone;
    static void ReadAvail (void *arg);
    static void WriteDone (void *arg);

public:
    SynchConsole(char *in, char *out);

    ~SynchConsole();

    void PutChar(char c);
    
    char GetChar();
    
};
