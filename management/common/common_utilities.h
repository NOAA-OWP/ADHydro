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

//load csv for monthly parameters
void load_csv_target_volume(int, int, int, double (&)[12]);

//load csv for general data
void load_csv_general_data(int, int, int, double *, double *, double *, double *, double *, bool *);

//calculate daily release rate
double calc_general_daily_release(double , double , int , double , double ,	double , double , double , double (&)[12]);

#endif /* COMMON_UTILITIES_H_ */
