// addrspace.h
//	Data structures to keep track of executing user programs
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"

struct noffHeader;
typedef struct noffHeader NoffHeader;

#define UserStackSize 8192 // increase this as necessary!

class AddrSpace
{
public:
  AddrSpace(OpenFile *executable);
  AddrSpace(AddrSpace *parent);
  ~AddrSpace();


  void InitRegisters(); // Initialize user-level CPU registers,
                        // before jumping to user code

  void SaveState();    // Save/restore address space-specific
  void RestoreState(); // info on a context switch

  #ifdef VM
  void HandlePageFault(int virtualPage);
  #endif
private:
  TranslationEntry *pageTable; // Assume linear page table translation
                               // for now!
  unsigned int numPages;       // Number of pages in the virtual
                               // address space
  bool *ownedPages;

  #ifdef VM
  OpenFile *executableFile;
  NoffHeader *noffHeader;
  #endif
};

#endif // ADDRSPACE_H
