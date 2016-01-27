/////////////////////////////////////////////////////////////////
//    Created by Mikaela Smit on 07/11/2014.				   //
//    Copyright (c) 2014 Mikaela Smit. All rights reserved.    //
//    These event are added to the EventQ.					   //
/////////////////////////////////////////////////////////////////

#include <stdio.h> 
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <ctime>
#include <math.h>                                                          // For 'log'
#include "eventfunctions.h"
#include "event.h"															// Need to add these to be able to	
#include "eventQ.h"															// [...]run Global Time and Recurrent events pointers
#include "person.h"
#include "errorcoutmacro.h"
#include "CParamReader.hpp"
#include "LoadParams.h"


using namespace std;


//// --- OUTSIDE INFORMATION --- ////
extern double *p_GT;														// Tell this .cpp that there is pointer to Global Time defined externally
extern int EndYear;															// Include endyear so that we can avoid putting unnecessary items into EventQ
extern priority_queue<event*, vector<event*>, timeComparison> *p_PQ;		// Tell this .cpp that there is a priorty_queue externall and define pointer to it
extern int total_population;												// Update total population for output and for next new entry
extern person** MyArrayOfPointersToPeople;									// Pointer to MyArrayOfPointersToPeople
extern int *p_PY;															// Pointer to show which year range we are on
extern vector<event*> Events;

extern double*** CD4_startarray;
extern double**  CD4_prog_rates;
extern double*** Death_CD4_rates;
extern double*** ART_CD4_rates;

extern double*** NCDArray;
extern int*      NCDAgeArrayMin;
extern int*      NCDAgeArrayMax;


//// --- Important Internal informtaion --- ////
int RandomMinMax_2(int min, int max){							// Provide function for random number generator between min and max number
    return rand()%(max-min+1)+min;							// !!!!Note: if min=0 and max=4 it will generate 0,1,2,3,4
}

//?? --- NCD INTERACTION PARAMETERS --- ////
double Risk_DiabHC=1.12;
double Risk_DiabHT=1.4;
double Risk_DiabCKD=1.5;
double Risk_DiabCVD=2.31;

double Risk_HCHT=1.277;
double Risk_HCCVD=1.41;

double Risk_HTCKD=1.69;
double Risk_HTCVD=1.26;


double Risk_NCD_Diabetes[5]={Risk_DiabHC, Risk_DiabHT, Risk_DiabCVD, Risk_DiabCKD, Risk_DiabCVD};
int relatedNCDs_Diab[5]={1, 2, 4, 6, 7};
int nr_NCD_Diab=sizeof(relatedNCDs_Diab)/sizeof(relatedNCDs_Diab[0]);

                       
double Risk_NCD_HC[3]={Risk_HCHT, Risk_HCCVD, Risk_HCCVD};
int relatedNCDs_HC[3]={2, 4, 7};
int nr_NCD_HC=sizeof(relatedNCDs_HC)/sizeof(relatedNCDs_HC[0]);


double Risk_NCD_HT[3]={Risk_HTCVD, Risk_HTCKD, Risk_HTCVD};
int relatedNCDs_HT[3]={4, 6, 7};
int nr_NCD_HT=sizeof(relatedNCDs_HT)/sizeof(relatedNCDs_HT[0]);




//////////////////////////////////////
//// --- FUNCTIONS FOR EVENTS --- ////
//////////////////////////////////////

//// --- NEW YEAR FUNCTION --- ////
void EventTellNewYear(person *MyPointerToPerson){				
																		
	cout << "A new year has started, it is now " << *p_GT << endl;				// This whole function to output every time a new year starts - to make sure the model is actually running
	
	// Lets get the pointer to the right year range
	if (*p_GT<1955){*p_PY = 0; };
	if (*p_GT >= 1955 && *p_GT<1960){*p_PY = 1; };
	if (*p_GT >= 1960 && *p_GT<1965){*p_PY = 2; };
	if (*p_GT >= 1965 && *p_GT<1970){*p_PY = 3; };
	if (*p_GT >= 1970 && *p_GT<1975){*p_PY = 4; };
	if (*p_GT >= 1975 && *p_GT<1980){*p_PY = 5; };
	if (*p_GT >= 1980 && *p_GT<1985){*p_PY = 6; };
	if (*p_GT >= 1985 && *p_GT<1990){*p_PY = 7; };
	if (*p_GT >= 1990 && *p_GT<1995){*p_PY = 8; };
	if (*p_GT >= 1995 && *p_GT<2000){*p_PY = 9; };
	if (*p_GT >= 2000 && *p_GT<2005){*p_PY = 10;};
	if (*p_GT >= 2005){*p_PY = 11; };
	E(cout << "Let's check relevant things have been updated... *p_PY: " << *p_PY << " and Global Time: " << *p_GT << endl;)

	// Schedule event for next year
	event * RecurrentTellNewYear = new event;
	Events.push_back(RecurrentTellNewYear);
	RecurrentTellNewYear->time = *p_GT + 1;										
	RecurrentTellNewYear->p_fun = &EventTellNewYear;
	p_PQ->push(RecurrentTellNewYear);

	E(cout << "We have finished telling you the new year and setting fertility variables for the year." << endl;)	// Error message - can be switched on/off
}


//// --- DEATH EVENT --- ////	
void EventMyDeathDate(person *MyPointerToPerson){

	if (MyPointerToPerson->Alive==1){MyPointerToPerson->Alive=0;}
	E(cout << "Person " << MyPointerToPerson->PersonID << " just died. Their life status now is: " << MyPointerToPerson->Alive << endl;)
}


//// --- BIRTH EVENT AND MAKING NEW PERSON --- ////
void EventBirth(person *MyPointerToPerson){								
		
	E(cout << "A birth is about to happen and my life status: " << endl;)		// Error message - can be switched on/off

	if(MyPointerToPerson->Alive == 1) {											// Only let woman give birth if she is still alive 
		
		total_population=total_population+1;									// Update total population for output and for next new entry
		MyPointerToPerson->Age= (*p_GT - MyPointerToPerson->DoB);				// Update age to get age at birth for output
		

		// Creating a new person 
		MyArrayOfPointersToPeople[total_population-1]=new person();
		(MyArrayOfPointersToPeople[total_population-1])->PersonIDAssign(total_population-1);
		(MyArrayOfPointersToPeople[total_population-1])->Alive=1;
		(MyArrayOfPointersToPeople[total_population-1])->GenderDistribution();
		(MyArrayOfPointersToPeople[total_population-1])->GetMyDoBNewEntry();
		(MyArrayOfPointersToPeople[total_population-1])->GetDateOfDeath();
		(MyArrayOfPointersToPeople[total_population-1])->GetDateOfBaby();
		(MyArrayOfPointersToPeople[total_population-1])->GetMyDateOfHIVInfection();
        (MyArrayOfPointersToPeople[total_population-1])->GetMyDateNCD();
        

	
		// Link Mother and Child
		(MyArrayOfPointersToPeople[total_population-1])->MotherID=MyPointerToPerson->PersonID;			// Give child their mothers ID
		MyPointerToPerson->ChildIDVector.push_back((MyArrayOfPointersToPeople[total_population-1]));	// Give mothers their child's ID
			
		E(cout << "We have finished giving birth " << endl;)					// Error message - can be switched on/off
	
	}
}


//// --- HIV EVENT --- ////
void EventMyHIVInfection(person *MyPointerToPerson){
	
	E(cout << "Somebody is about to get infected with HIV: " << endl;)			// Error message - can be switched on/off

	if(MyPointerToPerson->Alive == 1) {											// Only execute this is patient is still alove
	
		MyPointerToPerson->Age= (*p_GT - MyPointerToPerson->DoB);				// Update age to get correct parameter below
	

		//// --- Get my CD4 count at start --- ////
		double	h = ((double)rand() / (RAND_MAX));								// Gets a random number between 0 and 1.
		int i=0;
		int j=0;
		int a=25;
	
		while (MyPointerToPerson->Age>=a && a<46){a=a+10; i++;};				// To get the right age-specific row in the above sex-specific arrays
		while (h>CD4_startarray[MyPointerToPerson->Sex-1][i][j] && j<3){j++;}	// To get the corresponding CD4 count for correct age and sex from random 'h' generated above
	
		MyPointerToPerson->CD4_cat=0+j;											// CD4 count cat (variable over time)
		MyPointerToPerson->CD4_cat_start=0+j;									// CD4 count cat at start (to keep in records)

	
		//// --- Let's see what will happen next (Death, CD4 count progression or ART initiation) ---- ////
        double FindART_CD4_rate = ART_CD4_rates[MyPointerToPerson->Sex-1][i][MyPointerToPerson->CD4_cat];
        double FindCD4_rate = CD4_prog_rates[MyPointerToPerson->Sex-1][MyPointerToPerson->CD4_cat];
        double FindDeath_CD4_rate = Death_CD4_rates[MyPointerToPerson->Sex-1][i][MyPointerToPerson->CD4_cat];
        
        
        // Lets see when ART would start
        double ART_test = 0;
        double art = ((double)rand() / (RAND_MAX));
        ART_test = (-1/FindART_CD4_rate) * log(art);
        
        
        // Lets see when CD4 count progression would start
        double CD4_test = 0;
        double cd4 = ((double)rand() / (RAND_MAX));
        CD4_test = (-1/FindCD4_rate) * log(cd4);
        
        
        // Lets see when death would happen
        double death_test = 0;
        double dd = ((double)rand() / (RAND_MAX));
        death_test = (-1/FindDeath_CD4_rate) * log(dd);
        
    
        //Before the introduction of ART
        if (*p_GT<2011){
            
            if (CD4_test<death_test){
                
                //cout << "We are scheduling a CD4 progression " << endl;
                
                event * CD4change = new event;
                Events.push_back(CD4change);
                CD4change->time = *p_GT+CD4_test;
                CD4change->p_fun = &EventCD4change;
                CD4change->person_ID = MyPointerToPerson;
                p_PQ->push(CD4change);
                
            }
            
            if (death_test<=CD4_test){
                
                //cout << "We are scheduling an HIV death " << endl;
                
                double death_test_date = *p_GT +death_test;                     // Get the actual date, not just time until death
                
                if (death_test_date<MyPointerToPerson->DateOfDeath){            // Check HIV deaths happens before natural death
                
                    MyPointerToPerson->DateOfDeath=death_test_date;
                    
                    event * DeathEvent = new event;								// In that case we need to add the new death date to the EventQ
                    Events.push_back(DeathEvent);
                    DeathEvent->time = MyPointerToPerson->DateOfDeath;
                    DeathEvent->p_fun = &EventMyDeathDate;
                    DeathEvent->person_ID = MyPointerToPerson;
                    p_PQ->push(DeathEvent);
                }
                
            }
            
        }
        
        // After the introduction of ART
        
      // TO INSERT!!!
        
     

    }

	E(cout << "Somebody has just been infected with HIV!" << endl;)				// Error message - can be switched on/off
}



void EventCD4change(person *MyPointerToPerson){
	
	E(cout << "Somebody is about to experience a drop in CD4 count: " << endl;)	// Error message - can be switched on/off

	if(MyPointerToPerson->Alive == 1) {											// Add any additional things to progression of CD4 count
	
		
        
        //// --- Let's get the right index for all relevant arrays used here and update important info ---- ////
        int i=0;
        int a=25;
        
        while (MyPointerToPerson->Age>=a && a<46){a=a+10; i++;};				// To get the right age-specific row in the above sex-specific arrays
        MyPointerToPerson->Age= (*p_GT - MyPointerToPerson->DoB);				// Update age to get correct parameter below
        MyPointerToPerson->CD4_cat=MyPointerToPerson->CD4_cat+1;                // Update CD4 count
        
        
        //// --- When CD4 count hits the lowest possible value --- ////
        if (MyPointerToPerson->CD4_cat==6){
            
            double FindDeath_CD4_rate = Death_CD4_rates[MyPointerToPerson->Sex-1][i][MyPointerToPerson->CD4_cat];
            
            // Lets see when death would happen
            double death_test = 0;
            double dd = ((double)rand() / (RAND_MAX));
            death_test = (-1/FindDeath_CD4_rate) * log(dd);
            //cout <<  death_test << endl;
            
            double death_test_date = *p_GT +death_test;                      // Get the actual date, not just time until death
            
            if (death_test_date<MyPointerToPerson->DateOfDeath){            // Check HIV deaths happens before natural death
                
                MyPointerToPerson->DateOfDeath=death_test_date;
                
                event * DeathEvent = new event;								// In that case we need to add the new death date to the EventQ
                Events.push_back(DeathEvent);
                DeathEvent->time = MyPointerToPerson->DateOfDeath;
                DeathEvent->p_fun = &EventMyDeathDate;
                DeathEvent->person_ID = MyPointerToPerson;
                p_PQ->push(DeathEvent);
            };
        };

    
        //// --- In case CD4 count is higher than minimum possible category --- ///
        if (MyPointerToPerson->CD4_cat<6){
        
            //// --- Let's see what will happen next (Death, CD4 count progression or ART initiation) ---- ////
            double FindART_CD4_rate = ART_CD4_rates[MyPointerToPerson->Sex-1][i][MyPointerToPerson->CD4_cat];
            double FindCD4_rate = CD4_prog_rates[MyPointerToPerson->Sex-1][MyPointerToPerson->CD4_cat];
            double FindDeath_CD4_rate = Death_CD4_rates[MyPointerToPerson->Sex-1][i][MyPointerToPerson->CD4_cat];
            
            
            // Lets see when ART would start
            double ART_test = 0;
            double art = ((double)rand() / (RAND_MAX));
            ART_test = (-1/FindART_CD4_rate) * log(art);
  
            
            // Lets see when CD4 count progression would start
            double CD4_test = 0;
            double cd4 = ((double)rand() / (RAND_MAX));
            CD4_test = (-1/FindCD4_rate) * log(cd4);
            
            
            // Lets see when death would happen
            double death_test = 0;
            double dd = ((double)rand() / (RAND_MAX));
            death_test = (-1/FindDeath_CD4_rate) * log(dd);
            
            
            //Before the introduction of ART
            if (*p_GT<2011){
                
                if (CD4_test<death_test){
                    //cout << "We are scheduling a CD4 progression " << endl;
                    event * CD4change = new event;
                    Events.push_back(CD4change);
                    CD4change->time = *p_GT+CD4_test;
                    CD4change->p_fun = &EventCD4change;
                    CD4change->person_ID = MyPointerToPerson;
                    p_PQ->push(CD4change);
                    
                }
                
                if (death_test<=CD4_test){
                    
                    //cout << "We are scheduling an HIV death " << endl;
                    
                    double death_test_date = *p_GT +death_test;                      // Get the actual date, not just time until death
                    
                    if (death_test_date<MyPointerToPerson->DateOfDeath){            // Check HIV deaths happens before natural death
                        
                        MyPointerToPerson->DateOfDeath=death_test_date;
                        
                        event * DeathEvent = new event;								// In that case we need to add the new death date to the EventQ
                        Events.push_back(DeathEvent);
                        DeathEvent->time = MyPointerToPerson->DateOfDeath;
                        DeathEvent->p_fun = &EventMyDeathDate;
                        DeathEvent->person_ID = MyPointerToPerson;
                        p_PQ->push(DeathEvent);
                    }
                    
                }
                
            }
        }
        
        // After the introduction of ART
        
        // TO INSERT!!!

	}
	
	E(cout << "Somebody has just experiences a drop in CD4 count!" << endl;)	// Error message - can be switched on/off
}
	


void EventMyDiabetesDate(person *MyPointerToPerson){
    
    E(cout << "I just developed Diabetes, lets see if I am at an increased risk of other NCDs!" << endl;)
    
    
    
    if (MyPointerToPerson->Diabetes==0)
    {
        // First lets update Diabetes status to make sure any over-written dates don't run the same cod again
        MyPointerToPerson->Diabetes=1;
    
        
        
        // Some basic code and finding index for not getting NCDs
        int ncd_nr=0;
        double Date_NCD=-998;                                       // As with HIV, if they don't get NCDs set it to -998 to show code was executed
    
        int max_index=0;                                            // This is to make it automatic.  We currently use 6 age groups for NCDs, but we may use more, less in the future
        int max_nr=1;
        while (max_nr>NCDArray[0][0][max_index]){max_index++;}

        
    
        // Re-evaluate HC/HT and Renal
        while (ncd_nr<nr_NCD_Diab)
        {
            // Get a random number for each NCD
            int i=0;
            double r = ((double) rand() / (RAND_MAX));
            while (r>(NCDArray[MyPointerToPerson->Sex-1][relatedNCDs_Diab[ncd_nr]][i]*Risk_NCD_Diabetes[ncd_nr]) && i<max_index){i++;}
        

            // If we are getting an NCD lets get the age and date of NCD
            if (i<max_index)
            {
                // Lets get the age and date they will have the NCD
                double Age_NCD = RandomMinMax_2(NCDAgeArrayMin[i],NCDAgeArrayMax[i]);     // Lets get the age they will develop the NCD
                double YearFraction=(RandomMinMax_2(1,12))/12.1;                          // This gets month of birth as a fraction of a year
                Age_NCD=Age_NCD+YearFraction;
                Date_NCD=MyPointerToPerson->DoB+Age_NCD;
            }
        
        
            // Lets see if this pushed forward the existing NCD date
            if (Date_NCD>=*p_GT && Date_NCD<MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_Diab[ncd_nr]))
            {
            
                // Lets update the Date everywhere and add to queue
                cout << "My old date is: " << MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_Diab[ncd_nr]) << endl;
                MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_Diab[ncd_nr])=Date_NCD;
                cout << "My new date is: " << MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_Diab[ncd_nr]) << endl;
            
                if (ncd_nr==0)
                {
                    MyPointerToPerson->HC=Date_NCD;
                
                    //// --- Lets feed Hypercholesterolaemia into the eventQ --- ////
                    int p=MyPointerToPerson->PersonID-1;
                    event * HCEvent = new event;
                    Events.push_back(HCEvent);
                    HCEvent->time = MyPointerToPerson->HC;
                    HCEvent->p_fun = &EventMyHypcholDate;
                    HCEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(HCEvent);
                }
                
                if (ncd_nr==1)
                {
                    MyPointerToPerson->HT=Date_NCD;
                
                    //// --- Lets feed Hypertension into the eventQ --- ////
                    int p=MyPointerToPerson->PersonID-1;
                    event * HTEvent = new event;
                    Events.push_back(HTEvent);
                    HTEvent->time = MyPointerToPerson->HT;
                    HTEvent->p_fun = &EventMyHyptenDate;
                    HTEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(HTEvent);
                }
                
                if (ncd_nr==2)
                {
                    MyPointerToPerson->MI=Date_NCD;
                
                    //// --- Lets feed MI into the eventQ --- ////
                    int p=MyPointerToPerson->PersonID-1;
                    event * MIEvent = new event;
                    Events.push_back(MIEvent);
                    MIEvent->time = MyPointerToPerson->MI;
                    MIEvent->p_fun = &EventMyMIDate;
                    MIEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(MIEvent);
                }
                
                if (ncd_nr==3)
                {
                    MyPointerToPerson->CKD=Date_NCD;
                
                    //// --- Lets feed CKD into the eventQ --- ////
                    int p=MyPointerToPerson->PersonID-1;
                    event * CKDEvent = new event;
                    Events.push_back(CKDEvent);
                    CKDEvent->time = MyPointerToPerson->CKD;
                    CKDEvent->p_fun = &EventMyCKDDate;
                    CKDEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(CKDEvent);
                }
                
                if (ncd_nr==4)
                {
                    MyPointerToPerson->Stroke=Date_NCD;
                    
                    //// --- Lets feed Stroke into the eventQ --- ////
                    int p=MyPointerToPerson->PersonID-1;
                    event * StrokeEvent = new event;
                    Events.push_back(StrokeEvent);
                    StrokeEvent->time = MyPointerToPerson->Stroke;
                    StrokeEvent->p_fun = &EventMyStrokeDate;
                    StrokeEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(StrokeEvent);
                }
            }
    
            ncd_nr++;
        }
    }
    E(cout << endl << endl << "Diabetes has developed and addition risks evaluated!" << endl;)
    
}


void EventMyHypcholDate(person *MyPointerToPerson)			// Function executed when person develops hypercholesterolaemia
{
    E(cout << "I just developed Hypercholesterol, lets see if I am at an increased risk of other NCDs!" << endl;)
    
    
    
    if (MyPointerToPerson->HC==0)
    {
        // First lets update Diabetes status to make sure any over-written dates don't run the same cod again
        MyPointerToPerson->HC=1;
        
        
        
        // Some basic code and finding index for not getting NCDs
        int ncd_nr=0;
        double Date_NCD=-998;                                       // As with HIV, if they don't get NCDs set it to -998 to show code was executed
        
        int max_index=0;                                            // This is to make it automatic.  We currently use 6 age groups for NCDs, but we may use more, less in the future
        int max_nr=1;
        while (max_nr>NCDArray[0][0][max_index]){max_index++;}
        
        
        
        // Re-evaluate HC/HT and Renal
        while (ncd_nr<nr_NCD_HC)
        {
            // Get a random number for each NCD
            int i=0;
            double r = ((double) rand() / (RAND_MAX));
            while (r>(NCDArray[MyPointerToPerson->Sex-1][relatedNCDs_HC[ncd_nr]][i]*Risk_NCD_HC[ncd_nr]) && i<max_index){i++;}
            
            
            // If we are getting an NCD lets get the age and date of NCD
            if (i<max_index)
            {
                // Lets get the age and date they will have the NCD
                double Age_NCD = RandomMinMax_2(NCDAgeArrayMin[i],NCDAgeArrayMax[i]);     // Lets get the age they will develop the NCD
                double YearFraction=(RandomMinMax_2(1,12))/12.1;                          // This gets month of birth as a fraction of a year
                Age_NCD=Age_NCD+YearFraction;
                Date_NCD=MyPointerToPerson->DoB+Age_NCD;
            }
            
            
            // Lets see if this pushed forward the existing NCD date
            if (Date_NCD>=*p_GT && Date_NCD<MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_HC[ncd_nr]))
            {
                
                // Lets update the Date everywhere and add to queue
                cout << "My old date is: " << MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_HC[ncd_nr]) << endl;
                MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_HC[ncd_nr])=Date_NCD;
                cout << "My new date is: " << MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_HC[ncd_nr]) << endl;
                
                
                if (ncd_nr==0)
                {
                    MyPointerToPerson->HT=Date_NCD;
                    
                    //// --- Lets feed Hypertension into the eventQ --- ////
                    int p=MyPointerToPerson->PersonID-1;
                    event * HTEvent = new event;
                    Events.push_back(HTEvent);
                    HTEvent->time = MyPointerToPerson->HT;
                    HTEvent->p_fun = &EventMyHyptenDate;
                    HTEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(HTEvent);
                }
                
                if (ncd_nr==1)
                {
                    MyPointerToPerson->MI=Date_NCD;
                    
                    //// --- Lets feed MI into the eventQ --- ////
                    int p=MyPointerToPerson->PersonID-1;
                    event * MIEvent = new event;
                    Events.push_back(MIEvent);
                    MIEvent->time = MyPointerToPerson->MI;
                    MIEvent->p_fun = &EventMyMIDate;
                    MIEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(MIEvent);
                }
                
                if (ncd_nr==2)
                {
                    MyPointerToPerson->Stroke=Date_NCD;
                    
                    //// --- Lets feed Stroke into the eventQ --- ////
                    int p=MyPointerToPerson->PersonID-1;
                    event * StrokeEvent = new event;
                    Events.push_back(StrokeEvent);
                    StrokeEvent->time = MyPointerToPerson->Stroke;
                    StrokeEvent->p_fun = &EventMyStrokeDate;
                    StrokeEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(StrokeEvent);
                }
            }
            
            ncd_nr++;
        }
    }
    E(cout << endl << endl << "Hypercholesterolaemia has developed and addition risks evaluated!" << endl;)

    
}

void EventMyHyptenDate(person *MyPointerToPerson)			// Function executed when person develops hypertension
{
    E(cout << "I just developed Hypercholesterol, lets see if I am at an increased risk of other NCDs!" << endl;)
    
    
    
    if (MyPointerToPerson->HT==0)
    {
        // First lets update Diabetes status to make sure any over-written dates don't run the same cod again
        MyPointerToPerson->HT=1;
        
        
        
        // Some basic code and finding index for not getting NCDs
        int ncd_nr=0;
        double Date_NCD=-998;                                       // As with HIV, if they don't get NCDs set it to -998 to show code was executed
        
        int max_index=0;                                            // This is to make it automatic.  We currently use 6 age groups for NCDs, but we may use more, less in the future
        int max_nr=1;
        while (max_nr>NCDArray[0][0][max_index]){max_index++;}
        
        
        
        // Re-evaluate HC/HT and Renal
        while (ncd_nr<nr_NCD_HT)
        {
            // Get a random number for each NCD
            int i=0;
            double r = ((double) rand() / (RAND_MAX));
            while (r>(NCDArray[MyPointerToPerson->Sex-1][relatedNCDs_HT[ncd_nr]][i]*Risk_NCD_HT[ncd_nr]) && i<max_index){i++;}
            
            
            // If we are getting an NCD lets get the age and date of NCD
            if (i<max_index)
            {
                // Lets get the age and date they will have the NCD
                double Age_NCD = RandomMinMax_2(NCDAgeArrayMin[i],NCDAgeArrayMax[i]);     // Lets get the age they will develop the NCD
                double YearFraction=(RandomMinMax_2(1,12))/12.1;                          // This gets month of birth as a fraction of a year
                Age_NCD=Age_NCD+YearFraction;
                Date_NCD=MyPointerToPerson->DoB+Age_NCD;
            }
            
            
            // Lets see if this pushed forward the existing NCD date
            if (Date_NCD>=*p_GT && Date_NCD<MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_HT[ncd_nr]))
            {
                
                // Lets update the Date everywhere and add to queue
                cout << "My old date is: " << MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_HT[ncd_nr]) << endl;
                MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_HT[ncd_nr])=Date_NCD;
                cout << "My new date is: " << MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_HT[ncd_nr]) << endl;
                
                
                
                if (ncd_nr==0)
                {
                    MyPointerToPerson->MI=Date_NCD;
                    
                    //// --- Lets feed MI into the eventQ --- ////
                    int p=MyPointerToPerson->PersonID-1;
                    event * MIEvent = new event;
                    Events.push_back(MIEvent);
                    MIEvent->time = MyPointerToPerson->MI;
                    MIEvent->p_fun = &EventMyMIDate;
                    MIEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(MIEvent);
                }
                
                if (ncd_nr==1)
                {
                    MyPointerToPerson->CKD=Date_NCD;
                    
                    //// --- Lets feed CKD into the eventQ --- ////
                    int p=MyPointerToPerson->PersonID-1;
                    event * CKDEvent = new event;
                    Events.push_back(CKDEvent);
                    CKDEvent->time = MyPointerToPerson->CKD;
                    CKDEvent->p_fun = &EventMyCKDDate;
                    CKDEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(CKDEvent);
                }
                
                if (ncd_nr==2)
                {
                    MyPointerToPerson->Stroke=Date_NCD;
                    
                    //// --- Lets feed Stroke into the eventQ --- ////
                    int p=MyPointerToPerson->PersonID-1;
                    event * StrokeEvent = new event;
                    Events.push_back(StrokeEvent);
                    StrokeEvent->time = MyPointerToPerson->Stroke;
                    StrokeEvent->p_fun = &EventMyStrokeDate;
                    StrokeEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(StrokeEvent);
                }
            }
            
            ncd_nr++;
        }
    }
    E(cout << endl << endl << "Hypercholesterolaemia has developed and addition risks evaluated!" << endl;)
}

void EventMyMaligDate(person *MyPointerToPerson)			// Function executed when person develops hypertension
{
    E(cout << endl << endl << "This patient just developed cancer!" << endl;)
    MyPointerToPerson->Malig=1;
}

void EventMyMIDate(person *MyPointerToPerson)               // Function executed when person develops hypertension
{
    E(cout << endl << endl << "This patient just developed MI!" << endl;)
    MyPointerToPerson->MI=1;
}
    
void EventMyOsteoDate(person *MyPointerToPerson)			// Function executed when person develops hypertension
{
    E(cout << endl << endl << "This patient just developed Osteo!" << endl;)
    MyPointerToPerson->Osteo=1;
}
void EventMyCKDDate(person *MyPointerToPerson)              // Function executed when person develops hypertension
{
    E(cout << endl << endl << "This patient just developed CKD!" << endl;)
    MyPointerToPerson->CKD=1;
}

void EventMyStrokeDate(person *MyPointerToPerson)			// Function executed when person develops hypertension
{
    E(cout << endl << endl << "This patient just developed Stroke!" << endl;)
    MyPointerToPerson->Stroke=1;
}






