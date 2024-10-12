#include "copyright.h"
#include "system.h"
#include "synch.h"

struct personThread
{
    int id;
    int atFloor;
    int toFloor;
};

struct elevatorThread
{
    int numFloors;
    int currentFloor;
    int numPeopleIn;
};

typedef struct personThread pThread;
typedef struct elevatorThread eThread;

int nextPersonID = 1;

Lock *personIDLock = new Lock("PersonIDLock");
Lock *elevatorLock = new Lock("ElevatorLock");
Condition *elevatorCondition = new Condition("ElevatorCondition");

typedef struct Person
{
    int id;
    int atFloor;
    int toFloor;
} Person;

List *waitingList = new List();

void ElevatorThread(int numFloors)
{
    eThread elev;
    elev.numFloors = numFloors;
    elev.numPeopleIn = 0;
    elev.currentFloor = 1;

    while (true)
    {
        elevatorLock->Acquire();

        // Check if there are any waiting people
        if (!waitingList->IsEmpty())
        {
            Person *p = (Person *)waitingList->Remove();
            printf("Elevator moving to pick up person %d at floor %d\n", p->id, p->atFloor);
            elev.currentFloor = p->atFloor;
            printf("Elevator picked up person %d\n", p->id);
            printf("Elevator moving to drop off person %d at floor %d\n", p->id, p->toFloor);
            elev.currentFloor = p->toFloor;
            printf("Elevator dropped off person %d\n", p->id);
        }

        elevatorCondition->Wait(elevatorLock);
        elevatorLock->Release();
    }
}

void Elevator(int numFloors)
{
    Thread *t = new Thread("elevator thread");
    t->Fork((VoidFunctionPtr)ElevatorThread, numFloors);
}

void PersonThread(int person)
{
    Person *p = (Person *)person;
    printf("Person %d wants to go from floor %d to %d\n", p->id, p->atFloor, p->toFloor);

    elevatorLock->Acquire();
    waitingList->Append((void *)p);
    elevatorCondition->Signal(elevatorLock);
    elevatorLock->Release();
}

int getNextPersonID()
{
    personIDLock->Acquire();
    int id = nextPersonID++;
    personIDLock->Release();
    return id;
}

void ArrivingGoingFromTo(int atFloor, int toFloor)
{
    Person *p = new Person;
    p->id = getNextPersonID();
    p->atFloor = atFloor;
    p->toFloor = toFloor;

    char threadName[20];
    sprintf(threadName, "person %d", p->id);
    Thread *t = new Thread(threadName);
    t->Fork((VoidFunctionPtr)PersonThread, (int)p);
}