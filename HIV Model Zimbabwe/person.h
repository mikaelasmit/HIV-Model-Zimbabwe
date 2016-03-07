///////////////////////////////////////////////////////////////
//    Created by Mikaela Smit on 22/10/2014.				   //
//    This script makes the people in the cohort.			   //
/////////////////////////////////////////////////////////////////


#ifndef person_h 									// If not defined then defines it  # are pre-compiler definitions
#define person_h
#include <vector>

using namespace std;



//// --- CREATING PEOPLE --- ////

class person{										// Classes are considered private unless stated otherwise
public:
    
    person();										// Class constructor or calling function for person
    
    
    //// --- Persons Characteristics --- ////
    int PersonID;									// -- Basic variables --
    int Sex;
    
    double DoB;										// -- Variables related to age --
    double Age;
  
    int MotherID;									// -- Variables related to Mother and Children
    vector<person*> ChildIDVector;					// ChildIDVector is its name <now holds 'people pointer'> of type vector which is the structure
    vector<double> DatesBirth;						// Hold dates of the birth of all children - vector
    
    double DateOfDeath;								// -- Variables related to death --
    int Alive;
    int AgeAtDeath;
    
    double HIV;										// -- Variables related to HIV --
    int CD4_cat_start;
    int CD4_cat;
    int ART;
    
    double HT;                                      // -- Variables related to NCDs --
    int HT_status;
    double Depression;
    int Depression_status;
    double Asthma;
    int Asthma_status;
    double Stroke;
    int Stroke_status;
    double Diabetes;
    int Diabetes_status;
    vector<double> NCD_DatesVector;
    
    
    
    //// --- FUNCTIONS --- ////
    
    
    // --- Functions related to People ---
    void PersonIDAssign(int x);						// Function to get Person ID
    
    
    // --- Sex ---
    void GenderDistribution();						// Function to assign sex
    
    
    // --- Year of Birth ---
    void GetMyDoB();								// Function to assign AGE and DOB (with months)
    
    
    // --- Birth of First Child ---
    void GetDateOfBaby();							// Function to get the date of the children
    
    
    // --- Date of Death ---
    void GetDateOfDeath();							// Function to get date of Death - this is done by using UN Life expectancy by age in 1950 and random numbers (see person.cpp)
    
    
    // --- Age for babies ---
    void GetMyDoBNewEntry();						// Function to assign age 0 to babies
    
    
    // --- Date of Infection ---
    void GetMyDateOfHIVInfection();
    
    // --- Date of NCDs ---
    void GetMyDateNCD();
    
    
};	

#endif