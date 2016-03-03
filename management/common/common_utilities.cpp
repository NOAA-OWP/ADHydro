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
//#include <../common_utilities.h>
//#include "common_utilities.h"
//#include "../../../common/common_utilities.h"

//load data for monthly target volume
void load_csv_target_volume(int div, int dist, int wdid, double (&curtargetrate)[12]){
	  //counter
	  int cnt = 0;

	  //create a file name from a unique code for the structure (like wdid)
	  char divStr [10];
      char distStr [10];
      char wdidStr [10];


	  //11.5.2015 Zen Add: Change the location of data files
	  std::string dir_name;
	  std::string file_name;
	  std::string data_type;
	  std::string file_loc;

	  //Windows needs the absolute path to load a file
	  dir_name = "C:/Users/yigarash/Dropbox/Java_workspace/management_apt/src/database/";
	  //Linux
	  //dir_name = "./database/";

	  data_type = "_target_params.csv";
	  //std::string dir_name = "../database/";

	  //11.5.2015 Zen Add: choose file name for different levels
      sprintf(divStr,"%d",div);
      sprintf(distStr,"%d",dist);
      sprintf(wdidStr,"%d",wdid);
	  if(dist == -1){ //division level
		  file_name = "div" + std::string(divStr);
	  } else if(wdid == -1){ //district level
		  file_name = "div" + std::string(divStr)+"_dist" + std::string(distStr);
	  } else { //individual level
		  file_name = "div" + std::string(divStr)+"_dist" + std::string(distStr) +"_" + std::string(wdidStr);
	  }
	  file_loc  = dir_name + file_name + data_type;

	  //file name

	  std::ifstream ifs(file_loc.c_str());
	  //std::ifstream ifs("C:/Users/yigarash/Dropbox/Java_workspace/management_office/src/database/4304433_ind_target_params.csv");

	  std::string str;

	  if(!ifs){
		std::cout << file_loc << std::endl;
		std::cout << "Error:Input data file not found" <<std::endl;
	  }
//	  curtargetrate[0] = 1;
//	  std::cout << "test array:" <<curtargetrate[0] << std::endl;

	  while(std::getline(ifs, str)){
		  //Fix this later. I could not directory load data.
		  sscanf(str.data(), "%lf", &curtargetrate[cnt]);
//		  std::cout << curtargetrate[cnt] << "\n";
		  cnt++;
	  }
}

void load_csv_general_data(int div, int dist, int wdid, double *min_release, double *max_release, double *min_volume, double *max_volume, double *basemonth_volume, bool *useRegion){
	  //counter
	  int cnt, cnt2;
	  double temp_val;

	  //load basic data: max/min flow (cfs), max/min volume (AF), useRegion (T or F)
	  //std::ifstream ifs("C:/Users/yigarash/Documents/Research/Green River Basin Project/test run/management_testrun/database/4304433_ind_general_data.csv");
	  //std::ifstream ifs("C:/Users/Zen/Dropbox/GRB/Source Code/C++/management_apt/src/database/4304433_ind_general_data.csv");

	  //create a file name from a unique code for the structure (like wdid)
	  char divStr [10];
      char distStr [10];
      char wdidStr [10];

	  //11.5.2015 Zen Add: Change the location of data files
	  std::string dir_name;
	  std::string file_name;
	  std::string data_type;
	  std::string file_loc;

	  //Windows needs the absolute path to load a file
	  dir_name = "C:/Users/yigarash/Dropbox/Java_workspace/management_apt/src/database/";
	  //Linux
	  //dir_name = "./database/";


	  data_type = "_general_data.csv";
	  //std::string dir_name = "../database/";

	  //11.5.2015 Zen Add: choose file name for different levels
	  sprintf(divStr,"%d",div);
      sprintf(distStr,"%d",dist);
      sprintf(wdidStr,"%d",wdid);
	  if(dist == -1){ //division level
		  file_name = "div" + std::string(divStr);
	  } else if(wdid == -1){ //district level
		  file_name = "div" + std::string(divStr)+"_dist" + std::string(distStr);
	  } else { //individual level
		  file_name = "div" + std::string(divStr)+"_dist" + std::string(distStr) +"_" + std::string(wdidStr);
	  }
	  file_loc  = dir_name + file_name + data_type;


	  //load csv file
	  std::ifstream ifs(file_loc.c_str());
	  std::string str;

	  if(!ifs){
		std::cout << "Error:Input data file not found" <<std::endl;
		return;
	  }
	  //initilize the counter
	  cnt = 0;

	  //while(std::getline(ifs, str, ',')){
	  while(std::getline(ifs, str)){
		 std::istringstream iss(str);
		 std::string token;
		 cnt2 = 0;
		  while(std::getline(iss, token, ',')){
			  //sscanf(token.data(), "%s,%lf", colname, &temp_val);
			  if(cnt2 == 1){
				  temp_val = atof(token.c_str());
//				  std::cout << cnt2 << ":\t" << temp_val << "\n";
			  }
			  cnt2++;
		  }

		  switch(cnt){
			  case 0:
				  *min_release = temp_val;
				  break;
			  case 1:
				  *max_release = temp_val;
				  break;
			  case 2:
				  *min_volume = temp_val;
				  break;
			  case 3:
				  *max_volume = temp_val;
				  break;
			  case 4:
				  *basemonth_volume = temp_val;
				  break;
			  case 5:
	    		  //in the spreadsheet all values are double, so I convert useRegion (0 or 1) to (false or true)
				  *useRegion = ( temp_val == 0 ) ? false : true;
				  break;
			  //default:
			//	  std::cout << str <<" error" << str.data();
		  }
		  cnt++;
	  }
}

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

	std::cout << "days left " << daysleft << "\n";

	//Zen Add: 10/30/2015
	//The way to calculate a daily release (cfs)
	targetrelease = (volumediff/daysleft + curr_inflow*AFRATE)/AFRATE;

	//std::cout << "raw target release " << targetrelease << "\n";

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
	//std::cout << "dates" << tmcur.tm_year << tmcur.tm_mon <<tmcur.tm_mday << "\n";
	//std::cout << "dates" << tmnext.tm_year << tmnext.tm_mon <<tmnext.tm_mday << "\n";
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

    //std::cout << "diff days" << diff_days << "\n";
    return diff_days;
}
