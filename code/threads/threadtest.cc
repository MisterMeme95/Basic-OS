// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

#if defined(CHANGED) && defined(HW1_SEMAPHORES)
int SharedVariable;

Semaphore *s;
void
SimpleThread(int which)
{
    if(s == NULL)
    {
	    DEBUG('t', "USING SEMAPHORES\n");
	    s = new Semaphore("shared semaphore", 1);
    }

    int num, val;

    for (num = 0; num < 5; num++) {
        s->P();
        val = SharedVariable;
        printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
        SharedVariable = val+1;
        currentThread->Yield();

        s->V();
    }
    val=SharedVariable;

    while(threadsDone != 0)
	    currentThread->Yield();

    printf("Thread %d sees final value %d\n", which, val);
}

#elif defined(CHANGED) && defined(HW1_LOCKS)
int SharedVariable;
Lock *testLock;
void SimpleThread(int which)
{
    if(testLock == NULL)
    {
	DEBUG('t', "USING LOCKS\n");
	testLock = new  Lock("test");
    }
    int num, val;

    for(num = 0; num < 5; num++)
    {
	testLock->Acquire();

	val = SharedVariable;
	printf("*** thread %d sees value %d\n", which, val);
	currentThread->Yield();
	SharedVariable = val+1;

	testLock->Release();

	currentThread->Yield();
    }

    threadsDone--;
    while(threadsDone != 0)
	currentThread->Yield();
    val = SharedVariable;

    printf("Thread %d sees final value %d\n", which, val);
}

#else

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------
void
SimpleThread(int which)
{
    int num;

    for (num = 0; num < 5; num++) {
        printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

#endif

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
        case 1:
            ThreadTest1();
            break;
        default:
            printf("No test specified.\n");
            break;
    }
}

int numThreadsActive; // used to implement barrier upon completion

void
ThreadTest(int n) {
    DEBUG('t', "Entering SimpleTest");
    Thread *t;
    numThreadsActive = n;
    printf("NumthreadsActive = %d\n", numThreadsActive);

    for(int i=1; i<n; i++)
    {
        t = new Thread("forked thread");
        t->Fork(SimpleThread,i);
    }
    SimpleThread(0);
}

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "elevator.h"


int nextPersonID = 1;
Lock *personIDLock = new Lock("PersonIDLock");


ELEVATOR *e;


void ELEVATOR::start() {

    while(1) {

        // A. Wait until hailed

        // B. While there are active persons, loop doing the following
        //      0. Acquire elevatorLock
        //      1. Signal persons inside elevator to get off (leaving->broadcast(elevatorLock))
        //      2. Signal persons atFloor to get in, one at a time, checking occupancyLimit each time
        //      2.5 Release elevatorLock
        //      3. Spin for some time
        for(int j =0 ; j< 1000000; j++) {
            currentThread->Yield();
        }
        //      4. Go to next floor
        //  printf("Elevator arrives on floor %d", )
    }
}

void ElevatorThread(int numFloors) {

    printf("Elevator with %d floors was created!\n", numFloors);

    e = new ELEVATOR(numFloors);

    e->start();


}

ELEVATOR::ELEVATOR(int numFloors) {
    currentFloor = 1;
    entering = new Condition*[numFloors];
    // Initialize entering
    for (int i = 0; i < numFloors; i++) {
        entering[i] = new Condition("Entering " + i);
    }
    personsWaiting = new int[numFloors];
    elevatorLock = new Lock("ElevatorLock");

    // Initialize leaving
}


void Elevator(int numFloors) {
    // Create Elevator Thread
    Thread *t = new Thread("Elevator");
    t->Fork(ElevatorThread, numFloors);
}


void ELEVATOR::hailElevator(Person *p) {


    // 1. Increment waiting persons atFloor
    (e->personsWaiting[p->atFloor])++;

    // 2. Hail Elevator
    e->hailElevator(p);

    // 2.5 Acquire elevatorLock;


    // 3. Wait for elevator to arrive atFloor [entering[p->atFloor]->wait(elevatorLock)]




    // 5. Get into elevator
    printf("Person %d got into the elevator.\n", p->id);


    // 6. Decrement persons waiting atFloor [personsWaiting[atFloor]++]
    (e->personsWaiting[p->atFloor])--;

    // 7. Increment persons inside elevator [occupancy++]
    (e->occupancy)++;

    // 8. Wait for elevator to reach toFloor [leaving[p->toFloor]->wait(elevatorLock)]
    (e->leaving[p->toFloor])->Wait(elevatorLock);

    // 9. Get out of the elevator
    printf("Person %d got out of the elevator.\n", p->id);
    // 10. Decrement persons inside elevator
    // 11. Release elevatorLock;
}

void PersonThread(int person) {

    Person *p = (Person *)person;

    printf("Person %d wants to go from floor %d to %d\n", p->id, p->atFloor, p->toFloor);

    e->hailElevator(p);

}

int getNextPersonID() {
    int personID = nextPersonID;
    personIDLock->Acquire();
    nextPersonID = nextPersonID + 1;
    personIDLock->Release();
    return personID;
}


void ArrivingGoingFromTo(int atFloor, int toFloor) {


    // Create Person struct
    Person *p = new Person;
    p->id = getNextPersonID();
    p->atFloor = atFloor;
    p->toFloor = toFloor;

    // Creates Person Thread
    Thread *t = new Thread("Person " + p->id);
    t->Fork(PersonThread, (int)p);

}






