#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""

This is a template class for automatically generating 
Reservoir component classes for ADHydro.

"""
_cxx_abstract_class_header_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef RES_H
#define RES_H

/*
    Base class for reservoir managment components
*/
#include "pup.h"
#include <algorithm>
class Reservoir : public PUP::able
{
    protected:
        /*
         * Really don't want the abstract class copied, this will likely lead to slicing.
         * Putting empty copy constructor and assignment in protected will cause compiler to
         * error if it is attempted by anything except subclasses or the object itself.
         * Subclasses should be able to call the copy constructor so that the ReachCode gets
         * properly initalized in the copy.
         * Only pointers of abstract class should be copied/assigned so that the appropriate 
         * sublcass can handle the implementation.  Base instances should not!
         */
        Reservoir(const Reservoir& other);
        Reservoir& operator=(Reservoir other);
        friend void swap(Reservoir& first, Reservoir& second);
        
    public:
	/* ReachCode identifying the NHD waterbody corresponding to this reservoir */
	long long reachCode;

    /*
        Reservoir constructor
        Most construction will be unique to the individual sublcasses, but global constrcution
        can occur here.
        Parameters:
        reachCode_  the NHD reachCode identifier for the water body
    */
    Reservoir(long long reachCode_);
    Reservoir(){};

    /*
        Reservoir destrcutor
        Most destrcution will be unique to the individual subclasses, but global destrcution
        can occur here if needed.
    */
    virtual ~Reservoir();

    /*
        This function will implement the managment rules of Reservoir instances.
        TODO: All state variables that may be needed from the Model need to be passed as parameters
              to this function
        This is a PURE VIRTUAL FUNCTION, so all sublcasses must implement it
        NOTE:
        virtual double release() = 0;
        Cannot be pure virtual because charm cannot migrate an abstract object.
        "error: cannot allocate an object of abstract type ‘Reservoir’"
        This is because a pure abstract object cannot be instanciated in c++.

        Parameters:
            curr_inflow     The current, instantaneous flow into the reservoir in cubic meters per second
            curr_volume     The current, instantaneous volume of the reservoir in cubic meters
            referenceDate   A julian reference data used to define currentTime
            currentTime     Seconds since referenceDate

        Return Parameters:
            
            rate            The rate at which water should be "released" from the reservoir, in cubic meters per second.
            duration  Seconds since referenceDate; The amount of time the returned rate is valid for.
            
                  TODO: Should the abstract return rate be 0?  I think it would be better to release the curr_inflow?
                  We shouldn't ever really have to worry about it, but since the function cannot be pure virtual,
                  we cannot guarantee every subclass implements it, so a default return is a good idea.
                  Similarly, should duration be set to really large value in these cases?
    */
    virtual double release(double curr_inflow, double curr_volume,
                           double referenceDate, long currentTime,
                           double& rate, long& duration){rate=0;duration=86400;};

    /*
        Add PUP support
    */
    PUPable_decl(Reservoir);
    Reservoir(CkMigrateMessage* m);
    virtual void pup(PUP::er &p);

};

#endif
"""

_cxx_abstract_class_def_string = \
"""
#include "Reservoir.h"
#include "Reservoir.decl.h"
//Reservoir Constructor, sets reachCode on initialization
Reservoir::Reservoir(long long reachCode_):reachCode(reachCode_)
{
    //Empty
	/*
	 * This is where connections to a database could be established and data
	 * populated for the reservoirs.
	 * In this object oriented model, perhaps a database loader object???
	 *
	 * This could also be pushed to the subclasses to call and load data.
	 * Best to load whatever is standard across all reservoirs here, though.
	 */
}

Reservoir::Reservoir(const Reservoir& other):reachCode(other.reachCode)
{

}
/*
 * Since reachCode is primitave, just swap reachCodes, should be exception safe
 */
Reservoir& Reservoir::operator=(Reservoir other)
{
    swap(*this, other); return *this;
}

void swap(Reservoir& first, Reservoir& second)
{ 
    using std::swap; //Enable ADL (Arguement Dependent Lookup)
    swap(first.reachCode, second.reachCode);
}

//Reservoir Destructor
Reservoir::~Reservoir()
{

}


Reservoir::Reservoir(CkMigrateMessage* m) : PUP::able(m){}

void Reservoir::pup(PUP::er &p)
{
    if(p.isUnpacking()) CkPrintf("Unpacking reservoir\\n");
    else CkPrintf("Packing reservoir\\n");
    PUP::able::pup(p);
    p|reachCode;
    if(p.isUnpacking()) CkPrintf("Unpacked %ld\\n",reachCode);
}

#include "Reservoir.def.h"
"""
_charm_abstract_ci_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
module Reservoir
{
    PUPable Reservoir;

}
"""
_charm_ci_string= \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
module Reservoir
{
    PUPable Reservoir;

}
"""

_cxx_general_class_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef _${NAME}_
#define _${NAME}_
#include "Reservoir.h"
#include "common_utilities.h"
// TODO/FIXME might want to remove the subregion directory structure
// That way a reservoir could simple sublcass one SUBREGION and the
// correct substitution occurs
#include "${REGION}/${SUBREGION}.h"
#include <iostream>
#include "${NAME}.decl.h"
//Any Default comments we want to add TODO/FIXME

class ${NAME} : public Reservoir, ${SUBREGION}
{
private:
		double min_release;	//minimum release rate (cfs)
		double max_release;	//maximum release rate (cfs)
		double min_volume;	//maximum capacity of the reservoir (af)
		double max_volume;	//minimum capacity of the reservoir (af)
		double basemonth_volume;	//The average volume in the base month (af)
		//Old c++ standards don't like this initilization        
        //double curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};	//monthly target value (rate)
        static double curr_target_rate[12];
        
public:

	/* ReachCode identifying the NHD waterbody corresponding to this reservoir */
	/*
	 * To simplify construction of ReservoirCreators, this is a static member that can be accessed
	 * without having to instansiate the object.  The parent also keeps the reach code which is
	 * initialized when the child is constructed so the reach code can be accessed via a generic
	 * Reservoir*
	 */
	static const long long reachCode = ${REACHCODE};
    ${NAME}():Reservoir(this->reachCode){
        /*initialize reservoir data*/
        min_release = ${MIN_RELEASE};
        max_release = ${MAX_RELEASE};
        min_volume = ${MIN_VOLUME};
        max_volume = ${MAX_VOLUME};
        basemonth_volume = ${BASE_VOLUME};
        /*TODO/FIXME WHY DOES THIS HAVE TO BE MANUALLY REGISTERED FOR IT TO WORK????? BUG IN CHARM???*/
        _register${NAME}();
    };
    /*
     * Copy Constructor
     */
    ${NAME}(const ${NAME}& other):Reservoir(other),${SUBREGION}(other),
    min_release(other.min_release), max_release(other.max_release), min_volume(other.min_volume), basemonth_volume(other.basemonth_volume)
    {

    }
    /*
     * Swap function for Copy-Swap idiom 
     */
    friend void swap(${NAME}& first, ${NAME}& second)
    {
        using std::swap; //Enable ADL (Arguement Dependent Lookup)
        swap( (Reservoir&) first, (Reservoir&) second);
        swap( (${SUBREGION}&) first, (${SUBREGION}&) second);
        swap(first.min_release, second.min_release);
        swap(first.max_release, second.max_release);
        swap(first.min_volume, second.min_volume);
        swap(first.max_volume, second.max_volume);
        swap(first.basemonth_volume, second.basemonth_volume);
    }
    /*
     * Assignment Operator
     */
    ${NAME}& operator=(${NAME} other)
    {
        swap(*this, other);
        return *this;
    }
    ~${NAME}(){};

    /*
     Release
     Parameters:
            curr_inflow     The current, instantaneous flow into the reservoir in cubic meters per second
            curr_volume     The current, instantaneous volume of the reservoir in cubic meters
            referenceDate   A julian reference data used to define currentTime
            currentTime     Seconds since referenceDate

        Return Parameters:
            
            rate            The rate at which water should be "released" from the reservoir, in cubic meters per second.
            duration        Seconds since referenceDate; The amount of time the returned rate is valid for.
    */    
    double release(double curr_inflow, double curr_volume, double referenceDate, long currentTime, double& rate, long& duration)
    {
        return calc_general_daily_release(curr_inflow, curr_volume, 
                                          referenceDate, currentTime,
                                          max_release, min_release, 
                                          max_volume, min_volume, 
                                          basemonth_volume,curr_target_rate,
                                          rate, duration);
    }
    /*
        PUP support
    */
    PUPable_decl(${NAME});
    ${NAME}(CkMigrateMessage* m) : Reservoir(m) {}
    virtual void pup(PUP::er &p)
    {
        Reservoir::pup(p);
        if(p.isUnpacking()) CkPrintf("Unpacking ${NAME}1\\n");
        else CkPrintf("Packing ${NAME}\\n");
        p|min_release;
        p|max_release;
        p|min_volume;
        p|max_volume;
        p|basemonth_volume;
        p(curr_target_rate, 12);
        if(p.isUnpacking())
        {
            CkPrintf("Unpacked stuff %f, %f, %f\\n", min_release, max_volume, basemonth_volume);
            CkPrintf("Array value: %f\\n", curr_target_rate[11]);
        }
    }
    
};
double ${NAME}::curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};	//monthly target value (rate)
#include "${NAME}.def.h"
#endif
"""

_cxx_region_class_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef _${NAME}_
#define _${NAME}_
#include "Reservoir.h"
#include "common_utilities.h"
// TODO/FIXME might want to remove the subregion directory structure
// That way a reservoir could simple sublcass one SUBREGION and the
// correct substitution occurs
#include "${REGION}/${SUBREGION}.h"
#include <iostream>
#include "${NAME}.decl.h"
//Any Default comments we want to add TODO/FIXME

class ${NAME} : public Reservoir, ${SUBREGION}
{
private:
		double min_release;	//minimum release rate (cfs)
		double max_release;	//maximum release rate (cfs)
		double min_volume;	//maximum capacity of the reservoir (af)
		double max_volume;	//minimum capacity of the reservoir (af)
		double basemonth_volume;	//The average volume in the base month (af)
		//Old c++ standards don't like this initilization        
        //double curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};	//monthly target value (rate)
        static double curr_target_rate[12];
        
public:

	/* ReachCode identifying the NHD waterbody corresponding to this reservoir */
	/*
	 * To simplify construction of ReservoirCreators, this is a static member that can be accessed
	 * without having to instansiate the object.  The parent also keeps the reach code which is
	 * initialized when the child is constructed so the reach code can be accessed via a generic
	 * Reservoir*
	 */
	static const long long reachCode = ${REACHCODE};
    ${NAME}():Reservoir(this->reachCode){
        /*initialize reservoir data*/
        min_release = ${MIN_RELEASE};
        max_release = ${MAX_RELEASE};
        min_volume = ${MIN_VOLUME};
        max_volume = ${MAX_VOLUME};
        basemonth_volume = ${BASE_VOLUME};
        /*TODO/FIXME WHY DOES THIS HAVE TO BE MANUALLY REGISTERED FOR IT TO WORK????? BUG IN CHARM???*/
        _register${NAME}();
    };

    /*
     * Copy Constructor
     */
    ${NAME}(const ${NAME}& other):Reservoir(other),${SUBREGION}(other),
    min_release(other.min_release), max_release(other.max_release), min_volume(other.min_volume), basemonth_volume(other.basemonth_volume)
    {

    }
    /*
     * Swap function for Copy-Swap idiom 
     */
    friend void swap(${NAME}& first, ${NAME}& second)
    {
        using std::swap; //Enable ADL (Arguement Dependent Lookup)
        swap( (Reservoir&) first, (Reservoir&) second);
        swap( (${SUBREGION}&) first, (${SUBREGION}&) second);
        swap(first.min_release, second.min_release);
        swap(first.max_release, second.max_release);
        swap(first.min_volume, second.min_volume);
        swap(first.max_volume, second.max_volume);
        swap(first.basemonth_volume, second.basemonth_volume);
    }
    /*
     * Assignment Operator
     */
    ${NAME}& operator=(${NAME} other)
    {
        swap(*this, other);
        return *this;
    }
    ~${NAME}(){};
    
    
    /*
     Release
     Parameters:
            curr_inflow     The current, instantaneous flow into the reservoir in cubic meters per second
            curr_volume     The current, instantaneous volume of the reservoir in cubic meters
            referenceDate   A julian reference data used to define currentTime
            currentTime     Seconds since referenceDate

        Return Parameters:
            
            rate            The rate at which water should be "released" from the reservoir, in cubic meters per second.
            duration        Seconds since referenceDate; The amount of time the returned rate is valid for.
    */    
    double release(double curr_inflow, double curr_volume, double referenceDate, long currentTime, double& rate, long& duration)
    {
        return ${SUBREGION}::release(curr_inflow, curr_volume, referenceDate, currentTime, rate, duration);
    }
    /*
        PUP support
    */
    PUPable_decl(${NAME});
    ${NAME}(CkMigrateMessage* m) : Reservoir(m) {}
    virtual void pup(PUP::er &p)
    {
        Reservoir::pup(p);
        if(p.isUnpacking()) CkPrintf("Unpacking ${NAME}\\n");
        else CkPrintf("Packing ${NAME}\\n");
        p|min_release;
        p|max_release;
        p|min_volume;
        p|max_volume;
        p|basemonth_volume;
        p(curr_target_rate, 12);
        if(p.isUnpacking())
        {
            CkPrintf("Unpacked stuff %f, %f, %f\\n", min_release, max_volume, basemonth_volume);
            CkPrintf("Array value: %f\\n", curr_target_rate[11]);
        }
    }

};
double ${NAME}::curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};	//monthly target value (rate)
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

class ReservoirCI():
    
    def __init__(self, name):
        self.template = Template(_charm_ci_string)
        self.template = Template(self.template.safe_substitute(NAME=name))

    def __str__(self):
        return self.template.safe_substitute()

class Reservoir():

    def __init__(self, region):
        self.regional = region        
        if self.regional:
            self.template = Template(_cxx_region_class_string)
        else:
            self.template = Template(_cxx_general_class_string)

    def __str__(self):
        #Use safe_substitute to avoid modifying the template
        return self.template.safe_substitute()

    def name(self, n):
        #Safe_substitute only replaces the keys it is passed
        #in this case NAME, so any other keys will remain intact
        self.template = Template(self.template.safe_substitute(NAME=n))

    def reachCode(self, r):
        self.template = Template(self.template.safe_substitute(REACHCODE=r))

    def region(self, r):
        self.template = Template(self.template.safe_substitute(REGION=r))

    def subRegion(self, r):
        self.template = Template(self.template.safe_substitute(SUBREGION=r))

    def release(self, minimum, maximum):
        self.template = Template(self.template.safe_substitute(MIN_RELEASE=minimum, MAX_RELEASE=maximum))
    
    def volume(self, minimum, maximum, base):
        self.template = Template(self.template.safe_substitute(MIN_VOLUME=minimum, MAX_VOLUME=maximum, BASE_VOLUME=base))

    def targetRates(self, rates):
        #Make some assumption about how rates are passed
        #Assume a list of length = 12
        if len(rates) != 12:
            raise(Exception("targetRate exception: rates must be a list of length 12"))
        keys = ['JAN','FEB','MAR','APR','MAY','JUN','JUL','AUG','SEP','OCT','NOV','DEC']
        mapping = dict(zip(keys,rates))
        self.template = Template(self.template.safe_substitute(mapping))


def abstract():
    return {"Reservoir.h":_cxx_abstract_class_header_string, "Reservoir.cpp":_cxx_abstract_class_def_string, "Reservoir.ci":_charm_abstract_ci_string}
