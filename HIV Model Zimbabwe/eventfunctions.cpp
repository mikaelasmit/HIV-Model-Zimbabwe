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
extern int ART_start_yr;

extern double*** CD4_startarray;
extern double**  CD4_prog_rates;
extern double*** Death_CD4_rates;
extern double*** ART_CD4_rates;

extern double**  NCDArray;
extern int*      NCDAgeArrayMin;
extern int*      NCDAgeArrayMax;

extern double** DeathArray_Women;
extern double** DeathArray_Men;


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


//////////////////////////////////
///// CHANGE PARAMETERS HERE /////
//////////////////////////////////


// Note:
// 0. Hypertension
// 1. Depression
// 2. Asthma
// 3. Stroke
// 4. Diabetes

//double Risk_NCD_Diabetes[5]={Risk_DiabHC, Risk_DiabHT, Risk_DiabCVD, Risk_DiabCKD, Risk_DiabCVD};
//int relatedNCDs_Diab[5]={1, 2, 4, 6, 7};
double Risk_NCD_Diabetes[2]={Risk_DiabHT, Risk_DiabCVD};
int relatedNCDs_Diab[2]={0, 3};
int nr_NCD_Diab=sizeof(relatedNCDs_Diab)/sizeof(relatedNCDs_Diab[0]);


//double Risk_NCD_HC[3]={Risk_HCHT, Risk_HCCVD, Risk_HCCVD};
//int relatedNCDs_HC[3]={2, 4, 7};
//int nr_NCD_HC=sizeof(relatedNCDs_HC)/sizeof(relatedNCDs_HC[0]);


//double Risk_NCD_HT[3]={Risk_HTCVD, Risk_HTCKD, Risk_HTCVD};
//int relatedNCDs_HT[3]={4, 6, 7};
double Risk_NCD_HT[1]={Risk_HTCVD};
int relatedNCDs_HT[1]={3};
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
        MyPointerToPerson->CD4_change.at(MyPointerToPerson->CD4_cat)=*p_GT;
        //cout << "Date of CD4: " <<MyPointerToPerson->CD4_change.at(MyPointerToPerson->CD4_cat_start) << endl;

	
		//// --- Let's see what will happen next (Death, CD4 count progression or ART initiation) ---- ////
        double FindART_CD4_rate = ART_CD4_rates[MyPointerToPerson->Sex-1][i][MyPointerToPerson->CD4_cat];
        double FindCD4_rate = CD4_prog_rates[MyPointerToPerson->Sex-1][MyPointerToPerson->CD4_cat];
        double FindDeath_CD4_rate = Death_CD4_rates[MyPointerToPerson->Sex-1][i][MyPointerToPerson->CD4_cat];
        
    
        
        // Lets see when CD4 count progression would start
        double CD4_test = 0;
        double cd4 = ((double)rand() / (RAND_MAX));
        CD4_test = (-1/FindCD4_rate) * log(cd4);
        
        
        
        // Lets see when death would happen
        double death_test = 0;
        double dd = ((double)rand() / (RAND_MAX));
        death_test = (-1/FindDeath_CD4_rate) * log(dd);
        
        
        
        // Lets see if ART should start
        if (*p_GT>=ART_start_yr){
            
            // First lets see if they would start ART now
            
            // To do so we first need to find year ref for ART data arrays
            int p_cov=floor(*p_GT)-ART_start_yr;
            if (*p_GT>2015){p_cov=11;}
            
            if (p_cov>11){
                cout << "GT: " << *p_GT << " p_cov: " << p_cov << endl;
            }
            
            // Lets check if they start ART
            if (MyPointerToPerson->Age<=1.5){
                
                float ART_coverage[11]={0.00435, 0.01024, 0.03817, 0.05674, 0.08456, 0.12762, 0.19595, 0.23988, 0.29309, 0.29978, 0.37502};
                
                // Lets see if these kids qualify for ART - ASSUME only newborns start ART (not 2-15 year olds)
                double art = ((double)rand() / (RAND_MAX));
                if (art<ART_coverage[p_cov]){ MyPointerToPerson->ART=*p_GT; MyPointerToPerson->CD4_cat_ARTstart=MyPointerToPerson->CD4_cat;}
            }
            
            if (MyPointerToPerson->CD4_cat<4){
                //cout << "Lets check this" << endl;
            }
            
            // Lets evaluate for adults
            if (MyPointerToPerson->Age>15){
                
                // For Men
                if (MyPointerToPerson->Sex==1){
                    
                    float ART_at_CD4_M[12][7]={
                        {0,         0,          0,          0,          0.02649,	0.01485,	0.01396},
                        {0,         0,          0,          0,          0.06324,	0.03493,	0.03163},
                        {0,         0,          0,          0,          0.14185,	0.07740,	0.06815},
                        {0,         0,          0,          0,          0.18485,	0.09845,	0.08259},
                        {0,         0,          0,          0,          0.25465,	0.13323,	0.10852},
                        {0,         0,          0,          0,          0.37316,	0.18916,	0.14558},
                        {0.02209,	0.01951,	0.01238,	0.00821,	0.43253,	0.20912,	0.14376},
                        {0.01523,	0.01355,	0.07608,	0.05560,	0.24948,	0.11838,	0.07638},
                        {0.01691,	0.01510,	0.11378,	0.07907,	0.24297,	0.11511,	0.07166},
                        {0.01951,	0.01746,	0.14931,	0.09950,	0.25175,	0.12034,	0.07363},
                        {0.01873,	0.04799,	0.11570,	0.07740,	0.17805,	0.08599,	0.05173},
                        {0.03897,	0.14904,	0.17234,	0.11260,	0.19358,	0.08522,	0.04824},
                    };
                    
                    double art = ((double)rand() / (RAND_MAX));
                    if (art<ART_at_CD4_M[p_cov][MyPointerToPerson->CD4_cat]){ MyPointerToPerson->ART=*p_GT;MyPointerToPerson->CD4_cat_ARTstart=MyPointerToPerson->CD4_cat;}
                    
                    if (MyPointerToPerson->CD4_cat<4){
                        //cout << "Lets check this" << endl;
                        
                        
                        
                        
                        
                    }
                }
                
                // For Women
                if (MyPointerToPerson->Sex==2){
                    
                    float ART_at_CD4_F[12][7]={
                        {0,         0,          0,          0,          0.02511,	0.01399,	0.01320},
                        {0,         0,          0,          0,          0.06003,	0.03308,	0.03029},
                        {0,         0,          0,          0,          0.11665,	0.06372,	0.05703},
                        {0,         0,          0,          0,          0.22836,	0.12237,	0.10487},
                        {0,         0,          0,          0,          0.33454,	0.17126,	0.13470},
                        {0,         0,          0,          0,          0.45025,	0.21597,	0.15357},
                        {0.02937,	0.02652,	0.01729,	0.01182,	0.52829,	0.23224,	0.13967},
                        {0.01982,	0.01807,	0.09625,	0.07162,	0.29861,	0.12595,	0.07239},
                        {0.02186,	0.02001,	0.14027,	0.09733,	0.28823,	0.12135,	0.06745},
                        {0.02509,	0.02306,	0.18195,	0.11913,	0.29281,	0.12401,	0.06674},
                        {0.03044,	0.05959,	0.14018,	0.09169,	0.20540,	0.08747,	0.04602},
                        {0.05760,	0.13219,	0.17286,	0.10999,	0.20289,	0.08216,	0.04231},
                    };
                    
                    double art = ((double)rand() / (RAND_MAX));
                    if (art<ART_at_CD4_F[p_cov][MyPointerToPerson->CD4_cat]){ MyPointerToPerson->ART=*p_GT;MyPointerToPerson->CD4_cat_ARTstart=MyPointerToPerson->CD4_cat;}
                }
                
            }
            
        }
      
        
    
        //  If they do not start ART then we need to evaluate if they die or decrease in CD4 count
        if (MyPointerToPerson->ART<0){
            
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
        MyPointerToPerson->CD4_change.at(MyPointerToPerson->CD4_cat)=*p_GT;
        
        
        //// --- Lets see if they start ART now --- ////
        if (*p_GT>=ART_start_yr){
            
            // First lets see if they would start ART now
            
            // To do so we first need to find year ref for ART data arrays
            int p_cov=floor(*p_GT)-ART_start_yr;
            if (*p_GT>2015){p_cov=11;}
                    
            // Lets check if they start ART
            if (MyPointerToPerson->Age<=1.5){
                
                float ART_coverage[11]={0.00435, 0.01024, 0.03817, 0.05674, 0.08456, 0.12762, 0.19595, 0.23988, 0.29309, 0.29978, 0.37502};
                
                // Lets see if these kids qualify for ART - ASSUME only newborns start ART (not 2-15 year olds)
                double art = ((double)rand() / (RAND_MAX));
                if (art<ART_coverage[p_cov]){ MyPointerToPerson->ART=*p_GT; MyPointerToPerson->CD4_cat_ARTstart=MyPointerToPerson->CD4_cat;}
            }
            
            // Lets evaluate for adults
            if (MyPointerToPerson->Age>15){
                
                // For Men
                if (MyPointerToPerson->Sex==1){
                    
                    float ART_at_CD4_M[12][7]={
                        {0,         0,          0,          0,          0.02649,	0.01485,	0.01396},
                        {0,         0,          0,          0,          0.06324,	0.03493,	0.03163},
                        {0,         0,          0,          0,          0.14185,	0.07740,	0.06815},
                        {0,         0,          0,          0,          0.18485,	0.09845,	0.08259},
                        {0,         0,          0,          0,          0.25465,	0.13323,	0.10852},
                        {0,         0,          0,          0,          0.37316,	0.18916,	0.14558},
                        {0.02209,	0.01951,	0.01238,	0.00821,	0.43253,	0.20912,	0.14376},
                        {0.01523,	0.01355,	0.07608,	0.05560,	0.24948,	0.11838,	0.07638},
                        {0.01691,	0.01510,	0.11378,	0.07907,	0.24297,	0.11511,	0.07166},
                        {0.01951,	0.01746,	0.14931,	0.09950,	0.25175,	0.12034,	0.07363},
                        {0.01873,	0.04799,	0.11570,	0.07740,	0.17805,	0.08599,	0.05173},
                        {0.03897,	0.14904,	0.17234,	0.11260,	0.19358,	0.08522,	0.04824},
                    };
                    
                    double art = ((double)rand() / (RAND_MAX));
                    if (art<ART_at_CD4_M[p_cov][MyPointerToPerson->CD4_cat]){ MyPointerToPerson->ART=*p_GT;MyPointerToPerson->CD4_cat_ARTstart=MyPointerToPerson->CD4_cat;}
                }
                
                // For Women
                if (MyPointerToPerson->Sex==2){
                    
                    float ART_at_CD4_F[12][7]={
                        {0,         0,          0,          0,          0.02511,	0.01399,	0.01320},
                        {0,         0,          0,          0,          0.06003,	0.03308,	0.03029},
                        {0,         0,          0,          0,          0.11665,	0.06372,	0.05703},
                        {0,         0,          0,          0,          0.22836,	0.12237,	0.10487},
                        {0,         0,          0,          0,          0.33454,	0.17126,	0.13470},
                        {0,         0,          0,          0,          0.45025,	0.21597,	0.15357},
                        {0.02937,	0.02652,	0.01729,	0.01182,	0.52829,	0.23224,	0.13967},
                        {0.01982,	0.01807,	0.09625,	0.07162,	0.29861,	0.12595,	0.07239},
                        {0.02186,	0.02001,	0.14027,	0.09733,	0.28823,	0.12135,	0.06745},
                        {0.02509,	0.02306,	0.18195,	0.11913,	0.29281,	0.12401,	0.06674},
                        {0.03044,	0.05959,	0.14018,	0.09169,	0.20540,	0.08747,	0.04602},
                        {0.05760,	0.13219,	0.17286,	0.10999,	0.20289,	0.08216,	0.04231},
                    };
                    
                    double art = ((double)rand() / (RAND_MAX));
                    if (art<ART_at_CD4_F[p_cov][MyPointerToPerson->CD4_cat]){ MyPointerToPerson->ART=*p_GT;MyPointerToPerson->CD4_cat_ARTstart=MyPointerToPerson->CD4_cat;}
                    
                }
                
            }
            
        }
        
       
        
        //// --- When CD4 count hits the lowest possible value and no ART is started --- ////
        if (MyPointerToPerson->CD4_cat==6 && MyPointerToPerson->ART<0){
            
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

    
        //// --- In case CD4 count is higher than minimum possible category.  They can either die OR Progress AND they still have a 1 year risk of death if on ART --- ///
        if (MyPointerToPerson->CD4_cat<6){
        
            //// --- Let's see what will happen next (Death, CD4 count progression or ART initiation) ---- ////
            double FindART_CD4_rate = ART_CD4_rates[MyPointerToPerson->Sex-1][i][MyPointerToPerson->CD4_cat];
            double FindCD4_rate = CD4_prog_rates[MyPointerToPerson->Sex-1][MyPointerToPerson->CD4_cat];
            double FindDeath_CD4_rate = Death_CD4_rates[MyPointerToPerson->Sex-1][i][MyPointerToPerson->CD4_cat];
            
            
            // Lets see when CD4 count progression would start
            double CD4_test = 0;
            double cd4 = ((double)rand() / (RAND_MAX));
            CD4_test = (-1/FindCD4_rate) * log(cd4);
            
            
            // Lets see when death would happen
            double death_test = 0;
            double dd = ((double)rand() / (RAND_MAX));
            death_test = (-1/FindDeath_CD4_rate) * log(dd);
            
            
            // If patient hasn't started ART yet
            if (MyPointerToPerson->ART<0){
                
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


	}
	
	E(cout << "Somebody has just experiences a drop in CD4 count!" << endl;)	// Error message - can be switched on/off
}
	

//////// NCD EVENTS //////

void EventMyDepressionDate(person *MyPointerToPerson)		// Function executed when person develops depression
{
    E(cout << endl << endl << "This patient just developed depression!" << endl;)
    MyPointerToPerson->Depression_status=1;
    
    // Lets see if we need to update death date
    double MortRisk[6]={1.52, 0, 1.3, 1.59, 1.77, 0.8};
    int ncd_index=1;
    
    // Lets see if they die earlier
    int k=(MyPointerToPerson->DoB-1800);					// To find corresponding year of birth from mortality array
    int j=0;												// This will be matched to probability taken from random number generator
    double	d = ((double) rand() / (RAND_MAX)) ;			// get a random number to determine Life Expectancy
    double TestDeathDate;
    
    if (MyPointerToPerson->Sex==1){
        while(d>DeathArray_Men[k][j]*MortRisk[ncd_index] && j<121){j++;}
        TestDeathDate=(MyPointerToPerson->DoB+j);
    }
    
    if (MyPointerToPerson->Sex==2) {
        while(d>DeathArray_Women[k][j]*MortRisk[ncd_index] && j<121){j++;}
        TestDeathDate=(MyPointerToPerson->DoB+j);
    }
    
    if (TestDeathDate<MyPointerToPerson->DateOfDeath){
        
        MyPointerToPerson->DateOfDeath=TestDeathDate;
        
        // 2. Lets feed death into the eventQ
        if (MyPointerToPerson->DateOfDeath<EndYear){
            int p=MyPointerToPerson->PersonID-1;
            event * DeathEvent = new event;
            Events.push_back(DeathEvent);
            DeathEvent->time = MyPointerToPerson->DateOfDeath;
            DeathEvent->p_fun = &EventMyDeathDate;
            DeathEvent->person_ID = MyArrayOfPointersToPeople[p];
            p_PQ->push(DeathEvent);
            
            // Update cause of death
            MyPointerToPerson->CauseOfDeath=ncd_index+2;
        }
    }
    
}

void EventMyAsthmaDate(person *MyPointerToPerson)			// Function executed when person develops asthma
{
    E(cout << endl << endl << "This patient just developed asthma!" << endl;)
    MyPointerToPerson->Asthma_status=1;
    
    // Lets see if we need to update death date
    double MortRisk[6]={1.52, 0, 1.3, 1.59, 1.77, 0.8};
    int ncd_index=2;
    
    // Lets see if they die earlier
    int k=(MyPointerToPerson->DoB-1800);					// To find corresponding year of birth from mortality array
    int j=0;												// This will be matched to probability taken from random number generator
    double	d = ((double) rand() / (RAND_MAX)) ;			// get a random number to determine Life Expectancy
    double TestDeathDate;
    
    if (MyPointerToPerson->Sex==1){
        while(d>DeathArray_Men[k][j]*MortRisk[ncd_index] && j<121){j++;}
        TestDeathDate=(MyPointerToPerson->DoB+j);
    }
    
    if (MyPointerToPerson->Sex==2) {
        while(d>DeathArray_Women[k][j]*MortRisk[ncd_index] && j<121){j++;}
        TestDeathDate=(MyPointerToPerson->DoB+j);
    }
    
    if (TestDeathDate<MyPointerToPerson->DateOfDeath){
        
        MyPointerToPerson->DateOfDeath=TestDeathDate;
        
        // 2. Lets feed death into the eventQ
        if (MyPointerToPerson->DateOfDeath<EndYear){
            int p=MyPointerToPerson->PersonID-1;
            event * DeathEvent = new event;
            Events.push_back(DeathEvent);
            DeathEvent->time = MyPointerToPerson->DateOfDeath;
            DeathEvent->p_fun = &EventMyDeathDate;
            DeathEvent->person_ID = MyArrayOfPointersToPeople[p];
            p_PQ->push(DeathEvent);
            
            // Update cause of death
            MyPointerToPerson->CauseOfDeath=ncd_index+2;
        }
    }
}

void EventMyStrokeDate(person *MyPointerToPerson)			// Function executed when person develops hypertension
{
    E(cout << endl << endl << "This patient just developed stroke!" << endl;)
    MyPointerToPerson->Stroke_status=1;
    
    // Lets see if we need to update death date
    double MortRisk[6]={1.52, 0, 1.3, 1.59, 1.77, 0.8};
    int ncd_index=3;
    
    // Lets see if they die earlier
    int k=(MyPointerToPerson->DoB-1800);					// To find corresponding year of birth from mortality array
    int j=0;												// This will be matched to probability taken from random number generator
    double	d = ((double) rand() / (RAND_MAX)) ;			// get a random number to determine Life Expectancy
    double TestDeathDate;
    
    if (MyPointerToPerson->Sex==1){
        while(d>DeathArray_Men[k][j]*MortRisk[ncd_index] && j<121){j++;}
        TestDeathDate=(MyPointerToPerson->DoB+j);
    }
    
    if (MyPointerToPerson->Sex==2) {
        while(d>DeathArray_Women[k][j]*MortRisk[ncd_index] && j<121){j++;}
        TestDeathDate=(MyPointerToPerson->DoB+j);
    }
    
    if (TestDeathDate<MyPointerToPerson->DateOfDeath){
        
        MyPointerToPerson->DateOfDeath=TestDeathDate;
        
        // 2. Lets feed death into the eventQ
        if (MyPointerToPerson->DateOfDeath<EndYear){
            int p=MyPointerToPerson->PersonID-1;
            event * DeathEvent = new event;
            Events.push_back(DeathEvent);
            DeathEvent->time = MyPointerToPerson->DateOfDeath;
            DeathEvent->p_fun = &EventMyDeathDate;
            DeathEvent->person_ID = MyArrayOfPointersToPeople[p];
            p_PQ->push(DeathEvent);
            
            // Update cause of death
            MyPointerToPerson->CauseOfDeath=ncd_index+2;
        }
    }
}


void EventMyDiabetesDate(person *MyPointerToPerson){
    
    E(cout << "I just developed Diabetes, lets see if I am at an increased risk of other NCDs!" << endl;)
    
    
    if (MyPointerToPerson->Diabetes_status==0)
    {
        // First lets update Diabetes status to make sure any over-written dates don't run the same cod again
        MyPointerToPerson->Diabetes_status=1;
        
        // Lets see if we need to update death date
        double MortRisk[6]={1.52, 0, 1.3, 1.59, 1.77, 0.8};
        int ncd_index=4;
        
        // Lets see if they die earlier
        int k=(MyPointerToPerson->DoB-1800);					// To find corresponding year of birth from mortality array
        int j=0;												// This will be matched to probability taken from random number generator
        double	d = ((double) rand() / (RAND_MAX)) ;			// get a random number to determine Life Expectancy
        double TestDeathDate;
        
        if (MyPointerToPerson->Sex==1){
            while(d>DeathArray_Men[k][j]*MortRisk[ncd_index] && j<121){j++;}
            TestDeathDate=(MyPointerToPerson->DoB+j);
        }
        
        if (MyPointerToPerson->Sex==2) {
            while(d>DeathArray_Women[k][j]*MortRisk[ncd_index] && j<121){j++;}
            TestDeathDate=(MyPointerToPerson->DoB+j);
        }
        
        if (TestDeathDate<MyPointerToPerson->DateOfDeath){
            
            MyPointerToPerson->DateOfDeath=TestDeathDate;
            
            // 2. Lets feed death into the eventQ
            if (MyPointerToPerson->DateOfDeath<EndYear){
                int p=MyPointerToPerson->PersonID-1;
                event * DeathEvent = new event;
                Events.push_back(DeathEvent);
                DeathEvent->time = MyPointerToPerson->DateOfDeath;
                DeathEvent->p_fun = &EventMyDeathDate;
                DeathEvent->person_ID = MyArrayOfPointersToPeople[p];
                p_PQ->push(DeathEvent);
                
                // Update cause of death
                MyPointerToPerson->CauseOfDeath=ncd_index+2;
            }
        }
        
        
        
        // Some basic code and finding index for not getting NCDs
        int ncd_nr=0;
        double DateNCD=-997;                                       // As with HIV, if they don't get NCDs set it to -998 to show code was executed
        
    
        // Re-evaluate NCD related to diabetes
        while (ncd_nr<nr_NCD_Diab)
        {
            // Get a random number for each NCD
            //cout << "Ncd_nr: " << ncd_nr << " position in diabetes array: " << relatedNCDs_Diab[ncd_nr] << endl;
            double r = ((double) rand() / (RAND_MAX));
            //cout << "New max risk: " << NCDArray[relatedNCDs_Diab[ncd_nr]][120]*Risk_NCD_Diabetes[ncd_nr] << " old risk: " << NCDArray[ncd_nr][120] << endl;
        
           
            // If we are getting an NCD lets get the age and date of NCD
            if (r<NCDArray[relatedNCDs_Diab[ncd_nr]][120]*Risk_NCD_Diabetes[ncd_nr])
            {
                // Lets get the index for age at NCD
                int i=0;
                while (r>NCDArray[relatedNCDs_Diab[ncd_nr]][i]*Risk_NCD_Diabetes[ncd_nr]){i++;}

                
                // Lets get the age and date they will have the NCD
                double YearFraction=(RandomMinMax_2(1,12))/12.1;                          // This gets month of birth as a fraction of a year
                DateNCD=MyPointerToPerson->DoB+i+YearFraction;

            }
        
        
            // Lets see if this pushed forward the existing NCD date
            if (DateNCD>=*p_GT && DateNCD<MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_Diab[ncd_nr]))
            {
            
                // Lets update the Date everywhere and add to queue
                //cout << "My old date is: " << MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_Diab[ncd_nr]) << endl;
                MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_Diab[ncd_nr])=DateNCD;
                //cout << "My new date is: " << MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_Diab[ncd_nr]) << endl;
            
                
                if (ncd_nr==0)
                {
                    MyPointerToPerson->HT=DateNCD;
                
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
                    MyPointerToPerson->Stroke=DateNCD;
                    
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

void EventMyHyptenDate(person *MyPointerToPerson)			// Function executed when person develops hypertension
{
    E(cout << "I just developed Hypercholesterol, lets see if I am at an increased risk of other NCDs!" << endl;)
    
    
    if (MyPointerToPerson->HT_status==0)
    {
        // First lets update Diabetes status to make sure any over-written dates don't run the same cod again
        MyPointerToPerson->HT_status=1;
        
        // Lets see if we need to update death date
        double MortRisk[6]={1.52, 0, 1.3, 1.59, 1.77, 0.8};
        int ncd_index=0;
        
        // Lets see if they die earlier
        int k=(MyPointerToPerson->DoB-1800);					// To find corresponding year of birth from mortality array
        int j=0;												// This will be matched to probability taken from random number generator
        double	d = ((double) rand() / (RAND_MAX)) ;			// get a random number to determine Life Expectancy
        double TestDeathDate;
        
        if (MyPointerToPerson->Sex==1){
            while(d>DeathArray_Men[k][j]*MortRisk[ncd_index] && j<121){j++;}
            TestDeathDate=(MyPointerToPerson->DoB+j);
        }
        
        if (MyPointerToPerson->Sex==2) {
            while(d>DeathArray_Women[k][j]*MortRisk[ncd_index] && j<121){j++;}
            TestDeathDate=(MyPointerToPerson->DoB+j);
        }
        
        if (TestDeathDate<MyPointerToPerson->DateOfDeath){
            
            MyPointerToPerson->DateOfDeath=TestDeathDate;
            
            // 2. Lets feed death into the eventQ
            if (MyPointerToPerson->DateOfDeath<EndYear){
                int p=MyPointerToPerson->PersonID-1;
                event * DeathEvent = new event;
                Events.push_back(DeathEvent);
                DeathEvent->time = MyPointerToPerson->DateOfDeath;
                DeathEvent->p_fun = &EventMyDeathDate;
                DeathEvent->person_ID = MyArrayOfPointersToPeople[p];
                p_PQ->push(DeathEvent);
                
                // Update cause of death
                MyPointerToPerson->CauseOfDeath=ncd_index+2;
            }
        }
        
        
        // Some basic code and finding index for not getting NCDs
        int ncd_nr=0;
        double DateNCD=-997;                                       // As with HIV, if they don't get NCDs set it to -998 to show code was executed
        
    
        // Re-evaluate HC/HT and Renal
        while (ncd_nr<nr_NCD_HT)
        {
            // Get a random number for each NCD
            double r = ((double) rand() / (RAND_MAX));
            //while (r>(NCDArray[MyPointerToPerson->Sex-1][relatedNCDs_HT[ncd_nr]][i]*Risk_NCD_HT[ncd_nr]) && i<max_index){i++;}
            
            
            // If we are getting an NCD lets get the age and date of NCD
            if (r<NCDArray[relatedNCDs_HT[ncd_nr]][120]*Risk_NCD_HT[ncd_nr])
            {
                // Lets get the age and date they will have the NCD
                int i=0;
                while (r>NCDArray[relatedNCDs_HT[ncd_nr]][i]*Risk_NCD_HT[ncd_nr]){i++;}
                
                
                // Lets get the age and the date they will have the NCD
                double YearFraction=(RandomMinMax_2(1,12))/12.1;                          // This gets month of birth as a fraction of a year
                DateNCD=MyPointerToPerson->DoB+i+YearFraction;
            }
            
            
            // Lets see if this pushed forward the existing NCD date
            if (DateNCD>=*p_GT && DateNCD<MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_HT[ncd_nr]))
            {
                
                // Lets update the Date everywhere and add to queue
                //cout << "My old date is: " << MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_HT[ncd_nr]) << endl;
                MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_HT[ncd_nr])=DateNCD;
                //cout << "My new date is: " << MyPointerToPerson->NCD_DatesVector.at(relatedNCDs_HT[ncd_nr]) << endl;
            
                
                if (ncd_nr==0)
                {
                    MyPointerToPerson->Stroke=DateNCD;
                    
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


void EventMyCKDDate (person *MyPointerToPerson)			// Function executed when person develops hypertension
{
    
    E(cout << endl << endl << "This patient just developed CKD!" << endl;)
    MyPointerToPerson->CKD_status=1;
    
    // Lets see if we need to update death date
    double MortRisk[6]={1.52, 0, 1.3, 1.59, 1.77, 0.8};
    int ncd_index=5;
    
    // Lets see if they die earlier
    int k=(MyPointerToPerson->DoB-1800);					// To find corresponding year of birth from mortality array
    int j=0;												// This will be matched to probability taken from random number generator
    double	d = ((double) rand() / (RAND_MAX)) ;			// get a random number to determine Life Expectancy
    double TestDeathDate;
    
    if (MyPointerToPerson->Sex==1){
        while(d>DeathArray_Men[k][j]*MortRisk[ncd_index] && j<121){j++;}
        TestDeathDate=(MyPointerToPerson->DoB+j);
    }
    
    if (MyPointerToPerson->Sex==2) {
        while(d>DeathArray_Women[k][j]*MortRisk[ncd_index] && j<121){j++;}
        TestDeathDate=(MyPointerToPerson->DoB+j);
    }
    
    if (TestDeathDate<MyPointerToPerson->DateOfDeath){
        
        MyPointerToPerson->DateOfDeath=TestDeathDate;
        
        // 2. Lets feed death into the eventQ
        if (MyPointerToPerson->DateOfDeath<EndYear){
            int p=MyPointerToPerson->PersonID-1;
            event * DeathEvent = new event;
            Events.push_back(DeathEvent);
            DeathEvent->time = MyPointerToPerson->DateOfDeath;
            DeathEvent->p_fun = &EventMyDeathDate;
            DeathEvent->person_ID = MyArrayOfPointersToPeople[p];
            p_PQ->push(DeathEvent);
            
            // Update cause of death
            MyPointerToPerson->CauseOfDeath=ncd_index+2;
        }
    }
}

