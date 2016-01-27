#!/usr/bin/env python
import pandas as pd
import os
from subprocess import call

# This script will read the mesh geometry from this directory
# the files it will read are:
#
# input_directory_path/mesh.1.ele
# input_directory_path/mesh.1.node
input_directory_path = "/share/CI-WATER_Simulation_Data/upper_colorado_mesh"

# This script will write its output to this directory
# the files it will write are:
#
# output_directory_path/elements.wkt
output_directory_path = "/share/CI-WATER_Simulation_Data/upper_colorado_mesh"
#Triangle Node and Element files
ELEfilepath           = os.path.join(input_directory_path, 'mesh.1.ele')
NODEfilepath          = os.path.join(input_directory_path, 'mesh.1.node')
#WKT output file
output_element_wkt_file = os.path.join(output_directory_path, 'elements.wkt')


def addWKT(s):
    """
        Helper fucntion that takes an element series and appends information to its WKT column
    """
    #Get coordinates for vertex 1
    s['WKT'] += str(nodes.loc[s['V1']]['X']) + " " + str(nodes.loc[s['V1']]['Y'])
    #Get coordinates for vertex 2
    s['WKT'] += ", " + str(nodes.loc[s['V2']]['X']) + " " + str(nodes.loc[s['V2']]['Y'])
    #Get coordinates for vertex 3
    s['WKT'] += ", " + str(nodes.loc[s['V3']]['X']) + " " + str(nodes.loc[s['V3']]['Y'])
    #Complete polygon by adding coordinates for vertex 1
    s['WKT'] += ", " + str(nodes.loc[s['V1']]['X']) + " " + str(nodes.loc[s['V1']]['Y'])
    #Complete the WKT string
    s['WKT'] += "))"
    
    return s


#Read nodes and elements into pandas data frames
elements = pd.read_csv(ELEfilepath, sep=' ', skipinitialspace=True, comment='#', skiprows=1, names=['ID', 'V1', 'V2', 'V3', 'CatchmentNumber'], index_col=0, engine='c').dropna()
nodes = pd.read_csv(NODEfilepath, sep=' ', skipinitialspace=True, comment='#', skiprows=1, names=['ID', 'X', 'Y', 'Z'], index_col=0, engine='c').dropna()

#Create the WKT polygon prefix for each element
elements['WKT'] = 'POLYGON(('
#Complete the polygon information
elements = elements.apply(addWKT, axis=1)

elements['WKT'].to_csv(output_element_wkt_file, index=False, header=['Polygon'])

gdal_vrt="\
<OGRVRTDataSource>
    <OGRVRTLayer name="mesh">
        <SrcDataSource>CSV:elements.wkt</SrcDataSource>
        <SrcLayer>elements</SrcLayer>
        <LayerSRS>+proj=sinu +lon_0=-109 +x_0=20000000 +y_0=10000000 +datum=WGS84 +units=m +no_defs</LayerSRS>
        <GeometryType>wkbPolygon</GeometryType>
        <GeometryField encoding="WKT" field="Polygon"/>
    </OGRVRTLayer>
</OGRVRTDataSource>"
with open(os.path.join(output_directory_path, 'elements.vrt'), 'wb') as f:
  f.write(gdal_vrt)

call(['ogr2ogr', '-f', '"ESRI Shapefile"', 'elements.shp', 'elements.vrt'])
