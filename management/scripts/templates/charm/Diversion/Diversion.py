#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""

This is a template class for automatically generating 
Diversion component classes for ADHydro.

"""
_cxx_abstract_class_header_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef DIVERSION_H
#define DIVERSION_H
/*
This is an Abstract class representing Diversion points.
Diversion points manage a set of Parcels and
can "irrigate" each of these with the water 
provided to the Diversion.
*/
#include "pup.h"
#include "pup_stl.h"

class Diversion : public PUP::able
{
    protected:
        /*
         * Really don't want the abstract class copied, this will likely lead to slicing.
         * Putting empty copy constructor and assignment in protected will cause compiler to
         * error if it is attempted by anything except subclasses or the object itself.
         * Subclasses should be able to call the copy constructor so that the elementID gets
         * properly initalized in the copy.
         * Only pointers of abstract class should be copied/assigned so that the appropriate 
         * sublcass can handle the implementation.  Base instances should not!
         */
        Diversion(const Diversion& other);
        Diversion& operator=(Diversion other);
        friend void swap(Diversion& first, Diversion& second);

    public:
    /*
        These are attributes of a Diversion object.  Every diversion will have these.
    */
    int elementID; /* The ADHydro identifier for the channel element this diversion corresponds to. */
    
    /*
        Diversion Constructor. Most construction will be unique to the individual sublcasses,
        but global constrcution can occur here.
        Parameters:
        identifier_  the ADHydro identifier for the channel element this diversion corresponds to.
    */
    
    Diversion(int elementID_);
    /* Empty constructor */
    Diversion(){};

    /*
        Diversion Destructor.
        Most destrcution will be unique to the individual subclasses, but global destrcution
        can occur here if needed.
    */

    virtual ~Diversion();

    /*
        divert

        This function will implement the managment rules of Diversion instances.
        TODO: All state variables that may be needed from the Model need to be passed as parameters
              to this function.
        NOTE:
        Ideally should be a PURE VIRTUAL FUNCTION, so all sublcasses must implement it.
        Cannot be pure virtual because charm cannot migrate an abstract object because it cannot be instanciated.
        "error: cannot allocate an object of abstract type ‘Diversion’"
        
        Input Parameters:
            available           Amount of water available to the Diversion for use
            referenceDate       Reference date to define the currentTime
            currentTime         The current simulation time
            endTime             The end of the current time step
        
        Output Parameters:    
            available           Amount of water left at this diversion which is available to the calling element
                                after all diversion rules have been applied.
    */
    virtual void divert(double* available, const double& referenceDate, const double& currentTime, const double& endTime){}


    /*
        Add PUP support
    */
    
    PUPable_decl(Diversion);
    Diversion(CkMigrateMessage* m);
    virtual void pup(PUP::er &p);

};

#endif
"""

_cxx_abstract_class_def_string = \
"""
#include "Diversion.h"
#include "Diversion.decl.h"
//Diversion Constructor, sets identity
Diversion::Diversion(int elementID_):elementID(elementID_)
{

}

Diversion::Diversion(const Diversion& other):elementID(other.elementID)
{

}
/*
 * Since elementID is primitave, just swap elementID be exception safe
 */
Diversion& Diversion::operator=(Diversion other)
{
    swap(*this, other); 
    return *this;
}

void swap(Diversion& first, Diversion& second)
{ 
    using std::swap; //Enable ADL (Arguement Dependent Lookup)
    swap(first.elementID, second.elementID);
}

//Diversion Destructor
Diversion::~Diversion()
{

}
/* PUP support */
Diversion::Diversion(CkMigrateMessage* m) : PUP::able(m){}

void Diversion::pup(PUP::er &p)
{
    if(p.isUnpacking()) CkPrintf("Unpacking Diversion\\n");
    else CkPrintf("Packing Diversion\\n");

    PUP::able::pup(p);
    p|elementID;

    if(p.isUnpacking()) CkPrintf("Unpacked %ld\\n", elementID);
}
#include "Diversion.def.h"
"""

_charm_abstract_ci_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
module Diversion
{
    PUPable Diversion;

}
"""

_cxx_general_class_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef _${NAME}_
#define _${NAME}_
#include "Diversion.h"
#include "common_utilities.h"
#include "${NAME}.decl.h"
#include <iostream>
#include "Parcel.h"
#include "ParcelFactory.h"

class ${NAME} : public Diversion
{
    private:
        int parcelCount;
        double decreedAmount; /*The decreed amount of water this diversion is allowed to divert.
                                This should be the sum of decrees for all objects this diversion 
                                can send water to. These amounts are flows in m^3/s.
                              */
        double requestedAmount; /*The total amount of water a diversion would like to have
                                  in order to satisfy the demands of all objects it provides water to.
                                  This should be the sum of the requestedAmounts of all these objects,
                                  and may change over time. These amounts are flows in m^3/s.
                                */
        std::vector<Parcel*> parcels; /*The parcels that this diversion supplies water to for irrigation*/
        //Appropriation date of the Diversion, year=yyyy, month=mm, day=dd
        double appropriationYear;
        double appropriationMonth;
        double appropriationDay;
        double totalLandSize;   /*Total land area this diversion serves. */
        std::vector<int> upstreamDiversions; /*The elementID's of upstream diversions with newer appropriation dates.  These should be ordered by date, youngest to oldest.*/
        int diversionID; /*This is a unique identifier for this diversion that doesn't nessicarliy
                           correspond with the elementID that the model associates with the diverion.
                           This is used to link parcels and diversions using identities not assoiciated with the calling model.*/

    public:
    static const int elementID = ${ELEMENT_ID};
    
    ${NAME}():Diversion(this->elementID)
    {
        diversionID = ${DIVERSION_ID};
        //total decreed amount per diversion (cfs)
        decreedAmount = 0.0;
        //total requested amount per diversion (cfs)
        requestedAmount = 0.0;
        //appropriation date: format=yyyymmdd
        appropriationYear = ${APPROPRIATION_YEAR};
        appropriationMonth = ${APPROPRIATION_MONTH};
        appropriationDay = ${APPROPRIATION_DAY};
        //number of parcels
        parcelCount = ${PARCEL_COUNT};
        int parcel_ids[${PARCEL_COUNT}] = {${PARCEL_IDS}};
        
        #Create all parcels, and accumulate the initial requestedAmount and decreedAmount
        ParcelFactory* pf = new ParcelFactory();
        for(int i=0; i < parcelCount; i++)
        {
            parcels.push_back(pf->create(parcel_ids[i]));
            requestedAmount += parcels[i]->getRequestedAmount(diversionID);
            decreedAmount += parcels[i]->getDecree(diversionID);
        }
        delete pf;

        totalLandSize = 0; //acres
        for(int i = 0; i < parcels.size(); i++) totalLandSize += parcels[i]->getArea();
        int upstream[${UPSTREAM_COUNT}] = {${UPSTREAM_IDS}};
        for(int i=0; i < ${UPSTREAM_COUNT}; i++)
        {
            upstreamDiversions.push_back(upstream[i]);
        }
        /*TODO/FIXME WHY DOES THIS HAVE TO BE MANUALLY REGISTERED FOR IT TO WORK????? BUG IN CHARM???*/
        _register${NAME}();
    }

    /*
     * Copy Constructor
     */
    ${NAME}(const ${NAME}& other):Diversion(other),
    decreedAmount(other.decreedAmount), requestedAmount(other.requestedAmount),
    appropriationYear(other.appropriationYear), appropriationMonth(other.appropriationMonth),
    appropriationDay(other.appropriationDay), parcelCount(other.parcelCount), totalLandSize(totalLandSize),
    upstreamDiversions(other.upstreamDiversions), diversionID(other.diversionID)
    {
        /*Cheat and re create new parcel pointers from factory...*/
        //TODO IF we ever have dynamically changing parcels, probably don't want to treat ID's this way...        
        int parcel_ids[${PARCEL_COUNT}] = {${PARCEL_IDS}};
        ParcelFactory* pf = new ParcelFactory();
        for(int i =0; i < parcelCount; i++)
        {
            parcels.push_back(pf->create(parcel_ids[i]));
        }
        delete pf;
    }
    /*
     * Swap function for Copy-Swap idiom 
     */
    friend void swap(${NAME}& first, ${NAME}& second)
    {
        using std::swap; //Enable ADL (Arguement Dependent Lookup)
        swap( (Diversion&) first, (Diversion&) second);
        swap(first.parcelCount, second.parcelCount);
        swap(first.decreedAmount, second.decreedAmount);
        swap(first.requestedAmount, second.requestedAmount);
        swap(first.parcels, second.parcels);
        swap(first.appropriationYear, second.appropriationYear);
        swap(first.appropriationMonth, second.appropriationMonth);
        swap(first.appropriationDay, second.appropriationDay);
        swap(first.totalLandSize, second.totalLandSize);
        swap(first.upstreamDiversions, second.upstreamDiversions);
        swap(first.diversionID, second.diversionID);
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
    	//Clean up all of the allocated Parcels
	    for(int i = 0; i < parcels.size(); i++)
	    {
	    	delete parcels[i];
	    }
    }

    /*
        divert

        This function will implement the managment rules of Diversion instances.
        It is called at every time step, and is responsible for calling proper functions
        on each object that recieves water from this diversion point.
        TODO: All state variables that may be needed from the Model need to be passed as parameters
              to this function.
        
        Input Parameters:
            available           Amount of water available to the Diversion for use (m^3/s), valid for endTime-currentTime
            referenceDate       Reference date to define the currentTime
            currentTime         The current simulation time
            endTime             The end of the current time step
        
        Output Parameters:    
            available           Amount of water left at this diversion which is available to the calling element
                                after all diversion rules have been applied.

    */

    virtual void divert(double* available, const double& referenceDate, const double& currentTime, const double& endTime)
    {
        /*Handle irrigation of land Parcels
          For each parcel, figure out the percentage of the total request it accounts for,
          Then send it that percentage of the total available water.  The parcel can then use up to
          that much, but can use less and return the unused amount.  Gauranteed not to use more than available.
          This way, when water is plentiful, each parcel will get at least what it wanted.  When it is not,
          then each parcel gets an amount of water proportional to its requested amount.  If the requested
          amount changes, this proportion changes, allowing a dynamic way of using more or less water on parcels
          by decreasing use on one to allow more on another.
        */
        for(int i = 0; i < parcels.size(); i++)
        {
            //Irrigate each parcel
        	*available -= parcels[i]->irrigate(available*(parcels[i]->getRequestedAmount(diversionID)/requestedAmount), diversionID, referenceDate, currentTime, endTime);
            
        }
    }

    /*
        PUP support
    */

    PUPable_decl(${NAME});
    ${NAME}(CkMigrateMessage* m) : Diversion(m) {}
    virtual void pup(PUP::er &p)
    {
        Diversion::pup(p);
        if(p.isUnpacking()) CkPrintf("Unpacking ${NAME}\\n");
        else CkPrintf("Packing ${NAME}\\n");
        p|parcelCount;
        p|decreedAmount;
        p|requestedAmount;
        p|parcels;
        p|appropriationYear;
        p|appropriationMonth;
        p|appropriationDay;
        p|totalLandSize;
        p|upstreamDiversions;
        if(p.isUnpacking())
        {
            CkPrintf("Unpacked ${NAME}\\n");
        }
    }
};
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

class DiversionCI():
    
    def __init__(self, name):
        self.template = Template(_charm_ci_string)
        self.template = Template(self.template.safe_substitute(NAME=name))

    def __str__(self):
        return self.template.safe_substitute()

class Diversion():

    def __init__(self):
        self.template = Template(_cxx_general_class_string)

    def __str__(self):
        #Use safe_substitute to avoid modifying the template
        return self.template.safe_substitute()

    def name(self, n):
        #Safe_substitute only replaces the keys it is passed
        #in this case NAME, so any other keys will remain intact
        self.template = Template(self.template.safe_substitute(NAME=n))

    def elementID(self, i):
        self.template = Template(self.template.safe_substitute(ELEMENT_ID=i))

    def decreedAmount(self, amount):
        self.template = Template(self.template.safe_substitute(DECREED_AMOUNT=amount))
    
    def requestedAmount(self, amount):
        self.template = Template(self.template.safe_substitute(REQUESTED_AMOUNT=amount))
        
    def appropriationDate(self, year, month, day):
        self.template = Template(self.template.safe_substitute(APPROPRIATION_YEAR=year, APPROPRIATION_MONTH=month, APPROPRIATION_DAY=day))

    def parcelIds(self, parcels):
        #Make some assumption about how parcel ids are passed
        #Assume a list
        count = len(parcels)
        if count < 1:
            raise(Exception("parcelIds exception: must have 1 or more parcelIDs"))
        idString = ','.join(parcels)
        self.template = Template(self.template.safe_substitute(PARCEL_IDS=idString))
        self.template = Template(self.template.safe_substitute(PARCEL_COUNT=count))

    def upstreamIds(self, diversions):
        #Make some assumption about how parcel ids are passed
        #Assume a list
        count = len(diversions)
        if count < 0:
            raise(Exception("upstreamIds exception: must have 0 or more elementIDs"))
        idString = ','.join(diversions)
        self.template = Template(self.template.safe_substitute(UPSTREAM_IDS=idString))
        self.template = Template(self.template.safe_substitute(UPSTREAM_COUNT=count))    

def abstract():
    return {"Diversion.h":_cxx_abstract_class_header_string, "Diversion.cpp":_cxx_abstract_class_def_string, "Diversion.ci":_charm_abstract_ci_string}
