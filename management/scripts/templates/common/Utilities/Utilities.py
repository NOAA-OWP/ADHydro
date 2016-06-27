#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""

This is a template module for automatically generating 
common utilities for ADHydro Management

"""
from string import Template
from os import linesep

_cxx_utilities_header_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
/*
 * common_utilities.h
 *
 *  Created on: Nov 3, 2015
 *      Author: yigarash
 */

#ifndef COMMON_UTILITIES_H_
#define COMMON_UTILITIES_H_
#include "all.h"
//calculate the difference between the current date and target date.
double diff_dates(int, int, int);

/*
     Release
     Parameters:
            curr_inflow         The current, instantaneous flow into the reservoir in cubic meters per second
            curr_volume         The current, instantaneous volume of the reservoir in cubic meters
            referenceDate       A julian reference data used to define currentTime
            currentTime         Seconds since referenceDate
            max_release
            min_release
            max_volume
            min_volume
            basemonth_volume    
            curr_target_rate

        Return Parameters:
            
            rate            The rate at which water should be "released" from the reservoir, in cubic meters per second.
            expirationTime  Seconds since referenceDate; The time at which the returned rate needs to be updated (by calling release again).
*/
void calc_general_daily_release (const double& curr_inflow, const double& curr_volume,
                                 const double& referenceDate, const double& currentTime,
                                 const double& basemonth_volume, const double (&curr_target_rate)[12],
                                 double& release, double& expirationTime);

#endif /* COMMON_UTILITIES_H_ */
"""

_cxx_utilities_def_string = \
"""
/*
    This is auto-generated code from TODO/FIXME!!!
*/
/*
 * common_utilities.cpp
 *
 *  Created on: Nov 3, 2015
 *      Author: yigarash
 */
//10.28.2015 Zen Add:
#include <ctime>
#include <iostream>
#include <stdlib.h>

#include "common_utilities.h"

#define NODATA  -999   //2015.11.6 Zen Add: I temporary substituted -999 for missing data

// calculate daily release using a general frame work
/*
     Release
     Parameters:
            curr_inflow         The current, instantaneous flow into the reservoir in cubic meters per second
            curr_volume         The current, instantaneous volume of the reservoir in cubic meters
            referenceDate       A julian reference data used to define currentTime
            currentTime         Seconds since referenceDate
            max_release
            min_release
            max_volume
            min_volume
            basemonth_volume    
            curr_target_rate

        Return Parameters:
            
            rate            The rate at which water should be "released" from the reservoir, in cubic meters per second.
            duration        Seconds since referenceDate; The amount of time the returned rate is valid for.
*/
void calc_general_daily_release (const double& curr_inflow, const double& curr_volume,
                                 const double& referenceDate, const double& currentTime,
                                 const double& basemonth_volume, const double (&curr_target_rate)[12],
                                 double& release, double& expirationTime)
{
	double volumediff; //difference between the target volume and current volume, unit cubic meters
	double daysleft;	//number of days left in the month
	double targetrelease; //target m^3/s rate to release
    
    long year, month, day, hour, minute;
    double second;
    julianToGregorian(referenceDate+currentTime/86400, &year, &month, &day, &hour, &minute, &second);
    
    //NJF This needs to be curr_volume - target_volume so that we can stop releasing water if we don't have enough.
	//volumediff = basemonth_volume*(curr_target_rate)[month] - curr_volume;
    volumediff = curr_volume - basemonth_volume*(curr_target_rate)[month];
/*
	//Zen Add: 10/30/2015
	//Calculate how many days left in the month
	daysleft = diff_dates(year, month, day);

  //FIXME
  //Think of a better way to avoid large fluctions of release between end
  //of month and beginning of next month
  if( daysleft < 7.0 )
  {
    daysleft = 7.0;
  }
*/
    //NJF 2016-06-22
    //Use a constant days left to avoid asymtotic behavior
    daysleft = 30.0;
	//Zen Add: 10/30/2015
	//The way to calculate a daily release (cubic meters per second)
    //NJF Account for needing more water to reach target volume
    targetrelease = 0; //Don't release any water untill we know we don't need to fill up first    
    //NOTE: If we know a reasonable duration, then we can figure outhow much of the 
    //curr_inflow we want to "keep", while still allowing for some release...
    if(volumediff >= 0)
    {
        //we have at least our target volume, maybe more, so we can release water
        //Figure out how many m^3/s to release if we wanted to reach reach our target volume
        //in daysleft days, then add in the current flow into the reservoir and release that much
        targetrelease = (volumediff/daysleft/86400 + curr_inflow);
    }

	//Zen Update: 11/6/2015
	//check the targetrelease is between min and max flow (m^3/s)
	//if min or max flow does not have, ignore the next if-statement
    //NJF Not sure if this is a good idea.  What if we simulate several super dry years and there
    //simply isn't enough water to release?  Similarly, if we simulate heavy rain/flooding, max historical may not
    //best represent this.  Perhaps this is best left to the decision of the reservoir itself.  For example, perhaps there is a 
    //physical maximum that the reservoir is capable of releasing.  For minimum, I would say 0.  It could be that there simply isn't
    //enough water to release, for example volume is 2 and target volume is 50,000.  We may not be able to physically satisfy this minimum
    //requirement.  Better suited for an individual reservoir to check this and modify, but in general...not a good idea.
    /*
	if(targetrelease < min_release && min_release != NODATA){
		targetrelease = min_release;
	}else if(targetrelease > max_release && min_release != NODATA){
		targetrelease = max_release;
	}*/
    //Cannot release a negative amount of water!
	release = targetrelease < 0.0 ? 0.0 : targetrelease;
    expirationTime = currentTime + 86400; //TODO/FIXME Change this ?!?!?!  Could also let reservoir set this. Currently sets expiration to 24 hours after currentTime
}

// Make a tm structure representing this date
double diff_dates(int year, int month, int day)
{
    std::tm tmcur = {0};
    std::tm tmnext = {0};

	tmcur.tm_year = year - 1900; // years count from 1900
    tmcur.tm_mon = month - 1;    // months count from January=0
    tmcur.tm_mday = day;         // days count from 1

    //if the curent month is december
    if(month == 11){
    	tmnext.tm_year = year - 1900 + 1; // years count from 1900
    	tmnext.tm_mon = 0;    // months count from January=0
    	tmnext.tm_mday = 1;         // days count from 1
    } else {
    	tmnext.tm_year = year - 1900; // years count from 1900
    	tmnext.tm_mon = month;    // months count from January=0
    	tmnext.tm_mday = 1;         // days count from 1
    }
	//std::cout << "dates" << tmcur.tm_year << tmcur.tm_mon <<tmcur.tm_mday << "\\n";
	//std::cout << "dates" << tmnext.tm_year << tmnext.tm_mon <<tmnext.tm_mday << "\\n";
    // Arithmetic time values.
    // On a posix system, these are seconds since 1970-01-01 00:00:00 UTC
    std::time_t timecur = std::mktime(&tmcur);
    std::time_t timenext = std::mktime(&tmnext);

    // Divide by the number of seconds in a day
    const int seconds_per_day = 60*60*24;
    std::time_t difference = (timenext - timecur) / seconds_per_day;

    //subtract 1 because the difference is the current date and the 1st day of the next month. By subtracting 1, adjust this to the end of the month.
    double diff_days = (double) difference - 1;
    //double diffdays = std::difftime(timecur, timecur);
    //  double diffdays = std::difftime(timecur, timecur) / seconds_per_day;

    //std::cout << "diff days" << diff_days << "\\n";
    return diff_days;
}
"""

def classes():
    return {'common_utilities.h':_cxx_utilities_header_string,'common_utilities.cpp':_cxx_utilities_def_string}
