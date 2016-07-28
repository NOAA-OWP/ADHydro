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
#wdid = '4503643'
startDate = '01/01/1990'
endDate = '09/30/2015'

#Change division IDs
divID = 6
distID = 58 #58 is used for the workshop

#Here we have the client, HBGuest, call a service.  Parameters to the service are passed as parameters to the function in order,
#Or if the parameter names of the service are known, they can be specified explicitly, i.e. (wdid, startDate = '08/01/1990')
try:
#    response = HBGuest.service.GetHBGuestStructureByDIV(divID)
    response = HBGuest.service.GetHBGuestStructureByWD(distID)

except (WebFault, e):
    print (e)

print("Connected\n")
"""
Now we can access the returned data by iterating over the array of results (if the results is an array structure)
Response holds values from the response, and lower levels can be accessec via a "dot" hiarchy: response.attribute.name
"""
#print response

cnt = 0

#WDID
wdidListRes = []
wdidListDitch = []

#Water Division
divListRes = []
divListDitch = []

#Water District
wdListRes = []
wdListDitch = []

#Name of structures
nameListRes = []
nameListDitch = []

#Type of structures
typeListRes = []
typeListDitch = []

#Status of structures
statusListRes = []
statusListDitch = []

#Water Source Name for structures
sourceListRes = []
sourceListDitch = []

#Decreed Amount (cfs)
dcr_rate_ListDitch = []
dcr_rate_ListRes = []

#Decreed Amount (AF)
dcr_vol_ListDitch = []
dcr_vol_ListRes = []

#Location Information of structures
latListDitch = []
longListDitch = []

latListRes = []
longListRes = []
longListRes = []

#Accuracy based on utm
utmAccuracyListRes = []

#Use for Debug
print("Before Loop")

#for res in response.GetHBGuestStructureByDIV:
for res in response.Structure:
    #District 47 is not included in UCRB
    #if(res.wd != 47):
    #we only collect the data in the District 58
    if(res.wd == distID):
        #strype == 1 represents ditches, and strtype == DS represents the ditch structures.
        if(res.strtype == '1' or res.strtype == 'DS' ):
#        if(res.str_type_desc == 'Ditch' and res.ciu == 'A'):
            wdidListDitch.append(res.wdid)              #WDID
            divListDitch.append(res.div)                #Div ID
            wdListDitch.append(res.wd)                  #District ID
            nameListDitch.append(res.str_name)          #Structure Name
            typeListDitch.append(res.strtype)           #Strucutre Type: 1 or DS
            statusListDitch.append(res.ciu)             #Active or Inactive
            sourceListDitch.append(res.strname)         #Water Source Name
            dcr_rate_ListDitch.append(res.dcr_rate_abs) #Decreed rate absolute. (CFS)
            dcr_vol_ListDitch.append(res.dcr_vol_abs)   #Decreed volume absolute. (AF)
            latListDitch.append(res.latdecdeg)              #Latitude (decimal degrees). (NAD83 datum)
            longListDitch.append(res.longdecdeg)            #Longitude (decimal degrees). (NAD83 datum)

        elif(res.strtype == '3' or res.strtype == 'RS'):
        #elif(res.str_type_desc == 'Reservoir'):
            wdidListRes.append(res.wdid)              #WDID
            divListRes.append(res.div)                #Div ID
            wdListRes.append(res.wd)                  #District ID
            nameListRes.append(res.str_name)          #Structure Name
            sourceListRes.append(res.strname)         #Water Source Name
            typeListRes.append(res.strtype)           #Strucutre Type: 3 or RS
            statusListRes.append(res.ciu)             #Active or Inactive
            dcr_rate_ListRes.append(res.dcr_rate_abs) #Decreed rate absolute. (CFS)
            dcr_vol_ListRes.append(res.dcr_vol_abs)   #Decreed volume absolute. (AF)
            latListRes.append(res.latdecdeg)          #Latitude (decimal degrees). (NAD83 datum)
            longListRes.append(res.longdecdeg)        #Longitude (decimal degrees). (NAD83 datum)
            utmAccuracyListRes.append(res.accuracy)          #Accuracy of UTM coordinates. Constrained by [ref_Location_Accuracy]

print("Data Frame")
#Create Data Frame
#Reservoir
resDF = pd.DataFrame({'wdid':wdidListRes,            #WDID
                    'div':divListRes,                #Div ID
                    'wd':wdListRes,                  #District ID
                    'strname':nameListRes,           #Structure Name
                    'strtype':typeListRes,           #Strucutre Type
                    'status':statusListRes,          #Current Status: Active or Inactive
                    'source':sourceListRes,          #Water Source Name
                    'dec_cfs':dcr_rate_ListRes,      #Decreed rate absolute. (CFS)
                    'dec_af':dcr_vol_ListRes,        #Decreed volume absolute. (AF)
                    'lat':latListRes,                #Latitude (decimal degrees). (NAD83 datum)
                    'long':longListRes,              #Longitude (decimal degrees). (NAD83 datum)
                    'utmAccuracy':utmAccuracyListRes #Accuracy of UTM coordinates. Constrained by [ref_Location_Accuracy]
                    })
#Dicth
ditchDF = pd.DataFrame({'wdid':wdidListDitch,         #WDID
                    'div':divListDitch,               #Div ID
                    'wd':wdListDitch,                 #District ID
                    'strname':nameListDitch,          #Structure Name
                    'strtype':typeListDitch,          #Strucutre Type
                    'status':statusListDitch,         #Current Status: Active or Inactive
                    'source':sourceListDitch,         #Water Source Name
                    'dec_cfs':dcr_rate_ListDitch,     #Decreed rate absolute. (CFS)
                    'dec_af':dcr_vol_ListDitch,       #Decreed volume absolute. (AF)
                    'lat':latListDitch,               #Latitude (decimal degrees). (NAD83 datum)
                    'long':longListDitch              #Longitude (decimal degrees). (NAD83 datum)
                    })

#Output File Name
#csvRes = outputDir + 'wdidInDiv6_Structure_All_Res_type_desc.csv'
ResOutput = temp_dir + 'Res_Div'+str(divID)+'Dist'+str(distID)+'_Structure_Data.csv'
DitchOutput = temp_dir + 'Ditch_Div'+str(divID)+'Dist'+str(distID)+'_Structure_Data.csv'

#Output to csv
resDF.to_csv(ResOutput,sep=',')
ditchDF.to_csv(DitchOutput,sep=',')

#print the time it took
print (datetime.now()-start)

