#include "thread.hh"
#include "userprog/syscall.h"

#define MAX_NUMBER_PROC 1000

class ProcessTable {

public:
    ProcessTable();

    ~ProcessTable();

    SpaceId AddProcess(Thread *process);

    void RemoveProcess(SpaceId pid);

private:
    Thread *table[MAX_NUMBER_PROC];
};
