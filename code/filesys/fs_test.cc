/// Simple test routines for the file system.
///
/// We implement:
/// * Copy -- copy a file from UNIX to Nachos.
/// * Print -- cat the contents of a Nachos file.
/// * Perftest -- a stress test for the Nachos file system read and write a
///   really large file in tiny chunks (will not work on baseline system!)
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "file_system.hh"
#include "machine/disk.hh"
#include "machine/statistics.hh"
#include "threads/thread.hh"
#include "threads/system.hh"
#include "threads/utility.hh"


#define TransferSize  10  // Make it small, just to be difficult.

/// Copy the contents of the UNIX file `from` to the Nachos file `to`.
void
Copy(const char *from, const char *to)
{
    FILE     *fp;
    OpenFile *openFile;
    int       amountRead, fileLength;
    char     *buffer;

    // Open UNIX file.
    if ((fp = fopen(from, "r")) == NULL) {
        printf("Copy: could not open input file %s\n", from);
        return;
    }

    // Figure out length of UNIX file.
    fseek(fp, 0, 2);
    fileLength = ftell(fp);
    fseek(fp, 0, 0);

    // Create a Nachos file of the same length.
    DEBUG('f', "Copying file %s, size %d, to file %s\n",
          from, fileLength, to);
    if (!fileSystem->Create(to, fileLength)) {  // Create Nachos file.
        printf("Copy: could not create output file %s\n", to);
        fclose(fp);
        return;
    }

    openFile = fileSystem->Open(to);
    ASSERT(openFile != NULL);

    // Copy the data in TransferSize chunks.
    buffer = new char[TransferSize];
    while ((amountRead = fread(buffer, sizeof(char),
                               TransferSize, fp)) > 0)
        openFile->Write(buffer, amountRead);
    delete [] buffer;

    // Close the UNIX and the Nachos files.
    delete openFile;
    fclose(fp);
}

/// Print the contents of the Nachos file `name`.
void
Print(const char *name)
{
    OpenFile *openFile;
    int       i, amountRead;
    char     *buffer;

    if ((openFile = fileSystem->Open(name)) == NULL) {
        printf("Print: unable to open file %s\n", name);
        return;
    }

    buffer = new char[TransferSize];
    while ((amountRead = openFile->Read(buffer, TransferSize)) > 0)
        for (i = 0; i < amountRead; i++)
            printf("%c", buffer[i]);
    delete [] buffer;

    delete openFile;  // close the Nachos file
    return;
}


/// Performance test
///
/// Stress the Nachos file system by creating a large file, writing it out a
/// bit at a time, reading it back a bit at a time, and then deleting the
/// file.
///
/// Implemented as three separate routines:
/// * `FileWrite` -- write the file.
/// * `FileRead` -- read the file.
/// * `PerformanceTest` -- overall control, and print out performance #'s.

#define FileName     "TestFile"
#define Contents     "1234567890"
#define ContentSize  strlen(Contents)
#define FileSize     ((int) (ContentSize * 5000))

static void
FileWrite()
{
    OpenFile *openFile;
    int       i, numBytes;

    printf("Sequential write of %d byte file, in %d byte chunks\n",
    FileSize, (int) ContentSize);
    if (!fileSystem->Create(FileName, 0)) {
        printf("Perf test: cannot create %s\n", FileName);
        return;
    }
    openFile = fileSystem->Open(FileName);
    if (openFile == NULL) {
        printf("Perf test: unable to open %s\n", FileName);
        return;
    }
    for (i = 0; i < FileSize; i += ContentSize) {
        numBytes = openFile->Write(Contents, ContentSize);
        if (numBytes < 10) {
            printf("Perf test: unable to write %s\n", FileName);
            delete openFile;
            return;
        }
    }
    delete openFile;
}

static void
FileRead()
{
    OpenFile *openFile;
    char     *buffer = new char[ContentSize];
    int       i, numBytes;

    printf("Sequential read of %d byte file, in %d byte chunks\n",
    FileSize, (int) ContentSize);

    if ((openFile = fileSystem->Open(FileName)) == NULL) {
        printf("Perf test: unable to open file %s\n", FileName);
        delete [] buffer;
        return;
    }
    for (i = 0; i < FileSize; i += ContentSize) {
        numBytes = openFile->Read(buffer, ContentSize);
        if ((numBytes < 10) || strncmp(buffer, Contents, ContentSize)) {
            printf("Perf test: unable to read %s\n", FileName);
            delete openFile;
            delete [] buffer;
            return;
        }
    }
    delete [] buffer;
    delete openFile;
}

void
PerformanceTest()
{
    printf("Starting file system performance test:\n");
    stats->Print();
    FileWrite();
    FileRead();
    if (!fileSystem->Remove(FileName)) {
        printf("Perf test: unable to remove %s\n", FileName);
        return;
    }
    stats->Print();
}