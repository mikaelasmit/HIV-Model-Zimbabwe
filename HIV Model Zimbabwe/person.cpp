/////////////////////////////////////////////////////////////////
//    Created by Mikaela Smit on 22/10/2014.				   //
//    Copyright (c) 2014 Mikaela Smit. All rights reserved.    //
//    This script makes the people in the cohort.			   //
/////////////////////////////////////////////////////////////////


#include <stdio.h>	 									// Include some essential libraries and files
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include "person.h"
#include "event.h"
#include "eventfunctions.h"
#include "errorcoutmacro.h"
#include "eventQ.h"
#include "LoadParams.h"


//// --- OUTSIDE INFORMATION --- ////
extern double   *p_GT;									// Tell this .cpp that there is pointer to Global Time defined externally
extern double   StartYear;								// Include Start Year so only have to change it once in main()
extern int      EndYear;								// Include endyear so that we can avoid putting unnecessary items into EventQ
extern int      *p_PY;									// Pointer to show which year range we are on
extern          person** MyArrayOfPointersToPeople;		// Pointer to MyArrayOfPointersToPeople
extern int      nr_NCDs;
extern          priority_queue<event*, vector<event*>, timeComparison> *p_PQ;
extern          vector<event*> Events;



//// --- POINTERS TO EXTERNAL ARRAYS --- ////
extern double** BirthArray;							 	// This  is a pointer to an array!! i.e pointer to pointer :)
extern double** DeathArray_Women;
extern double** DeathArray_Men;
extern double** HIVArray_Women;
extern double** HIVArray_Men;
extern double** NrChildrenArray;
extern double*  NrChildrenProb;
extern double** Age1950Array;
extern int*     ArrayMin;
extern int*     ArrayMax;
extern double** NCDArray;
extern int*     NCDAgeArrayMin;
extern int*     NCDAgeArrayMax;



//// --- Important Internal informtaion --- ////
vector <float> HIVReservoir(0);

int RandomMinMax(int min, int max){							// Provide function for random number generator between min and max number
    return rand()%(max-min+1)+min;}							// !!!!Note: if min=0 and max=4 it will generate 0,1,2,3,4

int RandomLONGMinMax(long int min, long int max){			// Provide function for random number generator between min and max number
    return rand()%(max-min+1)+min;}							// !!!!Note: if min=0 and max=4 it will generate 0,1,2,3,4


//// --- CLASS (POPULATION) CONSTRUCTOR --- ////

person::person()											// First 'person' class second constructor/variable and no return type means its a constructor
{
    PersonID=0;												// --- Peoples' basic information ---
    Sex=-999;
    
    DoB=-999;												// --- Varibales related to peoples' age and birthday ---
    Age=-999;
                                                            // --- Variables related to birth of children ---
    MotherID=-999;											// Dummy value (i.e. those born before 1950 will not have the ID of mother)
    ChildIDVector.resize(0);								// Vector to store pointer to children.  Make sure it's starting size is 0 at the beginning
    DatesBirth.resize(0);									// This will be used to push in all births of every child
    
    DateOfDeath=9999;										// --- Varibles related to death ---
    CauseOfDeath=-999;                                      // 0=other, 1=HIV, 2=HT, 3=depressopm, 4=Asthma, 5=Stroke, 6=Diabetes, 7=CKD
    AgeAtDeath=-999;										// NOTE: VERY IMPORTANT this number needs to be HIGH as it entres EventQ
    Alive=-999;												// Variable to update eventQ - global check to see if person is still alive
    
    HIV=-999;												// --- Variables related to HIV-infection ---
    CD4_cat_start=-999;
    CD4_cat_ARTstart=-999;
    CD4_cat=-999;											// Where 0=>500, 1=350-500, 2=250-350, 3=200-250, 4=100-200, 5=50-100, and 6=<50
    CD4_change.resize(7);
    ART=-999;												// Where HIV and ART 0=No and 1=Yes
    
    HT=-999;                                                // --- Variables related to NCDs ---
    Depression=-999;
    Asthma=-999;
    Stroke=-999;
    Diabetes=-999;
    CKD=-999;
    HT_status=0;
    Depression_status=0;
    Asthma_status=0;
    Stroke_status=0;
    Diabetes_status=0;
    CKD_status=0;
    NCD_DatesVector.resize(0);
    
}



// --- Assign Person ID ---
void person::PersonIDAssign(int x){
    PersonID=x+1;
    
    E(cout<< "PersonID: " << PersonID<< endl;)
}


// --- Assign Gender Distribution ---
void person::GenderDistribution(){
    
    E(cout << "We are assigning gender!" << endl;)
    
    double	r = ((double) rand() / (RAND_MAX)) ;
    if (r<=0.4986){Sex=1;}									// Where 1 = man and 2= woman
    else {Sex=2;}
    
    E(cout << "We finished assigning gender!" << endl;)
}


// --- Assign Year Of Birth, Age, etc ---
void person::GetMyDoB(){
    
    E(cout << "We are assigning DoB!" << endl;)
    
    double a = ((double) rand() / (RAND_MAX));
    
    int i=0;
    while (a>Age1950Array[Sex-1][i] && i<17){i++;}
    Age = RandomMinMax(ArrayMin[i],ArrayMax[i]);
    double YearFraction=(RandomMinMax(1,12))/12.1;			// This gets month of birth as a fraction of a year
    Age=Age+YearFraction;									// This 'distributes' birthdays across the year as per month of birth (see above)
    DoB=(StartYear-Age);
    
    E(cout << "We finished assigning DoB!" << endl;)
}


// --- Get Dates of all my future babies ---
void person::GetDateOfBaby(){								// This method already calculates the child's month of birth by providing a year of birth with decimal
    
    E(cout << "We are assigning Births!" << endl;)
    
    Age= (*p_GT - DoB);										// Update age ... just in case
    if (Sex == 2 && Alive == 1 && Age<50 && AgeAtDeath>=15){
        
        //// --- Lets first see how many children I will have --- ////
        double r_nr=RandomMinMax(0,100)*0.01;				// To get a random number to later assign whether a woman gets min or max nr of children for time period
        int index=DoB-1901+15;								// index for large arrays, related to year the patient will be 15 and ready for birth
        int NrChildren = 0;
        
        
        if (r_nr<NrChildrenProb[index]){NrChildren=NrChildrenArray[0][index];}
        if (r_nr>=NrChildrenProb[index]){NrChildren=NrChildrenArray[1][index];}
        
        
        //// --- Lets see when I will having all my children --- ////
        // The csv files are loaded elsewhere
        int m=0;											// Count of how many children I will have - can be used to manage ChildVector and loop
        double DateOfBirthTest=-9999;
        int ChildBearingYears=AgeAtDeath-15;
        
        
        //// --- Just in case the person is going to die early, lets let her have as many kdis as possible first --- ////
        if(ChildBearingYears<NrChildren){
            NrChildren=ChildBearingYears;
        }
        
        
        //// --- Let's see when the first birth will happen and check it doesn' ocurr before death --- ////
        while (m<NrChildren){
            double	f = ((double)rand() / (RAND_MAX));		// Get random fertility value
            int j=0;
            while (f>BirthArray[index][j] && j<35){ j++; };	// Find out in which age group I will have my child
            
            DateOfBirthTest = DoB + 15 +j;
            
            while (DateOfBirthTest>=DateOfDeath){			// Run this loop in case death ocurrs before birth
                double	f = ((double)rand() / (RAND_MAX));
                int j = 0;
                while (f>BirthArray[index][j] && j<35){ j++; };
                DateOfBirthTest = DoB + 15+j;
                
            };
            
            if (j>=34){
            }
            
            DatesBirth.push_back(DateOfBirthTest);			// Once we checked birth doesn't happen before birth lets push that in
            
            
            //// --- Lets check if I am already giving birth at this age --- ////
            if (DatesBirth.size()>1){						// Only need to check if there are already more than 1 child
                int n=0;									// Index to count past births with this newly assigned birth
                
                while (n<m){
                    double Diff_1 = DatesBirth.at(n) - DatesBirth.at(m);	// This Diff is the original difference
                    double Diff_2 = DatesBirth.at(n) - DatesBirth.at(m);	// This Diff is the new one if new one is needed.
                    // [...] If both the same we can move on, if not we need to check the new Birth against births 1 to max again
                    while ((DatesBirth.at(n) - DatesBirth.at(m) > -0.75 && DatesBirth.at(n) - DatesBirth.at(m)<0.75) || DatesBirth.at(m)>=DateOfDeath){
                        double	f = ((double)rand() / (RAND_MAX));
                        int j = 0;
                        while (f>BirthArray[*p_PY][j] && j<35){ j++; };
                        
                        DateOfBirthTest = DoB + 15+j;
                        DatesBirth.at(m)=DateOfBirthTest;
                        Diff_2 = DatesBirth.at(n) - DatesBirth.at(m);
                    };
                    
                    if (Diff_1 == Diff_2){ n++;};			// If we didn't have to change BirthOfDate we can check against next birth
                    if (Diff_1 != Diff_2){n = 0;};			// If we have changed DateOfBirth we need to check this new Date against all existing ones
                }
            }
            
            m++;
        }
        
        //// --- Lets feed the births into the EventQ --- ////
        int p=PersonID-1;
        
        for (int c = 0; c < NrChildren; c++){
            if (DatesBirth.at(c) >= *p_GT && DatesBirth.at(c) <EndYear){
                event * BabyBirth = new event;
                Events.push_back(BabyBirth);
                BabyBirth->time = DatesBirth.at(c);
                BabyBirth->p_fun = &EventBirth;
                BabyBirth->person_ID = MyArrayOfPointersToPeople[p];
                p_PQ->push(BabyBirth);
            }
        }
        
    }
    
    E(cout << "We have finished assigning births!" << endl;)
};



// --- Assign Date of death ---
void person::GetDateOfDeath(){								// This is done by assigning life expactancy according to age in 1950
    
    E(cout << "We are assigning deaths!" << endl;)
    
    int i=(DoB-1800);										// To find corresponding year of birth from mortality array
    int j=0;												// This will be matched to probability taken from random number generator
    double	d = ((double) rand() / (RAND_MAX)) ;			// get a random number to determine Life Expectancy
    
    if (Sex==1){
        while(d>DeathArray_Men[i][j] && j<121){j++;}
        DateOfDeath=(DoB+j);
        while (DateOfDeath<*p_GT){							// Run this again if Death happens in the past (important for 1950)
            double	d = ((double) rand() / (RAND_MAX)) ;	// get a random number to determine Life Expectancy
            while(d>DeathArray_Men[i][j] && j<121){j++;}
            DateOfDeath=(DoB+j);}
    }
    
    if (Sex==2) {
        while(d>DeathArray_Women[i][j] && j<121){j++;}
        DateOfDeath=(DoB+j);
        while (DateOfDeath<*p_GT){							// Run this again if Death happens in the past (important for 1950)
            double	d = ((double) rand() / (RAND_MAX)) ;	// Gets a random number to determine Life Expectancy
            while(d>DeathArray_Women[i][j] && j<121){j++;}
            DateOfDeath=(DoB+j);};
    }
    
    AgeAtDeath=DateOfDeath-DoB;
    
    // 2. Lets feed death into the eventQ
    if (DateOfDeath<EndYear){
        int p=PersonID-1;
        event * DeathEvent = new event;
        Events.push_back(DeathEvent);
        DeathEvent->time = DateOfDeath;
        DeathEvent->p_fun = &EventMyDeathDate;
        DeathEvent->person_ID = MyArrayOfPointersToPeople[p];
        p_PQ->push(DeathEvent);
        
        // Update cause of death
        CauseOfDeath=0;
    }
    
    E(cout << "We have finished assigning death dates!" << endl;)
    
}


//// --- FUNCTIONS FOR NEW ENTRY --- ////
void person::GetMyDoBNewEntry(){							// --- Assign Age for New Entry ---
    
    E(cout << "This baby is now 0 years old!" << endl;)
    
    Age=0;													// Set all new entries as 'newborns'
    DoB=*p_GT;
};


//// --- FUNCTIONS RELATED TO HIV --- ////
void person::GetMyDateOfHIVInfection(){
    
    E(cout << "Lets see if this person will get HIV!" << endl;)
    
    if(DoB>=1900){											// Only people born from 1900 can ever experience HIV in their life
        
        int year = floor(*p_GT);
        double months = floor(((1-(*p_GT-year+0.01))*12));
        double fractionyear = 1-(*p_GT-year);
        
        // Let's see when people are getting infected with HVI
        if(HIV==-999){
            int i=(DoB-1900);										// To find corresponding year of birth from mortality array
            int j=0;												// This will be matched to probability taken from random number generator
            float TestHIVDate=0;
            double YearFraction=-999;
            if(months>=1){YearFraction=(RandomMinMax(0,months))/12.1;}			// This gets month of birth as a fraction of a year
            if(months<1){YearFraction=0;}
            double	h = ((double)rand() / (RAND_MAX));				// Get a random number between 0 and 1.  NB/ THIS SHOULD HAVE A PRECISION OF 15 decimals which should be enough but lets be careful!!
            
            if (Sex==1){
                if (h>HIVArray_Men[i][120]){HIV=-988;}				// In case they do NOT get HIV ever
                if (h<=HIVArray_Men[i][120]){						// In case they DO get HIV in their life
                    while(h>HIVArray_Men[i][j] && j<121){j++;}
                    TestHIVDate=(DoB+j)+YearFraction;
                    if (TestHIVDate<DateOfDeath && TestHIVDate>=1975){HIV=TestHIVDate;}
                    if (TestHIVDate>=DateOfDeath && TestHIVDate>=1975) {HIV=-977;}
                    if (TestHIVDate<1975) {HIV=-989;}
                }
            }
            
            if (Sex==2){
                if (h>HIVArray_Women[i][120]){HIV=-988;}			// In case they do NOT get HIV ever
                if (h<=HIVArray_Women[i][120]){						// In case they DO get HIV in their life
                    while(h>HIVArray_Women[i][j] && j<121){j++;}
                    TestHIVDate=(DoB+j);
                    if (TestHIVDate<DateOfDeath && TestHIVDate>=1975){HIV=TestHIVDate;}
                    if (TestHIVDate>=DateOfDeath && TestHIVDate>=1975) {HIV=-977;}
                    if (TestHIVDate<1975) {HIV=-989;}
                }
            }
            
            // Error message:
            if (months>12){cout << "Error 2: There is an error and HIV infection will ocurr in the wrong year: " << months << endl;}
            if (YearFraction>fractionyear){cout << "Error 2: There is an error!" << YearFraction << " and fraction " << fractionyear<< endl;cout << "Global Time "<< *p_GT << " and months " << months << endl;}
            if(YearFraction==-999){cout << "Error 3: Yearfraction hasn't been initialised" << months << endl;}
        }
        
        
        
        //// --- Lets feed HIV infection into the eventQ --- ////
        if (HIV>=*p_GT && HIV<EndYear){
            int p=PersonID-1;
            event * HIVEvent = new event;												
            Events.push_back(HIVEvent);
            HIVEvent->time = HIV;													
            HIVEvent->p_fun = &EventMyHIVInfection;
            HIVEvent->person_ID = MyArrayOfPointersToPeople[p];
            p_PQ->push(HIVEvent);
        }
    }
    
    
    //// --- Some warning code - just in case ... --- ////
    E(if (HIV>-977 && DoB>1900){
        cout << endl <<  "This DIDNT WORK!! WARNING!! "<< endl;
        cout << "HIV: " << HIV << " (Alive: " << Alive << " and Date of Death: " << DateOfDeath << ")" << endl;
        cout << "Size reservoir: " << HIVReservoir.size() << endl << endl;
    })
    
    E(cout << "We have finished checking if this person will get HIV in their lfe time, the person's future HIV status is " << HIV << endl;)
    
};


// --- Assign NCD  ---
void person::GetMyDateNCD(){
    
    E(cout << endl << endl << "We are assigning NCDs!" << endl;)
    
    double age_at_death=DateOfDeath-DoB;

    if (age_at_death>18)
    {
    
    // Some basic code and finding index for not getting NCDs
    int ncd=0;                                                  // Assisgn all the possible NCDs in this code
    double DateNCD=-997;

    
    // Lets get the dates for the NCDs
    while (ncd<nr_NCDs){
        double r = ((double) rand() / (RAND_MAX));                  // Get a random number for each NCD
        
        
        if (r>NCDArray[ncd][120])                               // If they DO NOT get an NCD set date to -998
        {
            if      (ncd==0)    {HT=-998;}
            else if (ncd==1)    {Depression=-998;}
            else if (ncd==2)    {Asthma=-998;}
            else if (ncd==3)    {Stroke=-998;}
            else if (ncd==4)    {Diabetes=-998;}
            else if (ncd==5)    {CKD=-998;}
        }
        
        
        if (r<=NCDArray[ncd][120])                              // If they will get and NCD lets get the age and date and also update mortality
        {
            
            
            // Lets get the index for age at NCD
            int i=0;
            while (r>NCDArray[ncd][i]){i++;}
            
            // Lets get the age and date they will have the NCD
            double YearFraction=(RandomMinMax(1,12))/12.1;                          // This gets month of birth as a fraction of a year
            DateNCD=DoB+i+YearFraction;
            
            if (DateNCD>DateOfDeath && i<50)
            {
                while (DateNCD>DateOfDeath)
                {
                    double Age_NCD = RandomMinMax(NCDAgeArrayMin[0],NCDAgeArrayMax[0]);     // Lets get the age they will develop the NCD
                    double YearFraction=(RandomMinMax(1,12))/12.1;                          // This gets month of birth as a fraction of a year
                    Age_NCD=Age_NCD+YearFraction;
                    DateNCD=DoB+Age_NCD;
                }
                
            }
            
            
            
            if (ncd==0)
            {
                HT=DateNCD;
                //// --- Lets feed Hypertension into the eventQ --- ////
                if (HT>=*p_GT && HT<EndYear){
                    int p=PersonID-1;
                    event * HTEvent = new event;
                    Events.push_back(HTEvent);
                    HTEvent->time = HT;
                    HTEvent->p_fun = &EventMyHyptenDate;
                    HTEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(HTEvent);
                }
            }
            
            
            else if (ncd==1)
            {
                Depression=DateNCD;
                //// --- Lets feed Hypercholesterolaemia into the eventQ --- ////
                if (Depression>=*p_GT && Depression<EndYear){
                    int p=PersonID-1;
                    event * DepressionEvent = new event;
                    Events.push_back(DepressionEvent);
                    DepressionEvent->time = Depression;
                    DepressionEvent->p_fun = &EventMyDepressionDate;
                    DepressionEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(DepressionEvent);
                }
            }
    
            
            else if (ncd==2)
            {
                Asthma=DateNCD;
                //// --- Lets feed Hypertension into the eventQ --- ////
                if (Asthma>=*p_GT && Asthma<EndYear){
                    int p=PersonID-1;
                    event * AsthmaEvent = new event;
                    Events.push_back(AsthmaEvent);
                    AsthmaEvent->time = Asthma;
                    AsthmaEvent->p_fun = &EventMyAsthmaDate;
                    AsthmaEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(AsthmaEvent);
                }
            }
            
            else if (ncd==3)
            {
                Stroke=DateNCD;
                //// --- Lets feed Stroke into the eventQ --- ////
                if (Stroke>=*p_GT && Stroke<EndYear){
                    int p=PersonID-1;
                    event * StrokeEvent = new event;
                    Events.push_back(StrokeEvent);
                    StrokeEvent->time = Stroke;
                    StrokeEvent->p_fun = &EventMyStrokeDate;
                    StrokeEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(StrokeEvent);
                }
            }
            
            
            else if (ncd==4)
            {
                Diabetes=DateNCD;
                //// --- Lets feed MI into the eventQ --- ////
                if (Diabetes>=*p_GT && Diabetes<EndYear){
                    int p=PersonID-1;
                    event * DiabetesEvent = new event;
                    Events.push_back(DiabetesEvent);
                    DiabetesEvent->time = Diabetes;
                    DiabetesEvent->p_fun = &EventMyDiabetesDate;
                    DiabetesEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(DiabetesEvent);
                }
            }
            
            else if (ncd==5)
            {
                CKD=DateNCD;
                //// --- Lets feed MI into the eventQ --- ////
                if (CKD>=*p_GT && CKD<EndYear){
                    int p=PersonID-1;
                    event * CKDEvent = new event;
                    Events.push_back(CKDEvent);
                    CKDEvent->time = CKD;
                    CKDEvent->p_fun = &EventMyCKDDate;
                    CKDEvent->person_ID = MyArrayOfPointersToPeople[p];
                    p_PQ->push(CKDEvent);
                }
            }

        
        }
       
        NCD_DatesVector.push_back(DateNCD);
        ncd++;                                                                      // Lets do the next NCD
    }
    }
    E(cout << "We finished assigning NCDs!" << endl;)
}





