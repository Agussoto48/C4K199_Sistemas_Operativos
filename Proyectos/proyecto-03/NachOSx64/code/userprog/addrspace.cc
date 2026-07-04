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
    referenceCount = 1;

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

        physicalPageTable[physicalPage].owner = this;
        physicalPageTable[physicalPage].virtualPage = i;
#endif

        pageTable[i].use = false;
        pageTable[i].dirty = false;
        pageTable[i].readOnly = false;
        pageTable[i].lastUsed = 0;
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
#ifdef VM
    InitPhysicalPageTable();

    executableFile = parent->executableFile;

    noffHeader = new NoffHeader;
    *noffHeader = *(parent->noffHeader);

    swapFile = parent->swapFile;

    inSwap = new bool[numPages];
    swapLocation = new int[numPages];
#endif

    this->pageTable = new TranslationEntry[numPages];
    this->ownedPages = new bool[numPages];

    int stackPages = divRoundUp(UserStackSize, PageSize);
    int firstStackPage = numPages - stackPages;

    for (unsigned int i = 0; i < numPages; i++)
    {
        pageTable[i] = parent->pageTable[i];
        ownedPages[i] = false;

#ifdef VM
        inSwap[i] = false;
        swapLocation[i] = -1;

        if (parent->pageTable[i].valid || parent->inSwap[i])
        {
            int physicalPage = GetFreePage();

            pageTable[i].physicalPage = physicalPage;
            pageTable[i].valid = true;
            pageTable[i].use = true;
            pageTable[i].dirty = false;
            pageTable[i].lastUsed = stats->totalTicks;

            ownedPages[i] = true;

            physicalPageTable[physicalPage].owner = this;
            physicalPageTable[physicalPage].virtualPage = i;

            int childAddr = physicalPage * PageSize;

            if (parent->pageTable[i].valid)
            {
                int parentAddr = parent->pageTable[i].physicalPage * PageSize;

                bcopy(&(machine->mainMemory[parentAddr]),
                      &(machine->mainMemory[childAddr]),
                      PageSize);
            }
            else
            {
                parent->swapFile->ReadAt(&(machine->mainMemory[childAddr]), PageSize, parent->swapLocation[i]);
                stats->numDiskReads++;
            }
        }
        else
        {
            pageTable[i].physicalPage = -1;
            pageTable[i].valid = false;
            pageTable[i].use = false;
            pageTable[i].dirty = false;
            pageTable[i].lastUsed = 0;
            ownedPages[i] = false;
        }
#else
        if (parent->ownedPages[i])
        {
            int physicalPage = memoryMap->Find();
            ASSERT(physicalPage != -1);

            pageTable[i].physicalPage = physicalPage;
            pageTable[i].valid = true;
            ownedPages[i] = true;

            bcopy(&(machine->mainMemory[parent->pageTable[i].physicalPage * PageSize]), &(machine->mainMemory[physicalPage * PageSize]), PageSize);
        }
        else
        {
            pageTable[i].physicalPage = -1;
            pageTable[i].valid = false;
            ownedPages[i] = false;
        }
#endif
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
            int physicalPage = pageTable[i].physicalPage;

            memoryMap->Clear(pageTable[i].physicalPage);

#ifdef VM
            physicalPageTable[physicalPage].owner = NULL;
            physicalPageTable[physicalPage].virtualPage = -1;
#endif
        }
    }
    delete[] this->ownedPages;
    delete[] this->pageTable;

#ifdef VM
    delete noffHeader;
    delete[] inSwap;
    delete[] swapLocation;
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
#ifdef USE_TLB
    // Recorre todas las entradas de la TLB
    for (int i = 0; i < TLBSize; i++)
    {
        if (machine->tlb[i].valid)
        {
            int pagina_virtual = machine->tlb[i].virtualPage;

            if (pagina_virtual >= 0 && pagina_virtual < (int)numPages)
            {
                if (machine->tlb[i].use)
                    pageTable[pagina_virtual].use = true;

                if (machine->tlb[i].dirty)
                    pageTable[pagina_virtual].dirty = true;

                pageTable[pagina_virtual].lastUsed = machine->tlb[i].lastUsed;
            }

            // Invalida la entrada para el siguiente proceso
            machine->tlb[i].valid = false;
        }
    }
#endif
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
#ifdef USE_TLB
    // Con TLB, la máquina no usa directamente la page table
    machine->pageTable = NULL;
    machine->pageTableSize = 0;

    // Se limpia la TLB al cambiar/restaurar un proceso
    for (int i = 0; i < TLBSize; i++)
    {
        machine->tlb[i].valid = false;
    }
#else
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
#endif
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
    pageTable[virtualPage].lastUsed = stats->totalTicks;

    ownedPages[virtualPage] = true;
    physicalPageTable[physicalPage].owner = this;
    physicalPageTable[physicalPage].virtualPage = virtualPage;
}
#endif
#ifdef VM

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
#ifdef USE_TLB
    for (int j = 0; j < TLBSize; j++)
    {
        if (machine->tlb[j].valid)
        {
            int pagina_virtual = machine->tlb[j].virtualPage;

            if (pagina_virtual >= 0 && pagina_virtual < (int)numPages)
            {
                if (machine->tlb[j].use)
                    pageTable[pagina_virtual].use = true;

                if (machine->tlb[j].dirty)
                    pageTable[pagina_virtual].dirty = true;

                pageTable[pagina_virtual].lastUsed = machine->tlb[j].lastUsed;
            }
        }
    }
#endif
    int victim = -1;
    int oldestTime = 0;

    for (int i = 0; i < NumPhysPages; i++)
    {
        AddrSpace *candidateSpace = physicalPageTable[i].owner;
        int candidateVirtualPage = physicalPageTable[i].virtualPage;

        if (candidateSpace == NULL || candidateVirtualPage < 0)
            continue;

        int candidateTime = candidateSpace->pageTable[candidateVirtualPage].lastUsed;

        if (victim == -1 || candidateTime < oldestTime)
        {
            victim = i;
            oldestTime = candidateTime;
        }
    }

    ASSERT(victim != -1);

    AddrSpace *victimSpace = physicalPageTable[victim].owner;
    int victimVirtualPage = physicalPageTable[victim].virtualPage;

#ifdef USE_TLB
    for (int j = 0; j < TLBSize; j++)
    {
        if (machine->tlb[j].valid && machine->tlb[j].virtualPage == victimVirtualPage)
        {
            if (machine->tlb[j].use)
                victimSpace->pageTable[victimVirtualPage].use = true;

            if (machine->tlb[j].dirty)
                victimSpace->pageTable[victimVirtualPage].dirty = true;

            victimSpace->pageTable[victimVirtualPage].lastUsed = machine->tlb[j].lastUsed;
            machine->tlb[j].valid = false;
        }
    }
#endif

    if (victimSpace->pageTable[victimVirtualPage].dirty)
        victimSpace->WritePageToSwap(victimVirtualPage);

    victimSpace->pageTable[victimVirtualPage].valid = false;
    victimSpace->pageTable[victimVirtualPage].physicalPage = -1;
    victimSpace->pageTable[victimVirtualPage].use = false;
    victimSpace->pageTable[victimVirtualPage].dirty = false;
    victimSpace->pageTable[victimVirtualPage].lastUsed = 0;
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

#ifdef USE_TLB

// Indica si una página virtual ya se encuentra cargada en memoria.
bool AddrSpace::IsPageValid(int virtualPage)
{
    return pageTable[virtualPage].valid;
}

// Actualiza la TLB cuando la traducción no está cargada.
// Usa LRU para reemplazar la entrada menos usada recientemente.
void AddrSpace::UpdateTLB(int virtualPage)
{
    ASSERT(pageTable[virtualPage].valid);

    int victim = -1;
    int mas_viejo = 0;

    // Primero busca una entrada libre en la TLB
    for (int i = 0; i < TLBSize; i++)
    {
        if (!machine->tlb[i].valid)
        {
            victim = i;
            break;
        }
    }

    // Si no hay entradas libres busca la menos usada recientemente
    if (victim == -1)
    {
        for (int i = 0; i < TLBSize; i++)
        {
            int tiempo_actual = machine->tlb[i].lastUsed;

            if (victim == -1 || tiempo_actual < mas_viejo)
            {
                victim = i;
                mas_viejo = tiempo_actual;
            }
        }
    }

    // Antes de reemplazar, guarda los bits importantes en la page table
    if (machine->tlb[victim].valid)
    {
        int pagina_anterior = machine->tlb[victim].virtualPage;

        if (pagina_anterior >= 0 && pagina_anterior < (int)numPages)
        {
            if (machine->tlb[victim].use)
                pageTable[pagina_anterior].use = true;

            if (machine->tlb[victim].dirty)
                pageTable[pagina_anterior].dirty = true;

            pageTable[pagina_anterior].lastUsed = machine->tlb[victim].lastUsed;
        }
    }

    // Carga la nueva traducción en la TLB
    machine->tlb[victim] = pageTable[virtualPage];
    machine->tlb[victim].valid = true;
    machine->tlb[victim].lastUsed = stats->totalTicks;
}
#endif