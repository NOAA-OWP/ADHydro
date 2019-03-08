#!/usr/bin/env python

import netCDF4 as nc
import geopandas as gpd
import pandas as pd
import os

starting_link = 7510
parameter = nc.Dataset('parameter.nc')
geometry = nc.Dataset('geometry.nc')

def pprint_array(line_length, name, array):
    print("{}: {}".format(name, len(array)))
    array.sort()
    output = ''
    for i, data in zip(range(1, len(array)+1), array):
            output += '{}'.format(int(data))

            if i != len(array):
                output += ','
                if i%line_length == 0:
                    output += '\n'
                else:
                    output += ' '
    print( output )


"""
#Grab param/geom info from first instance
neighbors = pd.DataFrame(geometry.variables['channelChannelNeighbors'][0]).replace(-1, pd.np.nan)
downstream = pd.DataFrame(geometry.variables['channelChannelNeighborsDownstream'][0])
print( neighbors.head())
print( downstream.head())
neighbors = neighbors[ downstream == 0 ]
print (neighbors.head())
#print(parameter['channelReachCode'])
df = pd.Dataframe(parameter['channelReachCode'])
"""
cRegions =  pd.Series(parameter.variables['channelRegion'][0][:])
#print(cRegions[7512])
#os._exit(1)
"""
print("Number of channel regions: {}".format(len( pd.Series(parameter.variables['channelRegion'][0][:]).unique())))
print("Number of mesh regions: {}".format(len( pd.Series(parameter.variables['meshRegion'][0][:]).unique())))
cRegions =  pd.Series(parameter.variables['channelRegion'][0][:])
mRegions =  pd.Series(parameter.variables['meshRegion'][0][:])

regionListFile = '/project/ci-water/tools/ADHydro/ADHydro/activeRegions.txt'
with open(regionListFile) as regionFile:
    regions = regionFile.read().splitlines()
    regions.sort(key=int)
    index = cRegions[ cRegions.isin(regions) ]
    pprint_array( 20, "channels", index.index.values )
os._exit(1)
"""
downstream = geometry.variables['channelChannelNeighborsDownstream'][0][:]
neighbors = geometry.variables['channelChannelNeighbors'][0][:]

allElementNumbers = []
#starting_link = 6332

def gatherUpstreamChannels(link):
    #print("processing link {}".format(link)) 
    myNeighbors = pd.Series(neighbors[link], name=link).replace(-1, pd.np.nan).dropna()
    myUp_stream = pd.Series(downstream[link], name=link)
    #Mask for UPSTREAM neighbors
    up_stream_mask = (myUp_stream==0)
    #Get the set of upstream neighbors
    myUp_stream = pd.Series(myNeighbors[up_stream_mask])
    """
    elements.append(link)
    print(myUp_stream)
    if len(myUp_stream) == 0:
        return
    else:
        for element in myUp_stream:
            gatherUpstreamChannels(link, elements)
    """
    return myUp_stream.values.astype(int)

process_links = [starting_link]

while process_links:
   link = process_links.pop()
   allElementNumbers.append(link)
   process_links.extend( gatherUpstreamChannels(link) )
   #print( process_links )
#print( allElementNumbers )
regions = pd.Series(parameter.variables['channelRegion'][0][allElementNumbers]).unique()
reaches = pd.Series(parameter.variables['channelReachCode'][0][allElementNumbers]).unique()
meshCatchments = pd.Series(parameter.variables['meshCatchment'][0][:])
meshRegions = pd.Series(parameter.variables['meshRegion'][0][:])
#Subset catchments to those that drain into the channels
meshCatchments = meshCatchments[ meshCatchments.isin(reaches) ]
#Find the ADHydro regions that these catchments belong to
meshRegions = meshRegions.iloc[ meshCatchments.index ]
meshRegions = meshRegions.unique() 
"""
print("Number of channel regions: {}".format(len(regions)))
print("Number of mesh regions: {}".format(len(meshRegions)))
regions.sort()
print( regions )
print( meshRegions )
os._exit(1)
"""
#pprint_array(10, 'reaches', reaches)
#pprint_array(10, 'meshCatchments', meshCatchments.unique())
#pprint_array(10, "meshRegions", meshRegions)
#ADHydro regions that need to be active to contribute streamflow to the starting_link
pprint_array(10, "channelRegions", regions)
pprint_array(10, "meshRegions", meshRegions)

"""
print_string = ''
for c in meshCatchments.unique():
    print_string += '{},\n'.format(c)
print(print_string)
"""
#print("REIONS:\n\n")
#for r in regions:
#    print(r)
