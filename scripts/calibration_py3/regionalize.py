#!/usr/bin/env python
from adhydro import * 
from utils import Configuration
import glob
import subprocess
import os
import pandas as pd

config = Configuration()
"""
FIXME this section
"""
data_path = '/project/ci-water/data/'
"""
FIXME this section ^^^^^^^^
"""
print("Regionalizing calibrated parameters")
calibrated = pd.read_msgpack('/gscratch/nfrazie1/sugar_creek/calibration_ddt_7.5_final_correct_forcing/calibration_df_state.msg')
#needed variables for calibration ['meshRegion', 'meshVegetationType', 'meshSoilType', 'meshManningsN', 'meshConductivity', 'channelRegion', 'channelManningsN']
#Read the calibration parameter netcdf file
parameter_nc = nc.Dataset( config.calibration_parameter_file, 'a')
mesh_regions = pd.Series( parameter_nc.variables['meshRegion'][0,:] )
#find all elements 
mesh_elements = mesh_regions.index

#Get the list of channel regions
channel_regions = pd.Series( parameter_nc.variables['channelRegion'][0,:] )
channel_elements = channel_regions.index
#Build up the calibration data/parameters

veg = MeshVegitationType('land_cover', mesh_elements, parameter_nc, config.geometry_file)
soil = MeshSoilType('soil_texture', mesh_elements, parameter_nc)
#TODO if the interface is well defined, we can hold an arbitrary collection of calibration parameters
#and simply let each class handle the details of updating ect...
#calibration_variables = []
#FIXME uncomment
meshManningsN = MeshManningsN('manningsN', mesh_elements, parameter_nc, veg)
ksat = MeshConductivity('meshConductivity', mesh_elements, parameter_nc, veg, soil)
channelManningsN = ChannelManningsN('channelManningsN', channel_elements, parameter_nc)
#Collection of vars used to make calling functions work on arbitray params which are currently hard coded above ^^^ FIXME
calibration_vars = [meshManningsN, ksat, channelManningsN]
for v in calibration_vars:
    v.update_state(calibrated, 150)
    v.update_parameter(parameter_nc)
parameter_nc.sync()

