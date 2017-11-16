/*	User-level thread system
 *
 */

#include <setjmp.h>
#include <string.h>

#include "aux.h"
#include "umix.h"
#include "mythreads.h"

/* helper methods */
void MyAddFront(int id);
void MyAddBack(int id);
void MyRemove(int id);

static int MyInitThreadsCalled = 0;	// 1 if MyInitThreads called, else 0
static int curThread = 0;
static int prevThread;
static int curThreadID;

static struct thread {			// thread table
	int valid;			// 1 if entry is valid, else 0
  int myParam;
	jmp_buf env;			// current context
  jmp_buf cleanSlat;
  void (*myFunc)();
} thread[MAXTHREADS];

static int myQueue[MAXTHREADS];
//static int myFront = -1;
//static int myBack = -1;
static int threadNum = 0;


#define STACKSIZE	65536		// maximum size of thread stack

/*	MyInitThreads () initializes the thread package. Must be the first
 *	function called by any user program that uses the thread package.  
 */

void MyInitThreads ()
{
	int i;

	if (MyInitThreadsCalled) {		// run only once
		Printf ("InitThreads: should be called only once\n");
		Exit ();
	}

	for (i = 0; i < MAXTHREADS; i++) {	// initialize thread table
		thread[i].valid = 0;
    //thread[i].myParam = -1;
	}

  for( int j = 0; j < MAXTHREADS; j++)
  {
    myQueue[j] = -1;
  }

	thread[0].valid = 1;			// initialize thread 0
  prevThread = -1;
  MyAddFront(0);  // add thread 0 to queue
  curThreadID = 0;

  for( i = 0; i < MAXTHREADS; i++)
  {
    char s[STACKSIZE * i];
		if (((int) &s[STACKSIZE-1]) - ((int) &s[0]) + 1 != STACKSIZE) {
			Printf ("Stack space reservation failed\n");
			Exit ();
		}
    // partition stack space
    if( setjmp(thread[i].cleanSlat) != 0)
    {
      (*(thread[curThread].myFunc))(thread[curThread].myParam);
      MyExitThread();
    }

  }

	MyInitThreadsCalled = 1;
}

/*	MyCreateThread (func, param) creates a new thread to execute
 *	func (param), where func is a function with no return value and
 *	param is an integer parameter.  The new thread does not begin
 *	executing until another thread yields to it. 
 */

/*
int MyCreateThread (func, param)
	void (*func)();			// function to be executed
	int param;			// integer parameter
{
	if (! MyInitThreadsCalled) {
		Printf ("CreateThread: Must call InitThreads first\n");
		Exit ();
	}

	if (setjmp (thread[0].env) == 0) {	// save context of thread 0

		// The new thread will need stack space.  Here we use the
		 * following trick: the new thread simply uses the current
		 * stack, and so there is no need to allocate space.  However,
		 * to ensure that thread 0's stack may grow and (hopefully)
		 * not bump into thread 1's stack, the top of the stack is
		 * effectively extended automatically by declaring a local
		 * variable (a large "dummy" array). This array is never
		 * actually used; to prevent an optimizing compiler from
		 * removing it, it should be referenced.  
		 //

		char s[STACKSIZE];	// reserve space for thread 0's stack
    void (*f)() = func;	// f saves func on top of stack
		int p = param;		// p saves param on top of stack

		if (((int) &s[STACKSIZE-1]) - ((int) &s[0]) + 1 != STACKSIZE) {
			Printf ("Stack space reservation failed\n");
			Exit ();
		}

		if (setjmp (thread[1].env) == 0) {	// save context of 1
			longjmp (thread[0].env, 1);	// back to thread 0
		}

		// here when thread 1 is scheduled for the first time 

		(*f) (p);			// execute func (param)

		MyExitThread ();		// thread 1 is done - exit
	}

	thread[1].valid = 1;	// mark the entry for the new thread valid

	return (1);		// done, return new thread ID
}
*/


int MyCreateThread (func, param)
	void (*func)();			// function to be executed
	int param;			// integer parameter
{
	if (! MyInitThreadsCalled) {
		Printf ("CreateThread: Must call InitThreads first\n");
		Exit ();
	}
  /*
	if (setjmp (thread[0].env) == 0) {	// save context of thread 0

    char s[STACKSIZE];
    void (*f)() = func;
    int p = param;

    if(((int) &s[STACKSIZE-1]) - ((int) &s[0]) + 1 != STACKSIZE)
    {
      Printf("Stack space reservation failed\n");
      Exit();
    }

    if(setjmp(thread[1].env) == 0) 
    {
      longjmp(thread[0].env, 1);
    }

    (*f) (p);
    MyExitThread();
  }

  thread[1].valid = 1;
  return(1);
  */

/*
  int count = 0;
  while( thread[((curThreadID + 1) % MAXTHREADS)].valid == 1)
  {
    curThreadID = (curThreadID + 1) % MAXTHREADS;
  
    if( count > MAXTHREADS + 3 )
    {
      return -1;
    }
    count++;
  }

  // found valid index for new thread
  curThreadID =  (curThreadID + 1) % MAXTHREADS;
*/
  
  if( threadNum >= MAXTHREADS)
    return -1;

  curThreadID = (curThreadID + 1) % MAXTHREADS;

  while( thread[curThreadID].valid == 1)
  {
    curThreadID = (curThreadID + 1) % MAXTHREADS;
  }

  memcpy(thread[curThreadID].env, thread[curThreadID].cleanSlat,
      sizeof(thread[curThreadID].cleanSlat));

  // update new thread's info
  thread[curThreadID].valid = 1;
  thread[curThreadID].myFunc = func;
  thread[curThreadID].myParam = param;

  // add new thread at the end of queue
  MyAddBack(curThreadID);
  
  return curThreadID;
} 


/*	MyYieldThread (t) causes the running thread, call it T, to yield to
 *	thread t.  Returns the ID of the thread that yielded to the calling
 *	thread T, or -1 if t is an invalid ID. Example: given two threads
 *	with IDs 1 and 2, if thread 1 calls MyYieldThread (2), then thread 2
 *	will resume, and if thread 2 then calls MyYieldThread (1), thread 1
 *	will resume by returning from its call to MyYieldThread (2), which
 *	will return the value 2.
 */

int MyYieldThread (t)
	int t;				// thread being yielded to
{
	if (! MyInitThreadsCalled) {
		Printf ("YieldThread: Must call InitThreads first\n");
		Exit ();
	}

	if (t < 0 || t >= MAXTHREADS) {
		Printf ("YieldThread: %d is not a valid thread ID\n", t);
		return (-1);
	}
	if (! thread[t].valid) {
		Printf ("YieldThread: Thread %d does not exist\n", t);
		return (-1);
	}

     //   if (setjmp (thread[1-t].env) == 0) {
       //         longjmp (thread[t].env, 1);
       // }

  //int temp = myQueue[myFront];
  int temp = myQueue[0];
  //remove front thread from queue
  MyRemove(temp);
  // then add it to back of queue
  MyAddBack(temp);
  //remove thread from queue ( the one going to be yield)
  MyRemove(t);
  //then add it to front of queue
  MyAddFront(t);

  prevThread = MyGetThread();
  if( MyGetThread() == t )
    return t;

  
  if( setjmp(thread[prevThread].env) == 0)
  {
    curThread = t;
    longjmp(thread[curThread].env, 1);
  }  

  return prevThread;
}


/*	MyGetThread () returns ID of currently running thread.  
 */

int MyGetThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("GetThread: Must call InitThreads first\n");
		Exit ();
	}
  return curThread;
}

/*	MySchedThread () causes the running thread to simply give up the
 *	CPU and allow another thread to be scheduled.  Selecting which
 *	thread to run is determined here. Note that the same thread may
 * 	be chosen (as will be the case if there are no other threads).  
 */

void MySchedThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("SchedThread: Must call InitThreads first\n");
		Exit ();
	}

  if( threadNum == 0)
    return;
  
  if( threadNum == 1)
    MyYieldThread(myQueue[0]);
  else
    MyYieldThread(myQueue[1]);

}

/*	MyExitThread () causes the currently running thread to exit.  
 */

void MyExitThread ()
{

	if (! MyInitThreadsCalled) {
		Printf ("ExitThread: Must call InitThreads first\n");
		Exit ();
	}


  int temp = MyGetThread();
  //temp = myQueue[0];
  thread[temp].valid = 0;
  // delete thread from thread queue
  MyRemove(temp);
  
  // if there is no more thread left
  if( threadNum == 0)
    Exit();
  else
    MyYieldThread(myQueue[0]);
}

/* add thread to front of queue */
void MyAddFront(int id)
{
  if( threadNum == MAXTHREADS)
    return;
  if( threadNum == 0)
  {
    //myFront = 0;
    //myBack = 0;
    myQueue[0] = id;
    threadNum++;
    //return;
  }
  else
  {
    for(int i = threadNum; i > 0; i-- )
    {
      myQueue[i] = myQueue[i - 1];
    }
    myQueue[0] = id;
    threadNum++;
    //myBack++;
  }
    
}

/* add thread to the end of queue */
void MyAddBack(int id)
{
  if( threadNum == MAXTHREADS)
    return;

  if( threadNum == 0)
  {
    //myBack = 0;
    myQueue[0] = id;
    threadNum++;
  }
  else
  {
    //myBack++;
    myQueue[threadNum] = id;
    threadNum++;
  }  
}

/* delete thread from queue */
void MyRemove(int id)
{
  int i;
  // find index
  for( i = 0; i < threadNum; i++)
  {
    if( myQueue[i] == id)
      break;
  }
  // check if didnt find
  if( i == threadNum)
    return;

  if( threadNum == 1)
  {
    myQueue[0] = -1;
    threadNum--;
    //myBack = 0 
  }
  else
  {
    // shift left
    for( int j = i; j < threadNum - 1; j++)
    {
      myQueue[j] = myQueue[j + 1];
    }
    myQueue[threadNum - 1] = -1;

    //myBack--;
    threadNum--;
  }
}


