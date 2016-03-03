#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""

This is a template class for automatically generating 
Region subclasses for ADHydro.

"""

_cxx_abstract_region_class_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef WATER_REGION
#define WATER_REGION

/*
    Region parent class used to hold data at a regional scope
*/
class Region
{
    private:
    /*
        Any private data that is common to all Regions can be declared here
        If there is private data declared, an appropriate constructor should
        also be implemented.
    */

    public:
    /*
        Virtual functions that subclasses should implement
    */
    virtual double release(double curr_inflow, double curr_volume, int curr_date) = 0;
    virtual ~Region(){}
};

#endif
"""

_cxx_region_class_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef _${NAME}_
#define _${NAME}_
#include "../Region.h"
#define AFRATE  1.9835 //AFrate converts the flow rate (cfs) to volue (AF)

#include <iostream>
#include <ctime>

#include "../../common/common_utilities.h"

/*
    A regional sublcass implementing the virtual release function
*/
class ${NAME} : Region
{
    private:
        double min_release_region;	//minimum release rate (cfs)
        double max_release_region;	//maximum release rate (cfs)
        double min_volume_region;	//maximum capacity of the reservoir (af)
        double max_volume_region;	//minimum capacity of the reservoir (af)
        double basemonth_volume_region;	//The average volume in the base month (af)
        double curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};	//monthly target value (rate)

    	
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
        ${NAME} Destructor
    */
    ~${NAME}(){}
    /*
        This is ${NAME}'s release function implementation
    */
    double release(double curr_inflow, double curr_volume, int curr_date)
    {
        return calc_general_daily_release(curr_inflow, curr_volume, curr_date, max_release_region, min_release_region, max_volume_region, min_volume_region, basemonth_volume_region,curr_target_rate);
    }

};

#endif
"""

_cxx_subregion_class_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef _${NAME}_
#define _${NAME}_
#define AFRATE  1.9835 //AFrate converts the flow rate (cfs) to volue (AF)

#include "${PARENT_REGION}.h"
#include <iostream>
#include <ctime>

#include "../../common/common_utilities.h"

/*
    A regional sublcass implementing the virtual release function
*/
class ${NAME} : public ${PARENT_REGION}
{
    private:
        double min_release_region;	//minimum release rate (cfs)
        double max_release_region;	//maximum release rate (cfs)
        double min_volume_region;	//maximum capacity of the reservoir (af)
        double max_volume_region;	//minimum capacity of the reservoir (af)
        double basemonth_volume_region;	//The average volume in the base month (af)
        double curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};	//monthly target value (rate)

    	
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
        ${NAME} Destructor
    */
    ~${NAME}(){}
    /*
        This is ${NAME}'s release function implementation
    */
    double release(double curr_inflow, double curr_volume, int curr_date)
    {
        return calc_general_daily_release(curr_inflow, curr_volume, curr_date, max_release_region, min_release_region, max_volume_region, min_volume_region, basemonth_volume_region,curr_target_rate);
    }

};

#endif
"""

_cxx_subregion_parent_class_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef _${NAME}_
#define _${NAME}_
#define AFRATE  1.9835 //AFrate converts the flow rate (cfs) to volue (AF)

#include "${PARENT_REGION}.h"
#include <iostream>
#include <ctime>

#include "../../common/common_utilities.h"

/*
    A regional sublcass implementing the virtual release function
*/
class ${NAME} : public ${PARENT_REGION}
{
    private:
        double min_release_region;	//minimum release rate (cfs)
        double max_release_region;	//maximum release rate (cfs)
        double min_volume_region;	//maximum capacity of the reservoir (af)
        double max_volume_region;	//minimum capacity of the reservoir (af)
        double basemonth_volume_region;	//The average volume in the base month (af)
        double curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};	//monthly target value (rate)

    	
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
        ${NAME} Destructor
    */
    ~${NAME}(){}
    /*
        This is ${NAME}'s release function implementation
    */
    double release(double curr_inflow, double curr_volume, int curr_date)
    {
        return ${PARENT_REGION}::release(curr_inflow, curr_volume, curr_date);
    }

};

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
    return {'Region.h':_cxx_abstract_region_class_string}
