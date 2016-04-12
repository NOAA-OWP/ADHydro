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
        This function will implement the managment rules of Diversion instances.
        TODO: All state variables that may be needed from the Model need to be passed as parameters
              to this function.
        This is a PURE VIRTUAL FUNCTION, so all sublcasses must implement it
    */
    //Cannot be pure virtual because charm cannot migrate an abstract object.
    //"error: cannot allocate an object of abstract type ‘Reservoir’"
    /*
        FIXME Can it be the case that a Diversion cannot use all the water it requested?
        If so, should we return that amount back to ADHydro?
    */
    virtual double irrigate(double amount_diverted){return amount_diverted;}
    /*Nessicary?  Or just have channel element read Diversion.requestedAmount ? */
    virtual double getRequestedDiversionAmount(){return 0;};

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
        double decreedAmount; /*The decreed amount of water this diversion is allowed to divert.*/
        double requestedAmount; /*The total amount of water a diversion is requesting from ADHydro. (May be > decreedAmount)*/
        std::vector<Parcel*> parcels; /* Map of parcel identifiers to Parcel objects*/
        //Appropriation date of the Diversion, year=yyyy, month=mm, day=dd
        double appropriationYear;
        double appropriationMonth;
        double appropriationDay;
        double totalLandSize;   /*Total land area this diversion serves. */

    public:
    static const int elementID = ${ELEMENT_ID};
    
    ${NAME}():Diversion(this->elementID)
    {
        //total decreed amount per diversion (cfs)
        decreedAmount = ${DECREED_AMOUNT};
        //total requested amount per diversion (cfs)
        requestedAmount = ${REQUESTED_AMOUNT};
        //appropriation date: format=yyyymmdd
        appropriationYear = ${APPROPRIATION_YEAR};
        appropriationMonth = ${APPROPRIATION_MONTH};
        appropriationDay = ${APPROPRIATION_DAY};
        //number of parcels
        parcelCount = ${PARCEL_COUNT};
        int parcel_ids[${PARCEL_COUNT}] = {${PARCEL_IDS}};
        

        ParcelFactory* pf = new ParcelFactory();
        for(int i =0; i < parcelCount; i++)
        {
            parcels.push_back(pf->create(parcel_ids[i]));
        }
        delete pf;

        totalLandSize = 0; //acres
        for(int i = 0; i < parcels.size(); i++) totalLandSize += parcels[i]->getArea();
        /*TODO/FIXME WHY DOES THIS HAVE TO BE MANUALLY REGISTERED FOR IT TO WORK????? BUG IN CHARM???*/
        _register${NAME}();
    }

    /*
     * Copy Constructor
     */
    ${NAME}(const ${NAME}& other):Diversion(other),
    decreedAmount(other.decreedAmount), requestedAmount(other.requestedAmount),
    appropriationYear(other.appropriationYear), appropriationMonth(other.appropriationMonth),
    appropriationDay(other.appropriationDay), parcelCount(other.parcelCount), totalLandSize(totalLandSize)
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

    double irrigate(double amount_diverted)
    {
        double not_used = 0.0;
        std::cout<<"Diversion Irrigating with "<<amount_diverted<<"\\n";

        for(int i = 0; i < parcels.size(); i++)
        {
            //Irrigate each parcel
        	//TODO/FIXME 3.14.2016 Zen. Currently, water allocation is equally splitted. But, parcels can get different diversions.
        	//not_used += parcels[i]->irrigate(amount_diverted/parcelCount);

        	//Zen. water allocation is splitted by the propotion of land sizes
        	not_used += parcels[i]->irrigate(amount_diverted*(parcels[i]->getArea()/totalLandSize));

       	    //std::cout<<"unused"<<i<<"\t"<<not_used<<".\\n";
        }
   	    //std::cout<<"unused_end\t"<<not_used<<".\\n";
        return not_used; //Return the amount of unused water
    }

    double getRequestedDiversionAmount()
    {
        std::cout<<"In getRequestedAmount ${NAME}\\n";
        return requestedAmount;
        /*
        double total = 0;
        for(int i = 0; i < parcels.size(); i++)
        {
            total += parcels[i].decreedAmount[elementID];
        }
        */
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


def abstract():
    return {"Diversion.h":_cxx_abstract_class_header_string, "Diversion.cpp":_cxx_abstract_class_def_string, "Diversion.ci":_charm_abstract_ci_string}
