/////////////////////////////////////////////////////////////////
//    Created by Mikaela Smit on 07/11/2014.				   //	
//    These event are added to the EventQ.					   //
/////////////////////////////////////////////////////////////////

#include <stdio.h> 
#include "person.h"
using namespace std;


//// ---- NOTE: Think about having different types of events ----

	//// --- Demographic Events ---
	void EventMyDeathDate(person *MyPointerToPerson);			// Function to be executed upon death
	void EventBirth(person *MyPointerToPerson);					// Function to be executed when someone gives birth
	
	
	//// --- Calendar Events ---
	void EventTellNewYear(person *MyPointerToPerson);			// Tell us when a new year start an update calendar-related items
	
	
	//// --- HIV Events ---
	void EventMyHIVInfection(person *MyPointerToPerson);       // Function executed once person gets infecte with HIV
	void EventCD4change(person *MyPointerToPerson);			   // Function executed when CD4 count is due to change (go down)


    //// --- NCD Events ---
    void EventMyDiabetesDate(person *MyPointerToPerson);        // Function executed when person develops diabetes
    void EventMyHypcholDate(person *MyPointerToPerson);			// Function executed when person develops hypercholesterolaemia
    void EventMyHyptenDate(person *MyPointerToPerson);			// Function executed when person develops hypertension
    void EventMyMaligDate(person *MyPointerToPerson);			// Function executed when person develops hypertension
    void EventMyMIDate(person *MyPointerToPerson);              // Function executed when person develops hypertension
    void EventMyOsteoDate(person *MyPointerToPerson);			// Function executed when person develops hypertension
    void EventMyCKDDate(person *MyPointerToPerson);             // Function executed when person develops hypertension
    void EventMyStrokeDate(person *MyPointerToPerson);			// Function executed when person develops hypertension

