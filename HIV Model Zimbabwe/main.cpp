 ////////////////////////////////////////////////////////////////
//    Mikaela Epidemiology Model							  //
//    Created by Mikaela Smit on 7/11/2014.				      //
//    Copyright (c) 2014 Mikaela Smit. All rights reserved.   //
////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <ctime>
#include "person.h"
#include "event.h"
#include "eventQ.h"
#include "eventfunctions.h"
#include "errorcoutmacro.h"
#include "LoadParams.h"
#include "CParamReader.hpp"

#include <fstream>									// some important libraries for reading in the arrays
#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <cmath>

using namespace std;


// TO FIX GENERAL
// 1. Remove all unecessary code - tidy up
// 2. When person dies, remove all related events, do something about events that should no longer happen
// 3. Make various event types (people-specific/annual events,...)
// 4. Use 'const' in model to stop model from changing variables thta it shouldn't change
// 5. Add class destructors
// 6. Make C++ run with Matlab ? THIS IS TAKING A WHILE AND I AM STILL BUSY WITH THIS.  I am going to look at SJ?s code for this
// 7. Private versus public
// 8. Think about making your own priortiy queue
// 9. Think about how to organise this program
// 10. Think about a parameter document - transfer HIV parameter to seperate file


// TO DO List for HIV:
// 1. Add HIV status using incidence
// 2. Add HIV natural history (CD4 count)
// 3. Change mortality once they have HIV and dependant on CD4 coutn
// 4. Add more aspects of HIV care cascade (diagnosis, start cART,?)
// 5. Update HIV knowledge based on HIV test


// !!!  IMPORTANT NOTES FOR MODEL !!!
// 1. Note that the population should be running at 2565 (1/100) for population check and 25650 (1/1000) for HIV check!
// 2. Note that model should be able to run up to 2035!! BUt it doesn't at the moment
// 3. Make main parameters centrally available!!
// 4. Main parameters for NCD can be changed at strat of eventfunction.cpp


//// --- MAIN PARAMETERS - CENTRALLY AVAILABLE --- ////
double *p_GT;																// Pointer to global time
int *p_PY;																	// Pointer to show which year range we are on
int PY=0;																	// Set the first pointer to year range reference to 0
double StartYear=1950;														// Define Start Year if the model and set it to year of choice
int EndYear=2030;															// If endyear is more than 2010, some things will need to get changes, an error message below has been set up as reminder


const long long int final_number_people=100000000;							// To determine the final size of the total population to be modeled - Have to now change [init_pop] to [final_number_people] to give the final size of 'matrix'
int UN_ZimPop=2565000;
int init_pop =UN_ZimPop/100;                                                // Initial population 1st Jan 1950 as 2,675 (x thousands) or 26,750 (x hundred) (see Excel for calculation - Zimbabwe)
int total_population=init_pop;												// Update total population for output and for next new entry


priority_queue<event*, vector<event*>, timeComparison> *p_PQ;				// Pointer to event queue so as to be able to push-in/pop-out new events that are ocurreing  as a result of 'primary' events in the queue, e.g. recurrent birthdays
person** MyArrayOfPointersToPeople = new person*[final_number_people];		// First 'person*' is a pointer (address) and 'new person' and space for x person which will point to actual person below
vector<event *> Events;

int ART_start_yr=2004;



//// --- RUN THE MAIN MODEL --- ////
int main(){
    
    cout << "Hello, Mikaela!" << endl << endl ;								// Check if model is running
    
    
    //// --- Load parameters --- ////
    cout << "Section 1 - We are loading the arrays" << endl;
    
    // Load HIV Arrays
    loadCD4StartArray();
    loadCD4ProgArray();
    loadCD4DeathArray();
    loadCD4ARTArray();
    
    
    // Load Demographic Arrays
    loadAgeDistribution();
    loadAgeMin();
    loadAgeMax();
    loadNrChildren();
    loadNrChildrenProb();

    
    // Load NCD Arrays
    loadNCDAgeArrayMin();
    loadNCDAgeArrayMax();
   
    
    // Load Large Arrays
    loadBirthArray();
    loadDeathArray_Women();
    loadDeathArray_Men();
    loadHIVArray_Women();
    loadHIVArray_Men();
    loadNCDArray();
    
    cout << "Section 2 - We finished loading the arrays" << endl;
    
    
    //// ---- Warning Code --- ////
    E(cout << "NOTE: The Macro for error code is working" << endl << endl;)
    if (StartYear > 1954){cout << "CAREFUL!! Fertility at the beginning is based on a TRF_init of 0, based on GlobalTime being between 1950 and 1954" << endl << endl;}
    if (EndYear > 2010){cout << "CAREFUL!! The model runs after 2010 and output things need to be updated to avoid errors" << endl << endl;}
    
    
    //// --- Some pointers ---- ////
    double GlobalTime=StartYear;											// Define Global Time and set it to 0 at the beginning of the model
    p_GT=&GlobalTime;														// Define the location the pointer to Global time is pointing to
    srand(time(NULL));														// Random Number generator using PC time
    priority_queue<event*, vector<event*>, timeComparison> iQ;				// Define th ePriority Q
    p_PQ=&iQ;																// Define pointer to event Q
    p_PY=&PY;
    
    
    //// --- MAKING POPULATION--- ////

    cout << "Section 3 - We are going to create a population" << endl;
    
    for(int i=0; i<total_population; i++){									// REMEMBER: this needs to stay "final_number_people" or it will give error with CSV FILES!!!!
        MyArrayOfPointersToPeople[i]=new person();							// The 'new person' the actual new person
        int a=i;
        (MyArrayOfPointersToPeople[i])->PersonIDAssign(a);					// --- Assign PersonID ---
    }
    
    for(int i=0; i<total_population; i++){
        (MyArrayOfPointersToPeople[i])->Alive=1;							// --- Assign Life Status ---
        (MyArrayOfPointersToPeople[i])->GenderDistribution();				// --- Assign Sex- ---
        (MyArrayOfPointersToPeople[i])->GetMyDoB();							// --- Assign DoB/Age ---
        (MyArrayOfPointersToPeople[i])->GetDateOfDeath();					// --- Assign date of death ---
        if (MyArrayOfPointersToPeople[i]->Sex == 2 && MyArrayOfPointersToPeople[i]->Age<50 && MyArrayOfPointersToPeople[i]->AgeAtDeath>=15) {(MyArrayOfPointersToPeople[i])->GetDateOfBaby();}              // --- Assign Birth of all Children- ---
        (MyArrayOfPointersToPeople[i])->GetMyDateOfHIVInfection();          // --- Get date of HIV infection ---
        (MyArrayOfPointersToPeople[i])->GetMyDateNCD();                     // --- Get date of NCDs ---
    }
    
    cout << "Section 4 - We finished crating a population" << endl;

    
    //// --- EVENTQ --- ////
    cout << "Section 5 - We are going to create the annual events" << endl;
    
    event * TellNewYear = new event;										// --- Tell me every time  a new year start ---
    Events.push_back(TellNewYear);
    TellNewYear->time = StartYear;
    TellNewYear->p_fun = &EventTellNewYear;
    iQ.push(TellNewYear);
    
    
    //// --- LETS RUN THE EVENTQ --- ////
    cout << endl << endl << "The characteristics of the event queue:" << endl;
    cout << "the first event will ocurr in " << iQ.top()->time << ".  " << endl;
    cout << "the size of the event queue is " << iQ.size() << endl;
    
    while(iQ.top()->time< EndYear +1){                                      // this loop throws up error because no recurrent birthday pushing gt over 5 yrs and iq.pop means gt cannot be updated after pop
        GlobalTime=iQ.top()->time;											// careful with order of global time update - do not touch or touch and check!!
        iQ.top()-> p_fun(iQ.top()->person_ID);
        iQ.pop();
    }
    
    
    //// --- Output the results in a csv file --- ////
    FILE* ProjectZim;
    //ProjectZim = fopen("/Users/Mikaela/Dropbox/MATLAB/HIV check/ProjectZim.csv","w");
    //ProjectZim = fopen("/Users/Mikaela/Dropbox/MATLAB/Demography check - Zimbabwe/ProjectZim.csv","w");
    ProjectZim = fopen("/Users/Mikaela/Dropbox/MATLAB/HIV check/ProjectZim.csv","w");
    
    
    for (int i=0; i<total_population; i++) {								// Change the i< X here as well as the "%d!!
            fprintf(ProjectZim,"%d,%d,%f,%f,%d,%d, %f, %d, %f, %d, %d, %f, %f, %f, %f, %f, %d, %f, %f, %f, %f, %f, %f, %f, %d \n",
                MyArrayOfPointersToPeople[i]->PersonID,
                MyArrayOfPointersToPeople[i]->Sex,
                MyArrayOfPointersToPeople[i]->DoB,
                MyArrayOfPointersToPeople[i]->Age,
                MyArrayOfPointersToPeople[i]->MotherID,
                MyArrayOfPointersToPeople[i]->DatesBirth.size(),
                MyArrayOfPointersToPeople[i]->DateOfDeath,
                MyArrayOfPointersToPeople[i]->AgeAtDeath,
                MyArrayOfPointersToPeople[i]->HIV,
                MyArrayOfPointersToPeople[i]->CD4_cat,
                MyArrayOfPointersToPeople[i]->ART,
                MyArrayOfPointersToPeople[i]->HT,
                MyArrayOfPointersToPeople[i]->Depression,
                MyArrayOfPointersToPeople[i]->Asthma,
                MyArrayOfPointersToPeople[i]->Stroke,
                MyArrayOfPointersToPeople[i]->Diabetes,
                MyArrayOfPointersToPeople[i]->CD4_cat_ARTstart,
                MyArrayOfPointersToPeople[i]->CD4_change.at(0),
                MyArrayOfPointersToPeople[i]->CD4_change.at(1),
                MyArrayOfPointersToPeople[i]->CD4_change.at(2),
                MyArrayOfPointersToPeople[i]->CD4_change.at(3),
                MyArrayOfPointersToPeople[i]->CD4_change.at(4),
                MyArrayOfPointersToPeople[i]->CD4_change.at(5),
                MyArrayOfPointersToPeople[i]->CD4_change.at(6),
                MyArrayOfPointersToPeople[i]->CauseOfDeath
                    
                
                );}
    fclose(ProjectZim);
    
    
    //// --- LETS AVOID MEMORY LEAKS AND DELETE ALL NEW EVENTS --- ////
    cout << "Lets delete the heap! " << endl;
    for(int i=0; i<Events.size()-1; i++){
        E(cout << "Event " << Events.size() << " is " << Events.at(i)->time << endl;)
        delete Events.at(i);
        E(cout << "Event " << Events.size() << " is " << Events.at(i)->time << endl;)
    }
    
    for(int i=0; i<total_population; i++){									// REMEMBER: this needs to stay "final_number_people" or it will give error with CSV FILES!!!!
        delete MyArrayOfPointersToPeople[i];								// The 'new person' the actual new person
    }
    
    // --- End of code ---
    cout << endl << "Hi Jack, so sorry\n";
    system("pause");
    return 0;
}