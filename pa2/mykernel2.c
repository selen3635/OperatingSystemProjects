/* mykernel2.c: your portion of the kernel
 *
 *	Below are procedures that are called by other parts of the kernel. 
 *	Your ability to modify the kernel is via these procedures.  You may
 *	modify the bodies of these procedures any way you wish (however,
 *	you cannot change the interfaces).  
 * 
 */

#include "aux.h"
#include "sys.h"
#include "mykernel2.h"

#define TIMERINTERVAL 1000000	// in ticks (tick = 10 msec)
int L = 100000;

void printrrQueue();
/*	A sample process table. You may change this any way you wish.  
 */

static struct {
	int valid;		// is this entry valid: 1 = yes, 0 = no
	int pid;		// process ID (as provided by kernel)
} proctab[MAXPROCS];

/* structure for FIFO and LIFO */
static struct 
{
  int valid;
  int pid;
} LFQueue[MAXPROCS];

/* Structure for RoundRobin */
static struct
{
  int valid;
  int pid;

}rrQueue[MAXPROCS];

/* structure for PROPORTIONAL */
static struct
{
  int valid;
  int pid;
  int cpuRatio;
  int myFlag;  // indicate whether this process has request cpu 
  //int myRequest;
  int myStride;
  int totalStride;

}proQueue[MAXPROCS];

int rrIndex;
int rrPointer = 0;
int rrProc;
int leftCPU = 100;

int totalCPU = 0;


/*	InitSched () is called when kernel starts up.  First, set the
 *	scheduling policy (see sys.h). Make sure you follow the rules
 *	below on where and how to set it.  Next, initialize all your data
 *	structures (such as the process table).  Finally, set the timer
 *	to interrupt after a specified number of ticks. 
 */

void InitSched ()
{
	int i;

  rrIndex = 0;
  rrProc = 0;

	/* First, set the scheduling policy.  You should only set it
	 * from within this conditional statement.  While you are working
	 * on this assignment, GetSchedPolicy will return NOSCHEDPOLICY,
	 * and so the condition will be true and you may set the scheduling
	 * policy to whatever you choose (i.e., you may replace ARBITRARY). 
	 * After the assignment is over, during the testing phase, we will
	 * have GetSchedPolicy return the policy we wish to test, and so
	 * the condition will be false and SetSchedPolicy will not be
	 * called, thus leaving the policy to whatever we chose to test.  
	 */
	if (GetSchedPolicy () == NOSCHEDPOLICY) {	// leave as is
		SetSchedPolicy (ROUNDROBIN);		// set policy here
	}
		
	/* Initialize all your data structures here */
	for (i = 0; i < MAXPROCS; i++) {
		proctab[i].valid = 0;
    LFQueue[i].valid = 0;
    proQueue[i].valid = 0;
    rrQueue[i].valid = 0;
    //proQueue[i].myFlag = 0;
    //proQueue[i].cpuRatio = 0;
    //proQueue[i].myRequest = 0;
    proQueue[i].myStride = 0;
    proQueue[i].totalStride = 0;
 
	}

	/* Set the timer last */
	SetTimer (TIMERINTERVAL);
}


/*	StartingProc (pid) is called by the kernel when the process
 *	identified by pid is starting.  This allows you to record the
 *	arrival of a new process in the process table, and allocate
 *	any resources (if necessary). Returns 1 if successful, 0 otherwise.  
 */

int StartingProc (pid)
	int pid;			// process that is starting
{
	int i, j;
/*
	for (i = 0; i < MAXPROCS; i++) {
		if (! proctab[i].valid) {
			proctab[i].valid = 1;
			proctab[i].pid = pid;
			return (1);
		}
	}
*/
  /* starting process for RoundRobin */
  if( GetSchedPolicy() == ROUNDROBIN) 
  {
    if ( !rrQueue[rrPointer].valid )
    {
      rrQueue[rrPointer].valid = 1;
      rrQueue[rrPointer].pid = pid;
      if( (rrPointer + 1) <= MAXPROCS)
      {
        rrPointer++;
        return (1);
      }

    }
  }

  /* starting process for FIFO and LIFO */
  if( GetSchedPolicy() == FIFO || GetSchedPolicy() == LIFO )
  {
    if( pid <= MAXPROCS && LFQueue[pid - 1].valid == 0) {

      LFQueue[pid - 1].valid = 1;
      LFQueue[pid - 1].pid = pid;
      if ( GetSchedPolicy() == LIFO ) {
      
        DoSched();
      }
      return (1);
    }
  }

  /* starting process for proportional */
  if( GetSchedPolicy() == PROPORTIONAL )
  {
    for( j = 0; j < MAXPROCS; j++ ) 
    {
      if( !proQueue[j].valid)
      {
        proQueue[j].valid = 1;
        proQueue[j].pid = pid;
        proQueue[j].cpuRatio = 0;
        proQueue[j].myFlag = 0;
        proQueue[j].myStride = 0;
        proQueue[j].totalStride = 0;
        return (1);
      }

    }
  }  

  /* starting process for Arbitrary */
  if( GetSchedPolicy() == ARBITRARY )
  {
	  for (i = 0; i < MAXPROCS; i++) {
		  if (! proctab[i].valid) {
		  	proctab[i].valid = 1;
		  	proctab[i].pid = pid;
		  	return (1);
	  	}
	  }
  }

 
	Printf ("Error in StartingProc: no free table entries\n");
	return (0);
}
			

/*	EndingProc (pid) is called by the kernel when the process
 *	identified by pid is ending.  This allows you to update the
 *	process table accordingly, and deallocate any resources (if
 *	necessary). Returns 1 if successful, 0 otherwise.  
 */


int EndingProc (pid)
	int pid;			// process that is ending
{
	int i;

  /* For RoundRobin */
  if( GetSchedPolicy() == ROUNDROBIN )
  {
    for( i = 0; i < MAXPROCS; i++ ) 
    {
      if( rrQueue[i].valid && rrQueue[i].pid == pid) 
      {
        rrQueue[i].valid = 0;
        return (1);
      }

    }
  }

  /* for FIFO and LIFO */
  if( GetSchedPolicy() == FIFO || GetSchedPolicy() == LIFO )
  {
    for ( i = 0; i < MAXPROCS; i++ ) 
    {
      if( LFQueue[i].valid && LFQueue[i].pid == pid)
      {
        LFQueue[i].valid = 0;
        return (1);
      }
    }
  }

  /* for proportional */
  if( GetSchedPolicy() == PROPORTIONAL )
  {
    for( i = 0; i < MAXPROCS; i++ ) 
    {
      if( proQueue[i].valid && proQueue[i].pid == pid)
      {
        proQueue[i].valid = 0;

        proQueue[i].totalStride = 0;
        if( proQueue[i].myFlag)
          totalCPU = totalCPU - proQueue[i].cpuRatio;
        
        proQueue[i].myFlag = 0;
        leftCPU = (100 - totalCPU);
        int count = 0;
        for( int i = 0; i < MAXPROCS; i++ ) 
        {
          if ( proQueue[i].valid && !proQueue[i].myFlag)
            count++;

        }
       if ( count == 0 ) 
       {
          return (1);
       }
       else 
       { 
          int equalAssign = (leftCPU / count);

          for( int j = 0; j < MAXPROCS; j++ )
          {
            if( proQueue[j].valid && !proQueue[j].myFlag )
            {
              if( equalAssign == 0)
              {
                proQueue[j].cpuRatio = equalAssign;
                proQueue[j].totalStride = 555555555;
              }
              else
              {
                proQueue[j].cpuRatio = equalAssign;
                proQueue[j].myStride = L / equalAssign;
              //  proQueue[j].totalStride = 0;
                  for( int k = 0; k < MAXPROCS; k++ )
                  {
                    if(proQueue[k].valid)
                      proQueue[k].totalStride = 0;
                  }
 
              }
            }
          }
       }
        return (1);
      }
    }
  }

  /* For Arbitrary */
  if( GetSchedPolicy() == ARBITRARY ) 
  {
	  for (i = 0; i < MAXPROCS; i++) {
		  if (proctab[i].valid && proctab[i].pid == pid) {
		  	proctab[i].valid = 0;
		  	return (1);
	  	}
	  }
  }

	Printf ("Error in EndingProc: can't find process %d\n", pid);
	return (0);
}


/*	SchedProc () is called by kernel when it needs a decision for
 *	which process to run next.  It calls the kernel function
 *	GetSchedPolicy () which will return the current scheduling policy
 *	which was previously set via SetSchedPolicy (policy). SchedProc ()
 *	should return a process id, or 0 if there are no processes to run.  
 */

int SchedProc ()
{
	int i;
  int j;
  int k;
  //double val = 1000000.0;

	switch (GetSchedPolicy ()) {

	case ARBITRARY:

		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid) {
				return (proctab[i].pid);
			}
		}
		break;

	case FIFO:

		/* your code here */
    for( i = 0; i < MAXPROCS; i++) {

      if(LFQueue[i].valid) {
        return (LFQueue[i].pid);
      }
    }

		break;

	case LIFO:

		/* your code here */

    for( i = MAXPROCS -1; i >=0; i-- ) {

      if(LFQueue[i].valid) {
        return (LFQueue[i].pid);
      }
    }


		break;

	case ROUNDROBIN:

		/* your code here */
    for( i = rrIndex, j = 0; j < MAXPROCS; i = (i +1)%MAXPROCS, j++ )
    {
      if ( rrQueue[i].valid)
      {
        rrIndex = (i + 1)%MAXPROCS;
        rrProc = rrQueue[i].pid;
//        Printf(" process id is  %d : \n", rrProc);
  //      Printf(" The index is  %d :  \n ", i);
        return rrProc;

      }

    }

		break;

	case PROPORTIONAL:

    {
      int i, beginPID, minStride; 
      int myBool = 0;
      int j;
      
       //   Printf("%d\n",proQueue[3].pid);
      for( i = 0; i < MAXPROCS; i++ )
      {
        if( proQueue[i].valid)
        {
         // Printf(" i = %d, %d's totalStride: %d\n",i, proQueue[i].pid,
              //proQueue[i].totalStride);
          if( !myBool)
          { 
            beginPID = proQueue[i].pid;
            minStride = proQueue[i].totalStride;
            j = i;
            myBool = 1;

          }
          else if ( proQueue[i].totalStride < minStride)
          {
            if( proQueue[i].myStride != 0 )
              beginPID = proQueue[i].pid;
            minStride = proQueue[i].totalStride;
            j = i;

          }

        }
      }
      // optimize proportional
      for( i = 0; i < MAXPROCS; i++ )
      {
        if( proQueue[i].valid )
        {
          proQueue[i].totalStride -= minStride;

        }

      }
      if ( myBool)
      {
        proQueue[j].totalStride += proQueue[j].myStride;
        return (beginPID);

      }
      
    }
    break;
	}
	
	return (0);
}


/*	HandleTimerIntr () is called by the kernel whenever a timer
 *	interrupt occurs.  
 */

void HandleTimerIntr ()
{
	SetTimer (TIMERINTERVAL);

	switch (GetSchedPolicy ()) {	// is policy preemptive?

	case ROUNDROBIN:		// ROUNDROBIN is preemptive
    DoSched();
    break;
	case PROPORTIONAL:		// PROPORTIONAL is preemptive

		DoSched ();		// make scheduling decision
		break;

	default:			// if non-preemptive, do nothing
		break;
	}
}

/*	MyRequestCPUrate (pid, n) is called by the kernel whenever a process
 *	identified by pid calls RequestCPUrate (n). This is a request for
 *	n% of CPU time, i.e., requesting a CPU whose speed is effectively
 *	n% of the actual CPU speed.  Roughly n out of every 100 quantums
 *	should be allocated to the calling process.  n must be greater than
 *	0 and must be less than or equal to 100. MyRequestCPUrate (pid, n)
 *	should return 0 if successful, i.e., if such a request can be
 *	satisfied, otherwise it should return -1, i.e., error (including if
 *	n < 1 or n > 100).  If MyRequestCPUrate (pid, n) fails, it should
 *	have no effect on scheduling of this or any other process, i.e., AS
 *	IF IT WERE NEVER CALLED.
 */

int MyRequestCPUrate (pid, n)
	int pid;			// process whose rate to change
	int n;				// percent of CPU time
{
  int i;
	/* your code here */
  if ( n < 1 || n > 100 ){
    return -1;
  }
  //double curCPU = (double)n;
  for( i = 0; i < MAXPROCS; i++) 
  {
    if ( proQueue[i].valid && proQueue[i].pid == pid)
    {
      /* check if it requested cpu before */
      if(proQueue[i].myFlag)
      {
        /* check if over allocate */
        if((totalCPU - proQueue[i].cpuRatio + n) > 100)
        {
          return -1;
        }

        totalCPU = totalCPU - proQueue[i].cpuRatio + n;
       
      }
      /* this process didnt request cpu before */
      else
      {
        /* check if it over allocate */
        if( (totalCPU + n ) > 100 )
        {
          return -1;

        }
        totalCPU = totalCPU + n;
        proQueue[i].myFlag = 1;
      }

      proQueue[i].cpuRatio = n;
      proQueue[i].myStride = L / n;
   //   proQueue[i].totalStride = 0;
      for( int j = 0; j < MAXPROCS; j++ )
      {
        if(proQueue[j].valid)
          proQueue[j].totalStride = 0;
      }
    }

  }
  
  leftCPU = (100 - totalCPU);

  
  int count = 0;
  for( int i = 0; i < MAXPROCS; i++ ) 
  {
    if ( proQueue[i].valid && !proQueue[i].myFlag)
      count++;

  }

  if ( count == 0 ) 
  { 
    return (0);
  }
  else 
  { 
    int equalAssign = (leftCPU / count);

    for( int i = 0; i < MAXPROCS; i++ )
    {
      if( proQueue[i].valid && !proQueue[i].myFlag )
      {
        if( equalAssign == 0)
        {
          proQueue[i].cpuRatio = equalAssign;
          proQueue[i].totalStride = 555555555;
        }
        else
        {
          proQueue[i].cpuRatio = equalAssign;
          proQueue[i].myStride = L / equalAssign;
       //   proQueue[i].totalStride = 0;
          for( int j = 0; j < MAXPROCS; j++ )
          {
            if(proQueue[j].valid)
              proQueue[j].totalStride = 0;
          }
 
        }
      }
    }
  }
  
  return (0);
}
