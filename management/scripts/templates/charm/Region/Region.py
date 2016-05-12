#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""

This is a template class for automatically generating 
WaterManagementRegion subclasses for ADHydro.

"""

_cxx_abstract_region_class_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef WATER_REGION
#define WATER_REGION

/*
    WaterManagementRegion parent class used to hold data at a regional scope
*/
class WaterManagementRegion
{
    private:
    /*
        Any private data that is common to all WaterManagementRegions can be declared here
        If there is private data declared, an appropriate constructor should
        also be implemented.
    */

    public:
    /*
        Virtual functions that subclasses should implement
    */

    /*
     Release
     Parameters:
            curr_inflow     The current, instantaneous flow into the reservoir in cubic meters per second
            curr_volume     The current, instantaneous volume of the reservoir in cubic meters
            referenceDate   A julian reference data used to define currentTime
            currentTime     Seconds since referenceDate

        Return Parameters:
            
            rate            The rate at which water should be "released" from the reservoir, in cubic meters per second.
            expirationTime  Seconds since referenceDate; The time at which the returned rate needs to be updated (by calling release again).
    */    
    virtual void release(const double& curr_inflow, const double& curr_volume, 
                         const double& referenceDate, const double& currentTime, 
                         double& rate, double& expirationTime) = 0;
    virtual ~WaterManagementRegion(){}
};

#endif
"""

_cxx_region_class_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef _${NAME}_
#define _${NAME}_
#include "WaterManagementRegion.h"
#include <iostream>
#include <ctime>

#include "common_utilities.h"

/*
    A regional sublcass implementing the virtual release function
*/
class ${NAME} : WaterManagementRegion
{
    private:
        double min_release_region;	//minimum release rate (m^3/s)
        double max_release_region;	//maximum release rate (m^3/s)
        double min_volume_region;	//maximum capacity of the reservoir (m^3)
        double max_volume_region;	//minimum capacity of the reservoir (m^3)
        double basemonth_volume_region;	//The average volume in the base month (m^3)
        //Old c++ standards don't like this initilization        
        //double curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};	//monthly target value (rate)
        static double curr_target_rate[12];
    	
    public:
    ${NAME}()
    {
        min_release_region = ${MIN_RELEASE};
        max_release_region = ${MAX_RELEASE};
        min_volume_region = ${MIN_VOLUME};
        max_volume_region = ${MAX_VOLUME};
        basemonth_volume_region = ${BASE_VOLUME};
    }
    /*
     * Copy Constructor
     */
    ${NAME}(const ${NAME}& other):WaterManagementRegion(other),
    min_release_region(other.min_release_region), max_release_region(other.max_release_region),
    min_volume_region(other.min_volume_region), basemonth_volume_region(other.basemonth_volume_region)
    {

    }
    /*
     * Swap function for Copy-Swap idiom 
     */
    friend void swap(${NAME}& first, ${NAME}& second)
    {
        using std::swap; //Enable ADL (Arguement Dependent Lookup)
        swap(first.min_release_region, second.min_release_region);
        swap(first.max_release_region, second.max_release_region);
        swap(first.min_volume_region, second.min_volume_region);
        swap(first.max_volume_region, second.max_volume_region);
        swap(first.basemonth_volume_region, second.basemonth_volume_region);
    }
    /*
     * Assignment Operator
     */
    ${NAME}& operator=(${NAME} other)
    {
        swap(*this, other);
        return *this;
    }
    /*
        ${NAME} Destructor
    */
    ~${NAME}(){}
    /*
        This is ${NAME}'s release function implementation
    */
    /*
     Release
     Parameters:
            curr_inflow     The current, instantaneous flow into the reservoir in cubic meters per second
            curr_volume     The current, instantaneous volume of the reservoir in cubic meters
            referenceDate   A julian reference data used to define currentTime
            currentTime     Seconds since referenceDate

        Return Parameters:
            
            rate            The rate at which water should be "released" from the reservoir, in cubic meters per second.
            expirationTime  Seconds since referenceDate; The time at which the returned rate needs to be updated (by calling release again).
    */    
    void release(const double& curr_inflow, const double& curr_volume,
            const double& referenceDate, const double& currentTime,
            double& rate, double& expirationTime)
    {
        calc_general_daily_release(curr_inflow, curr_volume,
                                   referenceDate, currentTime,
                                   basemonth_volume_region, curr_target_rate,
                                   rate, expirationTime);
    }

};
double ${NAME}::curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};
#endif
"""

_cxx_subregion_class_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef _${NAME}_
#define _${NAME}_
#include "${PARENT_REGION}.h"
#include <iostream>
#include <ctime>

#include "common_utilities.h"

/*
    A regional sublcass implementing the virtual release function
*/
class ${NAME} : public ${PARENT_REGION}
{
    private:
        double min_release_region;	//minimum release rate (m^3/s)
        double max_release_region;	//maximum release rate (m^3/s)
        double min_volume_region;	//maximum capacity of the reservoir (m^3)
        double max_volume_region;	//minimum capacity of the reservoir (m^3)
        double basemonth_volume_region;	//The average volume in the base month (m^3)
        //Old c++ standards don't like this initilization        
        //double curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};	//monthly target value (rate)
        static double curr_target_rate[12];
    	
    public:
    ${NAME}()
    {
        min_release_region = ${MIN_RELEASE};
        max_release_region = ${MAX_RELEASE};
        min_volume_region = ${MIN_VOLUME};
        max_volume_region = ${MAX_VOLUME};
        basemonth_volume_region = ${BASE_VOLUME};
    }
    /*
     * Copy Constructor
     */
    ${NAME}(const ${NAME}& other):${PARENT_REGION}(other),
    min_release_region(other.min_release_region), max_release_region(other.max_release_region),
    min_volume_region(other.min_volume_region), basemonth_volume_region(other.basemonth_volume_region)
    {

    }
    /*
     * Swap function for Copy-Swap idiom 
     */
    friend void swap(${NAME}& first, ${NAME}& second)
    {
        using std::swap; //Enable ADL (Arguement Dependent Lookup)
        swap((${PARENT_REGION}&) first, (${PARENT_REGION}&) second);
        swap(first.min_release_region, second.min_release_region);
        swap(first.max_release_region, second.max_release_region);
        swap(first.min_volume_region, second.min_volume_region);
        swap(first.max_volume_region, second.max_volume_region);
        swap(first.basemonth_volume_region, second.basemonth_volume_region);
    }
    /*
     * Assignment Operator
     */
    ${NAME}& operator=(${NAME} other)
    {
        swap(*this, other);
        return *this;
    }
    /*
        ${NAME} Destructor
    */
    ~${NAME}(){}
    /*
        This is ${NAME}'s release function implementation
    */
    /*
     Release
     Parameters:
            curr_inflow     The current, instantaneous flow into the reservoir in cubic meters per second
            curr_volume     The current, instantaneous volume of the reservoir in cubic meters
            referenceDate   A julian reference data used to define currentTime
            currentTime     Seconds since referenceDate

        Return Parameters:
            
            rate            The rate at which water should be "released" from the reservoir, in cubic meters per second.
            expirationTime  Seconds since referenceDate; The time at which the returned rate needs to be updated (by calling release again).
    */    
    void release(const double& curr_inflow, const double& curr_volume,
                 const double& referenceDate, const double& currentTime, 
                 double& rate, double& expirationTime)
    {
        calc_general_daily_release(curr_inflow, curr_volume,
                                   referenceDate, currentTime,
                                   basemonth_volume_region, curr_target_rate,
                                   rate, expirationTime);
    }

};
double ${NAME}::curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};
#endif
"""

_cxx_subregion_parent_class_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef _${NAME}_
#define _${NAME}_
#include "${PARENT_REGION}.h"
#include <iostream>
#include <ctime>

#include "common_utilities.h"

/*
    A regional sublcass implementing the virtual release function
*/
class ${NAME} : public ${PARENT_REGION}
{
    private:
        double min_release_region;	//minimum release rate (m^3/s)
        double max_release_region;	//maximum release rate (m^3/s)
        double min_volume_region;	//maximum capacity of the reservoir (m^3)
        double max_volume_region;	//minimum capacity of the reservoir (m^3)
        double basemonth_volume_region;	//The average volume in the base month (m^3)
        //Old c++ standards don't like this initilization        
        //double curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};	//monthly target value (rate)
        static double curr_target_rate[12];

    	
    public:
    ${NAME}()
    {
        min_release_region = ${MIN_RELEASE};
        max_release_region = ${MAX_RELEASE};
        min_volume_region = ${MIN_VOLUME};
        max_volume_region = ${MAX_VOLUME};
        basemonth_volume_region = ${BASE_VOLUME};
    }
    /*
     * Copy Constructor
     */
    ${NAME}(const ${NAME}& other):${PARENT_REGION}(other),
    min_release_region(other.min_release_region), max_release_region(other.max_release_region),
    min_volume_region(other.min_volume_region), basemonth_volume_region(other.basemonth_volume_region)
    {

    }
    /*
     * Swap function for Copy-Swap idiom 
     */
    friend void swap(${NAME}& first, ${NAME}& second)
    {
        using std::swap; //Enable ADL (Arguement Dependent Lookup)
        swap((${PARENT_REGION}&) first, (${PARENT_REGION}&) second);
        swap(first.min_release_region, second.min_release_region);
        swap(first.max_release_region, second.max_release_region);
        swap(first.min_volume_region, second.min_volume_region);
        swap(first.max_volume_region, second.max_volume_region);
        swap(first.basemonth_volume_region, second.basemonth_volume_region);
    }
    /*
     * Assignment Operator
     */
    ${NAME}& operator=(${NAME} other)
    {
        swap(*this, other);
        return *this;
    }
    /*
        ${NAME} Destructor
    */
    ~${NAME}(){}
    /*
        This is ${NAME}'s release function implementation
    */
    /*
     Release
     Parameters:
            curr_inflow     The current, instantaneous flow into the reservoir in cubic meters per second
            curr_volume     The current, instantaneous volume of the reservoir in cubic meters
            referenceDate   A julian reference data used to define currentTime
            currentTime     Seconds since referenceDate

        Return Parameters:
            
            rate            The rate at which water should be "released" from the reservoir, in cubic meters per second.
            expirationTime  Seconds since referenceDate; The time at which the returned rate needs to be updated (by calling release again).
    */    
    void release(const double& curr_inflow, const double& curr_volume, 
                   const double& referenceDate, const double& currentTime, 
                   double& rate, double& expirationTime)
    {
        ${PARENT_REGION}::release(curr_inflow, curr_volume, referenceDate, currentTime, rate, expirationTime);
    }

};
double ${NAME}::curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};
#endif
"""

from string import Template

class Region():

    def __init__(self, subRegion, useRegion):
        """
        Params:
            subRegion: Bool, indicates if this is a sub region
            useRegion: Bool, indicates if a parent region should
                        be used for release calculation or not
        """
        self.useRegion = useRegion
        self.subRegion = subRegion
        if(self.subRegion):
            if(self.useRegion):
                self.template = Template(_cxx_subregion_parent_class_string)
            else:
                self.template = Template(_cxx_subregion_class_string)
        else:
            self.template = Template(_cxx_region_class_string)

    def __str__(self):
        #Use safe_substitute to avoid modifying the template
        return self.template.safe_substitute()

    def name(self, n):
        #Safe_substitute only replaces the keys it is passed
        #in this case NAME, so any other keys will remain intact
        self.template = Template(self.template.safe_substitute(NAME=n))

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

    def region(self, parent):
        #in either of these cases, we have a PARENT_REGION to subsititue
        if self.subRegion or self.useRegion:
            self.template = Template(self.template.safe_substitute(PARENT_REGION=parent))

def abstract():
    #Return the abstract region class string
    return {'WaterManagementRegion.h':_cxx_abstract_region_class_string}
