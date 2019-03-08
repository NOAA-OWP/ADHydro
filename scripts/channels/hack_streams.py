#!/usr/bin/env python

import netCDF4 as nc
#import geopandas as gpd
import pandas as pd
import os

"""
This script is a hack to reroute water around an erronous waterbody.
However, it can work to arbitrarily remove any channel segement and
connect its upstream neighbor/s to its downstream neighbor.  Currently,
it is only designed to connect to a single downstream neighbor, and will
error otherwise. It is possible to remove a reservoir with multiple downstream,
but this isn't an easy thing to do, and not recommended.  A new channel would
need to be inserted to connect one or more upstream to multiple downstream.

This code also makes sure not to intruduce a digital dam, and will attempt
to break the digital dam using the same algorithim that ADHydro mesh massage
uses in file_manager.cpp

NOTE: Once geometry has been re-connected, you likely have to rerun draindown,
    otherwise the nominal flow rates between newly connected neighbors won't match
    int the state, and it causes errors.  It may be possible to re-compute those flow
    rates from an existing state (i.e. the result of a previous draindown) given the new
    relationships, but so far, this script makes no attempt to do so.
"""


link_to_remove = 988 
parameter = nc.Dataset('parameter.nc', 'a')
geometry = nc.Dataset('geometry.nc', 'a')

def print_neighbor_list(elements, neighbors, flags, regions, prefix=''):
    if prefix:
        prefix = prefix+' '
    print("{}neighbor list of element/s \n{}".format(prefix, elements))
    print(neighbors)
    print("Downstream Flags: {}".format(flags))
    print("Regions: {}\n\n".format(regions))

#Grab param/geom info from first instance
#df = pd.Dataframe(parameter['channelReachCode'])
downstream = geometry.variables['channelChannelNeighborsDownstream'][0][:]
neighbors = geometry.variables['channelChannelNeighbors'][0][:]
regions = parameter.variables['channelRegion'][0][:]
neighbor_regions = parameter.variables['channelChannelNeighborsRegion'][0][:]
#need channelZBed and channelZBank for digital dam check

print_neighbor_list(link_to_remove, neighbors[link_to_remove], downstream[link_to_remove], neighbor_regions[link_to_remove])
#print(neighbor_regions[link_to_remove])
#print(neighbor_regions[7334])
#print(neighbor_regions[1945])


#os._exit(1)
myNeighbors = pd.Series(neighbors[link_to_remove], name=link_to_remove, dtype=int).replace(-1, pd.np.nan).dropna().astype(int)
#print(myNeighbors)
my_up_stream = pd.Series(downstream[link_to_remove], name=link_to_remove, dtype=int)
my_down_stream = pd.Series(downstream[link_to_remove], name=link_to_remove, dtype=int)

#Mask for UPSTREAM neighbors
up_stream_mask = (my_up_stream==0)
#Get the set of upstream neighbors
my_up_stream = pd.Series(myNeighbors[up_stream_mask], dtype=int)
#print(my_up_stream)

#Mask for DOWNSTREAM neighbors
down_stream_mask = (my_down_stream==1)
my_down_stream = pd.Series(myNeighbors[down_stream_mask], dtype=int)
if(len(my_down_stream) != 1):
    print("Can only attach streams to a single downstream neighbor")
    os._exit(1)
#print(my_down_stream)

"""
Now disconnect the upstream neighbors from the link_to_remove,
set upstream's downstream neighbor to be link_to_remove's downstream,
and dissconnect link_to_remove from its downstream neighbor, and set the
upstream to be downstream's up.
"""

print("Hooking neighbors")
#Every upstream connects to the same (single) downstream.
#If there is no upstream, no worries, only process downstream
print("Fixing upstream:\n")
if(len(my_up_stream) > 0):
    new_upstream_neighbors = neighbors[my_up_stream]
    new_flags = downstream[my_up_stream]
    new_regions = neighbor_regions[my_up_stream]
    index = pd.np.where( new_upstream_neighbors == link_to_remove )
    
    print_neighbor_list(my_up_stream, new_upstream_neighbors, new_flags, new_regions, 'Old')
    
    new_upstream_neighbors[ index ] = my_down_stream.iloc[0]
    new_flags[ index ] = 1
    new_regions[ index ] = regions[my_down_stream.iloc[0]]
    neighbors[my_up_stream] = new_upstream_neighbors
    neighbor_regions[my_up_stream] = new_regions
    #print(neighbors[my_up_stream])
    #new_flags = downstream[my_up_stream]
    #new_flags = 1
    downstream[my_up_stream] = new_flags
    
    print_neighbor_list(my_up_stream, neighbors[my_up_stream], downstream[my_up_stream], neighbor_regions[my_up_stream], 'New')

print("Fixing downstream:\n")
#The downstream has to consider one or more upstream
new_downstream_neighbors = neighbors[my_down_stream]
new_flags = downstream[my_down_stream]
new_regions = neighbor_regions[my_down_stream]
print_neighbor_list(my_down_stream, new_downstream_neighbors, new_flags, new_regions, 'Old')

#print(new_downstream_neighbors)

#Remove the old upstream neighbor
new_downstream_neighbors[ new_downstream_neighbors == link_to_remove ] = -1
#Find places we can put new neighbors
index = pd.np.where( new_downstream_neighbors == -1 )
#Now only get as many as needed
index = (index[0], index[1][0:len(my_up_stream)] )
#Now hook each upstream to a -1 spot in the list
new_downstream_neighbors[index] = my_up_stream.values
new_flags[index] = 0
new_regions[index] = regions[my_up_stream]
neighbors[my_down_stream] = new_downstream_neighbors
downstream[my_down_stream] = new_flags
neighbor_regions[my_down_stream] = new_regions
print_neighbor_list(my_down_stream, neighbors[my_down_stream], downstream[my_down_stream], neighbor_regions[my_down_stream], 'New')

print("Removing element {}".format(link_to_remove))
#Now make sure the removed element is cleaned up
print_neighbor_list(link_to_remove, neighbors[link_to_remove], downstream[link_to_remove], neighbor_regions[link_to_remove], 'Old')
neighbors[link_to_remove] = -1
downstream[link_to_remove] = 0
neighbor_regions[link_to_remove] = 0
print_neighbor_list(link_to_remove, neighbors[link_to_remove], downstream[link_to_remove], neighbor_regions[link_to_remove], 'New')

z_bed = geometry.variables['channelElementZBed'][0][:]

#Now we must check that we haven't introduced a digital dam at the new connection point.
def check_digital_dam(link):
    my_z_bed = z_bed[link]
    my_neighbors = pd.Series(neighbors[link])
    down_stream = pd.Series(downstream[link])
    down_stream_neighbors = my_neighbors[ down_stream==1 ]
    for n in down_stream_neighbors:
        print( "Checking for digital dam between elements {} and {}".format(link, n) )
        if n > 0:
            down_stream_z_bed = z_bed[n]
            print( "{} ---> {}".format(my_z_bed, down_stream_z_bed) )
            if my_z_bed < down_stream_z_bed:
                print("Error: created a digital dam removing element {}".format(link_to_remove))
                print("Digital Dam Fixing not yet implemented.")
                os._exit(1)

my_up_stream.apply(check_digital_dam)
#print(neighbor_regions[link_to_remove])
#print(neighbor_regions[my_up_stream])
#print(neighbor_regions[my_down_stream])

#Netcdf writing is an all or nothing kinda gig...have to have exact shape of numpy array to write...

geometry.variables['channelChannelNeighbors'][:] = neighbors[pd.np.newaxis, :]
geometry.variables['channelChannelNeighborsDownstream'][:] = downstream[pd.np.newaxis, :]
parameter.variables['channelChannelNeighborsRegion'][:] = neighbor_regions[pd.np.newaxis, :]

parameter.sync()
geometry.sync()

parameter.close()
geometry.close()

