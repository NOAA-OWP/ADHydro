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

//calculate the difference between the current date and target date.
double diff_dates(int );

//calculate daily release rate
double calc_general_daily_release(double , double , int , double , double ,	double , double , double , double (&)[12]);

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

//11.3.2015 Zen Add:
#include<fstream>
#include<string>
#include<sstream>

#include <stdlib.h>

#include "common_utilities.h"

#define AFRATE  1.9835 //AFrate converts the flow rate (cfs) to volue (AF)
#define NODATA  -999   //2015.11.6 Zen Add: I temporary substituted -999 for missing data

// calculate daily release using a general frame work
double calc_general_daily_release (double curr_inflow, double curr_volume, int curr_date, double max_release, double min_release,
		double max_volume, double min_volume, double basemonth_volume, double (&curr_target_rate)[12])
{
	double volumediff; //difference between the target volume and current volume, unit AF
	double daysleft;	//number of days left in the month
	double targetrelease; //target cfs rate to release

	int curr_month; //Remember Jan=0, Feb = 1,..., Dec = 11

	//calculate the current month from curr_date.
	curr_month = (curr_date%10000)/100 - 1;

	//summary of input
	std::cout << "date=" <<curr_date <<"\t inflow=" <<curr_inflow << "\t volume=" << curr_volume
			<< "\t max_release=" << max_release << "\t target_volume_rate=" << (curr_target_rate)[0] << "\t base_month_volume=" << basemonth_volume  << std::endl;

	volumediff = basemonth_volume*(curr_target_rate)[curr_month] - curr_volume;

	//Zen Add: 10/30/2015
	//Calculate how many days left in the month
	daysleft = diff_dates(curr_date);

	//daysleft = 10;
	//daysleft = 3;

	std::cout << "days left " << daysleft << "\\n";

	//Zen Add: 10/30/2015
	//The way to calculate a daily release (cfs)
	targetrelease = (volumediff/daysleft + curr_inflow*AFRATE)/AFRATE;

	//std::cout << "raw target release " << targetrelease << "\\n";

	//Zen Update: 11/6/2015
	//check the targetrelease is between min and max flow (cfs)
	//if min or max flow does not have, ignore the next if-statement
	if(targetrelease < min_release && min_release != NODATA){
		targetrelease = min_release;
	}else if(targetrelease > max_release && min_release != NODATA){
		targetrelease = max_release;
	}

	return targetrelease;
}

// Make a tm structure representing this date
double diff_dates(int date)
{
	//initilization
	int year, month, day;

    std::tm tmcur = {0};
    std::tm tmnext = {0};

    year = date/ 10000;
    date %= 10000;
    month = date / 100;
    day = date % 100;

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
