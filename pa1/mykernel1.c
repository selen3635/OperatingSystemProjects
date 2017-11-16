/* mykernel1.c: your portion of the kernel
 *
 *	Below are routines that are called by other parts of the kernel. 
 *	Your ability to modify the kernel is via these routines.  You may
 *	modify the bodies of these routines, and add code outside or them,
 *	in any way you wish (however, you cannot change their interfaces).  
 * 
 */

#include <strings.h>
#include "aux.h"
#include "sys.h"
#include "mykernel1.h"

/* NewContext (p, c) will be called by the kernel whenever a new process
 * is created.  This is essentially a notification (which you will make
 * use of) that this newly created process has an ID of p, and that its
 * initial context is pointed to by c.  Make sure you make a copy of the
 * contents of this context (i.e., don't copy the pointer, but the contents
 * pointed to by the pinter), as the pointer will become useless after this
 * routine returns.
 */

/* create a array of CONTEXT */
CONTEXT contextArray[MAXPROCS];

void NewContext (p, c)
	int p;				// ID of new process just created
	CONTEXT *c;			// initial context for this process
{
	/* create a CONTEXT structure */
  CONTEXT myContext;

  /* memory copy to structure */
  memcpy(&myContext, c, sizeof(CONTEXT));
  
  /* store process into array */
  myContext.pid =  p;
  contextArray[p-1] = myContext;
}

/* MySwitchContext (p) should cause a context switch from the calling
 * process to process p. It should return the ID of the calling process.  
 * The ID of the calling process can be determined by calling GetCurProc (),
 * which returns the currently running process's ID.  The routine
 * SwitchContext (p) is an internal working version of context switching. 
 * This is provided so that the kernel works without modification, to
 * allow the other exercises to execute and to illustrate proper behavior.  
 * For Exercise F, the call to SwitchContext (p) MUST be removed.  
 */

/*
int MySwitchContext (p)
	int p;				// process to switch to
{
// Printf("what is this 222222  ");
	return (SwitchContext (p));	// remove call to SwitchContext (p)
}*/

int MySwitchContext (p)
  int p;
{
  /* static integer to store current process ID */
  static int myCurProc;

  int mystery = 0;

  /* get current process ID */
  myCurProc = GetCurProc();

  /* save current process to CONTEXT array */
  SaveContext( &contextArray[myCurProc-1]);

  /* check if it is first time to run */
  if ( mystery == 1)
    return myCurProc;
  else
  {
    /* set mystery to 1 */ 
    mystery = 1;

   // Printf("%d\n", p);
   // Printf("my current process: %d\n ", myCurProc);

  }
  
  /* restore process p */
  RestoreContext(&contextArray[p-1]);
} 

