#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""

This is a template class for automatically generating 
Parcel component classes for ADHydro.

"""
_cxx_abstract_class_header_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef PARCEL_H
#define PARCEL_H
/*
    This is an abstract Parcel class representing a generic irrigated land.
    Each parcel knows about the ADHydro elements that it irrigates, and is
    told by its diversion how much water that parcel can use.  It is the up
    to the parcel to distribute the water to these elements in approriate ways.
*/
#include "pup.h"
#include "pup_stl.h"

class Parcel :  public PUP::able
{
    protected:
        /*
         * Really don't want the abstract class copied, this will likely lead to slicing.
         * Putting empty copy constructor and assignment in protected will cause compiler to
         * error if it is attempted by anything except subclasses or the object itself.
         * Subclasses should be able to call the copy constructor so that the parcelID gets
         * properly initalized in the copy.
         * Only pointers of abstract class should be copied/assigned so that the appropriate 
         * sublcass can handle the implementation.  Base instances should not!
         */
        
        Parcel& operator=(Parcel other);
        friend void swap(Parcel& first, Parcel& second);
    public:
    Parcel(const Parcel& other);
    /*
        These are attributes of a Parcel object, every Parcel sublcass will have these attributes.
    */
    int parcelID; /*UNIQUE identifier of a parcel*/
    
    /*
        Parcel Constructor. Most construction will be unique to the individual sublcasses,
        but global constrcution can occur here.
        Parameters:
    */

    Parcel(int parcelID_);
    /* Empty constructor */
    Parcel();

    /*
        Parcel Destructor.
        Most destrcution will be unique to the individual subclasses, but global destrcution
        can occur here if needed.
    */
    virtual ~Parcel();
    
    /*
        This function will implement the irrigation rules of Parcel instances.
        TODO: All state variables that may be needed from the Model need to be passed
              to the Diversion object managing the Parcel, and forwared onto the parcel.
        This is a PURE VIRTUAL FUNCTION, so all sublcasses must implement it
        //Cannot be pure virtual because charm cannot migrate an abstract object.
        //"error: cannot allocate an object of abstract type ‘Reservoir’"

    */
    /*
        irrigate

        Parameters: 
            amount              Amount of water this parcel is provided to use (provided as m^3/s)
                                for the duration endTime-currentTime
            diversionID         The ID of the diversion providing water for irrigation
            referenceDate       Reference date (Julian date) to define the currentTime
            currentTime         The current simulation time, seconds since referenceDate
            endTime             The end of the current time step
        
        Output Parameters:
            results             A vector of pairs, where each pair is (elementID, amount).  The amount is the amount of water
                                (m^3) to apply to elementID, valid from currentTime to endTime.
        Return:
            used                Amount of water (m^3) distributed to the parcels elements.
    */
    virtual double irrigate(double amount, const int& diversionID,
                            const double& referenceDate, const double& currentTime,
                            const double& endTime, std::vector<std::pair<int, double> >& results){return 0;}
    /*
        getArea
        
        Returns the area of the parcel.
    */
    virtual double getArea(){return 0;}

    /*
        getDecree
        Each parcel can have a decree from one or more diversions.
        This function returns the decreed amount for a given diversion.
        These amounts are flows in m^3/s.
        
        Parameters:
            id      The diversionID for the requested decree.
    */
    virtual double getDecree(int id){return 0;}
    
    /*
        getRequestedAmount
        Each parcel can request more (or less) than its decree from one or more diversions.
        This function returns the requested amount for a given diversion.
        These amounts are flows in m^3/s.
        
        Parameters:
            id      The diversionID for the desired requestedAmount.
    */

    virtual double getRequestedAmount(int id){return 0;}

    /*
        Add PUP support
    */
    
    PUPable_decl(Parcel);
    Parcel(CkMigrateMessage* m);
    virtual void pup(PUP::er &p);
};

#endif
"""

_cxx_abstract_class_def_string = \
"""
#include "Parcel.h"
#include "Parcel.decl.h"
//Parcel Constructor
Parcel::Parcel(int parcelID_):parcelID(parcelID_)
{

}

Parcel::Parcel()
{

}

Parcel::Parcel(const Parcel& other):parcelID(other.parcelID)
{

}
/*
 * Since parcelID is primitave, just swap parcelID be exception safe
 */
Parcel& Parcel::operator=(Parcel other)
{
    swap(*this, other); 
    return *this;
}

void swap(Parcel& first, Parcel& second)
{ 
    using std::swap; //Enable ADL (Arguement Dependent Lookup)
    swap(first.parcelID, second.parcelID);
}

//Parcel Destructor
Parcel::~Parcel()
{

}
/* PUP support */

Parcel::Parcel(CkMigrateMessage* m) : PUP::able(m){}

void Parcel::pup(PUP::er &p)
{
    PUP::able::pup(p);
    p|parcelID;
}
#include "Parcel.def.h"
"""

_charm_abstract_ci_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
module Parcel
{
    PUPable Parcel;

}
"""

_cxx_general_class_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef _${NAME}_
#define _${NAME}_
#include "all.h"
#include "Parcel.h"
#include "${NAME}.decl.h"
#include <iostream>

class ${NAME} : public Parcel
{
    private:
        double pivotMod;                        /*Modifier used for determing how much less water (as percent) pivot irrigation should used*/ 
        int setCounter;                         /*Counter used to determine which elements to water in current set (setCounter%elementCount) */
        int setSize;                            /*The number of elements to use as an irrigation set */
        double beginningTime;                   /*Julian date representing the starting simulation time */
        long startMonth;                        /*Month to start irrigating in (1-12) */
        long startDay;                          /*Day of the month to start irrigating in (1-31) */
        long startHour;                         /*Hour of day to stop irrigating at (0-23) */
        long stopMonth;                         /*Month to stop irrigating in (1-12) */
        long stopDay;                           /*Day of the month to stop irrigating in (1-31) */
        long stopHour;                          /*Hour of day to start irrigating at (0-23) */
        std::map<int, bool> isIrrigating;       /*Flag indicating if this parcel is irrigating from a given diversion */
        std::map<int, double> elapsedCycle;     /*Accumulates how long we have been irrigating (or not) from a given diversion */
        std::map<int, double> cycleInterval;    /*Interval at which our irrigation cycle switches */
        std::map<int, double> elapsedTime;      /*Accumulators for time since last update from each diversion. (in seconds) */
        std::map<int, double> updateInterval;   /*Interval at which irrigation decisions should be updated for each diversion. (in seconds) */
        double area;                            /*This is the area in square kilometers that this parcel covers*/
        std::map<int, double> percentToElement; /* This is a map of elementID's to the percent of available water each should be sent at any given time. */
        std::map<int, double> decreedAmount;    /* The legal decreed amount of water 
                                                    that this parcel can use for irrigation.
                                                    Since parcels can obtain water from one or
                                                    more diversion, the map indexes by Diversion ID
                                                    and returns the amount of water this diversion
                                                    is expected to supply. These amounts are flows in m^3/s.
                                                  */
        std::map<int, double> requestedAmount;    /* It may be that a parcel can get more than the "decreed" amount.
                                                      Either from simulation or from historic data, we can request up to this amount.
                                                      If the diversion can supply this much water without restricting other parcels,
                                                      then it should.  Otherwise it supplies the decreedAmount if possible.
                                                      These amounts are flows in m^3/s.
                                                    */
        static int elementCount;                    /* Keep track of the number of ADHydro elements representing the Parcel. */
        
        static int elements[${ELEMENT_COUNT}];      /* Every parcel is responsible for managing a list of ADHydro elements. */

        /*
            initCycle
            
            Helper function to initilize/reset cycle variables
        */
        void initCycle(int diversionID)
        {
            elapsedCycle[diversionID] = 0; //Initially, have not spent any time in an irrigation cycle
            isIrrigating[diversionID] = true; //We always start a cycle in irrigation phase
            switch(cropType)
            {
                case ALFALFA:
                    //Initially, start on an irrigation cycle that lasts 21 days
                    cycleInterval[diversionID] = 21*86400;
                    break;
                case GRASS:
                    //Initially, start an irrigation cycle that last 35 days
                    cycleInterval[diversionID] = 35*86400;
                    break;
                default:
                    //TODO/FIXME what should the default case be?
                    break;
            }
        }

        /*
            Helper function for switching between irrigation and drying phases
            of an irrigation cycle
        */
        void switchCycle(int diversionID)
        {
            if(isIrrigating[diversionID])
            {
                //we are in in irrigating phase of cycle, switch to drying
                elapsedCycle[diversionID] = 0;
                isIrrigating[diversionID] = false;
                switch(cropType)
                {
                    case ALFALFA:
                        //dry for 18 days
                        cycleInterval[diversionID] = 18*86400;
                        break;
                    case GRASS:
                        //dry for 18 days
                        cycleInterval[diversionID] = 18*86400;
                        break;
                    default:
                        //TODO/FIXME what should the default case be?
                        break;
                }
                //Now we don't need to update for a while
                updateInterval[diversionID] = cycleInterval[diversionID];
            }
            else
            {
                //we are in a drying phase of the cycle, switch to irrigating
                initCycle(diversionID);
                //update more often now
                updateInterval[diversionID] = 86400; //Update once a day
            }
        }

    public:
    enum CROP_TYPE {ALFALFA, GRASS, GRAIN, UNKNOWN};
    enum IRRIG_TYPE {FLOOD, PIVOT};
    CROP_TYPE cropType;
    IRRIG_TYPE irrigType;
    static const int parcelID = ${PARCEL_ID};       /* Unique Parcel Identifier */
    
    ${NAME}(double referenceDate):Parcel(this->parcelID), beginningTime(referenceDate),
                                  area(${AREA}), cropType(${CROPTYPE}), irrigType(${IRRIGTYPE}),
                                  pivotMod(1)
    {
        //Set the % modifier for pivot irrigation
        switch(cropType)
        {
            case ALFALFA:
                //Based on .90 efficiency for pivot irrigation vs .65 efficiency for flood (for alfalfa)
                pivotMod = 1-.27;
                break;
            case GRASS:
                //TODO/FIXME FIND EFFICIENCY FOR GRASS HAY, for now use a reasonable value
                pivotMod = 1-.27;
                break;
        }
        //Set the setSize and setCounter
        setSize = 1;
        setCounter = -1*setSize; //Every use of setCounter should increment by setSize first

        //Initially, set all elements to take 0 water.        
        for(int i = 0; i < elementCount; i++)
        {
            percentToElement[elements[i]] = 0.0;
        }

${DECREES}
${REQUESTS}
 
        /*
            Based on crop type, set start and end irrigation times
        */
        switch(cropType)
        {
            case ALFALFA:
                startMonth = 5;    //May
                startDay =   13;   //13
                startHour =   6;   //0600

                stopMonth = 8;     //August
                stopDay =   20;    //20
                stopHour =   6;    //0600
                break;

            case GRASS:
                startMonth = 6;  //June
                startDay = 10;   //10
                startHour = 6;   //0600    
        
                stopMonth = 7;  //July
                stopDay =   15; //20
                stopHour =   6; //0600
                break;
            default:
                //TODO/FIXME what should the default case be?
                break;
        }

        /*
            When a parcel is created, determine how long until we need to irrigate and set the
            update interval to that.  Or, if we are constructed in the middle of an irrigation season,
            figure out where in our cycle we are, and irrigate as normal.
        */

        long year, month, day, hour, minute;
        double second;
        julianToGregorian(referenceDate, &year, &month, &day, &hour, &minute, &second);
        double julianStart = gregorianToJulian(year, startMonth, startDay, startHour, 0, 0.0);
        double julianEnd = gregorianToJulian(year, stopMonth, stopDay, stopHour, 0 , 0.0);
        double initialInterval = 0;
        double initialElapsed = 0;
        //Decide if we are in the middle of an irrigation cycle
        if( referenceDate > julianStart && referenceDate < julianEnd )
        {
            //figure out where in our cycle we are and continue irrigating
            int daysIrrigating, daysDrying;
            switch(cropType)
            {
                case ALFALFA:
                    //alfalfa irrigates for 21 days, dries for 18. TODO/FIXME makes these cycles variables that can change...
                    daysIrrigating = 21;
                    daysDrying = 18;
                    
                    break;

                case GRASS:
                    //alfalfa irrigates for 35 days, dries for 18. TODO/FIXME makes these cycles variables that can change...
                    daysIrrigating = 35;
                    daysDrying = 18;
                    break;

                default:
                   //TODO/FIXME what should the default case be?
                    daysIrrigating = 35;
                    daysDrying = 18;
                    break; 
            }
            double elapsed = referenceDate - julianStart;
            double cycle_elapsed = 0;
            double cycle_interval = 0;
            bool irrig = false;
            //Variables for update date
            long uYear, uMonth, uDay, uHour, uMinute;
            double uSecond;
            long jCycle = (long) elapsed;
            double fCycle = elapsed - jCycle;
            double cycle = jCycle%(daysIrrigating+daysDrying) + fCycle;
            if(cycle <= daysIrrigating)
            {
                //We are starting in an irrigation cycle
                cycle_elapsed = cycle*86400;
                cycle_interval = daysIrrigating*86400;//Cycle changes after this much time
                irrig = true;
                initialInterval = 86400; //During irrigation, we update every day.
                //Get yesterday at 6 AM, last time a decision was made.
                julianToGregorian( referenceDate - 1, &uYear, &uMonth, &uDay, &uHour, &uMinute, &uSecond);
                double lastUpdated = gregorianToJulian(uYear, uMonth, uDay, startHour, 0, 0.0);
                initialElapsed = (referenceDate - lastUpdated)*86400;
            }
            else
            {
                //we are starting in a drying cycle
                cycle_elapsed = (cycle - daysIrrigating)*86400;
                cycle_interval = daysDrying*86400;
                irrig = false;
                initialInterval = daysDrying*86400; //During drying, we only update at the end of the drying period.
                //Get the last update time
                julianToGregorian( referenceDate - (cycle-daysIrrigating), &uYear, &uMonth, &uDay, &uHour, &uMinute, &uSecond);
                double lastUpdated = gregorianToJulian(uYear, uMonth, uDay, startHour, 0, 0.0);
                initialElapsed = (referenceDate - lastUpdated)*86400;
            }
            //Now have cycle info ready to initilize
            //Initilize cycles and intervals for in progress
            for(std::map<int, double>::iterator it = decreedAmount.begin(); it != decreedAmount.end(); ++it)
            {   //Set the elapsedTime and updateInterval for each diversion servicing this parcel
                updateInterval[it->first] = initialInterval; //Set initial update interval
                elapsedTime[it->first] = initialElapsed;  // start accumulating up to updateInterval
                elapsedCycle[it->first] = cycle_elapsed;
                cycleInterval[it->first] = cycle_interval;
            }
        }
        else
        {
            if(month <= startMonth)
            {
                //We are in the beginning of the year, calculate interval until startMonth/Day/Hour
                initialInterval = (julianStart - referenceDate)*86400;
            }
            else
            {
                //We are in the end of the year, after stopMonth/Day/Hour, so calculate interval until NEXT season
                julianStart = gregorianToJulian(year+1, startMonth, startDay, startHour, 0, 0.0);
                initialInterval = (julianStart - referenceDate)*86400;
            }
            //Initilize cycles and intervals for next irrigation season
            for(std::map<int, double>::iterator it = decreedAmount.begin(); it != decreedAmount.end(); ++it)
            {   //Set the elapsedTime and updateInterval for each diversion servicing this parcel
                updateInterval[it->first] = initialInterval; //Set initial update interval
                elapsedTime[it->first] = initialElapsed;  // start accumulating up to updateInterval

                //Initilize cycle intervals here as well
                initCycle(it->first);
                
            }
        }
        
        /*TODO/FIXME WHY DOES THIS HAVE TO BE MANUALLY REGISTERED FOR IT TO WORK????? BUG IN CHARM???*/
        _register${NAME}();
    }

    /*
     * Copy Constructor
     */
    ${NAME}(const ${NAME}& other):Parcel(other), pivotMod(other.pivotMod),
    setCounter(other.setCounter), setSize(other.setSize), beginningTime(other.beginningTime),
    startMonth(other.startMonth), startDay(other.startDay), startHour(other.startHour),
    stopMonth(other.stopMonth), stopDay(other.stopDay), stopHour(other.stopHour),
    isIrrigating(other.isIrrigating), elapsedCycle(other.elapsedCycle), cycleInterval(other.cycleInterval),
    elapsedTime(other.elapsedTime), updateInterval(other.updateInterval),
    area(other.area), percentToElement(other.percentToElement),
    decreedAmount(other.decreedAmount), requestedAmount(other.requestedAmount),
    cropType(other.cropType), irrigType(other.irrigType)
    {
        
    }
    /*
     * Swap function for Copy-Swap idiom 
     */
    friend void swap(${NAME}& first, ${NAME}& second)
    {
        using std::swap; //Enable ADL (Arguement Dependent Lookup)
        swap( (Parcel&) first, (Parcel&) second);
        swap(first.pivotMod, second.pivotMod);
        swap(first.setCounter, second.setCounter);
        swap(first.setSize, second.setSize);
        swap(first.beginningTime, second.beginningTime);
        swap(first.startMonth, second.startMonth);
        swap(first.startDay, second.startDay);
        swap(first.startHour, second.startHour);
        swap(first.stopMonth, second.stopMonth);
        swap(first.stopDay, second.stopMonth);
        swap(first.stopHour, second.stopHour);
        swap(first.isIrrigating, second.isIrrigating);
        swap(first.elapsedCycle, second.elapsedCycle);
        swap(first.cycleInterval, second.cycleInterval);
        swap(first.elapsedTime, second.elapsedTime);
        swap(first.updateInterval, second.updateInterval);
        swap(first.area, second.area);
        swap(first.percentToElement, second.percentToElement);
        swap(first.decreedAmount, second.decreedAmount);
        swap(first.requestedAmount, second.requestedAmount);
        swap(first.cropType, second.cropType);
        swap(first.irrigType, second.irrigType);
    }
    /*
     * Assignment Operator
     */
    ${NAME}& operator=(${NAME} other)
    {
        swap(*this, other);
        return *this;
    }

    ~${NAME}()
    {
        
    }


    /*
        update
        
        This function is responsible for updating the parcel's parameters for the next updateInterval time.
        
        paramters:
            diversionID     The diversion whose irrigate call initiaited the update
            referenceDate       Reference date (Julian date) to define the currentTime
            currentTime         The current simulation time, seconds since referenceDate
    */
    void update(const int& diversionID, const double& referenceDate, const double& currentTime)
    { 
        long year, month, day, hour, minute;
        double second;
        julianToGregorian(referenceDate+currentTime, &year, &month, &day, &hour, &minute, &second);
        //Check to see if we are done for the year
        if(month == stopMonth && day == stopDay && hour >= stopHour)
        {
            //We are done irrigating for this year.  Calculate the updateInterval till the next year
            //and set our water use to 0.
            double nextStart = gregorianToJulian(year+1, startMonth, startDay, startHour, 0, 0.0);
            //Set updateInterval: seconds until next year's irrigation start
            updateInterval[diversionID] = (nextStart - referenceDate)*86400+currentTime;
            for(int i = 0; i < elementCount; i++)
            {
                percentToElement[elements[i]] = 0.0;
            }
            //also need to reset our cycle structures
            initCycle(diversionID);
        }
        else
        {
            //Reset elapsedCycle since we have been updated
            elapsedCycle[diversionID] += updateInterval[diversionID];
            //If we have finished our current cycle, then switch
            if(elapsedCycle[diversionID] >= cycleInterval[diversionID])
            {
                switchCycle(diversionID);
            }
            //If we are in an irrigating phase, then we need to decide how much
            //water we want to send to each of our elements
            if(isIrrigating[diversionID])
            {
                switch(irrigType)
                {
                    case FLOOD:
                        //Rotate water over all elements in a "set" style
                        setCounter += setSize; //Increment the set counter
                        for(int i = 0; i < elementCount; i++)
                        {
                            percentToElement[elements[i]] = 0;
                        }
                        for(int i = setCounter%elementCount; i < (setCounter+setSize)%elementCount; i++)
                        {
                            percentToElement[elements[i]] = 1.0/setSize;
                        }
                        break;
                    case PIVOT:
                        //Evenly apply all water to all elements.
                        //TODO/FIXME To simulate applying less water evenly,
                        //I believe we can simply mod the percentToElement for 
                        //each one, since the irrigate function will take this
                        //percent of available water and send to each element
                        //for the next updateInterval time
                        //TODO/FIXME put this in constructor and make pivotMod a private var
                        //Now set each parcels allocation percent
                        for(int i = 0; i < elementCount; i++)
                        {
                            percentToElement[elements[i]] = 1.0/elementCount*pivotMod;
                        }
                        break;
                    default:
                        //TODO/FIXME what to do for default?
                        break;
                }
            }
            
        }
        
    }

    /*
    irrigate

        Parameters: 
            amount              Amount (maximum) of water this parcel is provided to use (provided as m^3)
                                for the duration endTime-currentTime.
            diversionID         The ID of the diversion providing water for irrigation
            referenceDate       Reference date (Julian date) to define the currentTime
            currentTime         The current simulation time, seconds since referenceDate
            endTime             The end of the current time step
        
        Output Parameters:
            results             A vector of pairs, where each pair is (elementID, amount).  The amount is the amount of water
                                (m^3) to apply to elementID, valid from currentTime to endTime.
        Return:
            used                Amount of water (m^3) distributed to the parcels elements.
    */

    virtual double irrigate(double amount, const int& diversionID,
                            const double& referenceDate, const double& currentTime,
                            const double& endTime, std::vector<std::pair<int, double> >& results)
    {
        //Find out our request (m^3) for this time step
        double request = requestedAmount[diversionID]/(endTime - currentTime);
        if(elapsedTime[diversionID] >= updateInterval[diversionID])
        {
            //Update parameters (i.e. flow to each element for the next updateInterval time
            update(diversionID, referenceDate, currentTime);
            //Reset elapsed time
            elapsedTime[diversionID] = 0.0;
        }
        //Update elapsed time
        elapsedTime[diversionID] += endTime - currentTime;
        //Every parcel needs to send a message to all its elements
        double use = 0.0;
        double used = 0.0;
        if(amount >= request)
        {
            //If we have our requestedAmount of water available, use it
            use = request;
        }
        else
        {
            //If we dont have as much as we want, we can only use what we are given
            use = amount;

            if(amount < decreedAmount[diversionID]*(endTime-currentTime))
            {
                //water is in short supply, can only use the amount given
                //but we need to initiaite a call for more water since we are < decreedAmount
                //init_water_call();
            }
        }
        for(int i = 0; i < elementCount; i++)
        {
        //    sendMessage(elements[i], use*percentToElement[elements[i]]);
              used += use*percentToElement[elements[i]];
              results.push_back(std::make_pair(elements[i], use*percentToElement[elements[i]]));
        }
        //Return the amount of water we used.
        return used;
    }

    double getArea()
    {
        return area;
    }
    double getDecree(int id)
    {
        return decreedAmount[id];
    }
    double getRequestedAmount(int id)
    {
        return requestedAmount[id];
    }
    /*
        PUP support
    */
    PUPable_decl(${NAME});
    ${NAME}(CkMigrateMessage* m) : Parcel(m) {}
    virtual void pup(PUP::er &p)
    {
        Parcel::pup(p);
        p|setCounter;
        p|setSize;
        p|beginningTime;
        p|startMonth;
        p|startDay;
        p|startHour;
        p|stopMonth;
        p|stopDay;
        p|stopHour;
        p|isIrrigating;
        p|elapsedCycle;
        p|cycleInterval;
        p|elapsedTime;
        p|updateInterval;
        p|area;
        p|decreedAmount;
        p|requestedAmount;
        p|elementCount;
        p(elements, elementCount);
        pup_bytes(&p, (void*)& cropType, sizeof(CROP_TYPE));
        pup_bytes(&p, (void*)& irrigType, sizeof(IRRIG_TYPE));
    }
    
};
int ${NAME}::elementCount = ${ELEMENT_COUNT};
int ${NAME}::elements[${ELEMENT_COUNT}] = {${ELEMENT_IDS}};
#include "${NAME}.def.h"
#endif
"""



from string import Template

_charm_ci_string=\
"""
module ${NAME}
{
    PUPable ${NAME};
}\n
"""

class ParcelCI():
    
    def __init__(self, name):
        self.template = Template(_charm_ci_string)
        self.template = Template(self.template.safe_substitute(NAME=name))

    def __str__(self):
        return self.template.safe_substitute()

class Parcel():

    def __init__(self):
        self.template = Template(_cxx_general_class_string)

    def __str__(self):
        #Use safe_substitute to avoid modifying the template
        return self.template.safe_substitute()

    def name(self, n):
        #Safe_substitute only replaces the keys it is passed
        #in this case NAME, so any other keys will remain intact
        self.template = Template(self.template.safe_substitute(NAME=n))

    def parcelID(self, i):
        self.template = Template(self.template.safe_substitute(PARCEL_ID=i))

    def decreedAmount(self, amounts):
        #Amounts should be a map, mapping DiversionID to the Amount that diversion can give the parcel
        decrees = ''
        for k,v in amounts.iteritems():
            decrees = decrees + '        decreedAmount['+k+'] = '+v+';\n'
        self.template = Template(self.template.safe_substitute(DECREES=decrees))
    
    def requestAmount(self, amounts):
        #Amounts should be a map, mapping DiversionID to the Amount that the parcel wants from the diversion
        requests = ''
        for k,v in amounts.iteritems():
            requests = requests + '        requestedAmount['+k+'] = '+v+';\n'
        self.template = Template(self.template.safe_substitute(REQUESTS=requests))

    def area(self, a):
        
        self.template = Template(self.template.safe_substitute(AREA=a))
        
    def elementIds(self, elements):
        #Make some assumption about how element ids are passed
        #Assume a list
        count = len(elements)
        if count < 1:
            raise(Exception("elementIds exception: must have 1 or more elementIDs"))
        idString = ','.join(elements)
        self.template = Template(self.template.safe_substitute(ELEMENT_IDS=idString))
        self.template = Template(self.template.safe_substitute(ELEMENT_COUNT=count))

    def cropType(self, t):
        self.template = Template(self.template.safe_substitute(CROPTYPE=t))
    def irrigType(self, t):
        self.template = Template(self.template.safe_substitute(IRRIGTYPE=t))


def abstract():
    return {"Parcel.h":_cxx_abstract_class_header_string, "Parcel.cpp":_cxx_abstract_class_def_string, "Parcel.ci":_charm_abstract_ci_string}
