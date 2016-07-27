#!/usr/bin/env python
from suds.xsd.doctor import Import, ImportDoctor
from suds.client import Client
from suds import WebFault

import csv

import pandas as pd
import numpy as np

#use for measuring the time for running a program
from datetime import datetime

#Start measuring time
start=datetime.now()

####################
## Change Dir Name #
####################
#directory for intermediate files
temp_dir = "C:/Users/Zen/Dropbox/Green River Basin Project/Source Codes/Python/4.20.2016_workshop_testcase/workshop/temp/"
######################


#Logging to see what the service is doing in case of no return data or errors
#import logging
#logging.basicConfig(level=logging.INFO)
#logging.getLogger('suds.client').setLevel(logging.DEBUG)

#Create a new object named HBGuest for calling the web service methods
url = 'http://www.dwr.state.co.us/HBGuest/HBGuestWebService.asmx?WSDL'
HBGuest = Client(url)#, doctor=ImportDoctor(imp))
#Manually create the authentication header
auth_header = HBGuest.factory.create('HBAuthenticationHeader')
auth_header.Token = 't6YNdmly'
auth_header.UserID = 0
HBGuest.set_options(soapheaders=auth_header)

#Parameters for GetHBGuestStructureResMeas
wdid = '4303202'
startDate = '01/01/1990'
endDate = '12/31/2015'

#Set division ID
divID = 6

distID = 58

wdidListRes = []
wdidListDitch = []

csvRes = temp_dir + 'Res_Div'+str(divID)+'Dist'+str(distID)+'_Structure_Data.csv'


#Load datd frame from csv
resDF_temp = pd.read_csv(csvRes)
resDF = resDF_temp.drop_duplicates('wdid')

#Initilize output data container
output_wdid = []
output_date_time = []
output_gage_height = []
output_storage_amt = []
output_fill_amt = []
output_release_amt = []
output_evap_loss_amt = []
#Contain WDID with no data
output_nodata_wdid = []

#Use for test
maxLoop = 9999999

cnt = 0

nonEmptyWdidListRes = []

#print ('1st Loop Start')
for wdid in resDF['wdid'].values:
    if(cnt > maxLoop):
        print("Size of data is too big")
        break
 #   print wdid

    try:
    #response = HBGuest.service.GetHBGuestStructureMonthlyTSByWDID(wdid, 1990, 2015)
        response = HBGuest.service.GetHBGuestStructureResMeas(wdid, startDate, endDate)

        #If a reservoir does not have response.StructureResMeas, skip it to avoid errors from null data
        if(not response):
            output_nodata_wdid.append(wdid)
            continue
        else:
            nonEmptyWdidListRes.append(wdid)

            for res in response.StructureResMeas:
                output_wdid.append(res.wdid)
                output_date_time.append(res.date_time)
                output_gage_height.append(res.gage_height)
                output_storage_amt.append(res.storage_amt)
                output_fill_amt.append(res.fill_amt)
                output_release_amt.append(res.release_amt)
                output_evap_loss_amt.append(res.evap_loss_amt)
    except WebFault, e:
        print e

    cnt = cnt + 1


#Search 10 data
cnt = 0

#Create a data frame
resDF = pd.DataFrame({'wdid':output_wdid,               #Seven digit combination of WD and ID; unique identifier to the structure
                    'date':output_date_time,            #Date and time of measurement, format = mm/dd/yyyy
                    'height':output_gage_height,        #Gage height measurement. (FEET)
                    'storage':output_storage_amt,       #Reservoir volume amount. (ACRE FEET)
                    'fill':output_fill_amt,             #Reservoir fill amount. (ACRE FEET)
                    'release':output_release_amt,       #Reservoir release amount. (ACRE FEET)
                    'evaploss':output_evap_loss_amt     #Reservoir volume amount of evaporative loss. (ACRE FEET)
                    })

#Output csv file
csvResMeas = temp_dir + 'Res_Div'+str(divID)+'Dist'+str(distID)+'_ResMeas_1990-2015.csv'
resDF.to_csv(csvResMeas,sep=',')

#print the time it took
print (datetime.now()-start)