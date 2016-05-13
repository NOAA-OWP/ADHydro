#!/usr/bin/env python
#-------------------------------------------------------------------------------
# Name:        module1
# Purpose:
#
# Author:      yigarash
#
# Created:     23/09/2015
# Copyright:   (c) yigarash 2015
# Licence:     <your licence>
#-------------------------------------------------------------------------------
import csv

import pandas as pd
import numpy as np

#use for measuring the time for running a program
from datetime import datetime

#Adjust display width to auto detect terminal size...
pd.set_option("display.width", None)
pd.set_option("display.precision", 2)
#try and error
import sys
import os

#use for math.floor()
import math

#use for piece-wise interpolation
from scipy.interpolate import pchip
#use for piece-wise interpolation
from scipy import interpolate

import time

####################
## Change Dir Name #
####################
#directory for intermediate files
temp_dir = "C:/Users/Zen/Dropbox/Green River Basin Project/Source Codes/Python/4.20.2016_workshop_testcase/workshop/temp/"

#directory for static input files
input_dir = "C:/Users/Zen/Dropbox/Green River Basin Project/Source Codes/Python/4.20.2016_workshop_testcase/workshop/input/"
#direcotry for output
output_dir = "C:/Users/Zen/Dropbox/Green River Basin Project/Source Codes/Python/4.20.2016_workshop_testcase/workshop/output/"
#####################

#############
##Functions #
#############
def map_reach(s):
	name = int(s['Name'][3:])
	try:
		s['Reach'] = mapping[name]
		return s
	except (KeyError, e):
		s['Reach'] = pd.np.nan
		return s

def convert_AF_to_CM(val):
#acre-foot (acre-ft) 1.233x10^3 cubic meter
    return(1.233*1000*val)

#The unit of surface area from NHD is square-kilometer.
def convert_SKM_to_SM(val):
#square-kilometer = 10^6 sqaure-meter
    return(1000000*val)

def convert_CM_to_CMS(val):
    return(val/86400)

#Add Dist
def add_dist_header(val):
    return('Dist'+str(val))


#Start measuring time
start=datetime.now()

#Division ID
divID = 6
#District ID
distID = 58

#The base month's reservoir volume is used in the model
baseMonth = 6
#Minimum number of months in the historical data to create a monthly parameter
minimum_monthly_params = 6
#Minimum number of data to calculate the min/max release/volume
data_threshold_num = 5
#Minimum number of data for the base month
base_month_data_threshold = 1


#Reservoir historical data
csvResMeas = temp_dir + 'Res_Div'+str(divID)+'Dist'+str(distID)+'_ResMeas_1990-2015.csv'

resDF = pd.read_csv(csvResMeas)

resDF['year'] = pd.DatetimeIndex(resDF['date']).year
resDF['month'] = pd.DatetimeIndex(resDF['date']).month
resDF['day'] = pd.DatetimeIndex(resDF['date']).day
month_names = [None,'Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec']
#Slice the resDF to contain only relevant data for storage
resDF = resDF[['wdid','storage','release','month']]
StoragebyWDID = resDF.groupby('wdid')

#Create Data Frame for output
#names Name,Reach,Region,SubRegion,UseRegion,MinRelease,MaxRelease,MinVolume,MaxVolume,BaseVolume,Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec
cols = ['Reach','Region','SubRegion','UseRegion','MinRelease','MaxRelease','MinVolume','MaxVolume','BaseVolume']+month_names[1:]
allData_DF = pd.DataFrame(columns=cols,index=['Name']).dropna()
#Iterate through the groups, each one being the data for a single wdid
for wdid, data in StoragebyWDID:
    #reservoir name
    res_name = 'res'+str(wdid)

    #default value of useRegion = True
    #useRegion = "True"
    useRegion = 'True'

    #print wdid
    #print data
    #group the data for the wdid by month, aggregate storage by mean
    monthly = data.groupby('month').mean()
    #print data.groupby('month').count()
    #Check whether the June data is Empty, is 0 or less, or if not enough storage data exists on a montly basis
    #print "MONTHLY\n", monthly
    res_df = pd.DataFrame()
    #If data contains information for the base month
    #if data contains at least threshold number of basemonth observations
    #if at least mothly_param_threshold number of MONTHS exist
    #if base month average volume is not 0 or less
    if baseMonth in data['month'].values\
        and data.groupby(['month']).count().ix[baseMonth]['storage'] > base_month_data_threshold\
        and monthly['storage'].count() >= minimum_monthly_params\
        and monthly.loc[baseMonth]['storage'] > 0:
        base_month_vol = monthly.loc[baseMonth]['storage']
        #Slightly misleading name, res is created as a series, and is later upcasted to a dataframe
        res_df = pd.Series(index=pd.Index(range(1,13), type=pd.np.int, name='month'))
        res_df.ix[monthly.index] = monthly['storage']/base_month_vol
        #print "PERCENT DIFF\n", res_df
        useRegion = 'False'
        #do interpolation
        #Create the circular month index
#        res_df = res_df.reindex(range(baseMonth,13)+range(1,baseMonth))
        res_df = res_df.reindex(list(range(baseMonth,13))+list(range(1,baseMonth)))

        #let the 13th month be 1, the same value as the base_month
        res_df[13] = 1
        #Now reset the index so that the interpolation has a monotonic index to work with
        res_df = res_df.reset_index()
        #print "INTERPOLATE\n", res_df
        #Do the interpolation, then reset the index to the month again
        res_df = res_df.interpolate(method='pchip').set_index('month')
        #Don't need the mysterious 13th month anymore, drop it
        res_df.drop(13, inplace=True)
        #Reorder the index
        res_df = res_df.sort_index()
        #Add month names for each month
        res_df['month_name'] = map(lambda x: month_names[x], res_df.index)
        #print "INTERPOLATED\n", res_df
        #Now that names are present, reset the index, drop the month column and transpose the dataframe
        res_df = res_df.reset_index().drop('month', axis = 1).T
        #Assign the column names to be the month names then drop the month_name series
        res_df.columns = res_df.loc['month_name']
        res_df.drop('month_name', inplace=True)
        #Add the additional data to this data frame
        res_df['Name'] = res_name
        res_df['UseRegion'] = useRegion
        res_df['BaseVolume'] = base_month_vol
        #Set the index to the Name and then append to the global allData_DF
        res_df.set_index('Name', inplace=True)
        #TODO/FIXME Should the min/max be applied even if no base_month data is available?
        if data['storage'].count() > data_threshold_num:
            #initilization
            maxstorage = 0;
            minstorage = 0;

            maxstorage = data['storage'].max()
            minstorage = data['storage'].min()

            #NaN represents no data
            res_df['MinVolume'] = np.nan if minstorage < 0 else minstorage
            res_df['MaxVolume'] = np.nan if maxstorage < 0 else maxstorage
        else:
            #NaN represents no data
            res_df['MinVolume'] = np.nan
            res_df['MaxVolume'] = np.nan
            #print res_df
        if data['release'].count() > data_threshold_num:

            #initilization
            maxrelease = 0;
            minrelease = 0;

            maxrelease = data['release'].max()
            minrelease = data['release'].min()

            #NaN represents no data
            res_df['MinRelease'] = np.nan if minrelease < 0 else minrelease
            res_df['MaxRelease'] = np.nan if maxrelease < 0 else maxrelease
        else:
            #NaN represents no data
            res_df['MinRelease'] = np.nan
            res_df['MaxRelease'] = np.nan
            #print res_df
    else:
        #print wdid, " has no data for month ", baseMonth," or doesn't have enough montly data"
        res_df = res_df.append(pd.Series(name=res_name))
        #TODO/FIXME Temporary:  Use baseVolume as a possible scaling factor when regional level is nessicary
        if baseMonth in data['month'].values\
        and data.groupby(['month']).count().ix[baseMonth]['storage'] > base_month_data_threshold\
        and monthly.loc[baseMonth]['storage'] > 0:
            res_df['BaseVolume'] = monthly.loc[baseMonth]['storage']

    res_df['UseRegion'] = useRegion
    res_df['Region'] = 'Div'+str(divID)
    res_df['SubRegion'] = str(wdid)[0:2]
    #print(res_df)
    allData_DF = allData_DF.append(res_df)

#Map reach code from the wdid
csvMapfile = input_dir + 'reach_wdid_mapping.csv'

#csv that includes WDID and Reach
map_df = pd.read_csv(csvMapfile)

#copy res_df to res_Reach_df
#res_df includes all reservoirs that have historical data, and res_Reach_df is a subset of reservoirs found in NHD
res_Reach_df = allData_DF

#create a list from res_Reach_df.index
for wdid in res_Reach_df.index.tolist():
    #wdid includes "res" in front, so let's get rid of it
    wdid = int(wdid[3:])

    #Find WDIDs that are inlcluded in both map_df and res_Reach_df
    if wdid in map_df['WDID'].values:
        #Zen. I put [0] at the end of map_df['Reach'][map_df['WDID']==wdid].values[0]
        #because map_df['Reach'][map_df['WDID']==wdid.values] returns an array with one element
        res_Reach_df.ix[res_Reach_df.index.values == 'res'+str(wdid),'Reach'] = map_df['Reach'][map_df['WDID']==wdid].values[0]

#Export the all reservoirs
#Reorder the columns to look nice
res_Reach_df = res_Reach_df.reindex(columns=cols)
#print res_Reach_df
ind_count = res_Reach_df['Jan'].count()

#Export the reservoirs included in NHD
#res_NHD_df is a subset of res_Reach_df that has Reach codes
res_NHD_df = res_Reach_df
res_NHD_df.dropna(subset=['Reach'], inplace=True)
#Reorder the columns to look nice
cols_with_Name = ['Name','Reach','Region','SubRegion','UseRegion','MinRelease','MaxRelease','MinVolume','MaxVolume','BaseVolume']+month_names[1:]
#res_Reach_df['Name'] = res_Reach_df.index
res_NHD_df.reset_index(level=0, inplace=True)
res_NHD_df.rename(columns={'index':'Name'}, inplace=True)

res_NHD_df = res_NHD_df.reindex(columns=cols_with_Name)

#Output in a csv form
csvOutput = output_dir + 'res_data.csv'
res_NHD_df.to_csv(csvOutput,sep=',',index_label='Name')
#time.sleep(3)

##Originally, this is the beginning of the second script
mapping = pd.read_csv(os.path.join(input_dir,'reach_wdid_mapping.csv'))
monthly_parameters_df = res_NHD_df
#monthly_parameters_df = pd.read_csv(os.path.join(outputDir,'res_data.csv'))

#Convert units from AF to cubic-meter
monthly_parameters_df['MinRelease'] = monthly_parameters_df['MinRelease'].apply(lambda x: convert_AF_to_CM(x))
monthly_parameters_df['MaxRelease'] = monthly_parameters_df['MaxRelease'].apply(lambda x: convert_AF_to_CM(x))
monthly_parameters_df['MinVolume'] = monthly_parameters_df['MinVolume'].apply(lambda x: convert_AF_to_CM(x))
monthly_parameters_df['MaxVolume'] = monthly_parameters_df['MaxVolume'].apply(lambda x: convert_AF_to_CM(x))
monthly_parameters_df['BaseVolume'] = monthly_parameters_df['BaseVolume'].apply(lambda x: convert_AF_to_CM(x))

#Convert the units from m^3 to m^3/s
monthly_parameters_df['MinRelease'] = monthly_parameters_df['MinRelease'].apply(lambda x: convert_CM_to_CMS(x))
monthly_parameters_df['MaxRelease'] = monthly_parameters_df['MaxRelease'].apply(lambda x: convert_CM_to_CMS(x))

monthly_parameters_df['WDID'] = monthly_parameters_df['Name'].apply(lambda x: int(x[3:]))

#create a new column in mapping
maping_structure_df = mapping
# maping_with_general_info['CDSS_record'] is 1 if the reservoir has a historical record
maping_structure_df['CDSS_record'] = 0

for wdid in monthly_parameters_df['WDID']:
# Zen. to use "in" expression. The next data type of a variable after "in" must be an array or a set
    if wdid in mapping['WDID'].values:
        #insert 1 if the reservoir is found
        maping_structure_df['CDSS_record'][mapping.WDID==wdid] = 1
        #res_status[cnt] = 1

## Add surface sizes to maping_structure_df by Reach Code
surface_size_df = pd.read_csv(os.path.join(input_dir,'res_Div6_surface_area.csv'))

#Convert the unit from square-kilometer to square-meter
surface_size_df['AreaSqKm'] = surface_size_df['AreaSqKm'].apply(lambda x: convert_SKM_to_SM(x))

#Drop columns that are not needed in surface_size_df
surface_size_df.drop('Shape_Leng', axis=1, inplace=True)
surface_size_df.drop('Shape_Area', axis=1, inplace=True)
surface_size_df.drop('Elevation', axis=1, inplace=True)

#Merge two data frames
surface_reach_df = maping_structure_df
surface_reach_df['Surface_Area'] = None

for reach in surface_size_df['Reach']:
    if wdid in mapping['WDID'].values:
        surface_reach_df.ix[surface_reach_df.Reach==reach,'Surface_Area'] = surface_size_df.AreaSqKm[surface_size_df.Reach == reach].values[0]

#Create district IDs from WDID
monthly_parameters_df['Name'].apply(lambda x: int(x[3:]))
surface_reach_df['SubRegion'] = surface_reach_df['WDID'].apply(lambda x: math.floor(x/100000))

#show the number of reservoirs by district
#Count data
surface_reach_df.groupby(['SubRegion','CDSS_record']).count()

#Mean
#Modify the datatype to float because the data type in surface area is not numeric
surface_reach_df['Surface_Area']=surface_reach_df['Surface_Area'].astype(float)

surface_reach_df.groupby(['SubRegion']).mean()

#Mean value for Surface_Area
surface_reach_df.groupby(['SubRegion'])['Surface_Area'].mean()
mean_surface_area = surface_reach_df.groupby(['SubRegion'])['Surface_Area'].mean()


#Calculate average [BaseVolume] of reservoirs by [subRegion] using groupby
mean_base_volume = monthly_parameters_df.groupby(['SubRegion'])['BaseVolume'].mean()
monthly_parameters_df.groupby(['SubRegion'])['BaseVolume'].count()

#mean_rate_vol_per_area = mean_monthly_parameters/surface_area_mean_by_WD
#Calculate the ratio Basevolume (AF/square-kilometer)
#If [UseRegion] == True, the value in the row is calculated the mean in the division level.
#Volume_per_Area = Depth (meter)
d_rate = {'SubRegion':pd.Series(mean_surface_area.index.values),'UseRegion':False,'Volume_per_Area':0}
volume_per_area_df = pd.DataFrame(data=d_rate)

##FIX ME: Use apply for efficiency
for subRegion in mean_surface_area.index:
    #subRegion = subRegion.astype(int)
    subRegion = int(subRegion)

#    volume_per_area.insert(mean_base_volume[subRegion]/mean_surface_area[subRegion],1)
    try:
        volume_per_area_df.ix[volume_per_area_df['SubRegion']==int(subRegion),'Volume_per_Area'] =(mean_base_volume[str(subRegion)]/mean_surface_area[int(subRegion)])
    except:
        #volume_per_area_df.ix[volume_per_area_df['SubRegion']==subRegion,'Volume_per_Area'] = -1
        #calculate the average of all SubRegions
        volume_per_area_df.ix[volume_per_area_df['SubRegion']==int(subRegion),'Volume_per_Area'] = mean_base_volume.mean()/mean_surface_area.mean()
        volume_per_area_df.ix[volume_per_area_df['SubRegion']==int(subRegion),'UseRegion'] = True

#Concatinate reservoirs that do not have historical data
#Add a new column, [Surface_Area]
monthly_parameters_df['Surface_Area'] = 0

for reach in surface_reach_df['Reach']:
    if reach in monthly_parameters_df['Reach'].values:
        monthly_parameters_df.ix[monthly_parameters_df['Reach']==reach,'Surface_Area'] = surface_reach_df.ix[surface_reach_df['Reach']==reach,'Surface_Area'].values[0]
    #Add data that does not have historical data
    else:
        wdid = surface_reach_df.ix[surface_reach_df['Reach']==reach,'WDID'].values[0]
        name = 'res' + str(wdid)
        subRegion = surface_reach_df.ix[surface_reach_df['Reach']==reach,'SubRegion'].values[0]

        surfaceArea = surface_reach_df.ix[surface_reach_df['Reach']==reach,'Surface_Area'].values[0]
        #All values missing are -1
        monthly_parameters_df.loc[len(monthly_parameters_df)] = [name,reach,'Div6',subRegion.astype(int),'True',-1,-1,-1,-1,np.nan,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,wdid.astype(int),surfaceArea]

#Extrapolate the BaseVolume in the dataest from res_data.csv, which is monthly_parameters_df
for reach in monthly_parameters_df['Reach']:
    ##FIXME: I used ".values[0]" to extract the value from the array.
    basevol = monthly_parameters_df[monthly_parameters_df['Reach'] == reach]['BaseVolume'].values[0]
    subRegion = monthly_parameters_df.ix[monthly_parameters_df['Reach']==reach,'SubRegion'].values[0]
    if(np.isnan(basevol)):
        #Find SubRegion, and multiply the surface_area by the average depth
        monthly_parameters_df.ix[monthly_parameters_df['Reach']==reach,'BaseVolume'] = \
            monthly_parameters_df.ix[monthly_parameters_df['Reach']==reach,'Surface_Area'].values[0] *\
            volume_per_area_df.ix[volume_per_area_df['SubRegion']==int(subRegion),'Volume_per_Area'].values[0]

#Reindex dataframe
monthly_parameters_df = monthly_parameters_df.reindex(columns=cols_with_Name)

#Add District surfix
monthly_parameters_df['SubRegion'] = monthly_parameters_df['SubRegion'].apply(lambda x: add_dist_header(x))

#Only Dist 58
monthly_parameters_df = monthly_parameters_df.ix[monthly_parameters_df['SubRegion']=='Dist'+str(distID),]

#fill -1 into the missing data
monthly_parameters_df = monthly_parameters_df.fillna(-1)

csvOutput = output_dir + 'res_data.csv'

monthly_parameters_df.to_csv(csvOutput,header=True, index = False)


###################
# Disctrict Level #
###################
#Initialize the dataframe to contain the results for district level
district_cols = ['UseRegion','BaseVolume','Region','MinRelease','MaxRelease','MinVolume','MaxVolume']+month_names[1:]

district_Data_DF = pd.DataFrame(columns=district_cols,index=['Name']).dropna()

#Reuse allData_DF that is used in the individual level to calculate the average values in the ditrict level
allData_float_DF = allData_DF

#change the data types of columns because we cannot calculate mean when the type of columns are object
month_name_list = ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec']
general_info_name_list = ['BaseVolume','MinRelease','MaxRelease','MinVolume','MaxVolume']

for month_name in month_name_list:
    allData_float_DF[month_name] = allData_float_DF[month_name].astype('float64')

for general_info_name in general_info_name_list:
    allData_float_DF[general_info_name] = allData_float_DF[general_info_name].astype('float64')

data_distID = allData_float_DF.groupby('SubRegion').mean()

#Check the number of data in districts.
#If the nuymber of data is less than the district level threshold value (ex. 5), then set useRegion as False
district_data_threshold_num = 10

district_monthly_vol_change = data_distID.loc[:,'Jan':'Dec']

#create a mask to filter data less than district_data_threshold_num
num_data_by_subRegion = allData_float_DF.groupby('SubRegion').count()

#Zen.TODO: To understand .loc, read http://pandas.pydata.org/pandas-docs/stable/indexing.html#indexing-view-versus-copy
threshold_mask = num_data_by_subRegion.loc[:,'Jan':'Dec'] > district_data_threshold_num

#From threshold_mask, the positions that have False turn to NaN
district_monthly_vol_change = district_monthly_vol_change.loc[:,'Jan':'Dec'][threshold_mask]

#update data_distID by district_monthly_vol_change
data_distID.loc[:,'Jan':'Dec'] = district_monthly_vol_change

#Append the avarage value of districts to districtData_DF
district_Data_DF = district_Data_DF.append(data_distID)
#After append values, the order of dataframe becomes A-Z
#To solve this, reindex in the following way.
district_Data_DF = district_Data_DF.reindex_axis(district_cols,axis=1)

for distID in district_monthly_vol_change.index.get_values():
    #Count the number of data of monthly volume change other than NaN
    #and, if the number of data is less than minimum_monthly_params
    #then, set useRegion to False
    if(district_monthly_vol_change.loc[distID].count() > minimum_monthly_params):
#        district_Data_DF.loc[distID,'useRegion'] = False
        district_Data_DF.loc[distID,'UseRegion'] = True
    else:
#        district_Data_DF.loc[distID,'useRegion'] = True
        district_Data_DF.loc[distID,'UseRegion'] = False

##Add district Names
#Set division ID
district_Data_DF['Region'] = 'Div'+str(divID)
dist_name = pd.DataFrame({'Name':('Dist'+district_Data_DF.index.get_values())})
#Reset index of district_Data_DF
district_Data_DF = district_Data_DF.reset_index()
#Add column of DistID
district_Data_DF = dist_name.join(district_Data_DF)
#Delete the 'index' column created by the df.resent_index() command
district_Data_DF.drop('index', axis=1,inplace=True)

for dist_index in range(0,len(district_Data_DF)):
    #initilize the array
    dist_df = pd.Series(index=pd.Index(range(1,13), type=pd.np.int, name='month'))

    #Substitute the values from district_Data_DF
    dist_df.loc[1:12] = district_Data_DF.loc[dist_index,'Jan':'Dec'].get_values()

    #Create the circular month index
    dist_df = dist_df.reindex(range(baseMonth,13)+range(1,baseMonth))
    #let the 13th month be 1, the same value as the base_month
    dist_df[13] = 1
    #Now reset the index so that the interpolation has a monotonic index to work with
    dist_df = dist_df.reset_index()

    #interpolation
    dist_df = dist_df.interpolate(method='pchip').set_index('month')

    #Don't need the mysterious 13th month anymore, drop it
    dist_df.drop(13, inplace=True)
    #Reorder the index
    dist_df = dist_df.sort_index()

    #TODO: I want to return dist_df.get_values() to district_Data_DF.loc[dist_index,'Jan':'Dec']
    #but, the array dist_df.get_values() includes brackets in side the array
    #Ad-hoc to remove [] in the array
    value_return = [l[0] for l in dist_df.get_values()]

    #return the interpolated value to the dataframe
    district_Data_DF.loc[dist_index,'Jan':'Dec'] = value_return

district_Data_DF['SubRegion'] = True


#Convert units from AF to cubic-meter
district_Data_DF['MinRelease'] = district_Data_DF['MinRelease'].apply(lambda x: convert_AF_to_CM(x))
district_Data_DF['MaxRelease'] = district_Data_DF['MaxRelease'].apply(lambda x: convert_AF_to_CM(x))
district_Data_DF['MinVolume'] = district_Data_DF['MinVolume'].apply(lambda x: convert_AF_to_CM(x))
district_Data_DF['MaxVolume'] = district_Data_DF['MaxVolume'].apply(lambda x: convert_AF_to_CM(x))
district_Data_DF['BaseVolume'] = district_Data_DF['BaseVolume'].apply(lambda x: convert_AF_to_CM(x))

#Convert the units from m^3 to m^3/s
district_Data_DF['MinRelease'] = district_Data_DF['MinRelease'].apply(lambda x: convert_CM_to_CMS(x))
district_Data_DF['MaxRelease'] = district_Data_DF['MaxRelease'].apply(lambda x: convert_CM_to_CMS(x))


##################
##Divisoin Level #
##################
#average of all districts
div_Data = district_Data_DF.groupby('Name').mean()
div_Data['Name'] = 'Div6'
div_Data['SubRegion'] = False
div_Data['UseRegion'] = False
div_Data['Region'] = 'Div6'

district_Data_DF = district_Data_DF.append(div_Data)

#reindex
region_cols = ['Name','SubRegion','UseRegion','Region','MinRelease','MaxRelease','MinVolume','MaxVolume','BaseVolume']+month_names[1:]
district_Data_DF = district_Data_DF.reindex(columns=region_cols)

#Output in a csv form
csvOutput = output_dir + 'region_data.csv'
district_Data_DF.to_csv(csvOutput,sep=',', index = False)


#print the time it took
print (datetime.now()-start)
