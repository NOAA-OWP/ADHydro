#!/usr/bin/python

import pandas as pd
import numpy as np

import os
import sys

#use for measuring the time for running a program
from datetime import datetime
#Start measuring time
start=datetime.now()

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

############
## Functions
############
def convert_CFS_to_CMS(val):
#acre-foot (acre-ft) 1.233x10^3 cubic meter
    return(0.0283*val)

#The format of apro_date is mm/dd/yyyy. I convert this to yyyymmdd
def convert_date_format(val):
    date = val.split('/')
    if(len(date[0]) == 1):
        month = str(0) + str(date[0])
    else:
        month = str(date[0])
    if(len(date[1]) == 1):
        day = str(0) + str(date[1])
    else:
        day = str(date[1])
    year = str(date[2])

    return (int(year+month+day))

def enumerate_croptype(val):
    crop_type = 0
    if(val == 'GRASS_PASTURE'):
        crop_type = 1
    elif(val == 'ALFALFA'):
        crop_type = 2
    elif(val == 'BLUEGRASS'):
        crop_type = 3
    elif(val == 'SMALL_GRAINS'):
        crop_type = 4
    elif(val == 'ORCHARD_WO_COVER'):
        crop_type = 5
    elif(val == 'ORCHARD_WITH_COVER'):
        crop_type = 6
    elif(val == 'UNKOWN'):
        crop_type = 98
    else:
        #Other
        crop_type = 99
    return crop_type

def enumerate_irrigtype(val):
    irrig_type = 0
    if(val == 'FLOOD'):
        irrig_type = 1
    elif(val == 'FURROW'):
        irrig_type = 2
    elif(val == 'SPRINKLER'):
        irrig_type = 3
    elif(val == 'DRIP'):
        irrig_type = 4
    elif(val == 'GRATE_PIPE'):
        irrig_type = 5
    elif(val == 'UNNOWN'):
        irrig_type = 98
    else:
        #Other
        irrig_type = 99
    return irrig_type


#Div and Dist IDs
divID = 6
distID = 58

#location of working directory
dir_loc = "C:/Users/Zen/Dropbox/Green River Basin Project/Source Codes/Python/4.20.2016_workshop_testcase/irrigation/input/"

#GIS product for irrigated lands in 2010
irrg_gis_2010_raw_df = pd.read_csv(os.path.join(input_dir,'irrigated_lands_in_Div6_2010.csv'))

#Useful columns
selected_column_names = ['CAL_YEAR','DIV','DISTRICT','PARCEL_ID','CROP_TYPE','IRRIG_TYPE','Shape_Area','SW_WDID1','SW_WDID2','SW_WDID3','SW_WDID4']

#Dataframe
irrg_gis_2010_df = irrg_gis_2010_raw_df[selected_column_names]

#Drop the data that is not included in the Yampa River Basin
#irrg_gis_2010_Yampa_df = irrg_gis_2010_df[(irrg_gis_2010_df.DISTRICT != 43) & (irrg_gis_2010_df.DISTRICT != 47) & (irrg_gis_2010_df.DISTRICT != 56)]
#Drop the data other than district 58
irrg_gis_2010_Yampa_df = irrg_gis_2010_df[(irrg_gis_2010_df.DISTRICT == 58)]


##FixMe: I should use enumerate(), but I haven't figured out a right way to use it.
#Initilization: Substitute numbers in irrigation type and crop type in irrg_gis_2010_Yampa_df
#'CROP_TYPE'
irrg_gis_2010_Yampa_df.loc[:,'CROP_TYPE'] = irrg_gis_2010_Yampa_df['CROP_TYPE'].apply(lambda x: enumerate_croptype(x))
#'IRRIG_TYPE'
irrg_gis_2010_Yampa_df.loc[:,'IRRIG_TYPE'] = irrg_gis_2010_Yampa_df['IRRIG_TYPE'].apply(lambda x: enumerate_irrigtype(x))

#sory by shape_area
irrg_gis_2010_Yampa_df = irrg_gis_2010_Yampa_df.sort(['Shape_Area'], ascending=0)

#print(irrg_gis_2010_Yampa_df.head())

#### FOR TEST: FIX ME after the test-run
#irrg_gis_2010_Yampa_df = irrg_gis_2010_Yampa_top_5_df

#print_col = ['PARCEL_ID','Shape_Area','SW_WDID1','SW_WDID2','SW_WDID3','SW_WDID4']
#Find how many lands are getting water from the same SW_WDID
#Initilization
irrg_gis_2010_Yampa_df.loc[:,'SW_WDID1_DIV_RATIO'] = np.nan
irrg_gis_2010_Yampa_df.loc[:,'SW_WDID2_DIV_RATIO'] = np.nan
irrg_gis_2010_Yampa_df.loc[:,'SW_WDID3_DIV_RATIO'] = np.nan
irrg_gis_2010_Yampa_df.loc[:,'SW_WDID4_DIV_RATIO'] = np.nan

#The maximum number of diversion diverting water to an irrigated land in Div 6 is 4
max_div_cnt = 4

for parcel in irrg_gis_2010_Yampa_df['PARCEL_ID']:
    #print('parcel',parcel)
    for div_cnt in range (1,max_div_cnt+1):
        #If SW_WDID == np.nan, then move on to the next parcel
        if(np.isnan(irrg_gis_2010_Yampa_df.ix[irrg_gis_2010_Yampa_df['PARCEL_ID']==parcel,'SW_WDID'+str(div_cnt)].values[0])):
            #print('break')
            break
        else:
            wdid = irrg_gis_2010_Yampa_df.ix[irrg_gis_2010_Yampa_df['PARCEL_ID']==parcel,'SW_WDID'+str(div_cnt)].values[0]
            index = irrg_gis_2010_Yampa_df[irrg_gis_2010_Yampa_df['PARCEL_ID']==parcel].index.values[0]
            #print("\nPARCEL_ID="+str(parcel)+' gets diverted from WDID='+str(wdid))

            #Create a DF that includes the parcels diverted from wdid
            temp_df = irrg_gis_2010_Yampa_df[(irrg_gis_2010_Yampa_df['SW_WDID1']==wdid) | (irrg_gis_2010_Yampa_df['SW_WDID2']==wdid) | (irrg_gis_2010_Yampa_df['SW_WDID3']==wdid) | (irrg_gis_2010_Yampa_df['SW_WDID4']==wdid)]
            #print(temp_df[print_col])

            #Substitute the ratio for the corresponding parcel
            parcel_div_ratio = (temp_df['Shape_Area']/temp_df['Shape_Area'].sum(axis=0)).loc[index]
            irrg_gis_2010_Yampa_df.ix[irrg_gis_2010_Yampa_df['PARCEL_ID']==parcel,'SW_WDID'+str(div_cnt)+'_DIV_RATIO'] = parcel_div_ratio

print_col2 = ['PARCEL_ID','SW_WDID1','SW_WDID2','SW_WDID1_DIV_RATIO','SW_WDID2_DIV_RATIO']
#print(irrg_gis_2010_Yampa_df[print_col2])

#Get decreed amount from net_amts table
net_amts_raw_df = pd.read_csv(os.path.join(temp_dir,'Ditch_Div'+str(divID)+'Dist'+str(distID)+'_Net_Amts_Data.csv'))

#Select the data in the yampa river basin
#net_amts_yampa_df = net_amts_raw_df[(net_amts_raw_df.wd !=43) & (net_amts_raw_df.wd !=47) & (net_amts_raw_df.wd !=56)]

#Select the data in the District 58
net_amts_yampa_df = net_amts_raw_df[net_amts_raw_df.wd == 58]


#In the for loop below I do two things:
#1. Standadize the decreed amount in AF to cfs using 1 cfs for a day = 1.9835 AF
#2. Eliminate the entry if the type_use does not include "1" = irrigation
cfs_to_af_rate = 1.9835
use_type_irrig = "1"
for index in net_amts_yampa_df.index.values:
    #drop the entry from the dataframe if use_type does not include irrigation
    #4.3.2016. I used isinstance(), because it seems np.isnan could not solve the problem when the argument is a string.
    if(isinstance(net_amts_yampa_df.ix[net_amts_yampa_df.index==index,'use_type'].values[0],str)):
        use_type_one_by_one = list(net_amts_yampa_df.ix[net_amts_yampa_df.index==index,'use_type'].values[0])
        if (not use_type_irrig in use_type_one_by_one):
            net_amts_yampa_df.drop(index, inplace=True)
            continue

    #Standadize the decreed amount in AF to cfs using 1 cfs for a day = 1.9835 AF
    if(np.isnan(net_amts_yampa_df.ix[net_amts_yampa_df.index==index,'dec_cfs'].values[0]) and not(np.isnan(net_amts_yampa_df.ix[net_amts_yampa_df.index==index,'dec_af'].values[0]))):
        net_amts_yampa_df.ix[net_amts_yampa_df.index==index,'dec_cfs'] = net_amts_yampa_df.ix[net_amts_yampa_df.index==index,'dec_af']/cfs_to_af_rate

#Group by wdid
net_amts_group_by_wdid_df = net_amts_yampa_df.groupby(['wdid'])

#Sum up the decreed amount because all rows in net_amts_yampa_df include "1"=irrigation use
net_amts_group_by_wdid_df.sum()

#Use the oldest decreed amount
dec_amts_array = net_amts_group_by_wdid_df.sum()['dec_cfs']

#Create a data for dateframe
dec_data = {'total_dec_rate': pd.Series(dec_amts_array).values}
water_right_div_df = pd.DataFrame(data=dec_data, index=pd.Series(dec_amts_array).index)

#convert the unit decreed amount from cfs to cms
water_right_div_df['total_dec_rate'] = water_right_div_df['total_dec_rate'].apply(lambda x: convert_CFS_to_CMS(x))

#initilization
water_right_div_df['apro_date'] = np.nan

#net_amts_yampa_df.dtypes

for wdid in net_amts_group_by_wdid_df['wdid']:
    wdid = wdid[0]
    #Because the water right data in CDSS is stored from the olded to newest, stores the
    water_right_div_df.ix[water_right_div_df.index==wdid,'apro_date'] = str(net_amts_yampa_df.ix[net_amts_yampa_df.wdid==wdid,'apro_date'].values[0])

#The format of apro_date is mm/dd/yyyy. I convert this to yyyymmdd
water_right_div_df['apro_date'] = water_right_div_df['apro_date'].apply(lambda x: convert_date_format(x))

#Calculate the decreed amount for parcels using water_right_div_df['total_dec_amt']
#Initilization
irrg_gis_2010_Yampa_df.loc[:,'SW_WDID1_dec_rate'] = np.nan
irrg_gis_2010_Yampa_df.loc[:,'SW_WDID2_dec_rate'] = np.nan
irrg_gis_2010_Yampa_df.loc[:,'SW_WDID3_dec_rate'] = np.nan
irrg_gis_2010_Yampa_df.loc[:,'SW_WDID4_dec_rate'] = np.nan

#err_wdid
err_widi_net_amt = []

for parcel in irrg_gis_2010_Yampa_df['PARCEL_ID']:
    for div_cnt in range (1,max_div_cnt+1):
        #If SW_WDID == np.nan, then move on to the next parcel
        if(np.isnan(irrg_gis_2010_Yampa_df.ix[irrg_gis_2010_Yampa_df['PARCEL_ID']==parcel,'SW_WDID'+str(div_cnt)].values[0])):
            #print('break')
            break
        else:
            wdid = irrg_gis_2010_Yampa_df.ix[irrg_gis_2010_Yampa_df['PARCEL_ID']==parcel,'SW_WDID'+str(div_cnt)].values[0]
            try:
            #Calculate the decreed rate for a parcel
                parcel_dec_rate = irrg_gis_2010_Yampa_df.loc[irrg_gis_2010_Yampa_df['PARCEL_ID']==parcel,'SW_WDID'+str(div_cnt)+'_DIV_RATIO'].values[0]*\
                water_right_div_df.ix[water_right_div_df.index==wdid,'total_dec_rate'].values[0]
                irrg_gis_2010_Yampa_df.loc[irrg_gis_2010_Yampa_df['PARCEL_ID']==parcel,'SW_WDID'+str(div_cnt)+'_dec_rate'] = parcel_dec_rate
            except:
                #net_amts table may not have a widi in the shape file of the irrigated lands
                err_widi_net_amt.append(wdid)
                ##FIXME: I temporary substitute 0 in the someirrigated lands
                #76/1439 = 0.052 (5.2%) of wdids are missing
#                irrg_gis_2010_Yampa_df.loc[irrg_gis_2010_Yampa_df['PARCEL_ID']==parcel,'SW_WDID'+str(div_cnt)+'_dec_rate'] = np.nan
                irrg_gis_2010_Yampa_df.loc[irrg_gis_2010_Yampa_df['PARCEL_ID']==parcel,'SW_WDID'+str(div_cnt)+'_dec_rate'] = 0

#Copy Data frame
irrg_gis_2010_Yampa_final_df = irrg_gis_2010_Yampa_df

#Fill -1 in places with NaN
irrg_gis_2010_Yampa_final_df.fillna(value=-1,axis=1,inplace=True)

#Convert the format of decreed rates from different sources
irrg_gis_2010_Yampa_final_df['DecreedRate'] = np.nan
irrg_gis_2010_Yampa_final_df['ParcelRatioToDiv'] = np.nan

for parcel in irrg_gis_2010_Yampa_df['PARCEL_ID']:
    #print(parcel)
    #Select a part of data frame for the corresponding parcel ID
    irrg_df = irrg_gis_2010_Yampa_df[irrg_gis_2010_Yampa_df['PARCEL_ID']==parcel]

    str_decreed = ""
    str_alloc_ratio = ""

    #The range of i is between 1 and 4 using range(1,5) because the maximum number of diversions feeding to one parcel is 4
    for i in range(1,5):
        source_num = 'SW_WDID'+str(int(i))
        dec_rate = source_num+'_dec_rate'
        alloc_ratio = source_num+'_DIV_RATIO'

        if(irrg_df[source_num].values[0] > 0):
            if(i == 1):
                str_decreed = ""+str(int(irrg_df[source_num].values[0]))+"="+str(irrg_df[dec_rate].values[0])+""
                str_alloc_ratio = ""+str(int(irrg_df[source_num].values[0]))+"="+str(irrg_df[alloc_ratio].values[0])+""
            else:
                str_decreed = str_decreed +";"+ str(int(irrg_df[source_num].values[0]))+"="+str(irrg_df[dec_rate].values[0])+""
                str_alloc_ratio = str_alloc_ratio +";"+ str(int(irrg_df[source_num].values[0]))+"="+str(irrg_df[alloc_ratio].values[0])+""
        else:
            break
        #print(str_decreed)

    irrg_gis_2010_Yampa_final_df.ix[irrg_gis_2010_Yampa_df['PARCEL_ID']==parcel,'DecreedRate'] = str_decreed
    irrg_gis_2010_Yampa_final_df.ix[irrg_gis_2010_Yampa_df['PARCEL_ID']==parcel,'ParcelRatioToDiv'] = str_alloc_ratio

#Drop columns whose are no longer needed
for i in range(1,5):
        source_num = 'SW_WDID'+str(int(i))
        dec_rate = source_num+'_dec_rate'
        alloc_ratio = source_num+'_DIV_RATIO'
        #Drop SW_WDID1
        irrg_gis_2010_Yampa_final_df.drop(source_num,axis=1,inplace=1)
        irrg_gis_2010_Yampa_final_df.drop(dec_rate,axis=1,inplace=1)
        irrg_gis_2010_Yampa_final_df.drop(alloc_ratio,axis=1,inplace=1)

#Change ParcelID to Name
irrg_gis_2010_Yampa_final_df['Name'] = irrg_gis_2010_Yampa_final_df['PARCEL_ID'].apply(lambda x:'par'+str(x))

#Drop ParcelID
irrg_gis_2010_Yampa_final_df.drop('PARCEL_ID',axis=1,inplace=1)

#Rename and Reorder columns
col_irrig_names = ['CalYear','Region','SubRegion','CropType','IrrigType','LandSize','DecreedRate','ParcelRatioToDiv','Name']
irrg_gis_2010_Yampa_final_df.columns = col_irrig_names

col_irrig_ordered_names = ['Name','CalYear','Region','SubRegion','CropType','IrrigType','LandSize','DecreedRate','ParcelRatioToDiv']
irrg_gis_2010_Yampa_final_df = irrg_gis_2010_Yampa_final_df.reindex(columns=col_irrig_ordered_names)

#add suffix like "Div" and "Dist" in front of the division ID and district ID
irrg_gis_2010_Yampa_final_df['Region'] = irrg_gis_2010_Yampa_final_df['Region'].apply(lambda x:'Div'+str(x))
irrg_gis_2010_Yampa_final_df['SubRegion'] = irrg_gis_2010_Yampa_final_df['SubRegion'].apply(lambda x:'Dist'+str(x))

irrg_gis_2010_Yampa_final_df = irrg_gis_2010_Yampa_final_df.reset_index()
irrg_gis_2010_Yampa_final_df.drop('index',axis=1,inplace=1)

irrg_gis_2010_Yampa_final_df.head()

#Prepare for Diverstion Output
#water_right_div_df['total_dec_rate'] = water_right_div_df['total_dec_rate'].apply(lambda x: convert_CFS_to_CMS(x))
#Total ReqestAmt
div_2010_Yampa_final_df = water_right_div_df
div_2010_Yampa_final_df['TotalReqRate'] = div_2010_Yampa_final_df['total_dec_rate']

#Fill -1 in places with NaN
div_2010_Yampa_final_df.fillna(value=-1,axis=1,inplace=True)

#Rename and Reorder columns
div_2010_Yampa_final_df['Name'] = div_2010_Yampa_final_df.index
div_2010_Yampa_final_df['Name'] = div_2010_Yampa_final_df['Name'].apply(lambda x:'div'+str(x))

col_div_names = ['TotalDecRate','ApproDate','TotalReqRate','Name']
div_2010_Yampa_final_df.columns = col_div_names

col_div_ordered_names = ['Name','TotalDecRate','ApproDate','TotalReqRate']
div_2010_Yampa_final_df = div_2010_Yampa_final_df.reindex(columns=col_div_ordered_names)
div_2010_Yampa_final_df = div_2010_Yampa_final_df.reset_index()
div_2010_Yampa_final_df.drop('wdid',axis=1,inplace=1)

#Output
#For Irrigated Land
irrg_gis_2010_Yampa_final_df.to_csv(os.path.join(output_dir,'irrig_data.csv'),header=True,index=False)
#For Diversion
div_2010_Yampa_final_df.to_csv(os.path.join(output_dir,'div_data.csv'),header=True,index=False)

#print the time it took
print (datetime.now()-start)