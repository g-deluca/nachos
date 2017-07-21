#include "userprogtable.hh"

ProcessTable::ProcessTable()
{
    for(int i=0; i<MAX_NUMBER_PROC; i++)
        table[i]=NULL;
}

ProcessTable::~ProcessTable()
{
    return;
}
SpaceId
ProcessTable::AddProcess(Thread *process)
{
    for(int i=0; i<MAX_NUMBER_PROC; i++){
        if(table[i]==NULL){
            table[i]=process;
            return i;
        }
    }
    return -1;
}

void
ProcessTable::RemoveProcess(SpaceId pid)
{
    if(pid>0 && pid<MAX_NUMBER_PROC)
        table[pid]=NULL;
}

