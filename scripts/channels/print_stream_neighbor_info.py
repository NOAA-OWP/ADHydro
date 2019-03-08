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


link_to_remove = 1945 
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

