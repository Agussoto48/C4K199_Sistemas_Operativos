// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "bitmap.h"

#ifdef VM
struct PhysicalPageInfo
{
    AddrSpace *owner;
    int virtualPage;
};

static PhysicalPageInfo physicalPageTable[NumPhysPages];
static bool physicalPageTableInitialized = false;

static void InitPhysicalPageTable()
{
    if (!physicalPageTableInitialized)
    {
        for (int i = 0; i < NumPhysPages; i++)
        {
            physicalPageTable[i].owner = NULL;
            physicalPageTable[i].virtualPage = -1;
        }

        physicalPageTableInitialized = true;
    }
}
#endif

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void SwapHeader(NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

/*
    Carga un segmento del archivo ejecutable en memoria física.
    Usa la tabla de páginas para traducir cada dirección virtual
    a la página física asignada al proceso.
*/
static void LoadSegment(OpenFile *executable, TranslationEntry *pageTable, int virtualAddr, int size, int inFileAddr)
{
    int remaining = size;                 // Bytes que faltan por cargar
    int currentVirtualAddr = virtualAddr; // Dirección virtual actual del segmento
    int currentFileAddr = inFileAddr;     // Posición actual dentro del archivo ejecutable

    while (remaining > 0)
    {
        // Determina la página virtual y el desplazamiento dentro de esa página.
        int virtualPage = currentVirtualAddr / PageSize;
        int offset = currentVirtualAddr % PageSize;

        // Obtiene la página física asignada a esa página virtual.
        int physicalPage = pageTable[virtualPage].physicalPage;

        // Calcula la dirección real dentro de la memoria principal.
        int physicalAddr = physicalPage * PageSize + offset;

        // Calcula cuántos bytes se pueden leer en esta página.
        int bytesToRead = PageSize - offset;

        if (bytesToRead > remaining)
        {
            bytesToRead = remaining;
        }

        // Lee del archivo ejecutable y coloca los bytes en la memoria física correcta.
        executable->ReadAt(&(machine->mainMemory[physicalAddr]), bytesToRead, currentFileAddr);

        // Avanza a la siguiente parte del segmento.
        remaining -= bytesToRead;
        currentVirtualAddr += bytesToRead;
        currentFileAddr += bytesToRead;
    }
}
//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int i, size;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);

    if ((noffH.noffMagic != NOFFMAGIC) &&
        (WordToHost(noffH.noffMagic) == NOFFMAGIC))
    {
        SwapHeader(&noffH);
    }

    ASSERT(noffH.noffMagic == NOFFMAGIC);

#ifdef VM
    InitPhysicalPageTable();
    this->executableFile = executable;
    this->noffHeader = new NoffHeader;
    *(this->noffHeader) = noffH;
#endif

    size = noffH.code.size +
           noffH.initData.size +
           noffH.uninitData.size +
           UserStackSize;

    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

#ifndef VM
    ASSERT((int)numPages <= memoryMap->NumClear());
#endif

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
          numPages, size);

    pageTable = new TranslationEntry[numPages];
    ownedPages = new bool[numPages];

#ifdef VM
    inSwap = new bool[numPages];
    swapLocation = new int[numPages];

    for (i = 0; i < numPages; i++)
    {
        inSwap[i] = false;
        swapLocation[i] = -1;
    }

    fileSystem->Create("SWAP", 64 * PageSize);
    swapFile = fileSystem->Open("SWAP");
    ASSERT(swapFile != NULL);
#endif

    for (i = 0; i < numPages; i++)
    {
        pageTable[i].virtualPage = i;

#ifdef VM

        pageTable[i].physicalPage = -1;
        pageTable[i].valid = false;
        ownedPages[i] = false;
#else
        int physicalPage = memoryMap->Find();
        ASSERT(physicalPage != -1);

        pageTable[i].physicalPage = physicalPage;
        pageTable[i].valid = true;
        ownedPages[i] = true;
#endif

        pageTable[i].use = false;
        pageTable[i].dirty = false;
        pageTable[i].readOnly = false;
    }

#ifndef VM
    for (i = 0; i < numPages; i++)
    {
        int physicalPage = pageTable[i].physicalPage;
        bzero(&(machine->mainMemory[physicalPage * PageSize]), PageSize);
    }

    if (noffH.code.size > 0)
    {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",
              noffH.code.virtualAddr, noffH.code.size);

        LoadSegment(executable, pageTable,
                    noffH.code.virtualAddr,
                    noffH.code.size,
                    noffH.code.inFileAddr);
    }

    if (noffH.initData.size > 0)
    {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n",
              noffH.initData.virtualAddr, noffH.initData.size);

        LoadSegment(executable, pageTable,
                    noffH.initData.virtualAddr,
                    noffH.initData.size,
                    noffH.initData.inFileAddr);
    }
#endif
}

AddrSpace::AddrSpace(AddrSpace *parent)
{
    numPages = parent->numPages;

    this->pageTable = new TranslationEntry[numPages];
    this->ownedPages = new bool[numPages];

    int stackPages = divRoundUp(UserStackSize, PageSize);
    int firstStackPage = numPages - stackPages;

    for (unsigned int i = 0; i < numPages; i++)
    {
        pageTable[i] = parent->pageTable[i];

        if ((int)i >= firstStackPage)
        {
            int physicalPage = memoryMap->Find();
            ASSERT(physicalPage != -1);

            pageTable[i].physicalPage = physicalPage;
            ownedPages[i] = true;

            bzero(&(machine->mainMemory[physicalPage * PageSize]), PageSize);
        }
        else
        {
            ownedPages[i] = false;
        }
    }
}
//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    // Libera las páginas físicas asignadas a este proceso.
    for (unsigned int i = 0; i < numPages; i++)
    {
        if (ownedPages[i])
        {
            memoryMap->Clear(pageTable[i].physicalPage);
        }
    }
    delete[] this->ownedPages;
    delete[] this->pageTable;

#ifdef VM
    delete noffHeader;
#endif
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

#ifdef VM
void AddrSpace::HandlePageFault(int virtualPage)
{
    // Verifica que la página virtual solicitada exista dentro del espacio del proceso
    ASSERT(virtualPage >= 0 && virtualPage < (int)numPages);

    // busca un marco físico libe donde cargar la página
    int physicalPage = GetFreePage();

    int physicalAddr = physicalPage * PageSize;

    bzero(&(machine->mainMemory[physicalAddr]), PageSize);

    if (inSwap[virtualPage])
    {
        ReadPageFromSwap(virtualPage, physicalPage);
    }
    else
    {
        int virtualAddr = virtualPage * PageSize;

        // Carga la parte de código si la página virtual contiene código
        if (noffHeader->code.size > 0)
        {
            int codeStart = noffHeader->code.virtualAddr;
            int codeEnd = codeStart + noffHeader->code.size;

            int pageStart = virtualAddr;
            int pageEnd = virtualAddr + PageSize;

            // revisa si esta página contiene parte del código
            int start = pageStart > codeStart ? pageStart : codeStart;
            int end = pageEnd < codeEnd ? pageEnd : codeEnd;

            if (start < end)
            {
                int bytesToRead = end - start;
                int offsetInPage = start - pageStart;
                int offsetInFile = noffHeader->code.inFileAddr + (start - codeStart);

                executableFile->ReadAt(
                    &(machine->mainMemory[physicalAddr + offsetInPage]), bytesToRead, offsetInFile);
            }
        }
        // carga los datos inicializados
        if (noffHeader->initData.size > 0)
        {
            int dataStart = noffHeader->initData.virtualAddr;
            int dataEnd = dataStart + noffHeader->initData.size;

            int pageStart = virtualAddr;
            int pageEnd = virtualAddr + PageSize;

            // Ve si esta página contiene parte de los datos inicializados
            int start = pageStart > dataStart ? pageStart : dataStart;
            int end = pageEnd < dataEnd ? pageEnd : dataEnd;

            if (start < end)
            {
                int bytesToRead = end - start;
                int offsetInPage = start - pageStart;
                int offsetInFile = noffHeader->initData.inFileAddr + (start - dataStart);

                executableFile->ReadAt(
                    &(machine->mainMemory[physicalAddr + offsetInPage]), bytesToRead, offsetInFile);
            }
        }
    }
    // actualiza la page table para indicar que la página ya está en memoria
    pageTable[virtualPage].physicalPage = physicalPage;
    pageTable[virtualPage].valid = true;
    pageTable[virtualPage].use = true;
    pageTable[virtualPage].dirty = false;
    pageTable[virtualPage].readOnly = false;

    ownedPages[virtualPage] = true;
    physicalPageTable[physicalPage].owner = this;
    physicalPageTable[physicalPage].virtualPage = virtualPage;
}
#endif
#ifdef VM
int fifoVictim = 0;

int AddrSpace::GetFreePage()
{
    // encontrar un espacio libre en memoria
    int physicalPage = memoryMap->Find();

    if (physicalPage != -1)
    {
        return physicalPage;
    }

    // Si no hay espacio libre se reemplaza una página
    return ReplacePage();
}

int AddrSpace::ReplacePage()
{
    int victim = -1;

    while (victim == -1)
    {
        int candidate = fifoVictim;
        fifoVictim = (fifoVictim + 1) % NumPhysPages;

        AddrSpace *candidateSpace = physicalPageTable[candidate].owner;
        int candidateVirtualPage = physicalPageTable[candidate].virtualPage;

        if (candidateSpace == NULL || candidateVirtualPage < 0)
        {
            continue;
        }

        // Usando la vara de second chance, que sii fue usada recientemente, se le da otra oportunidad
        if (candidateSpace->pageTable[candidateVirtualPage].use)
        {
            candidateSpace->pageTable[candidateVirtualPage].use = false;
            continue;
        }

        victim = candidate;
    }

    AddrSpace *victimSpace = physicalPageTable[victim].owner;
    int victimVirtualPage = physicalPageTable[victim].virtualPage;

    // Si la página fue modificada se guarda en SWAP
    if (victimSpace->pageTable[victimVirtualPage].dirty)
    {
        victimSpace->WritePageToSwap(victimVirtualPage);
    }

    victimSpace->pageTable[victimVirtualPage].valid = false;
    victimSpace->pageTable[victimVirtualPage].physicalPage = -1;
    victimSpace->pageTable[victimVirtualPage].use = false;
    victimSpace->pageTable[victimVirtualPage].dirty = false;
    victimSpace->ownedPages[victimVirtualPage] = false;

    physicalPageTable[victim].owner = NULL;
    physicalPageTable[victim].virtualPage = -1;

    return victim;
}

void AddrSpace::WritePageToSwap(int virtualPage)
{
    int physicalPage = pageTable[virtualPage].physicalPage;
    int physicalAddr = physicalPage * PageSize;

    if (!inSwap[virtualPage])
    {
        swapLocation[virtualPage] = virtualPage * PageSize;
        inSwap[virtualPage] = true;
    }

    swapFile->WriteAt(&(machine->mainMemory[physicalAddr]), PageSize, swapLocation[virtualPage]);

    stats->numDiskWrites++;
}

void AddrSpace::ReadPageFromSwap(int virtualPage, int physicalPage)
{
    int physicalAddr = physicalPage * PageSize;

    swapFile->ReadAt(&(machine->mainMemory[physicalAddr]), PageSize, swapLocation[virtualPage]);
    stats->numDiskReads++;

    inSwap[virtualPage] = false;
}

#endif