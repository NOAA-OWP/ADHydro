#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""

This is a template class for automatically generating 
Reservoir component classes for ADHydro.

"""

_cxx_general_class_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef _${NAME}_
#define _${NAME}_
#include "../components/Reservoir.h"
#include "../common/common_utilities.h"
// TODO/FIXME might want to remove the subregion directory structure
// That way a reservoir could simple sublcass one SUBREGION and the
// correct substitution occurs
#include "${REGION}/${SUBREGION}.h"
#include <iostream>

//Any Default comments we want to add TODO/FIXME

class ${NAME} : public Reservoir, ${SUBREGION}
{
private:
		double min_release;	//minimum release rate (cfs)
		double max_release;	//maximum release rate (cfs)
		double min_volume;	//maximum capacity of the reservoir (af)
		double max_volume;	//minimum capacity of the reservoir (af)
		double basemonth_volume;	//The average volume in the base month (af)
		double curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};	//monthly target value (rate)
        
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

    };
    ~${NAME}(){};
    
    double release(double curr_inflow, double curr_volume, int curr_date)
    {
        return calc_general_daily_release(curr_inflow, curr_volume, curr_date, max_release, min_release, max_volume, min_volume, basemonth_volume,curr_target_rate);
        
    }
};

#endif
"""

_cxx_region_class_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef _${NAME}_
#define _${NAME}_
#include "../components/Reservoir.h"
#include "../common/common_utilities.h"
// TODO/FIXME might want to remove the subregion directory structure
// That way a reservoir could simple sublcass one SUBREGION and the
// correct substitution occurs
#include "${REGION}/${SUBREGION}.h"
#include <iostream>

//Any Default comments we want to add TODO/FIXME

class ${NAME} : public Reservoir, ${SUBREGION}
{
private:
		double min_release;	//minimum release rate (cfs)
		double max_release;	//maximum release rate (cfs)
		double min_volume;	//maximum capacity of the reservoir (af)
		double max_volume;	//minimum capacity of the reservoir (af)
		double basemonth_volume;	//The average volume in the base month (af)
		//double curr_target_rate[12] = {${JAN},${FEB},${MAR},${APR},${MAY},${JUN},${JUL},${AUG},${SEP},${OCT},${NOV},${DEC}};	//monthly target value (rate)
        
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

    };
    ~${NAME}(){};
    
    double release(double curr_inflow, double curr_volume, int curr_date)
    {
        return ${SUBREGION}::release(curr_inflow, curr_volume, curr_date);
        
    }
};

#endif
"""


from string import Template

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
