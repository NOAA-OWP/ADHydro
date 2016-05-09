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

        Return:
            used                Amount of water distributed to the parcels elements.
    */
    virtual double irrigate(double amount, const int& diversionID,
                            const double& referenceDate, const double& currentTime,
                            const double& endTime){return 0;}
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
    if(p.isUnpacking()) CkPrintf("Unpacking Parcel\\n");
    else CkPrintf("Packing Parcel\\n");

    PUP::able::pup(p);
    p|parcelID;
    if(p.isUnpacking()) CkPrintf("Unpacked %ld\\n", parcelID);
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
#include "Parcel.h"
#include "${NAME}.decl.h"
#include <iostream>

class ${NAME} : public Parcel
{
    private:
        double elapsedTime;                     /*Accumulator for time since last update. (in seconds)*/
        double updateInterval;                  /*Interval at which irrigation decisions should be updated. (in seconds)*/
        double area;                            /*This is the area in square kilometers that this parcel covers*/
        std::map<int, double> percentToElement; /* This is a map of elementID's to the percent of available water each should be sent at any given time.*/
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
    public:
    
    static const int parcelID = ${PARCEL_ID};       /* Unique Parcel Identifier */
    
    ${NAME}():Parcel(this->parcelID)
    {
        elapsedTime = 86400;    //Initilize same as updateInterval so initial irrigate call calculate new rates.
        updateInterval = 86400; //Set update interval to 24 horus
        //Initially, set all elements to take 0 water.        
        for(int i = 0; i < elementCount; i++)
        {
            percentToElement[elements[i]] = 0.0;
        }
        area = ${AREA};
${DECREES}
${REQUESTS}
 
        /*TODO/FIXME WHY DOES THIS HAVE TO BE MANUALLY REGISTERED FOR IT TO WORK????? BUG IN CHARM???*/
        _register${NAME}();
    }

    /*
     * Copy Constructor
     */
    ${NAME}(const ${NAME}& other):Parcel(other),
    elapsedTime(other.elapsedTime), updateInterval(other.updateInteraval),
    area(other.area), percentToElement(other.percentToElement),
    decreedAmount(other.decreedAmount), requestedAmount(other.requestedAmount)
    {
        
    }
    /*
     * Swap function for Copy-Swap idiom 
     */
    friend void swap(${NAME}& first, ${NAME}& second)
    {
        using std::swap; //Enable ADL (Arguement Dependent Lookup)
        swap( (Parcel&) first, (Parcel&) second);
        swap(first.elapsedTime, second.elapsedTime);
        swap(first.updateInterval, second.updateInterval);
        swap(first.area, second.area);
        swap(first.percentToElement, second.percentToElement);
        swap(first.decreedAmount, second.decreedAmount);
        swap(first.requestedAmount, second.requestedAmount);
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

    void

    irrigate

        Parameters: 
            amount              Amount (maximum) of water this parcel is provided to use (provided as m^3/s)
                                for the duration endTime-currentTime.
            diversionID         The ID of the diversion providing water for irrigation
            referenceDate       Reference date (Julian date) to define the currentTime
            currentTime         The current simulation time, seconds since referenceDate
            endTime             The end of the current time step

        Return:
            used                Amount of water distributed to the parcels elements.
    */
    virtual double irrigate(double amount, const int& diversionID,
                            const double& referenceDate, const double& currentTime,
                            const double& endTime)
    {
        if(elapsedTime >= updateInterval)
        {
            //Update parameters (i.e. flow to each element for the next updateInterval time
            update();
        }
     
        //TODO/FIXME Talk to Bob...does every parcel element need to send a message?  I.E. if it gets 0 water, do we need to send 0?
        //double using = 0.0;
        //for(int i = 0; i < elementCount; i++)
        //{
        //    using += elements[
        //    sendMessage(elements[i], amount*percentToElement[elements[i]]);
        //}













        double use = 0.5;
        //Zen.3.16.2016 Keep the information on the available water. This variable is used when water is in short.
        double available = amount;

        std::cout << "${NAME} is irrigating with "<<amount<<" water.\\n";
        for(int i = 0; i < elementCount; i++)
        {
            std::cout<<"${NAME} sending "<<use/elementCount<<" water to element "<<elements[i]<<".\\n";
            amount -= use/elementCount;
        }
        //Zen. 3.17.2016 Check not_used is not negative
        if(amount >=0){
            return amount;
        }
        //Zen. 3.17.2016 if available water is not enough, then recaculation the amount of water to allocation.
        else {
        	std::cout<<"Water is in short. Recalculate the amount of allocation.\\n";
        	amount = available;

        	for(int i = 0; i < elementCount; i++)
        	        {
        	            std::cout<<"${NAME} sending "<<available/elementCount<<" water to element "<<elements[i]<<".\\n";
        	            //Zen. 3.17.2016. if available water is not enough, then update the use by calculating the equal split of allocation.
        	            amount -= available/elementCount;
        	        }
        	 //std::cout<<"amount\t"<<amount<<".\\n";
        	 return amount;
        }

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
        if(p.isUnpacking()) CkPrintf("Unpacking ${NAME}\\n");
        else CkPrintf("Packing parcel1\\n");
        p|area;
        p|decreedAmount;
        p|requestedAmount;
        p|elementCount;
        p(elements, elementCount);
        if(p.isUnpacking())
        {
            CkPrintf("Unpacked ${NAME}\\n");
        }
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


def abstract():
    return {"Parcel.h":_cxx_abstract_class_header_string, "Parcel.cpp":_cxx_abstract_class_def_string, "Parcel.ci":_charm_abstract_ci_string}
