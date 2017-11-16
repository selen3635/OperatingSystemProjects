/* mykernel.c: your portion of the kernel
 *
 *	Below are procedures that are called by other parts of the kernel. 
 *	Your ability to modify the kernel is via these procedures.  You may
 *	modify the bodies of these procedures any way you wish (however,
 *	you cannot change the interfaces).  
 */

#include "aux.h"
#include "sys.h"
#include "mykernel3.h"

#define FALSE 0
#define TRUE 1

/*	A sample semaphore table. You may change this any way you wish.  
 */

static struct {
	int valid;	// Is this a valid entry (was sem allocated)?
	int value;	// value of semaphore
  int waitTable[MAXPROCS];
  int headSignal;
  int tailBlock;
} semtab[MAXSEMS];
/*
static struct
{
  int pid;
  int isBlock;
  int valid;
  int semIndex;
} myProcs[MAXPROCS];
*/
/*	InitSem () is called when kernel starts up.  Initialize data
 *	structures (such as the semaphore table) and call any initialization
 *	procedures here. 
 */

void InitSem ()
{
	int s, i;

	/* modify or add code any way you wish */

	for (s = 0; s < MAXSEMS; s++) {		// mark all sems free
		semtab[s].valid = FALSE;
    for( i = 0; i < MAXPROCS; i++)
    {
      semtab[s].waitTable[i] = 0;
    }
    semtab[s].headSignal = 0;
    semtab[s].tailBlock = 0;
	}
/*
  for( i = 0; i < MAXPROCS; i++ )
  {
    myProcs[i].valid = FALSE;
    myProcs[i].isBlock = FALSE;
    myProcs[i].semIndex = -1;
  }
  */
}

/*	MySeminit (p, v) is called by the kernel whenever the system
 *	call Seminit (v) is called.  The kernel passes the initial
 * 	value v, along with the process ID p of the process that called
 *	Seminit.  MySeminit should allocate a semaphore (find a free entry
 *	in semtab and allocate), initialize that semaphore's value to v,
 *	and then return the ID (i.e., index of the allocated entry). 
 */

int MySeminit (int p, int v)
{
	int s;

	/* modify or add code any way you wish */

	for (s = 0; s < MAXSEMS; s++) {
		if (semtab[s].valid == FALSE) {
			break;
		}
	}
	if (s == MAXSEMS) {
		Printf ("No free semaphores\n");
		return (-1);
	}

	semtab[s].valid = TRUE;
	semtab[s].value = v;
  semtab[s].headSignal = 0;
  semtab[s].tailBlock = 0;
  //semtab[s].pid = p;

  for( int i = 0; i < MAXPROCS; i++)
  {
    semtab[s].waitTable[i] = 0;
  }

  /*
  for(int i = 0; i < MAXPROCS; i++)
  {
    if( myProcs[i].valid == FALSE)
    {
      myProcs[i].valid = TRUE;
      myProcs[i].pid = p;
      myProcs[i].isBlock = FALSE;
      myProcs[i].semIndex = s;
      break;
    }

  }
  */

	return (s);
}

/*	MyWait (p, s) is called by the kernel whenever the system call
 *	Wait (s) is called.  
 */

void MyWait (p, s)
	int p;				// process
	int s;				// semaphore
{
	/* modify or add code any way you wish */
  semtab[s].value--;

  if( semtab[s].value < 0)
  {
    /*
    for( int i = 0; i < MAXPROCS; i++)
    {

      if( semtab[s].waitTable[i] == 0)
      {
        semtab[s].waitTable[i] = p;
        Block(p);
        break;
      }
    } */
    semtab[s].waitTable[semtab[s].tailBlock] = p;
    semtab[s].tailBlock = (semtab[s].tailBlock + 1 ) % MAXPROCS;
    Block(p);
  }
 
}

/*	MySignal (p, s) is called by the kernel whenever the system call
 *	Signal (s) is called.  
 */

void MySignal (p, s)
	int p;				// process
	int s;				// semaphore
{
	/* modify or add code any way you wish */
  semtab[s].value++;
  int temp = 0;

  if( semtab[s].value <= 0)
  {
    /*
    while(1)
    {
      if( semtab[s].waitTable[semtab[s].count] != 0 )
      {

        temp = semtab[s].waitTable[semtab[s].count];
        semtab[s].waitTable[semtab[s].count] = 0;
        Unblock(temp);
        break;
      }

      semtab[s].count = (semtab[s].count + 1) % MAXPROCS;
    } */

    temp = semtab[s].headSignal;
    semtab[s].headSignal = (semtab[s].headSignal + 1) % MAXPROCS;
    Unblock(semtab[s].waitTable[temp]);
  }
}

