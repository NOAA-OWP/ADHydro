#!/usr/bin/env python

import pandas as pd
from collections import defaultdict

ADHydro_div = '/project/CI-WATER/data/irrigated_land/co_dist_58/diversion_to_channel.csv'
ADHydro_parcel = '/project/CI-WATER/data/irrigated_land/co_dist_58/parcel_to_elements.csv'
div_data = '/project/CI-WATER/nfrazie1/alpha1/management/database/co_district_58/diversions/div_data.csv'
parcel_data =  '/project/CI-WATER/nfrazie1/alpha1/management/database/co_district_58/parcels/irrig_data.csv'

ad_div_df = pd.read_csv(ADHydro_div)
ad_par_df = pd.read_csv(ADHydro_parcel)
div_data_df = pd.read_csv(div_data)
par_data_df = pd.read_csv(parcel_data)


#print ad_div_df.head()
#print div_data_df.head()
#print ad_par_df.head()
#print par_data_df.head()
   
merged_div = div_data_df.merge(ad_div_df, on='Name')
merged_par = par_data_df.merge(ad_par_df, on='Name')
#print merged_par[merged_par['ElementIDs'].isnull()]
#print merged_div

div_map = defaultdict(list)

def div_to_parcel_mapping(s):
    #print s['Name'], s['ParcelRatioToDiv']
    parcelID = s['Name'][3:]
    divs = [x.split('=')[0] for x in s['ParcelRatioToDiv'].split(';')]
    #print parcelID, divs
    for d in divs:
        if parcelID not in div_map[d]:
            div_map[d].append(parcelID)

merged_par.apply(div_to_parcel_mapping, axis=1)

def create_parcelID(s):
    p = ';'.join(div_map[s['Name'][3:]])
    return p

merged_div['ParcelIds'] = merged_div.apply(create_parcelID, axis=1)
merged_div = merged_div.rename(columns={'TotalDecRate':'DecreedAmount', 'TotalReqRate':'RequestedAmount', 'ApproDate':'AppropriationDate'})
div_cols = ['Name', 'ElementID', 'DecreedAmount', 'RequestedAmount', 'AppropriationDate', 'ParcelIds']
merged_div = merged_div[div_cols]

"""
It could be that some diversions do not have any associated parcels.  
Since we are only working with irrigation diversions at the moment, drop these.
"""
#merged_div['ParcelIds'].replace('', pd.np.nan, inplace=True)
#merged_div.dropna(inplace=True)
#merged_par.dropna(inplace=True) 
#print merged_div
#print "\n\n\n"
#print merged_par
merged_div.to_csv('/project/CI-WATER/nfrazie1/alpha1/management/database/co_district_58/diversions/diversion_data.csv', index=False)
merged_par.to_csv('/project/CI-WATER/nfrazie1/alpha1/management/database/co_district_58/parcels/parcel_data.csv', index=False)


