// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.
//
// Copyright (c) -2025 Universidad de Costa Rica

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synch.h"
#include "NachosOpenFilesTable.h"

#define SC_NachOS 12345
#define MAX_PROCESSES 32

NachosOpenFilesTable *openFilesTable = new NachosOpenFilesTable();

struct ProcessInfo
{
   Thread *thread;
   Semaphore *joinSem;
   int exitStatus;
   bool active;
};

ProcessInfo processTable[MAX_PROCESSES];
int nextProcessId = 1;

// Declaracion de funciones
bool ReadStringFromUser(int addr, char *buffer, int maxSize);
void StartExecProcess(void *arg);
int RegisterProcess(Thread *thread);
void NachOSForkThread(void *p);
static void AdvancePC();
/*
 *  System call interface: Halt()
 */
void NachOS_Halt()
{ // System call 0

   DEBUG('a', "Shutdown, initiated by user program.\n");
   interrupt->Halt();
}

/*
 *  System call interface: void Exit( int )
 */
void NachOS_Exit()
{ // System call 1

   int status = machine->ReadRegister(4);
   DEBUG('u', "Exit system call %d\n", status);

   // Libera el espacio de direcciones de los procesos.
   for (int i = 1; i < MAX_PROCESSES; i++)
   {
      if (processTable[i].active && processTable[i].thread == currentThread)
      {
         processTable[i].exitStatus = status;
         processTable[i].joinSem->V();
         break;
      }
   }

   if (currentThread->space != NULL)
   {
      delete currentThread->space;
      currentThread->space = NULL;
   }
   // Finaliza el hilo actual.
   currentThread->Finish();
}

/*
 *  System call interface: SpaceId Exec( char * )
 */

void NachOS_Exec()
{ // System call 2
   int addr = machine->ReadRegister(4);
   char filename[256];

   if (!ReadStringFromUser(addr, filename, 256))
   {
      machine->WriteRegister(2, -1);
      return;
   }

   char *programName = new char[256];
   strcpy(programName, filename);

   Thread *newThread = new Thread(programName);
   int processId = RegisterProcess(newThread);
   if (processId == -1)
   {
      delete[] programName;
      delete newThread;
      machine->WriteRegister(2, -1);
      return;
   }
   newThread->Fork(StartExecProcess, (void *)programName);

   machine->WriteRegister(2, processId);
}

/*
 *  System call interface: int Join( SpaceId )
 */
void NachOS_Join()
{ // System call 3
   int processId = machine->ReadRegister(4);

   if (processId <= 0 || processId >= MAX_PROCESSES || !processTable[processId].active)
   {
      machine->WriteRegister(2, -1);
      return;
   }

   processTable[processId].joinSem->P();

   int status = processTable[processId].exitStatus;

   delete processTable[processId].joinSem;
   processTable[processId].joinSem = NULL;
   processTable[processId].thread = NULL;
   processTable[processId].active = false;

   machine->WriteRegister(2, status);
}

/*
 *  System call interface: void Create( char * )
 */
void NachOS_Create()
{ // System call 4
   int addr = machine->ReadRegister(4);
   char filename[256];

   if (!ReadStringFromUser(addr, filename, 256))
   {
      machine->WriteRegister(2, -1);
      return;
   }

   FILE *file = fopen(filename, "w");

   if (file == NULL)
   {
      machine->WriteRegister(2, -1);
      return;
   }

   fclose(file);
   machine->WriteRegister(2, 1);
}

/*
 *  System call interface: OpenFileId Open( char * )
 */
void NachOS_Open()
{ // System call 5
   int addr = machine->ReadRegister(4);
   char filename[256];

   if (!ReadStringFromUser(addr, filename, 256))
   {
      machine->WriteRegister(2, -1);
      return;
   }

   FILE *openedFile = fopen(filename, "r+");

   if (openedFile == NULL)
   {
      machine->WriteRegister(2, -1);
      return;
   }

   int fileId = openFilesTable->Open(openedFile);
   machine->WriteRegister(2, fileId);
}

/*
 *  System call interface: OpenFileId Read( char *, int, OpenFileId )
 */
void NachOS_Read()
{ // System call 6
   int addr = machine->ReadRegister(4);
   int size = machine->ReadRegister(5);
   int file = machine->ReadRegister(6);

   // Tamaño invalido
   if (size < 0)
   {
      machine->WriteRegister(2, -1);
      return;
   }

   char *buffer = new char[size];
   int bytesRead = 0;
   if (file == ConsoleInput)
   {
      for (int i = 0; i < size; i++)
      {
         int c = getchar();

         if (c == EOF)
            break;

         buffer[i] = (char)c;
         bytesRead++;
      }
   }
   else
   {
      FILE *openedFile = openFilesTable->GetFile(file);

      if (openedFile != NULL)
      {
         bytesRead = fread(buffer, 1, size, openedFile);
      }
      else
      {
         bytesRead = recv(file, buffer, size, 0);
         if (bytesRead < 0)
         {
            delete[] buffer;
            machine->WriteRegister(2, -1);
            return;
         }
      }
   }

   for (int i = 0; i < bytesRead; i++)
   {
      if (!machine->WriteMem(addr + i, 1, buffer[i]))
      {
         delete[] buffer;
         machine->WriteRegister(2, -1);
         return;
      }
   }

   delete[] buffer;

   machine->WriteRegister(2, bytesRead);
}

/*
 *  System call interface: OpenFileId Write( char *, int, OpenFileId )
 */
void NachOS_Write()
{                                       // System call 7
   int addr = machine->ReadRegister(4); // Dirección virtual del texto
   int size = machine->ReadRegister(5); // Cantidad de bytes a escribir
   int file = machine->ReadRegister(6); // Archivo destino

   // Tamaño invalido, devuelve error
   if (size < 0)
   {
      machine->WriteRegister(2, -1);
      return;
   }

   char *buffer = new char[size + 1];

   for (int i = 0; i < size; i++)
   {
      int value;
      // Algun error al leer
      if (!machine->ReadMem(addr + i, 1, &value))
      {
         delete[] buffer;
         machine->WriteRegister(2, -1);
         return;
      }

      buffer[i] = (char)value;
   }

   buffer[size] = '\0';

   switch (file)
   {
   case ConsoleInput:
      machine->WriteRegister(2, -1);
      break;

   case ConsoleOutput:
      printf("%s", buffer);
      machine->WriteRegister(2, size);
      break;

   case ConsoleError:
      // Mensaje de error
      printf("%d\n", machine->ReadRegister(4));
      machine->WriteRegister(2, size);
      break;

   default:
      FILE *openedFile = openFilesTable->GetFile(file);

      if (openedFile != NULL)
      {
         int written = fwrite(buffer, 1, size, openedFile);
         machine->WriteRegister(2, written);
      }
      else
      {
         int sent = send(file, buffer, size, 0);
         machine->WriteRegister(2, sent);
      }

      break;
   }

   delete[] buffer;
}
/*
 *  System call interface: void Close( OpenFileId )
 */
void NachOS_Close()
{ // System call 8
   int file = machine->ReadRegister(4);
   int result = openFilesTable->Close(file);
   if (result == -1 && file > 2)
   {
      result = close(file);
   }
   machine->WriteRegister(2, result);
}

/*
 *  System call interface: void Fork( void (*func)() )
 */
void NachOS_Fork()
{ // System call 9
   int addr = machine->ReadRegister(4);

   Thread *child = new Thread("Fork child");
   child->space = new AddrSpace(currentThread->space);
   child->Fork(NachOSForkThread, (void *)addr);

   machine->WriteRegister(2, 0);
}

/*
 *  System call interface: void Yield()
 */
void NachOS_Yield()
{ // System call 10
   currentThread->Yield();
}

/*
 *  System call interface: Sem_t SemCreate( int )
 */
void NachOS_SemCreate()
{ // System call 11
}

/*
 *  System call interface: int SemDestroy( Sem_t )
 */
void NachOS_SemDestroy()
{ // System call 12
}

/*
 *  System call interface: int SemSignal( Sem_t )
 */
void NachOS_SemSignal()
{ // System call 13
}

/*
 *  System call interface: int SemWait( Sem_t )
 */
void NachOS_SemWait()
{ // System call 14
}

/*
 *  System call interface: Lock_t LockCreate( int )
 */
void NachOS_LockCreate()
{ // System call 15
}

/*
 *  System call interface: int LockDestroy( Lock_t )
 */
void NachOS_LockDestroy()
{ // System call 16
}

/*
 *  System call interface: int LockAcquire( Lock_t )
 */
void NachOS_LockAcquire()
{ // System call 17
}

/*
 *  System call interface: int LockRelease( Lock_t )
 */
void NachOS_LockRelease()
{ // System call 18
}

/*
 *  System call interface: Cond_t LockCreate( int )
 */
void NachOS_CondCreate()
{ // System call 19
}

/*
 *  System call interface: int CondDestroy( Cond_t )
 */
void NachOS_CondDestroy()
{ // System call 20
}

/*
 *  System call interface: int CondSignal( Cond_t )
 */
void NachOS_CondSignal()
{ // System call 21
}

/*
 *  System call interface: int CondWait( Cond_t )
 */
void NachOS_CondWait()
{ // System call 22
}

/*
 *  System call interface: int CondBroadcast( Cond_t )
 */
void NachOS_CondBroadcast()
{ // System call 23
}

/*
 *  System call interface: Socket_t Socket( int, int )
 */
void NachOS_Socket()
{ // System call 30
   int family = machine->ReadRegister(4);
   int type = machine->ReadRegister(5);

   int socketId = socket(family, type, 0);

   if (socketId < 0)
   {
      machine->WriteRegister(2, -1);
      return;
   }

   machine->WriteRegister(2, socketId);
}

/*
 *  System call interface: Socket_t Connect( char *, int )
 */
void NachOS_Connect()
{ // System call 31
   int socketId = machine->ReadRegister(4);
   int addr = machine->ReadRegister(5);
   int port = machine->ReadRegister(6);

   char ip[64];
   if (!ReadStringFromUser(addr, ip, 64))
   {
      machine->WriteRegister(2, -1);
      return;
   }

   struct sockaddr_in server;
   bzero((char *)&server, sizeof(server));
   server.sin_family = AF_INET;
   server.sin_port = htons(port);
   server.sin_addr.s_addr = inet_addr(ip);

   int result = connect(socketId, (struct sockaddr *)&server, sizeof(server));
   if (result < 0)
   {
      machine->WriteRegister(2, -1);
      return;
   }

   machine->WriteRegister(2, 0);
}

/*
 *  System call interface: int Bind( Socket_t, int )
 */
void NachOS_Bind()
{ // System call 32
}

/*
 *  System call interface: int Listen( Socket_t, int )
 */
void NachOS_Listen()
{ // System call 33
}

/*
 *  System call interface: int Accept( Socket_t )
 */
void NachOS_Accept()
{ // System call 34
}

/*
 *  System call interface: int Shutdown( Socket_t, int )
 */
void NachOS_Shutdown()
{ // System call 25
}
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

void ExceptionHandler(ExceptionType which)
{
   int syscallCode = machine->ReadRegister(2);
   int type = machine->ReadRegister(2) - SC_Base;

   switch (which)
   {
   case SyscallException:
      switch (type)
      {
      case SC_Halt: // System call # 0
         NachOS_Halt();
         break;
      case SC_Exit: // System call # 1
         NachOS_Exit();
         break;
      case SC_Exec: // System call # 2
         NachOS_Exec();
         AdvancePC();
         break;
      case SC_Join: // System call # 3
         NachOS_Join();
         AdvancePC();
         break;

      case SC_Create: // System call # 4
         NachOS_Create();
         AdvancePC();
         break;
      case SC_Open: // System call # 5
         NachOS_Open();
         AdvancePC();
         break;
      case SC_Read: // System call # 6
         NachOS_Read();
         AdvancePC();
         break;
      case SC_Write: // System call # 7
         NachOS_Write();
         AdvancePC();
         break;
      case SC_Close: // System call # 8
         NachOS_Close();
         AdvancePC();
         break;

      case SC_Fork: // System call # 9
         NachOS_Fork();
         AdvancePC();
         break;
      case SC_Yield: // System call # 10
         NachOS_Yield();
         AdvancePC();
         break;

      case SC_SemCreate: // System call # 11
         NachOS_SemCreate();
         break;
      case SC_SemDestroy: // System call # 12
         NachOS_SemDestroy();
         break;
      case SC_SemSignal: // System call # 13
         NachOS_SemSignal();
         break;
      case SC_SemWait: // System call # 14
         NachOS_SemWait();
         break;

      case SC_LckCreate: // System call # 15
         NachOS_LockCreate();
         break;
      case SC_LckDestroy: // System call # 16
         NachOS_LockDestroy();
         break;
      case SC_LckAcquire: // System call # 17
         NachOS_LockAcquire();
         break;
      case SC_LckRelease: // System call # 18
         NachOS_LockRelease();
         break;

      case SC_CondCreate: // System call # 19
         NachOS_CondCreate();
         break;
      case SC_CondDestroy: // System call # 20
         NachOS_CondDestroy();
         break;
      case SC_CondSignal: // System call # 21
         NachOS_CondSignal();
         break;
      case SC_CondWait: // System call # 22
         NachOS_CondWait();
         break;
      case SC_CondBroadcast: // System call # 23
         NachOS_CondBroadcast();
         break;

      case SC_Socket: // System call # 30
         NachOS_Socket();
         AdvancePC();
         break;
      case SC_Connect: // System call # 31
         NachOS_Connect();
         AdvancePC();
         break;
      case SC_Bind: // System call # 32
         NachOS_Bind();
         break;
      case SC_Listen: // System call # 33
         NachOS_Listen();
         break;
      case SC_Accept: // System call # 32
         NachOS_Accept();
         break;
      case SC_Shutdown: // System call # 33
         NachOS_Shutdown();
         break;

      default:
         printf("NachOS version: %d-%d\n", (SC_Base + SC_NachOS) / 10, (SC_Base + SC_NachOS) % 10);
         printf("Unexpected syscall exception %d\n", type);
         ASSERT(false);
         break;
      }
      // AdvancePC();
      break;

   case PageFaultException:
   {
      break;
   }

   case ReadOnlyException:
      printf("Read Only exception (%d)\n", which);
      ASSERT(false);
      break;

   case BusErrorException:
      printf("Bus error exception (%d)\n", which);
      ASSERT(false);
      break;

   case AddressErrorException:
      printf("Address error exception (%d)\n", which);
      ASSERT(false);
      break;

   case OverflowException:
      printf("Overflow exception (%d)\n", which);
      ASSERT(false);
      break;

   case IllegalInstrException:
      printf("Ilegal instruction exception (%d)\n", which);
      ASSERT(false);
      break;

   default:
      printf("Unexpected exception %d\n", which);
      ASSERT(false);
      break;
   }
}

// Definicion de funciones

// Lee un string desde la memoria del programa usuario y lo copia a un buffer del kernel,
// se usa para traer nombres de archivos o direcciones como texto.
bool ReadStringFromUser(int addr, char *buffer, int maxSize)
{
   int value;
   int i = 0;

   do
   {
      if (!machine->ReadMem(addr + i, 1, &value))
      {
         buffer[0] = '\0';
         return false;
      }

      buffer[i] = (char)value;
      i++;

   } while (buffer[i - 1] != '\0' && i < maxSize - 1);

   buffer[maxSize - 1] = '\0';

   return true;
}
// Inicia la ejecución de un programa cargado por Exec, abre el archivo ejecutable, crea su espacio de direcciones y arranca la simulación.
void StartExecProcess(void *arg)
{
   char *filename = (char *)arg;

   OpenFile *executable = fileSystem->Open(filename);

   if (executable == NULL)
   {
      printf("Unable to open file %s\n", filename);
      delete[] filename;
      currentThread->Finish();
      return;
   }

   if (currentThread->space != NULL)
   {
      delete currentThread->space;
      currentThread->space = NULL;
   }

   currentThread->space = new AddrSpace(executable);

   delete executable;
   delete[] filename;

   currentThread->space->InitRegisters();
   currentThread->space->RestoreState();

   machine->Run();

   ASSERT(false);
}
// Registra un proceso nuevo en la tabla de procesos y devuelve un identificador que luego puede ser usado por Join.
int RegisterProcess(Thread *thread)
{
   for (int i = 1; i < MAX_PROCESSES; i++)
   {
      if (!processTable[i].active)
      {
         processTable[i].thread = thread;
         processTable[i].joinSem = new Semaphore("join semaphore", 0);
         processTable[i].exitStatus = 0;
         processTable[i].active = true;
         return i;
      }
   }

   return -1;
}
// Función que ejecuta el hilo hijo creado por Fork.
// Prepara los registros para que el hijo empiece en la función indicada por el usuario.
void NachOSForkThread(void *p)
{
   AddrSpace *space = currentThread->space;

   space->InitRegisters();
   space->RestoreState();

   machine->WriteRegister(RetAddrReg, 4);
   machine->WriteRegister(PCReg, (long)p);
   machine->WriteRegister(NextPCReg, (long)p + 4);

   machine->Run();

   ASSERT(false);
}
// Avanza los registros del contador de programa después de atender un system call.
// Esto evita que NachOS vuelva a ejecutar la misma instrucción de syscall.
static void AdvancePC()
{
   // Guarda la instrucción actual como la anterior.
   machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));

   // Avanza el PC a la siguiente instrucción.
   machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));

   // Prepara el siguiente PC, avanzando 4 bytes.
   machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
}