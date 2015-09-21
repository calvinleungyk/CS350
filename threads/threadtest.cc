// threadtest.cc
//  Simple test case for the threads assignment.
//
//  Create two threads, and have them context switch
//  back and forth between themselves by calling Thread::Yield,
//  to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#endif

using namespace std;


void ApplicationClerk(int myLine);
void createClerkThreads(Thread* t);
void clerkFactory();
void clerkSignalsNextCustomer(int myLine);
int chooseCustomerFromLine(int myLine);
void Part2();
void PassportClerk(int myLine);
void PictureClerk(int myLine);
void Cashier(int myLine);
void Customer(int custNumber);
void Senator(int custNumber);
void wakeUpClerks();
void printMoney();
void Manager();
bool customersAreAllDone();

class CustomerAttribute {
public:
    CustomerAttribute() {}
    CustomerAttribute(int ssn) {
        SSN = ssn;
        applicationIsFiled = false;
        likesPicture = false;
        hasCertification = false;
        isDone = false;
        clerkMessedUp = false;

        int moneyArray[4] = {100, 600, 1100, 1600};
        int randomIndex = rand() % 4; //0 to 3
        money = moneyArray[randomIndex];
    }
    ~CustomerAttribute() {}

    int SSN;
    bool likesPicture;
    bool applicationIsFiled;
    bool hasCertification;
    bool isDone;
    bool clerkMessedUp;
    int money;
    int currentLine;
};

//----------------------------------------------------------------------
// SimpleThread
//  Loop 5 times, yielding the CPU to another ready thread
//  each iteration.
//
//  "which" is simply a number identifying the thread, for debugging
//  purposes.
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

//----------------------------------------------------------OUR STUFF-------------------------------------

#ifdef CHANGED
// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done
Lock t1_l1("t1_l1");          // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock

    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
        t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
        t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();  // Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock

    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
        t1_l1.getName());
    t1_l1.Acquire();

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
        t1_l1.getName());
    for (int i = 0; i < 10; i++)
    ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
        t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

    t1_s2.P();  // Wait until t2 is ready to try to acquire the lock

    t1_s3.V();  // Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
    printf("%s: Trying to release Lock %s\n",currentThread->getName(),
           t1_l1.getName());
    t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");        // For mutual exclusion
Condition t2_c1("t2_c1");   // The condition variable to test
Semaphore t2_s1("t2_s1",0); // To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();  // release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();  // Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");        // For mutual exclusion
Condition t3_c1("t3_c1");   // The condition variable to test
Semaphore t3_s1("t3_s1",0); // To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();      // Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting

    for ( int i = 0; i < 5 ; i++ )
    t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}

// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");        // For mutual exclusion
Condition t4_c1("t4_c1");   // The condition variable to test
Semaphore t4_s1("t4_s1",0); // To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();      // Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting

    for ( int i = 0; i < 5 ; i++ )
    t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
       t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");        // For mutual exclusion
Lock t5_l2("t5_l2");        // Second lock for the bad behavior
Condition t5_c1("t5_c1");   // The condition variable to test
Semaphore t5_s1("t5_s1",0); // To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();  // release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();  // Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//   4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;

    // Test 1
    printf("Starting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);

    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
        t1_done.P();

    // Test 2

    printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");

    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);

    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
    t2_done.P();

    // Test 3

    printf("Starting Test 3\n");

    for (  i = 0 ; i < 5 ; i++ ) {
    name = new char [20];
    sprintf(name,"t3_waiter%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ ) {
        if(t3_l1.getLockOwner() != NULL) {
            printf("This is the current lockOwner: %s\n", t3_l1.getLockOwner()->getName());
        } else {
            printf("The lock is NULL\n");
        }

        t3_done.P();
        scheduler->Print();
        t3_l1.Print();
    }

    // Test 4

    printf("Starting Test 4\n");

    for (  i = 0 ; i < 5 ; i++ ) {
    name = new char [20];
    sprintf(name,"t4_waiter%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
    t4_done.P();

    // Test 5

    printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");

    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);

    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);
}

#endif

//----------------------------------------------------------------------
// ThreadTest
//  Set up a ping-pong between two threads, by forking a thread
//  to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

//HUNG: Some constants, since we are writing our own test cases,
//the number of clerks and customers is totally up to us
int testChosen = 1;
const int CLERK_NUMBER = 20;
const int CUSTOMER_NUMBER = 60;
const int CLERK_TYPES = 4;
const int SENATOR_NUMBER = 10;
int clerkCount = 0; //TODO make this NOT const and change to user input with cin >>
int customerCount = 0; //TODO make this NOT const and change to user input with cin >>
int senatorCount = 0; //TODO make this NOT const and change to user input with cin >>
int senatorLineCount = 0;

Lock* clerkLineLock = new Lock("ClerkLineLock");
int clerkLineCount[CLERK_NUMBER];
int clerkBribeLineCount[CLERK_NUMBER];
enum ClerkState {AVAILABLE, BUSY, ONBREAK};
ClerkState clerkStates[CLERK_NUMBER] = {AVAILABLE,AVAILABLE,AVAILABLE,AVAILABLE,AVAILABLE};
Condition* clerkLineCV[CLERK_NUMBER];
Condition* clerkBribeLineCV[CLERK_NUMBER];
Condition* clerkSenatorLineCV = new Condition("ClerkSenatorLineCV");
Lock* clerkSenatorCVLock[CLERK_NUMBER];

Condition* outsideLineCV = new Condition("OutsideLineCV");
//TODO: finish up break condition variables
Condition* breakCV[CLERK_NUMBER];

Lock* senatorLock = new Lock("SenatorLock");
// Condition* senatorCV = new Condition("SenatorCV");

int outsideLineCount = 0;
CustomerAttribute customerAttributes[CUSTOMER_NUMBER];
int clerkMoney[CLERK_NUMBER] = {0};
//Senator control variables
Condition* senatorLineCV = new Condition("SenatorLineCV");
Condition* clerkSenatorCV[CLERK_NUMBER];
//Second monitor
Lock* clerkLock[CLERK_NUMBER];
Condition* clerkCV[CLERK_NUMBER];
int customerData[CLERK_NUMBER]; //HUNG: Every clerk will use this to get the customer's customerAttribute index
string clerkTypesStatic[CLERK_TYPES] = { "ApplicationClerks : ", "PictureClerks     : ", "PassportClerks    : ", "Cashiers          : " };
string clerkTypes[CLERK_NUMBER];
int clerkArray[CLERK_TYPES];

Lock* breakLock[CLERK_NUMBER];

char* cStringDeepCopy(string str) {
    char *cstr = new char[str.length() + 1];
    strcpy(cstr, str.c_str());
    // do stuff
    //delete [] cstr;
    return cstr;
}

void clerkFactory(int countOfEachClerkType[]) {
    int tempClerkCount = 0;
    for(int i = 0; i < CLERK_TYPES; ++i) {
        if(testChosen == 0) {
            do {
                cout << clerkTypesStatic[i];
                cin >> tempClerkCount;
                if(tempClerkCount <= 0 || tempClerkCount > 5) {
                    cout << "    The number of clerks must be between 1 and 5 inclusive!" << endl;
                }
            } while(tempClerkCount <= 0 || tempClerkCount > 5);
            clerkCount += tempClerkCount;
            clerkArray[i] = tempClerkCount;    
        } else {
            cout << clerkTypesStatic[i] << countOfEachClerkType[i] << endl;
            clerkArray[i] = countOfEachClerkType[i];
        }
    }
    cout << "Number of ApplicationClerks = " <<  clerkArray[0] << endl;
    cout << "Number of PictureClerks = " << clerkArray[1] << endl;
    cout << "Number of PassportClerks = " << clerkArray[2] << endl;
    cout << "Number of CashiersClerks = " << clerkArray[3] << endl;
    cout << "Number of Senators = " << senatorCount << endl;
}

void createClerkThreads(Thread* t) {
    int clerkNumber = 0;
    for(int i = 0; i < 4; ++i) {
        if(i == 0) {
            for(int j = 0; j < clerkArray[i]; ++j) {
                stringstream sstm;
                sstm << "ApplicationClerk_" << clerkNumber;
                char* clerkName = cStringDeepCopy(sstm.str());
                t = new Thread(clerkName);
                // delete [] clerkName;
                t->Fork((VoidFunctionPtr)ApplicationClerk,clerkNumber);
                clerkTypes[clerkNumber] = "ApplicationClerk";
                ++clerkNumber;
            }
        } else if(i == 1) {
            for(int j = 0; j < clerkArray[i]; ++j) {
                stringstream sstm;
                sstm << "PictureClerk_" << clerkNumber;
                char* clerkName = cStringDeepCopy(sstm.str());
                t = new Thread(clerkName);
                // delete [] clerkName;
                t->Fork((VoidFunctionPtr)PictureClerk,clerkNumber);
                clerkTypes[clerkNumber] = "PictureClerk";
                ++clerkNumber;
            }
        } else if(i == 2) {
            for(int j = 0; j < clerkArray[i]; ++j) {
                stringstream sstm;
                sstm << "PassportClerk_" << clerkNumber;
                char* clerkName = cStringDeepCopy(sstm.str());
                t = new Thread(clerkName);
                // delete [] clerkName;
                t->Fork((VoidFunctionPtr)PassportClerk,clerkNumber);
                clerkTypes[clerkNumber] = "PassportClerk";
                ++clerkNumber;
            }
        } else { // i == 3
            for(int j = 0; j < clerkArray[i]; ++j) {
                stringstream sstm;
                sstm << "Cashier_" << clerkNumber;
                char* clerkName = cStringDeepCopy(sstm.str());
                t = new Thread(clerkName);
                // delete [] clerkName;
                t->Fork((VoidFunctionPtr)Cashier,clerkNumber);
                clerkTypes[clerkNumber] = "Cashier";
                ++clerkNumber;
            }
        }
    }
}

//Monitor variables
//First monitor

void createClerkLocksAndConditions() {
    for(int i = 0; i < clerkCount; ++i) {
        stringstream sstm;
        sstm << "ClerkLock_" << i;
        char* name = cStringDeepCopy(sstm.str());
        clerkLock[i] = new Lock(name);
        // delete [] name;

        sstm.str(string());
        sstm << "ClerkCV_" << i;
        name = cStringDeepCopy(sstm.str());
        clerkCV[i] = new Condition(name);
        // delete [] name;

        sstm.str(string());
        sstm << "ClerkLineCV_" << i;
        name = cStringDeepCopy(sstm.str());
        clerkLineCV[i] = new Condition(name);
        // delete [] name;

        sstm.str(string());
        sstm << "ClerkBribeLineCV_" << i;
        name = cStringDeepCopy(sstm.str());
        clerkBribeLineCV[i] = new Condition(name);
        // delete [] name;

        sstm.str(string());
        sstm << "BreakLock_" << i;
        name = cStringDeepCopy(sstm.str());
        breakLock[i] = new Lock(name);
        // delete [] name;

        sstm.str(string());
        sstm << "BreakCV_" << i;
        name = cStringDeepCopy(sstm.str());
        breakCV[i] = new Condition(name);
        // delete [] name;

        sstm.str(string());
        sstm << "ClerkSenatorCV_" << i;
        name = cStringDeepCopy(sstm.str());
        clerkSenatorCV[i] = new Condition(name);
        // delete [] name;

        sstm.str(string());
        sstm << "ClerkSenatorCV_" << i;
        name = cStringDeepCopy(sstm.str());
        clerkSenatorCVLock[i] = new Lock(name);
        // delete [] name;
    }
}

void createCustomerThreads(Thread *t) {
    for(int i = 0 ; i < customerCount; i++){
        stringstream sstm;
        sstm << "Customer_" << i;
        t = new Thread(cStringDeepCopy(sstm.str()));
        t->Fork((VoidFunctionPtr)Customer,i);
    }
}

void createSenatorThreads(Thread *t){
    for(int i = 0 ; i < senatorCount; i++){
        stringstream sstm;
        sstm << "Senator_" << i + 50;
        t = new Thread(cStringDeepCopy(sstm.str()));
        t->Fork((VoidFunctionPtr)Senator,i + 50);
    }
}

void createTestVariables(Thread* t, int countOfEachClerkType[]) {
    clerkFactory(countOfEachClerkType);
    createClerkLocksAndConditions();
    createClerkThreads(t);
    createCustomerThreads(t);
    createSenatorThreads(t);
    t->Fork((VoidFunctionPtr)Manager,0);
}

void Part2() {
    Thread *t;
    char *name;
    int countOfEachClerkType[CLERK_TYPES] = {0,0,0,0};

    printf("Starting Part 2\n");
    cout << "Test to run (put 0 for full program): ";
    cin >> testChosen;

    if(testChosen == 1) {
        printf("Starting Test 1\n"); //Customers always take the shortest line, but no 2 customers ever choose the same shortest line at the same time
        customerCount = 5;
        clerkCount = 4;
        senatorCount = 0;
        countOfEachClerkType[0] = 1; countOfEachClerkType[1] = 1; countOfEachClerkType[2] = 1; countOfEachClerkType[3] = 1;

        createTestVariables(t, countOfEachClerkType);
    } else if(testChosen == 2) {
        printf("Starting Test 2\n"); //Managers only read one from one Clerk's total money received, at a time
        customerCount = 5;
        clerkCount = 4;
        senatorCount = 0;
        countOfEachClerkType[0] = 1; countOfEachClerkType[1] = 1; countOfEachClerkType[2] = 1; countOfEachClerkType[3] = 1;

        createTestVariables(t, countOfEachClerkType);
    } else if(testChosen == 3) {
        printf("Starting Test 3\n"); //Customers do not leave until they are given their passport by the Cashier. 
                                     //The Cashier does not start on another customer until they know that the last Customer has left their area
        customerCount = 5;
        clerkCount = 4;
        senatorCount = 0;
        countOfEachClerkType[0] = 1; countOfEachClerkType[1] = 1; countOfEachClerkType[2] = 1; countOfEachClerkType[3] = 1;

        createTestVariables(t, countOfEachClerkType);
    } else if(testChosen == 4) {
        printf("Starting Test 4\n"); //Clerks go on break when they have no one waiting in their line
        customerCount = 5;
        clerkCount = 4;
        senatorCount = 0;
        countOfEachClerkType[0] = 1; countOfEachClerkType[1] = 1; countOfEachClerkType[2] = 1; countOfEachClerkType[3] = 1;

        createTestVariables(t, countOfEachClerkType);
    } else if(testChosen == 5) {
        printf("Starting Test 5\n"); //Managers get Clerks off their break when lines get too long
        customerCount = 7;
        clerkCount = 4;
        senatorCount = 0;
        countOfEachClerkType[0] = 1; countOfEachClerkType[1] = 1; countOfEachClerkType[2] = 1; countOfEachClerkType[3] = 1;

        createTestVariables(t, countOfEachClerkType);
    } else if(testChosen == 6) {
        printf("Starting Test 6\n"); //Total sales never suffers from a race condition
        customerCount = 25;
        clerkCount = 4;
        senatorCount = 0;
        countOfEachClerkType[0] = 1; countOfEachClerkType[1] = 1; countOfEachClerkType[2] = 1; countOfEachClerkType[3] = 1;

        createTestVariables(t, countOfEachClerkType);
    } else if(testChosen == 7) {

    } else if(testChosen == 0) {
        do {
            cout << "Number of customers: ";
            cin >> customerCount;
            if(customerCount <= 0 || customerCount > 50) {
                cout << "    The number of customers must be between 1 and 50 inclusive!" << endl;
            }
        } while(customerCount <= 0 || customerCount > 50);
        
        do {
            cout << "Number of Senators: ";
            cin >> senatorCount;
            if(senatorCount < 0 || senatorCount > 10) {
                cout << "    The number of senators must be between 1 and 10 inclusive!" << endl;
            }
        } while(senatorCount < 0 || senatorCount > 10);
        
        createTestVariables(t, countOfEachClerkType);
    }
}

int chooseCustomerFromLine(int myLine) {
    // clerkLineLock->Acquire();

    cout << "@@@@@@@@@@@@@ Senator line count is: " << senatorLineCount << endl;
    /*if(senatorLineCount > 0) {//HUNG: senator stuff
        clerkLock[myLine]->Acquire();
        cout << "@@@@@@@@@@@@@ " << currentThread->getName() << "::::: Has Acquired the clerkLock[" << myLine << "]" << endl;
        clerkCV[myLine]->Signal(clerkLock[myLine]);
        cout << "@@@@@@@@@@@@@ " << currentThread->getName() << "::::: Has Signaled using clerkLock[" << myLine << "]" << endl;
    }else{*/
    // if(senatorLineCount == 0) {
        bool testFlag = false;
        do {
            testFlag = false;
            if(senatorLineCount > 0 && clerkLock[myLine]->getLockOwner() == NULL){
                clerkSenatorCVLock[myLine]->Acquire();
                clerkSenatorCV[myLine]->Signal(clerkSenatorCVLock[myLine]);
                clerkSenatorCV[myLine]->Wait(clerkSenatorCVLock[myLine]);
                clerkStates[myLine] = BUSY;
                testFlag = true;
            }else{
                clerkLineLock->Acquire();
                if(clerkBribeLineCount[myLine] > 0) {
                    //if(testChosen != 1)
                    cout << currentThread->getName() << " is servicing a customer from bribe line" << endl;
                    clerkBribeLineCV[myLine]->Signal(clerkLineLock);
                    clerkStates[myLine] = BUSY; //redundant setting
                    // testFlag = false;
                } else if(clerkLineCount[myLine] > 0) {
                    //if(testChosen != 1)
                        cout << currentThread->getName() << " is servicing a customer from regular line"  <<  endl;
                    clerkLineCV[myLine]->Signal(clerkLineLock);
                    clerkStates[myLine] = BUSY; //redundant setting
                    // testFlag = false;
                // } else if(clerkLock[myLine]->getLockOwner() != NULL){
                //     clerkStates[myLine] = BUSY; //redundant setting

                }else{
                    //TODO: ONBREAK CODE
                    breakLock[myLine]->Acquire();
                  //  clerkLock[myLine]->Acquire(); //TODO: will this prevent race condition?
                    //if(testChosen != 1)
                    cout << currentThread->getName() << " is going on break because line count is: " << clerkBribeLineCount[myLine] + clerkLineCount[myLine] << endl;
                        
                    clerkStates[myLine] = ONBREAK;
                    if (testChosen == 4){
                        for (int i = 0; i < clerkCount; ++i){
                            if (clerkStates[i] == 2)
                            cout << "    " << clerkTypes[i] << "_" << i << " is on break." << endl;
                        }
                    }
                    clerkLineLock->Release();
                    breakCV[myLine]->Wait(breakLock[myLine]);
                    clerkStates[myLine] = AVAILABLE;
                  //  clerkLock[myLine]->Release();
                    breakLock[myLine]->Release();
                }
            }

        } while(clerkStates[myLine] != BUSY); //TODO: must be a better way of doing this
    clerkLock[myLine]->Acquire();
    // }
    clerkLineLock->Release();
    //wait for customer
    if(testFlag){
      clerkSenatorCV[myLine]->Signal(clerkSenatorCVLock[myLine]);
      clerkSenatorCVLock[myLine]->Release();
    }
    clerkCV[myLine]->Wait(clerkLock[myLine]);
    //Do my job -> customer waiting
    return customerData[myLine]; //HUNG: Customer would have set the customerData[myLine] to be their custNumber
}

void clerkSignalsNextCustomer(int myLine) {

    clerkCV[myLine]->Signal(clerkLock[myLine]);
    clerkCV[myLine]->Wait(clerkLock[myLine]);

    clerkLock[myLine]->Release();
}

void ApplicationClerk(int myLine) {
    while(true) {
        int custNumber = chooseCustomerFromLine(myLine);
        clerkStates[myLine] = BUSY;
        cout << currentThread->getName() << " has signalled a Customer to come to their counter. " << "(Customer_" << custNumber << ")" << endl;
        currentThread->Yield();
        cout << "Customer_" << custNumber << " has given SSN "<< custNumber << " to " << currentThread->getName() << endl;
        currentThread->Yield();
        cout << currentThread->getName() << " has received SSN " << custNumber << "from Customer_" << custNumber << endl;
        int numYields = rand() % 80 + 20;
        for(int i = 0; i < numYields; ++i) {
            currentThread->Yield();
        }

        customerAttributes[custNumber].applicationIsFiled = true;
        cout << currentThread->getName() << " has recorded a completed application for Customer_" << custNumber << endl;
        clerkSignalsNextCustomer(myLine);
    }
}

void PictureClerk(int myLine) {
    while(true) {
        int custNumber = chooseCustomerFromLine(myLine);
        clerkStates[myLine] = BUSY;
        cout << currentThread->getName() << " has signalled a Customer to come to their counter. " << "(Customer_" << custNumber << ")" << endl;
        currentThread->Yield();
        cout << "Customer_" << custNumber << " has given SSN "<< custNumber << " to " << currentThread->getName() << endl;
        currentThread->Yield();
        cout << currentThread->getName() << " has received SSN " << custNumber << "from Customer_" << custNumber << endl;
        
        int numYields = rand() % 80 + 20;

        while(!customerAttributes[custNumber].likesPicture) {
            cout << currentThread->getName() << " has taken a picture of Customer_" << custNumber << endl;
            
            int probability = rand() % 100;
            if(probability >= 25) {
                customerAttributes[custNumber].likesPicture = true;
                cout << "Customer_" << custNumber << " does like their picture from " << currentThread->getName() << endl;
                cout << currentThread->getName() << " has been told that Customer_" << custNumber << " does like their picture" << endl;
                for(int i = 0; i < numYields; ++i) {
                    currentThread->Yield();
                }
            }else{
                cout << "Customer_" << custNumber << " does not like their picture from " << currentThread->getName() << endl;
                cout << currentThread->getName() << " has been told that Customer_" << custNumber << " does not like their picture" << endl;
            }
        }
        clerkSignalsNextCustomer(myLine);
    }
}

void PassportClerk(int myLine) {
    while(true) {
        int custNumber = chooseCustomerFromLine(myLine);
        cout << currentThread->getName() << " has signalled a Customer to come to their counter. " << "(Customer_" << custNumber << ")" << endl;
        currentThread->Yield();
        cout << "Customer_" << custNumber << " has given SSN "<< custNumber << " to " << currentThread->getName() << endl;
        currentThread->Yield();
        cout << currentThread->getName() << " has received SSN " << custNumber << "from Customer_" << custNumber << endl;
        if(customerAttributes[custNumber].likesPicture && customerAttributes[custNumber].applicationIsFiled) {
            cout << currentThread->getName() << " has determined that Customer_" << custNumber << " has both their application and picture completed" << endl;
            clerkStates[myLine] = BUSY;

            int numYields = rand() % 80 + 20;
            
            // clerkStates[myLine] = AVAILABLE;
            
            int clerkMessedUp = rand() % 100;
            if(clerkMessedUp <= 5) { //Send to back of line
                cout << currentThread->getName() << ": Messed up for Customer_" << custNumber<< ". Sending customer to back of line."<<endl;
                customerAttributes[custNumber].clerkMessedUp = true; //TODO: customer uses this to know which back line to go to
            } else {
                cout << currentThread->getName() << " has recorded Customer_" << custNumber << " passport documentation" << endl;
                for(int i = 0; i < numYields; ++i) {
                    currentThread->Yield();
                }   
                customerAttributes[custNumber].clerkMessedUp = false;
                customerAttributes[custNumber].hasCertification = true;
            } 
        } else {
            cout << currentThread->getName() << " has determined that Customer_" << custNumber << " does not have both their application and picture completed" << endl;
        }
        
        // else { //customer gets sent to back of line
        //     cout << "SENT TO BACK OF LINE &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
        //     int waitTime = rand() % 900 + 100;
        //     for(int i = 0; i < waitTime; ++i) {
        //         currentThread->Yield();
        //     }
        // }
        clerkSignalsNextCustomer(myLine);
    }
}

void Cashier(int myLine) {
    while(true) {
        int custNumber = chooseCustomerFromLine(myLine);
        cout << currentThread->getName() << " has signalled a Customer to come to their counter. " << "(Customer_" << custNumber << ")" << endl;
        currentThread->Yield();
        cout << "Customer_" << custNumber << " has given SSN "<< custNumber << " to " << currentThread->getName() << endl;
        currentThread->Yield();
        cout << currentThread->getName() << " has received SSN " << custNumber << "from Customer_" << custNumber << endl;
        if(customerAttributes[custNumber].hasCertification) {
            cout << currentThread->getName() << " has verified that Customer_" << custNumber << "has been certified by a PassportClerk" << endl;
            customerAttributes[custNumber].money -= 100;
            cout << currentThread->getName() << " has received the $100 from Customer_" << custNumber << "after certification " << endl;
            cout << "Customer_" << custNumber << " has given " << currentThread->getName() << " $100 " << endl;
            
            clerkMoney[myLine] += 100;
            clerkStates[myLine] = BUSY;
            cout << currentThread->getName() << "::starting to process money for Customer_" << custNumber << endl;
            int numYields = rand() % 80 + 20;
            for(int i = 0; i < numYields; ++i) {
                currentThread->Yield();
            }
            int clerkMessedUp = rand() % 100;
            if(clerkMessedUp <= 5) { //Send to back of line
                cout << currentThread->getName() << ": Messed up for Customer_" << custNumber<< ". Sending customer to back of line."<<endl;
                customerAttributes[custNumber].clerkMessedUp = true; //TODO: customer uses this to know which back line to go to
            } else {
                cout << currentThread->getName() << " has provided Customer_" << custNumber << "their completed passport" << endl;
                currentThread->Yield();
                cout << currentThread->getName() << " has recorded that Customer_" << custNumber << " has been given their completed passport" << endl;
                customerAttributes[custNumber].clerkMessedUp = false;
                customerAttributes[custNumber].isDone = true;
                // clerkStates[myLine] = AVAILABLE;
                // cout << "Customer_" << custNumber << ": I'm DONE ##############################################################" << endl;


            }

        }
        // else { //customer gets sent to back of line
        //     cout << "SENT TO BACK OF LINE ============================================================" << endl;
        //     int waitTime = rand() % 900 + 100;
        //     for(int i = 0; i < waitTime; ++i) {
        //         currentThread->Yield();
        //     }
        //     //TODO send to back of CashierLine
        // }
        clerkSignalsNextCustomer(myLine);
    }
}

void printCurrentLineCounts() {
    for(int i = 0; i < clerkCount; ++i) {
        cout << "    " << clerkTypes[i] << "_" << i << " has Customers in regular line | bribe line : " << clerkLineCount[i] << "|" << clerkBribeLineCount[i] << " Total is "<< clerkLineCount[i] + clerkBribeLineCount[i] << endl;
    }
}

void Customer(int custNumber) {
    CustomerAttribute myCustAtt = CustomerAttribute(custNumber); //Hung: Creating a CustomerAttribute for each new customer
    customerAttributes[custNumber] = myCustAtt;
    while(!customerAttributes[custNumber].isDone) {
        clerkLineLock->Acquire();
        bool bribe = false; //HUNG: flag to know whether the customer has paid the bribe, can't be arsed to think of a more elegant way of doing this
        int myLine = -1;
        int lineSize = 1000;
        //cout << currentThread->getName() << ": I'm choosing a line: ";
        for(int i = 0; i < clerkCount; i++) {
            int totalLineCount = clerkLineCount[i]+clerkBribeLineCount[i];
            if(!customerAttributes[custNumber].applicationIsFiled &&
                //customerAttributes[custNumber].likesPicture &&
                !customerAttributes[custNumber].hasCertification &&
                !customerAttributes[custNumber].isDone &&
                clerkTypes[i] == "ApplicationClerk") {
                cout << currentThread->getName() << " is looking at line " << clerkTypes[i] << "_"<< i << endl;
                
                if(totalLineCount < lineSize ) { //&& clerkStates[i] != ONBREAK
                    myLine = i;
                    lineSize = totalLineCount;
                }
            } else if(//customerAttributes[custNumber].applicationIsFiled &&
                      !customerAttributes[custNumber].likesPicture &&
                      !customerAttributes[custNumber].hasCertification &&
                      !customerAttributes[custNumber].isDone &&
                      clerkTypes[i] == "PictureClerk") {
                cout << currentThread->getName() << " is looking at line " << clerkTypes[i] << "_"<< i << endl;
                if(totalLineCount < lineSize) {//&& clerkStates[i] != ONBREAK
                    myLine = i;
                    lineSize = totalLineCount;
                }
            } else if(customerAttributes[custNumber].applicationIsFiled &&
                      customerAttributes[custNumber].likesPicture &&
                      !customerAttributes[custNumber].hasCertification &&
                      !customerAttributes[custNumber].isDone &&
                      clerkTypes[i] == "PassportClerk") {
                cout << currentThread->getName() << " is looking at line " << clerkTypes[i] << "_"<< i << endl;
                if(totalLineCount < lineSize) {// && clerkStates[i] != ONBREAK
                    myLine = i;
                    lineSize = totalLineCount;
                }
            } else if(customerAttributes[custNumber].applicationIsFiled &&
                      customerAttributes[custNumber].likesPicture &&
                      customerAttributes[custNumber].hasCertification &&
                      !customerAttributes[custNumber].isDone &&
                      clerkTypes[i] == "Cashier") {
                cout << currentThread->getName() << " is looking at line " << clerkTypes[i] << "_"<< i << endl;
                if(totalLineCount < lineSize) {  //&& clerkStates[i] != ONBREAK
                    myLine = i;
                    lineSize = totalLineCount;
                }
            }
        }

        if(clerkTypes[myLine] == "PassportClerk") {
            switch(testChosen) {
                case 1:
                    customerAttributes[custNumber].hasCertification = true;
                    customerAttributes[custNumber].isDone = true;
                    break;
            }
        }
        // if(myLine == -1) {
            
        // }

        // if(myLine == -1) {
        //     outsideLineCount++;
        //     outsideLineCV->Wait(clerkLineLock);
        // } else {
            if(clerkStates[myLine] != AVAILABLE ) {//clerkStates[myLine] == BUSY
                //I must wait in line
                if(testChosen == 5) {
                    for(int i = 0; i < clerkCount; ++i) {
                        cout << clerkTypes[i] << "_" << i << "'s line has " << clerkLineCount[i] + clerkBribeLineCount[i] << " Customers" << endl;
                    }
                }
                if (testChosen == 1) {
                    printCurrentLineCounts();                    
                }
                cout << "    " << currentThread->getName() << " I'm choosing " << clerkTypes[myLine] << "_" << myLine << endl;
                //HUNG: adding code for line bribing TODO: check if all transactions are complete
                
                cout << "        ";
                if(customerAttributes[custNumber].money > 100){
                    cout << currentThread->getName() << " has gotten in bribe line for " << clerkTypes[myLine] << "_" << myLine << endl;

                    customerAttributes[custNumber].money -= 500;
                    clerkMoney[myLine] += 500;
                    clerkBribeLineCount[myLine]++;
                    bribe = true;
                    clerkBribeLineCV[myLine]->Wait(clerkLineLock);
                } else {
                    cout << currentThread->getName() << " has gotten in regular line for " << clerkTypes[myLine] << "_"<< myLine << endl;
                    clerkLineCount[myLine]++;
                    clerkLineCV[myLine]->Wait(clerkLineLock);
                }

                int totalLineCount = 0;
                for(int i = 0; i < clerkCount; ++i) {
                    totalLineCount = totalLineCount + clerkBribeLineCount[i] + clerkLineCount[i];
                }
                if(testChosen == 1 && totalLineCount == customerCount) {
                    cout << "Test 1 has passed!\n\n";
                    exit(0);
                }

                if(bribe) {
                    cout << currentThread->getName() << " is bribing " << clerkTypes[myLine] << "_"<< myLine << endl;
                    clerkBribeLineCount[myLine]--;
                } else {
                    cout << currentThread->getName() << " is NOT bribing " << clerkTypes[myLine] << "_"<< myLine << endl;
                    clerkLineCount[myLine]--;
                }
            } else {
                clerkStates[myLine] = BUSY;
            }
            clerkLineLock->Release();

            clerkLock[myLine]->Acquire();
            //Give my data to my clerk
            customerData[myLine] = custNumber;
                    cout << currentThread->getName() << " has given SSN "<< custNumber << " to " << clerkTypes[myLine] << "_" << myLine << endl;

            clerkCV[myLine]->Signal(clerkLock[myLine]);
            //wait for clerk to do their job
            clerkCV[myLine]->Wait(clerkLock[myLine]);
           //Read my data
            clerkCV[myLine]->Signal(clerkLock[myLine]);
            clerkLock[myLine]->Release();

            if(customerAttributes[custNumber].clerkMessedUp) {
                cout << "Clerk messed up.  Customer is going to the back of the line." << endl;
                int yieldTime = rand() % 900 + 100;
                for(int i = 0; i < yieldTime; ++i) {
                    currentThread->Yield();
                }
                customerAttributes[custNumber].clerkMessedUp = false;
            }
        // }
    }
    cout << currentThread->getName() << " is leaving the Passport Office." << endl;

}

void Senator(int custNumber){
    cout << "Senator_ " << custNumber << ": has been instantiated" << endl;
    CustomerAttribute myCustAtt = CustomerAttribute(custNumber); //Hung: custNumber == 50 to 59
    customerAttributes[custNumber] = myCustAtt;
    clerkLineLock->Acquire();
    senatorLock->Acquire();
    if(senatorLineCount > 0){
        senatorLineCV->Wait(senatorLock);
    }

    cout <<"1111111111" << endl;
    for(int i = 0; i < clerkCount; i++){
        clerkSenatorCVLock[i]->Acquire();
    }

        cout <<"2222222222"<< endl;
    senatorLineCount++;
    for(int i = 0; i < clerkCount; i++){
        clerkSenatorCV[i]->Wait(clerkSenatorCVLock[i]);
        cout << "Getting confirmation from clerk "<< i << endl;
    }


        cout <<"333333333333333"<< endl;
    int myLine = 0;
    while(!customerAttributes[custNumber].isDone) {
        for(int i = 0; i < clerkCount; i++) {
            if(!customerAttributes[custNumber].applicationIsFiled &&
                //customerAttributes[custNumber].likesPicture &&
                !customerAttributes[custNumber].hasCertification &&
                !customerAttributes[custNumber].isDone &&
                clerkTypes[i] == "ApplicationClerk") {
                cout << "    " << currentThread->getName() << "::: ApplicationClerk chosen" << endl;
                myLine = i;
            } else if(//customerAttributes[custNumber].applicationIsFiled &&
                      !customerAttributes[custNumber].likesPicture &&
                      !customerAttributes[custNumber].hasCertification &&
                      !customerAttributes[custNumber].isDone &&
                      clerkTypes[i] == "PictureClerk") {
                cout << "    " << currentThread->getName() << "::: PictureClerk chosen" << endl;
                myLine = i;
            } else if(customerAttributes[custNumber].applicationIsFiled &&
                      customerAttributes[custNumber].likesPicture &&
                      !customerAttributes[custNumber].hasCertification &&
                      !customerAttributes[custNumber].isDone &&
                      clerkTypes[i] == "PassportClerk") {
                cout << "    " << currentThread->getName() << "::: PassportClerk chosen" << endl;
                myLine = i;
            } else if(customerAttributes[custNumber].applicationIsFiled &&
                      customerAttributes[custNumber].likesPicture &&
                      customerAttributes[custNumber].hasCertification &&
                      !customerAttributes[custNumber].isDone &&
                      clerkTypes[i] == "Cashier") {
                cout << "    " << currentThread->getName() << "::: Cashier chosen" << endl;
                myLine = i;
            }
        }
            cout <<"44444444444444"<< endl;
        clerkSenatorCV[myLine]->Signal(clerkSenatorCVLock[myLine]);

            cout <<"555555555555"<< endl;
        clerkSenatorCV[myLine]->Wait(clerkSenatorCVLock[myLine]);

        clerkSenatorCVLock[myLine]->Release();
            cout <<"666666666"<< endl;
        clerkLock[myLine]->Acquire();
        //Give my data to my clerk

            cout <<"77777777777"<< endl;
        customerData[myLine] = custNumber;
        clerkCV[myLine]->Signal(clerkLock[myLine]);

                    cout <<"8888888888"<< endl;
        //wait for clerk to do their job
        clerkCV[myLine]->Wait(clerkLock[myLine]);
       //Read my data

                   cout <<"999999999"<< endl;

      }

      senatorLineCount--;

  for(int i = 0 ; i < clerkCount ; i++){
    clerkCV[i]->Signal(clerkLock[i]);
    clerkLock[i]->Release();
    clerkSenatorCVLock[i]->Release();

  }
  senatorLock->Release();
  clerkLineLock->Release();
}

void wakeUpClerks() {
    // while(outsideLineCount > 0){
    //     clerkLineLock->Acquire();
    //     outsideLineCV->Broadcast(clerkLineLock);
    // }
    if(testChosen == 5) {
        for(int i = 0; i < clerkCount; ++i) {
            cout << clerkTypes[i] << "_" << i << "'s line has " << clerkLineCount[i] + clerkBribeLineCount[i] << " Customers" << endl;
        }
    }
    for(int i = 0; i < clerkCount; ++i) {
        if(clerkStates[i] == ONBREAK) {
            cout << "Manger has woken up a " << clerkTypes[i] << "_" << i << endl;
            breakLock[i]->Acquire();
            breakCV[i]->Signal(breakLock[i]);
            breakLock[i]->Release();
//            currentThread->Yield();
            cout << clerkTypes[i] << "_" << i << " is coming off break" << endl;
        }
    }

    if(testChosen == 5) {
        cout << "Managers get all clerks off their break when one line has 3 or more Customers" << endl;
        cout << "Test 5 has passed!\n\n";
        exit(0);
    }
}

void printMoney() {
    int totalMoney = 0;
    int applicationMoney = 0;
    int pictureMoney = 0;
    int passportMoney = 0;
    int cashierMoney = 0;

    for(int i = 0; i < clerkCount; ++i) {
        if (i < clerkArray[0]){ //ApplicationClerk index
            applicationMoney += clerkMoney[i];
        } else if (i < clerkArray[0] + clerkArray[1]){ //PictureClerk index
            pictureMoney += clerkMoney[i];
        } else if (i < clerkArray[0] + clerkArray[1] + clerkArray[2]){ //PassportClerk index
            passportMoney += clerkMoney[i];
        } else if (i < clerkArray[0] + clerkArray[1] + clerkArray[2] + clerkArray[3]){ //Cashier index
            cashierMoney += clerkMoney[i];
        }
        totalMoney += clerkMoney[i];
        cout << "Manager checking money earned by " << clerkTypes[i] << "_" << i << ": " << clerkMoney[i] << endl;
    }

    cout << "Manager has counted a total of " << applicationMoney << " for ApplicationClerks" << endl;
    cout << "Manager has counted a total of " << pictureMoney << " for PictureClerks" << endl;
    cout << "Manager has counted a total of " << passportMoney << " for PassportClerks" << endl;
    cout << "Manager has counted a total of " << cashierMoney << " for Cashiers" << endl;
    cout << "Manager has counted a total of " << totalMoney << " for the passport office" << endl << endl;
    if(testChosen == 2) {
        cout << "\nManager has counted one clerks total money recieved at a time" << endl;
        cout << "Test 2 has passed!\n\n";
        exit(0);
    }
}

void Manager() {
     do {
        IntStatus oldLevel = interrupt->SetLevel(IntOff);
        int totalLineCount = 0;
        for(int i = 0; i < clerkCount; ++i) {
            totalLineCount += clerkLineCount[i] + clerkBribeLineCount[i];
            if(totalLineCount > 2 ) { // || outsideLineCount > 0
                wakeUpClerks();
                break;
            }
        }
        printMoney();
        (void) interrupt->SetLevel(oldLevel); //restore interrupts

        int waitTime = 100000;
        for(int i = 0; i < waitTime; ++i) {
            currentThread->Yield();
        }
    } while(!customersAreAllDone());
}

bool customersAreAllDone() {
    cout << "***********************************************************" << endl;
    int boolCount = 0;
    for(int i = 0; i < customerCount; ++i) {
        cout << "Customer " << i << " " << customerAttributes[i].applicationIsFiled <<
                                           customerAttributes[i].likesPicture <<
                                           customerAttributes[i].hasCertification <<
                                           customerAttributes[i].isDone << endl;
        boolCount += customerAttributes[i].isDone;
    }

    for(int i = 0; i < senatorCount; ++i) {
        i += 50;
        cout << "Senator " << i << " " << customerAttributes[i].applicationIsFiled <<
                                           customerAttributes[i].likesPicture <<
                                           customerAttributes[i].hasCertification <<
                                           customerAttributes[i].isDone << endl;
        boolCount += customerAttributes[i].isDone;
        i -= 50;
    }

    //HUNG'S DEBUG
    for(int i =0 ; i < clerkCount; i++){
        if(clerkLock[i]->getLockOwner() != NULL){
            cout << clerkLock[i]->getName() << ": lock owner is " << clerkLock[i]->getLockOwner()->getName() << endl;
        }else{
            cout << clerkLock[i]->getName() << ": has no owner"  << endl;
        }
    }
    cout << "customer_0 " << customerAttributes[0].isDone << endl;

    if(boolCount == customerCount + senatorCount) {
        return true;
    }
    return false;
}

void
ThreadTest()
{
    /*DEBUG('t', "Entering SimpleTest");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);*/
    //TestSuite();
    Part2();
}
