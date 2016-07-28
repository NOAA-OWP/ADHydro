#updated: Jan. 31, 2016

#!/usr/bin/env python
from suds.xsd.doctor import Import, ImportDoctor
from suds.client import Client
from suds import WebFault

import csv

import pandas as pd
import numpy as np
import os

#use for measuring the time for running a program
from datetime import datetime

#Start measuring time
start=datetime.now()
#Logging to see what the service is doing in case of no return data or errors
#import logging
#logging.basicConfig(level=logging.INFO)
#logging.getLogger('suds.client').setLevel(logging.DEBUG)

####################
## Change Dir Name #
####################
#directory for intermediate files
temp_dir = "C:/Users/Zen/Dropbox/Green River Basin Project/Source Codes/Python/4.20.2016_workshop_testcase/workshop/temp/"
######################

#Create a new object named HBGuest for calling the web service methods
url = 'http://www.dwr.state.co.us/HBGuest/HBGuestWebService.asmx?WSDL'
HBGuest = Client(url)#, doctor=ImportDoctor(imp))
#Manually create the authentication header
auth_header = HBGuest.factory.create('HBAuthenticationHeader')
auth_header.Token = 't6YNdmly'
auth_header.UserID = 0
HBGuest.set_options(soapheaders=auth_header)

#Change division IDs
divID = 6
distID = 58

#Here we have the client, HBGuest, call a service.  Parameters to the service are passed as parameters to the function in order,
#Or if the parameter names of the service are known, they can be specified explicitly, i.e. (wdid, startDate = '08/01/1990')
try:
#    response = HBGuest.service.GetHBGuestNetAmtsByDIV(divID)
    response = HBGuest.service.GetHBGuestNetAmtsByWD(distID)

except WebFault as e:
    print(e)

"""
Now we can access the returned data by iterating over the array of results (if the results is an array structure)
Response holds values from the response, and lower levels can be accessec via a "dot" hiarchy: response.attribute.name
"""
cnt = 0

#WDID
wdid_List = []
wd_List = []
#Water Uses (irrig use = 1)
use_List = []

#decreed amount
rate_amt_List = []
vol_amt_List = []

#Appropriation date
apro_date_List = []

#Structure type
strtype_List = []

#for res in response.GetHBGuestStructureByDIV:
for res in response.NetAmts:
    #if(res.wd != 47):
    if(res.wd == distID):

#choose only active ditch systems
#        if(res.str_type_desc == 'Ditch'):
        wdid_List.append(res.wdid)                  #WDID
        wd_List.append(res.wd)
        use_List.append(res.use_types)              #Water Uses (irrig use = 1)
        rate_amt_List.append(res.net_rate_abs)      #Decreed amount (cfs)
        vol_amt_List.append(res.net_vol_abs)        #Decreed amount (af)
        apro_date_List.append(res.apro_date)        #Structure Name
        strtype_List.append(res.strtype)            #Strucutre Type: 1 or DS

#Dicth
ditchDF = pd.DataFrame({'wdid':wdid_List,           #WDID
                    'wd':wd_List,                   #District ID
                    'use_type':use_List,            #Use type
                    'dec_cfs':rate_amt_List,        #Decreed rate absolute. (CFS)
                    'dec_af':vol_amt_List,          #Decreed rate absolute. (AF)
                    'apro_date':apro_date_List,     #Appropriation date
                    'strtype':strtype_List          #Strucutre Type
                    })


#csvRes = 'Div'+str(divID)+'_Net_Amts_Data.csv'
csvRes = 'Ditch_Div'+str(divID)+'Dist'+str(distID)+'_Net_Amts_Data.csv'
#Output to csv
ditchDF.to_csv(os.path.join(temp_dir,csvRes),sep=',')

#print the time it took
print (datetime.now()-start)

