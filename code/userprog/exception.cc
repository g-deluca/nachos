/// Entry point into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "syscall.h"
#include "threads/system.hh"
#include "machine/console.hh"
#include "filesys/file_system.hh"

/// Entry point into the Nachos kernel.  Called when a user program is
/// executing, and either does a syscall, or generates an addressing or
/// arithmetic exception.
///
/// For system calls, the following is the calling convention:
///
/// * system call code in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the pc before returning. (Or else you will
/// loop making the same system call forever!)
///
/// * `which` is the kind of exception.  The list of possible exceptions is
///   in `machine.hh`.

#define MAX_LONG_NAME 128

void
ReadStringFromUser (int userAddress, char *outString, unsigned maxByteCount)
{
    int buffer;
    for (unsigned i = 0; i<maxByteCount; i++) {
        ASSERT(machine->ReadMem(userAddress+i, 1, &buffer)); 
        outString[i] = buffer;
        if (outString[i] == '\0')
            break;
    }
}

void
ReadBufferFromUser (int userAddress, char *outBuffer, unsigned byteCount)
{
    int buffer;
    for (unsigned i = 0; i<byteCount; i++) {
        ASSERT(machine->ReadMem(userAddress+i, 1, &buffer)); 
        outBuffer[i] = buffer;
    }
}

void
WriteStringToUser (const char *buffer, int userAddress)
{
    int i=0;
    do {
        ASSERT(machine->WriteMem(userAddress+i, 1, buffer[i]));
    } while (buffer[i++] != '\0');
}

void
WriteBufferToUser (const char *buffer, int userAddress, unsigned byteCount)
{
    unsigned i=0;
    do {
        ASSERT(machine->WriteMem(userAddress+i, 1, buffer[i]));
    } while (i++ < byteCount);
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    if (which == SYSCALL_EXCEPTION) {
        switch (type){
            case SC_Halt:
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            case SC_Create: {
                char name[MAX_LONG_NAME];
                ReadStringFromUser(machine->ReadRegister(4), name, MAX_LONG_NAME);
                
                if (fileSystem->Create(name,0))
                    DEBUG('a', "New file: %s", name);
                else
                    DEBUG('a', "Error while creating file: %s", name);
                break;
            }
            case SC_Read: {
                int userBuff = machine->ReadRegister(4);
                int size = machine->ReadRegister(5);
                OpenFileId fid = machine->ReadRegister(6);
                
                int readBytes = 0;
                char buffer[size];
                
                if(fid == 1) {
                    DEBUG('a', "Error: Trying to read from output");
                    machine->WriteRegister(2, 0);
                }
                else if (fid == 0) {
                    char aux;
                    DEBUG('a', "Reading from console");
                    while ((aux=console->GetChar()) != '\n' && readBytes <= size) {
                        buffer[readBytes] = aux;
                        readBytes ++;
                    }    
                    WriteBufferToUser(buffer, userBuff, readBytes);
                    machine->WriteRegister(2, readBytes);
                }
                else {
                    OpenFile *f = currentThread->GetFile(fid);
                    readBytes = f->Read(buffer, size);
                    WriteBufferToUser(buffer, userBuff, readBytes);
                    machine->WriteRegister(2, readBytes);
                }
                break;
            }
            case SC_Write: {
                int userBuff = machine->ReadRegister(4);
                int size = machine->ReadRegister(5);
                OpenFileId fid = machine->ReadRegister(6);
                
                char buffer[size];
                ReadBufferFromUser(userBuff, buffer, size);
 
                if(fid == 0) {
                    DEBUG('a', "Error: Trying to write to input");
                }
                else if (fid == 1) {
                    DEBUG('a', "Writing to console");
                    for (int i = 0; i<size; i++) 
                        console->PutChar(buffer[i]);
                }
                else {
                    OpenFile *f = currentThread->GetFile(fid);
                    f->Write(buffer, size);
                }
                break;
            }
            case SC_Open: {
                char name[MAX_LONG_NAME];
                OpenFile *f;
                OpenFileId fid;
                ReadStringFromUser(machine->ReadRegister(4), name, MAX_LONG_NAME);
                f = fileSystem->Open(name);
                fid = currentThread->AddFile(f);
                if (fid != -1)
                    DEBUG('a', "File opened: %s", name);
                else
                    DEBUG('a', "Error while opening file: %s", name);
                machine->WriteRegister(2, fid);
                // incrementar pc 
                break;
            }
            case SC_Close: {
                OpenFileId fid = machine->ReadRegister(4);
                currentThread->CloseFile(fid);
                break;
            }
            case SC_Join: {
                //SpaceId pid = machine->ReadRegister(4);
                
                break; 
            }
            default:
                printf("Unexpected user mode exception %d %d\n", which, type);
                ASSERT(false);
        }                

    } else {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(false);
    }
    
}


