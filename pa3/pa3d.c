/* Programming Assignment 3: Exercise D
 *
 * Now that you have a working implementation of semaphores, you can
 * implement a more sophisticated synchronization scheme for the car
 * simulation.
 *
 * Study the program below.  Car 1 begins driving over the road, entering
 * from the East at 40 mph.  After 900 seconds, both Car 3 and Car 4 try to
 * enter the road.  Car 1 may or may not have exited by this time (it should
 * exit after 900 seconds, but recall that the times in the simulation are
 * approximate).  If Car 1 has not exited and Car 4 enters, they will crash
 * (why?).  If Car 1 has exited, Car 3 and Car 4 will be able to enter the
 * road, but since they enter from opposite directions, they will eventually
 * crash.  Finally, after 1200 seconds, Car 2 enters the road from the West,
 * and traveling twice as fast as Car 4.  If Car 3 was not coming from the
 * opposite direction, Car 2 would eventually reach Car 4 from the back and
 * crash.  (You may wish to experiment with reducing the initial delay of
 * Car 2, or increase the initial delay of Car 3, to cause Car 2 to crash
 * with Car 4 before Car 3 crashes with Car 4.)
 *
 *
 * Exercises
 *
 * 1. Modify the procedure driveRoad such that the following rules are obeyed:
 *
 *	A. Avoid all collisions.
 *
 *	B. Multiple cars should be allowed on the road, as long as they are
 *	traveling in the same direction.
 *
 *	C. If a car arrives and there are already other cars traveling in the
 *	SAME DIRECTION, the arriving car should be allowed enter as soon as it
 *	can. Two situations might prevent this car from entering immediately:
 *	(1) there is a car immediately in front of it (going in the same
 *	direction), and if it enters it will crash (which would break rule A);
 *	(2) one or more cars have arrived at the other end to travel in the
 *	opposite direction and are waiting for the current cars on the road
 *	to exit, which is covered by the next rule.
 *
 *	D. If a car arrives and there are already other cars traveling in the
 *	OPPOSITE DIRECTION, the arriving car must wait until all these other
 *	cars complete their course over the road and exit.  It should only wait
 *	for the cars already on the road to exit; no new cars traveling in the
 *	same direction as the existing ones should be allowed to enter.
 *
 *	E. This last rule implies that if there are multiple cars at each end
 *	waiting to enter the road, each side will take turns in allowing one
 *	car to enter and exit.  (However, if there are no cars waiting at one
 *	end, then as cars arrive at the other end, they should be allowed to
 *	enter the road immediately.)
 *	
 *	F. If the road is free (no cars), then any car attempting to enter
 *	should not be prevented from doing so.
 *
 *	G. All starvation must be avoided.  For example, any car that is
 *	waiting must eventually be allowed to proceed.
 *
 * This must be achieved ONLY by adding synchronization and making use of
 * shared memory (as described in Exercise C).  You should NOT modify the
 * delays or speeds to solve the problem.  In other words, the delays and
 * speeds are givens, and your goal is to enforce the above rules by making
 * use of only semaphores and shared memory.
 *
 * 2. Devise different tests (using different numbers of cars, speeds,
 * directions) to see whether your improved implementation of driveRoad
 * obeys the rules above.
 *
 * IMPLEMENTATION GUIDELINES
 * 
 * 1. Avoid busy waiting. In class one of the reasons given for using
 * semaphores was to avoid busy waiting in user code and limit it to
 * minimal use in certain parts of the kernel. This is because busy
 * waiting uses up CPU time, but a blocked process does not. You have
 * semaphores available to implement the driveRoad function, so you
 * should not use busy waiting anywhere.
 *
 * 2. Prevent race conditions. One reason for using semaphores is to
 * enforce mutual exclusion on critical sections to avoid race conditions.
 * You will be using shared memory in your driveRoad implementation.
 * Identify the places in your code where there may be race conditions
 * (the result of a computation on shared memory depends on the order
 * that processes execute).  Prevent these race conditions from occurring
 * by using semaphores.
 *
 * 3. Implement semaphores fully and robustly.  It is possible for your
 * driveRoad function to work with an incorrect implementation of
 * semaphores, because controlling cars does not exercise every use of
 * semaphores.  You will be penalized if your semaphores are not correctly
 * implemented, even if your driveRoad works.
 *
 * 4. Control cars with semaphores: Semaphores should be the basic
 * mechanism for enforcing the rules on driving cars. You should not
 * force cars to delay in other ways inside driveRoad such as by calling
 * the Delay function or changing the speed of a car. (You can leave in
 * the delay that is already there that represents the car's speed, just
 * don't add any additional delaying).  Also, you should not be making
 * decisions on what cars do using calculations based on car speed (since
 * there are a number of unpredictable factors that can affect the
 * actual cars' progress).
 *
 * GRADING INFORMATION
 *
 * 1. Semaphores: We will run a number of programs that test your
 * semaphores directly (without using cars at all). For example:
 * enforcing mututal exclusion, testing robustness of your list of
 * waiting processes, calling signal and wait many times to make sure
 * the semaphore state is consistent, etc.
 *
 * 2. Cars: We will run some tests of cars arriving in different ways,
 * to make sure that you correctly enforce all the rules for cars given
 * in the assignment.  We will use a correct semaphore implementation for
 * these tests so that even if your semaphores are not correct you could
 * still get full credit on the driving part of the grade.  Think about
 * how your driveRoad might handle different situations and write your
 * own tests with cars arriving in different ways to make sure you handle
 * all cases correctly.
 *
 *
 * WHAT TO TURN IN
 *
 * You must turn in two files: mykernel3.c and p3d.c.  mykernel3.c should
 * contain you implementation of semaphores, and p3d.c should contain
 * your modified version of InitRoad and driveRoad (Main will be ignored).
 * Note that you may set up your static shared memory struct and other
 * functions as you wish. They should be accessed via InitRoad and driveRoad,
 * as those are the functions that we will call to test your code.
 *
 * Your programs will be tested with various Main programs that will exercise
 * your semaphore implementation, AND different numbers of cars, directions,
 * and speeds, to exercise your driveRoad function.  Our Main programs will
 * first call InitRoad before calling driveRoad.  Make sure you do as much
 * rigorous testing yourself to be sure your implementations are robust.
 */

#include <stdio.h>
#include "aux.h"
#include "sys.h"
#include "umix.h"

void InitRoad (void);
void driveRoad (int from, int mph);
void tester1();
void tester2();
void tester3();
void test_2();
void test_3();
void test_4();
void test_5();
void test_999x();

static struct
{
  int wCarOnRoad;
  int eCarOnRoad;
  int roadPos[NUMPOS + 1];
  int wCarWait;
  int eCarWait;
  int wCount;
  int eCount;
  int wDoor;
  int eDoor;
  //int wMyFlag;
  //int eMyFlag;
  int wfirstFlag;
  int efirstFlag;
  int wFirstPos;
  int eFirstPos;
  int mutex;
  int printMutex;

}roadProc;

void Main ()
{
	InitRoad ();

	/* The following code is specific to this particular simulation,
	 * e.g., number of cars, directions, and speeds.  You should
	 * experiment with different numbers of cars, directions, and
	 * speeds to test your modification of driveRoad.  When your
	 * solution is tested, we will use different Main procedures,
	 * which will first call InitRoad before any calls to driveRoad.
	 * So, you should do any initializations in InitRoad.
	 */

	if (Fork () == 0) {
		Delay (1200);			// car 2
		driveRoad (WEST, 60);
		Exit ();
	}

	if (Fork () == 0) {
		Delay (900);			// car 3
		driveRoad (EAST, 50);
		Exit ();
	}

	if (Fork () == 0) {
		Delay (900);			// car 4
		driveRoad (WEST, 30);
		Exit ();
	}

	driveRoad (EAST, 40);			// car 1

	Exit ();
  


/* 
  if (Fork () == 0) {
    Delay (0);                      // car 2
    driveRoad (EAST, 30);
    Exit ();
   }
  if (Fork () == 0) {
    Delay (0);                      // car 3
    driveRoad (EAST, 30);
    Exit ();
  }
  if (Fork () == 0) {
    Delay (0);                 // car 4
    driveRoad (EAST, 900);
    Exit ();
  }
 if (Fork () == 0) {
    Delay (1200);                   //  car 5
    driveRoad (WEST, 30);
    Exit ();
  }
 
  if (Fork () == 0) {
    Delay (450);                    // car 6   // car 5
    driveRoad (EAST, 30);
    Exit ();
  }

  if (Fork () == 0) {
    Delay (460);                    // car 7
    driveRoad (EAST, 30);
    Exit ();
  }
  
  if (Fork () == 0) {
    Delay (450);                    // car 8
    driveRoad (WEST, 30);
    Exit ();
  }
  if (Fork () == 0) {
    Delay (900);                    // car 9
    driveRoad (EAST, 30);
    Exit ();
  }


 if (Fork () == 0) {
   Delay (0);                      // car 10  car 6
   driveRoad (EAST, 30);
   Exit ();
  }

  driveRoad (EAST, 30);                   // car 1

  Exit ();
*/             

 // tester1();
 // tester2();
 //tester3(); 
  //test_2();
  //test_3();
 //test_4();
 // test_5();
  //test_999();
 // test_999x();


}

/* Our tests will call your versions of InitRoad and driveRoad, so your
 * solution to the car simulation should be limited to modifying the code
 * below.  This is in addition to your implementation of semaphores
 * contained in mykernel3.c.
 */

void InitRoad ()
{
	/* do any initializations here */
  Regshm ((char *) &roadProc, sizeof(roadProc));

  roadProc.wCarOnRoad = 0;
  roadProc.eCarOnRoad = 0;
  roadProc.wCarWait = 0;
  roadProc.eCarWait = 0;
  roadProc.wfirstFlag = Seminit(0);
  roadProc.efirstFlag = Seminit(0);
  roadProc.wFirstPos = 0;
  roadProc.eFirstPos = 0;
  roadProc.wCount = 0;
  roadProc.eCount = 0;
  //roadProc.eMyFlag = 0;
  //roadProc.wMyFlag = 0;
  roadProc.wDoor = Seminit(0);
  roadProc.eDoor = Seminit(0);
  roadProc.mutex = Seminit(1);
  roadProc.printMutex = Seminit(1);

  for( int i = 1; i < NUMPOS + 1; i++)
  {
    roadProc.roadPos[i] = Seminit(1);
  }

}

#define IPOS(FROM)	(((FROM) == WEST) ? 1 : NUMPOS)

void driveRoad (from, mph)
	int from;			// coming from which direction
	int mph;			// speed of car
{
	int c;				// car ID c = process ID
	int p, np, i;			// positions

	c = Getpid ();			// learn this car's ID

//  Printf("car %d is coming \n", c);

  Wait(roadProc.mutex);
  if( from == EAST )
  {
    if( roadProc.eFirstPos == 1 )
    {
      roadProc.eCount++;
      Signal(roadProc.mutex);
      Wait(roadProc.efirstFlag);
    }
    else
      Signal(roadProc.mutex);
  }
  else
  {
    if( roadProc.wFirstPos == 1)
    {
      roadProc.wCount++;
      Signal(roadProc.mutex);
      Wait(roadProc.wfirstFlag);
    }
    else
      Signal(roadProc.mutex);
  }

  Wait(roadProc.mutex);

  // car from west
  if( from  == WEST)
  {
    if( roadProc.eCarOnRoad > 0 || roadProc.eCarWait > 0)
    {
      roadProc.wCarWait++;
      Signal(roadProc.mutex);
      Wait(roadProc.wDoor);
      /*
      if( roadProc.wCarWait > 0 && roadProc.eCarWait == 0)
      {
        roadProc.wCarWait--;
        Signal(roadProc.wDoor);
      }*/
    }
    else
      Signal(roadProc.mutex);


  }
  //car from east
  else
  {
    if( roadProc.wCarOnRoad > 0 || roadProc.wCarWait > 0)
    {
  //    Printf(" Which car is blocked %d\n", c);
      roadProc.eCarWait++;
      Signal(roadProc.mutex);
      Wait(roadProc.eDoor);
      /*
      if( roadProc.eCarWait> 0 && roadProc.wCarWait == 0)
      {
        roadProc.eCarWait--;
        Signal(roadProc.eDoor);
      }*/
    }
    else
      Signal(roadProc.mutex);

  }

  
  // wait for first position
  Wait( roadProc.roadPos[IPOS(from)]);
 // Printf("car id is entering %d \n", c);


  EnterRoad(from);

  // car from east, set first pos and num of car on road
  if( from == EAST )
  {

    Wait( roadProc.mutex);
    roadProc.eFirstPos = 1;
    roadProc.eCarOnRoad++;  
    Signal(roadProc.mutex);
  }
  //car from west, set first pos and num of car on road
  else
  {
    Wait(roadProc.mutex);
    roadProc.wFirstPos = 1;
    roadProc.wCarOnRoad++;
    Signal(roadProc.mutex);
  }
/*
  Wait( roadProc.mutex );
  // car from  east
  if( from == EAST ) 
  {
    Signal( roadProc.mutex);
    roadProc.eCarOnRoad++;
  }
  // car from west
  else
  {
    Signal(roadProc.mutex);
    roadProc.wCarOnRoad++;
  }
*/
  Wait(roadProc.printMutex);
	PrintRoad ();
	Printf ("Car %d enters at %d at %d mph\n", c, IPOS(from), mph);
  Signal(roadProc.printMutex);

	for (i = 1; i < NUMPOS; i++) {
		if (from == WEST) {
			p = i;
			np = i + 1;
		} else {
			p = NUMPOS + 1 - i;
			np = p - 1;
		}

		Delay (3600/mph);


    Wait(roadProc.roadPos[np]);

		ProceedRoad ();

    Signal(roadProc.roadPos[p]);

    Wait(roadProc.printMutex);
		PrintRoad ();
		Printf ("Car %d moves from %d to %d\n", c, p, np);
    Signal(roadProc.printMutex);
    
    // when car move from pos 1 to pos 2, do some modifications
    if( i == 1 && from == EAST)
    {
      Wait(roadProc.mutex);
      roadProc.eFirstPos = 0;
      if( roadProc.eCount > 0)
      {
        roadProc.eCount--;  // add here
        Signal(roadProc.efirstFlag);
      }
      if(roadProc.eCarWait > 0 && roadProc.wCarWait == 0)
      {
        Signal(roadProc.mutex);
        roadProc.eCarWait--;
        Signal(roadProc.eDoor);
      }
      else
        Signal(roadProc.mutex);
    }
    else if( i == 1 && from == WEST)
    {
      Wait( roadProc.mutex);
      roadProc.wFirstPos = 0;
      if( roadProc.wCount > 0)
      {
        roadProc.wCount--;   // add here
        Signal(roadProc.wfirstFlag);
      }
      if( roadProc.wCarWait > 0 && roadProc.eCarWait == 0)
      {
        Signal(roadProc.mutex);
        roadProc.wCarWait--;
        Signal(roadProc.wDoor);
      }
      else
        Signal(roadProc.mutex);
    }

	}

	Delay (3600/mph);
	ProceedRoad ();

  Signal(roadProc.roadPos[np]);

  Wait(roadProc.printMutex);
	PrintRoad ();
	Printf ("Car %d exits road\n", c);
  Signal(roadProc.printMutex);

  // car exit from east
  if ( from  == EAST )
  {
    Wait( roadProc.mutex );
    roadProc.eCarOnRoad--;
//    roadProc.eMyFlag--;
    Signal( roadProc.mutex);
    if( roadProc.eCarOnRoad == 0 && roadProc.wCarWait > 0 )
    {
      roadProc.wCarWait--;
      Signal(roadProc.wDoor);
    }

  }
  // car exit from west
  else if( from == WEST)
  {

    Wait( roadProc.mutex);
    roadProc.wCarOnRoad--;
  //  roadProc.wMyFlag--;
    Signal(roadProc.mutex);
    if( roadProc.wCarOnRoad == 0 && roadProc.eCarWait > 0 )
    {
      roadProc.eCarWait--;
      Signal(roadProc.eDoor);
    }

  }
 
}

void tester1()
{
  if(Fork() == 0)
  {
    Delay(20);   // car 2
    driveRoad(WEST, 50);
    Exit();
  }

  if(Fork() == 0)
  {
    Delay(50);  //car 3
    driveRoad(EAST, 50);
    Exit();
  }

  driveRoad(WEST, 1); //car 1

  Exit();

}

void tester2()
{
  if(Fork() == 0)
  {
    Delay(500);
    driveRoad(WEST, 50);   // car 2
    Exit();
  }

  if( Fork() == 0)
  {
    Delay(510);
    driveRoad(WEST, 50);   // car 3
    Exit();
  }
  
  if( Fork() == 0)
  {
    Delay(620);
    driveRoad(EAST, 50);  // car 4
    Exit();
  }

  if( Fork() == 0 )
  {
    Delay(600);
    driveRoad(WEST, 50);  // car 5
    Exit();
  }

  if( Fork() == 0 )
  {
    Delay(700);
    driveRoad(WEST, 50);  // car 6
    Exit();
  }
  if( Fork() == 0) 
  {
    Delay(700);
    driveRoad(WEST, 50);  // car 7
    Exit();
  }

  driveRoad(WEST, 60);  // car 1
  Exit();
}

void tester3()
{
if (Fork () == 0) {
      Delay(5);           //car 2
          DPrintf("Car 2 wants to drive!!!\n");
              driveRoad (WEST, 90);
                  Exit ();
                    }

  if (Fork () == 0) {
        Delay (10);
            DPrintf("Car 3 wants to drive!!!\n");
                driveRoad (EAST, 90); //car 3
                    Exit ();
                      }

    if (Fork () == 0) {
          Delay (5);
              DPrintf("Car 4 wants to drive!!!\n");
                  driveRoad (WEST, 90); //car 4
                      Exit ();
                        }

      if (Fork () == 0) {
            Delay (5000);                       // car 5
                DPrintf("Car 5 wants to drive!!!\n");
                    driveRoad (EAST, 90);
                        Exit ();
                          }

        if (Fork () == 0) {
              Delay (5);                  // car 6
                  Printf("Car 6 wants to drive!!!\n");
                      driveRoad (WEST, 90);
                          Exit ();
                            }

          if (Fork () == 0) {
                Delay (5);                  // car 7
                    DPrintf("Car 7 wants to drive!!!\n");
                        driveRoad (WEST, 5);
                            Exit ();
                              }
            if (Fork () == 0) {
                  Delay (10);                 // car 8
                      DPrintf("Car 8 wants to drive!!!\n");
                          driveRoad (EAST, 90);
                              Exit ();
                                }
              if (Fork () == 0) {
                    Delay (20);                 // car 9
                        DPrintf("Car 9 wants to drive!!!\n");
                            driveRoad (WEST, 90);
                                Exit ();
                                  }
                if (Fork () == 0) {
                      Delay (20);                 // car 10
                          DPrintf("Car 10 wants to drive!!!\n");
                              driveRoad (WEST, 90);
                                  Exit ();
                                    }

                  driveRoad (WEST, 10);                 // car 1
                    Exit ();
}


void test_2 ()
{
    // car 2
    if (Fork () == 0) {
          Delay (200);
              driveRoad (EAST, 20);
                  Exit ();
                    }
      
      // car 3
      if (Fork () == 0) {
            Delay (300);
                driveRoad (WEST, 20);
                    Exit ();
                      }
        
        // car 1
        driveRoad (WEST, 20);
          Exit ();
}

void test_3 ()
{
    // car 2
    if (Fork () == 0) {
          Delay (10);
              Printf("CAR %d HAS ARRIVED\n", Getpid());
                  driveRoad (WEST, 20);
                      Exit ();
                        }
      
      // car 3
      if (Fork () == 0) {
            Delay (15);
                Printf("CAR %d HAS ARRIVED\n", Getpid());
                    driveRoad (WEST, 20);
                        Exit ();
                          }
        
        // car 4
        if (Fork () == 0) {
              Delay (20);
                  Printf("CAR %d HAS ARRIVED\n", Getpid());
                      driveRoad (WEST, 20);
                          Exit ();
                            }
          
          // car 5
          if (Fork () == 0) {
                Delay (30);
                    Printf("CAR %d HAS ARRIVED\n", Getpid());
                        driveRoad (EAST, 20);
                            Exit ();
                              }
            
            // car 6
            if (Fork () == 0) {
                  Delay (35);
                      Printf("CAR %d HAS ARRIVED\n", Getpid());
                          driveRoad (EAST, 20);
                              Exit ();
                                }
              
              // car 7
              if (Fork () == 0) {
                    Delay (40);
                        Printf("CAR %d HAS ARRIVED\n", Getpid());
                            driveRoad (EAST, 20);
                                Exit ();
                                  }
                
                // car 1
                Printf("CAR %d HAS ARRIVED\n", Getpid());
                  driveRoad (WEST, 20);
                    Exit ();
}

void test_4 ()
{
    // car 2
    if (Fork () == 0) {
          Delay (5);
              driveRoad (WEST, 20);
                  Exit ();
                    }
      
      // car 3
      if (Fork () == 0) {
            Delay (15);
                driveRoad (EAST, 20);
                    Exit ();
                      }
        
        // car 1
        driveRoad (WEST, 20);
          Exit ();
}

void test_5 ()
{

    // car 2
    if (Fork () == 0) {
          Delay (100);
              Printf("CAR %d HAS ARRIVED\n", Getpid ());
                  driveRoad (EAST, 10);
                      Exit ();
                        }

      // car 3
      if (Fork () == 0) {
            Delay (150);
                Printf("CAR %d HAS ARRIVED\n", Getpid ());
                    driveRoad (EAST, 10);
                        Exit ();
                          }

        // car 4
        if (Fork () == 0) {
              Delay (200);
                  Printf("CAR %d HAS ARRIVED\n", Getpid ());
                      driveRoad (EAST, 10);
                          Exit ();
                            }

          // car 5
          if (Fork () == 0) {
                Delay (1800);
                    Printf("CAR %d HAS ARRIVED\n", Getpid ());
                        driveRoad (WEST, 10);
                            Exit ();
                              }

            //car 1
            Printf("CAR %d HAS ARRIVED\n", Getpid ());
              driveRoad (WEST, 20);
                Exit ();
}

void test_999x ()
{
  if (Fork () == 0) {
    driveRoad(EAST, 100);
    Exit ();
  }

  if (Fork () == 0) {
    driveRoad(WEST, 100);
    Exit ();
  }

  if (Fork () == 0) {
    driveRoad(EAST, 100);
    Exit ();
  }

  if (Fork () == 0) {
    driveRoad(WEST, 100);
    Exit ();
  }

  if (Fork () == 0) {
    driveRoad(EAST, 100);
    Exit ();
  }

  if (Fork () == 0) {
    driveRoad(WEST, 100);
    Exit ();
  }

  if (Fork () == 0) {
    driveRoad(EAST, 100);
    Exit ();
  }

  if (Fork () == 0) {
    driveRoad(WEST, 100);
    Exit ();
  }

  if (Fork () == 0) {
    driveRoad(EAST, 100);
    Exit ();
  }

  if (Fork () == 0) {
    driveRoad(WEST, 100);
    Exit ();
  }
  driveRoad(WEST, 100);
  Exit ();
}

