#!/usr/bin/env python
"""
Note, on Mt Moran, need to load 
"""

import os
import netCDF4
import pandas as pd

input_dir = '/project/CI-WATER/rsteinke/runs/small_green_mesh_3_18_16/output'
geometry = os.path.join(input_dir, 'geometry.nc')
state = os.path.join(input_dir, 'state.nc')

flow = 'channelSurfacewaterChannelNeighborsFlowRate'

nc = netCDF4.Dataset(state)
f = nc[flow]
#for each instance in the dataset, get the flow rate at channel element 1000 to each of its neighbors
#ts = [x[1000] for x in f]

#Test for element 2534, downstream 2535 (only ds neighbor)
#Should be near NWIS station 09188500, green river at warren bridge, near daniel, wy
for instance in f:
    print instance[2534][0]
    

print df
"""
p = pd.Panel(f[:])

def x(d):
    print d
    if (d.loc[1000] > 0).any():
       print d.loc[1000]

x(p[0])
print "\n\n\n", p
#p.apply(x, axis=0)
"""
