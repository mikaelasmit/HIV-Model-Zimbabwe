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
#include <math.h>


using namespace std;


// GENERAL TO FIX OR EXPLORE
// 1. Remove all unecessary code - tidy up
// 2. When person dies, remove all related events, do something about events that should no longer happen
// 3. Make various event types (people-specific/annual events,...)
// 4. Use 'const' in model to stop model from changing variables thta it shouldn't change
// 5. Add class destructors
// 6. Make C++ run with Matlab ? THIS IS TAKING A WHILE AND I AM STILL BUSY WITH THIS.  I am going to look at SJ?s code for this
// 7. Private versus public
// 8. Think about making your own priortiy queue


// TO DO List for HIV:
// 1. Update HIV knowledge based on HIV test


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
int EndYear=2035;															// If endyear is more than 2010, some things will need to get changes, an error message below has been set up as reminder


const long long int final_number_people=100000000;							// To determine the final size of the total population to be modeled - Have to now change [init_pop] to [final_number_people] to give the final size of 'matrix'
int UN_ZimPop=2565000;
int init_pop =UN_ZimPop/100;                                                // Initial population 1st Jan 1950 as 2,675 (x thousands) or 26,750 (x hundred) (see Excel for calculation - Zimbabwe)
int total_population=init_pop;												// Update total population for output and for next new entry


priority_queue<event*, vector<event*>, timeComparison> *p_PQ;				// Pointer to event queue so as to be able to push-in/pop-out new events that are ocurreing  as a result of 'primary' events in the queue, e.g. recurrent birthdays
person** MyArrayOfPointersToPeople = new person*[final_number_people];		// First 'person*' is a pointer (address) and 'new person' and space for x person which will point to actual person below
vector<event *> Events;

int ART_start_yr=2004;

// THIS IS FOR FITTING ONLY - REOMVE ONCE DONE
double one;
double two;
double three;
double four;
double five;
double six;
double seven;
double eight;
double nine;
double ten;
double eleven;
double twelve;
double thirteen;
double fourteen;

extern double MortRisk[6];
extern double MortRisk_Cancer[5];
extern double MortAdj;

double RandomMinMax_3(int min, int max){							// Provide function for random number generator between min and max number
    return rand()%(max-min+1)+min;							    // !!!!Note: if min=0 and max=4 it will generate 0,1,2,3,4
}


//// --- RUN THE MAIN MODEL --- ////
int main(){
    
    srand(time(NULL));														// Random Number generator using PC time
    
    cout << "Testing changes" << endl;
    // Lets do some fitting!
    one   = RandomMinMax_3(100, 100)/100;
    two   = RandomMinMax_3(100, 100)/100;
    three = RandomMinMax_3(100, 100)/100;
    four  = RandomMinMax_3(100, 100)/100;
    cout << "Strok Risk " << four << endl;
    five  = RandomMinMax_3(100, 100)/100;
    six   = RandomMinMax_3(100, 100)/100;
    seven = RandomMinMax_3(100, 100)/100;
    eight = RandomMinMax_3(100, 100)/100;
    nine  = RandomMinMax_3(100, 100)/100;
    ten   = RandomMinMax_3(100, 100)/100;
    eleven= RandomMinMax_3(100, 100)/100;
    twelve= RandomMinMax_3(100, 100)/100;
    thirteen= RandomMinMax_3(100, 100)/100;
    fourteen= RandomMinMax_3(100, 100)/100;
    
    MortRisk[0]= one;
    MortRisk[1]= two;
    MortRisk[2]= three;
    MortRisk[3]= four;
    MortRisk[4]= five;
    MortRisk[5]= six;
    
    MortRisk_Cancer[0] = seven;
    MortRisk_Cancer[1] = eight;
    MortRisk_Cancer[2] = nine;
    MortRisk_Cancer[3] = ten;
    MortRisk_Cancer[4] = eleven;
    MortRisk_Cancer[5] = twelve;
    MortRisk_Cancer[6] = thirteen;
    
    
    MortAdj = four;
    
    
    
    cout << endl << "Hello, Mikaela!" << endl << endl ;								// Check if model is running
    
    
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
    
    
    // Load Large Arrays
    loadBirthArray();
    loadDeathArray_Women();
    loadDeathArray_Men();
    loadHIVArray_Women();
    loadHIVArray_Men();
    loadNCDArray();
    loadCancerArray();
    
    cout << "Section 2 - We finished loading the arrays" << endl;
    
    
    //// ---- Warning Code --- ////
    E(cout << "NOTE: The Macro for error code is working" << endl << endl;)
    if (StartYear > 1954){cout << "CAREFUL!! Fertility at the beginning is based on a TRF_init of 0, based on GlobalTime being between 1950 and 1954" << endl << endl;}
    if (EndYear > 2010){cout << "CAREFUL!! The model runs after 2010 and output things need to be updated to avoid errors" << endl << endl;}
    
    
    //// --- Some pointers ---- ////
    double GlobalTime=StartYear;											// Define Global Time and set it to 0 at the beginning of the model
    p_GT=&GlobalTime;														// Define the location the pointer to Global time is pointing to
    
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
        (MyArrayOfPointersToPeople[i])->GetMyDateCancers();                     // --- Get date of NCDs ---
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
    ProjectZim = fopen("/Users/Mikaela/Dropbox/MATLAB/Zimbabwe Results HIV/ProjectZim.csv","w");
    
    
    
    for (int i=0; i<total_population; i++) {								// Change the i< X here as well as the "%d!!
        fprintf(ProjectZim,"%d,%d,%f,%f,%d,%d, %f, %d, %f, %d, %d, %f, %f, %f, %f, %f, %d, %f, %f, %f, %f, %f, %f, %f, %d, %f, %f, %f, %f, %f, %f, %f, %f, %d, \n",
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
                MyArrayOfPointersToPeople[i]->CauseOfDeath,
                MyArrayOfPointersToPeople[i]->CKD,
                MyArrayOfPointersToPeople[i]->Breast,
                MyArrayOfPointersToPeople[i]->Cervical,
                MyArrayOfPointersToPeople[i]->Colo,
                MyArrayOfPointersToPeople[i]->Liver,
                MyArrayOfPointersToPeople[i]->Oeso,
                MyArrayOfPointersToPeople[i]->Prostate,
                MyArrayOfPointersToPeople[i]->OtherCan,
                MyArrayOfPointersToPeople[i]->Stroke_status
                
                
                
                
                );}
    fclose(ProjectZim);
    
    
    
    
    // COUNT OUTPUT FOR FITTING
    int count_2013deaths=0;
    int count_causeofdeath[14]={0};
    
    
    
    for (int i=0; i<total_population; i++) {
        if (MyArrayOfPointersToPeople[i]->DateOfDeath>=2013 && MyArrayOfPointersToPeople[i]->DateOfDeath<2014)
        {
            count_2013deaths++;
            count_causeofdeath[MyArrayOfPointersToPeople[i]->CauseOfDeath-1]++;
        }
    }
    
    // Calculate percentages
    cout << "FINAL: total: " << count_2013deaths << " and nr background " << count_causeofdeath[0] << endl;
    
    double background_m =(count_causeofdeath[0]/(double)count_2013deaths)*100;
    double HIV_m        =(count_causeofdeath[1]/(double)count_2013deaths)*100;
    double HT_m         =(count_causeofdeath[2]/(double)count_2013deaths)*100;
    double Depression_m =(count_causeofdeath[3]/(double)count_2013deaths)*100;
    double Asthma_m     =(count_causeofdeath[4]/(double)count_2013deaths)*100;
    double Stroke_m     =(count_causeofdeath[5]/(double)count_2013deaths)*100;
    double Diabetes_m   =(count_causeofdeath[6]/(double)count_2013deaths)*100;
    double CKD_m        =(count_causeofdeath[7]/(double)count_2013deaths)*100;
    double Colo_m       =(count_causeofdeath[8]/(double)count_2013deaths)*100;
    double Liver_m      =(count_causeofdeath[9]/(double)count_2013deaths)*100;
    double Oeso_m       =(count_causeofdeath[10]/(double)count_2013deaths)*100;
    double Stomach_m    =(count_causeofdeath[11]/(double)count_2013deaths)*100;
    double OtherCan_m   =(count_causeofdeath[12]/(double)count_2013deaths)*100;
    
    // GBD percentages
    double background_d =56.6;
    double HIV_d        =29.6;
    double HT_d         =1.0;
    double Depression_d =0.0;
    double Asthma_d     =0.7;
    double Stroke_d     =4.6;
    double Diabetes_d   =1.7;
    double CKD_d        =1.3;
    double Colo_d       =0.3;
    double Liver_d      =0.3;
    double Oeso_d       =0.6;
    double Stomach_d    =0.2;
    double OtherCan_d   =3.1;
    
    // Output the model percentages
    cout << "Background " << background_m << endl;
    cout << "HIV "        << HIV_m << endl;
    cout << "HT "         << HT_m << endl;
    cout << "Depression " << Depression_m << endl;
    cout << "Asthma "     << Asthma_m << endl;
    cout << "Stroke "     << Stroke_m << endl;
    cout << "Diabetes "   << Diabetes_m << endl;
    cout << "CKD "        << CKD_m << endl;
    cout << "Colo "       << Colo_m << endl;
    cout << "Liver "      << Liver_m << endl;
    cout << "Oeso "       << Oeso_m << endl;
    cout << "Stomach "    << Stomach_m << endl;
    cout << "OtherCan "   << OtherCan_m << endl;
    
    cout << endl << "Printing the variable values " << endl;
    cout << one << endl;
    cout << two << endl;
    cout << three << endl;
    cout << four << endl;
    cout << five << endl;
    cout << six << endl;
    cout << seven << endl;
    cout << eight << endl;
    cout << nine << endl;
    cout << ten << endl;
    cout << eleven << endl;
    cout << twelve << endl;
    
    // Least square calculation
    double sum_MinLik=  pow ((background_m  - background_d),2) +
    pow ((HIV_m         - HIV_d),2) +
    pow ((HT_m          - HT_d),2) +
    pow ((Depression_m  - Depression_d),2) +
    pow ((Asthma_m      - Asthma_d),2) +
    pow ((Stroke_m      - Stroke_d),2) +
    pow ((Diabetes_m    - Diabetes_d),2) +
    pow ((CKD_m         - CKD_d),2) +
    pow ((Colo_m        - Colo_d),2) +
    pow ((Liver_m       - Liver_d),2) +
    pow ((Oeso_m        - Oeso_d),2) +
    pow ((Stomach_m     - Stomach_d),2) +
    pow ((OtherCan_m    - OtherCan_d),2);
    
    
    
    cout << "Least Square " << sum_MinLik << endl;
    
    /*FILE* FMin_Zim;
     FMin_Zim = fopen("/Users/Mikaela/Dropbox/MATLAB/FMin_Zimbabwe/FMin_Zim.csv","w");
     
     
     fprintf(FMin_Zim, "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f \n",
     sum_MinLik,
     one,
     MortRisk[1],
     MortRisk[2],
     MortRisk[3],
     MortRisk[4],
     MortRisk[5],
     MortRisk_Cancer[0],
     MortRisk_Cancer[1],
     MortRisk_Cancer[2],
     MortRisk_Cancer[3],
     MortRisk_Cancer[4],
     MortAdj,
     background_m,
     HIV_m,
     HT_m,
     Depression_m,
     Asthma_m,
     Stroke_m,
     Diabetes_m,
     CKD_m,
     Colo_m,
     Liver_m,
     Oeso_m,
     Stomach_m,
     OtherCan_m
     
     );
     fclose(FMin_Zim);
     */
    
    
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