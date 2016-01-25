#!/usr/bin/env python
import os
import pandas as pd
import numpy as np
import glob
from sys import stdout
from math import acos, cos

import psutil

#Return mem usage for process, as reported by OS, in MiB
def mem_usage():
    process = psutil.Process(os.getpid())
    mem = process.memory_info()[0]/float(2**20)
    return mem
import time #Can be removed, only for timing various functions of this script
# This script is set up to run from the command line and not from inside the QGIS python console.
# You need to add your QGIS python directory to the PYTHONPATH environment variable as shown below.
#
#$export PYTHONPATH=/opt/share/qgis/python:$PYTHONPATH
#Qgis setup, here so GLOBALS get setup right
from qgis.core import * #VERY BAD PRACTICE...SHOULD FIND ALL REFERENCES AND PREPEND QGIS.CORE. RATHER THAN IMPORTING ALL OF QGIS.CORE
qgishome = '/project/CI-WATER/nfrazie1/local'
app = QgsApplication([], True)
app.setPrefixPath(qgishome, True)
app.initQgis()

#GLOBALS
#These are reused each time getMUKEY is called, so create them once and save the overhead
#Can even put into an init function if the need arises
input_SSURGO_prefix = "/project/CI-WATER/data/SSURGO"
#Directory where extracted data files exist
input_SSURGO_unzipped = os.path.join(input_SSURGO_prefix, "unzipped")
input_SSURGO_file  = os.path.join(input_SSURGO_prefix, "index.shp")
SSURGO_layer       = QgsVectorLayer(input_SSURGO_file, "SSURGO",  "ogr")
SSURGO_provider    = SSURGO_layer.dataProvider()
SSURGO_AreaSymInd = SSURGO_provider.fieldNameIndex("AREASYMBOL")

input_STATSGO_prefix = "/project/CI-WATER/data/STATSGO"
#Directory where extracted data files exist
input_STATSGO_unzipped = os.path.join(input_STATSGO_prefix, "unzipped")
input_STATSGO_file  = os.path.join(input_STATSGO_prefix, "all.shp")
STATSGO_layer     = QgsVectorLayer(input_STATSGO_file, "STATSGO",  "ogr")
STATSGO_provider  = STATSGO_layer.dataProvider()
STATSGO_AreaSymInd        = STATSGO_provider.fieldNameIndex("AREASYMBOL")
STATSGO_MUKEYInd          = STATSGO_provider.fieldNameIndex("MUKEY")
STATSGO_MUKEYSymInd       = STATSGO_provider.fieldNameIndex("MUSYM")

input_GEOL_prefix = "/project/CI-WATER/data/Geologic_Units/"
#Directory where extracted data files exist
input_GEOL_State_prefix = os.path.join(input_GEOL_prefix, "MergingShapeFiles/")
input_GEOL_file  =  os.path.join(input_GEOL_prefix, "AllMerged.shp") 
GEOL_layer     = QgsVectorLayer(input_GEOL_file, "GEOLOGIC_UNITS",  "ogr")
GEOL_provider  = GEOL_layer.dataProvider()
GEOL_SourceInd = GEOL_provider.fieldNameIndex("SOURCE")

#This script will read the SSURGO soil mukey files from this directory
input_SSURGO_County_prefix = os.path.join(input_SSURGO_prefix, "all")
# This script will read the NLCD data form this directory
input_NLCD_directory_path = "/project/CI-WATER/data/NLCD/all_projected/"

# This script will read the mesh geometry from this directory
# the files it will read are:
#
# input_directory_path/mesh.1.ele
# input_directory_path/mesh.1.node
#input_directory_path = "/project/CI-WATER/data/WBNFLSnake_mesh/"
input_directory_path = "/project/CI-WATER/data/upper_colorado_mesh/"

# This script will write its output to this directory
# the files it will write are:
#
# output_directory_path/mesh.1.soilType
# output_directory_path/mesh.1.LandCover
# output_directory_path/mesh.1.geolType
# output_directory_path/element_data.pkl
#output_directory_path = "/project/CI-WATER/data/WBNFLSnake_mesh/"
output_directory_path = "/project/CI-WATER/data/upper_colorado_mesh/"

#Dictionary to hold QgsVector layers          
SSURGO_county_dict = {}

#Dictionaries to hold comp.txt data in, accessed by AreaSym, i.e. SSURGO_comp_dict[AreaSym]
#These will hold pandas data frames containing the comp data, NOTE: not all comp_table_names
#will be included in these dataframes, see the read_csv method in getCOKEYS
SSURGO_comp_dict = {}
STATSGO_comp_dict = {}
#Column names taken from SSURGO_Metadata_-Tables_and_Columns.pdf at /share/CI-WATER Third Party Files/SSURGO
comp_table_names = ['comppct_l', 'comppct_r', 'comppct_h', 'compname', 'compkind', 'majcompflag', 'otherph', 'localphase', 'slope_l', 'slope_r', 'slope_h', 'slopelenusle_l', 'slopelenusle_r', 'slopelenusle_h', 'runoff', 'tfact', 'wei', 'weg', 'erocl', 'earthcovkind1', 'earthcovkind2', 'hydricon', 'hydricrating', 'drainagecl', 'elev_l', 'elev_r', 'elev_h', 'aspectccwise', 'aspectrep', 'aspectcwise', 'geomdesc', 'albedodry_l', 'albedodry_r', 'albedodry_h', 'airtempa_l', 'airtempa_r', 'airtempa_h', 'map_l', 'map_r', 'map_h', 'reannualprecip_l', 'reannualprecip_r', 'reannualprecip_h', 'ffd_l', 'ffd_r', 'ffd_h', 'nirrcapcl', 'nirrcapscl', 'nirrcapunit', 'irrcapcl', 'irrcapscl', 'irrcapunit', 'cropprodindex', 'constreeshrubgrp', 'wndbrksuitgrp', 'rsprod_l', 'rsprod_r', 'rsprod_h', 'foragesuitgrpid', 'wlgrain', 'wlgrass', 'wlherbaceous', 'wlshrub', 'wlconiferous', 'wlhardwood', 'wlwetplant', 'wlshallowwat', 'wlrangeland', 'wlopenland', 'wlwoodland', 'wlwetland', 'soilslippot', 'frostact', 'initsub_l', 'initsub_r', 'initsub_h', 'totalsub_l', 'totalsub_r', 'totalsub_h', 'hydgrp', 'corcon', 'corsteel', 'taxclname', 'taxorder', 'taxsuborder', 'taxgrtgroup', 'taxsubgrp', 'taxpartsize', 'taxpartsizemod', 'taxceactcl', 'taxreaction', 'taxtempcl', 'taxmoistscl', 'taxtempregime', 'soiltaxedition', 'castorieindex', 'flecolcomnum', 'flhe', 'flphe', 'flsoilleachpot', 'flsoirunoffpot', 'fltemik2use', 'fltriumph2use', 'indraingrp', 'innitrateleachi', 'misoimgmtgrp', 'vasoimgtgrp', 'mukey', 'cokey']

#Dictionaries to hold chorizon.txt data, accessed by AreaSym, i.e. SSURGO_horizon_dict[AreaSym]
#These will hold pandas data frames contain chorizon data, NOTE: not all horizon_table_names
#will be included in these dataframes, see the read_csv method in getSoilContent
SSURGO_horizon_dict = {}
STATSGO_horizon_dict = {}
#Column names taken from SSURGO_Metadata_-Tables_and_Columns.pdf at /share/CI-WATER Third Party Files/SSURGO
horizon_table_names = ['hzname', 'desgndisc', 'desgnmaster', 'desgnmasterprime', 'desgnvert', 'hzdept_l', 'hzdept_r', 'hzdept_h', 'hzdepb_l', 'hzdepb_r', 'hzdepb_h', 'hzthk_l', 'hzthk_r', 'hzthk_h', 'fraggt10_l', 'fraggt10_r', 'fraggt10_h', 'frag3to10_l', 'frag3to10_r', 'frag3to10_h', 'sieveno4_l', 'sieveno4_r', 'sieveno4_h', 'sieveno10_l', 'sieveno10_r', 'sieveno10_h', 'sieveno40_l', 'sieveno40_r', 'sieveno40_h', 'sieveno200_l', 'sieveno200_r', 'sieveno200_h', 'sandtotal_l', 'sandtotal_r', 'sandtotal_h', 'sandvc_l', 'sandvc_r', 'sandvc_h', 'sandco_l', 'sandco_r', 'sandco_h', 'sandmed_l', 'sandmed_r', 'sandmed_h', 'sandfine_l', 'sandfine_r', 'sandfine_h', 'sandvf_l', 'sandvf_r', 'sandvf_h', 'silttotal_l', 'silttotal_r', 'silttotal_h', 'siltco_l', 'siltco_r', 'siltco_h', 'siltfine_l', 'siltfine_r', 'siltfine_h', 'claytotal_l', 'claytotal_r', 'claytotal_h', 'claysizedcarb_l', 'claysizedcarb_r', 'claysizedcarb_h', 'om_l', 'om_r', 'om_h', 'dbtenthbar_l', 'dbtenthbar_r', 'dbtenthbar_h', 'dbthirdbar_l', 'dbthirdbar_r', 'dbthirdbar_h', 'dbfifteenbar_l', 'dbfifteenbar_r', 'dbfifteenbar_h', 'dbovendry_l', 'dbovendry_r', 'dbovendry_h', 'partdensity', 'ksat_l', 'ksat_r', 'ksat_h', 'awc_l', 'awc_r', 'awc_h', 'wtenthbar_l', 'wtenthbar_r', 'wtenthbar_h', 'wthirdbar_l', 'wthirdbar_r', 'wthirdbar_h', 'wfifteenbar_l', 'wfifteenbar_r', 'wfifteenbar_h', 'wsatiated_l', 'wsatiated_r', 'wsatiated_h', 'lep_l', 'lep_r', 'lep_h', 'll_l', 'll_r', 'll_h', 'pi_l', 'pi_r', 'pi_h', 'aashind_l', 'aashind_r', 'aashind_h', 'kwfact', 'kffact', 'caco3_l', 'caco3_r', 'caco3_h', 'gypsum_l', 'gypsum_r', 'gypsum_h', 'sar_l', 'sar_r', 'sar_h', 'ec_l', 'ec_r', 'ec_h', 'cec7_l', 'cec7_r', 'cec7_h', 'ecec_l', 'ecec_r', 'ecec_h', 'sumbases_l', 'sumbases_r', 'sumbases_h', 'ph1to1h2o_l', 'ph1to1h2o_r', 'ph1to1h2o_h', 'ph01mcacl2_l', 'ph01mcacl2_r', 'ph01mcacl2_h', 'freeiron_l', 'freeiron_r', 'freeiron_h', 'feoxalate_l', 'feoxalate_r', 'feoxalate_h', 'extracid_l', 'extracid_r', 'extracid_h', 'extral_l', 'extral_r', 'extral_h', 'aloxalate_l', 'aloxalate_r', 'aloxalate_h', 'pbray1_l', 'pbray1_r', 'pbray1_h', 'poxalate_l', 'poxalate_r', 'poxalate_h', 'ph2osoluble_l', 'ph2osoluble_r', 'ph2osoluble_h', 'ptotal_l', 'ptotal_r', 'ptotal_h', 'excavdifcl', 'excavdifms', 'cokey', 'chkey']

#Dictionary to hold QgsVector layers for vegitation data
veg_parm_dict={}

# Dictionry with the Geologic units shape file for the states: WY, CO, AZ, NM, and UT
# The files do not have a standard name. For other states, insert the file name in the dictionary.
GEOL_Unit_State_VectorLayer_dict ={'AZ': QgsVectorLayer(input_GEOL_State_prefix + "azgeol_poly_dd.shp", "GEOL_UNITS_AZ",   "ogr"), 
                                   'CO': QgsVectorLayer(input_GEOL_State_prefix + "cogeol_dd_polygon.shp", "GEOL_UNITS_CO","ogr"), 
                                   'NM': QgsVectorLayer(input_GEOL_State_prefix + "nmgeol_dd_polygon.shp", "GEOL_UNITS_NM","ogr"),
                                   'UT': QgsVectorLayer(input_GEOL_State_prefix + "utgeol_poly_dd.shp", "GEOL_UNITS_UT",   "ogr"),
                                   'WY': QgsVectorLayer(input_GEOL_State_prefix + "wygeol_dd_polygon.shp", "GEOL_UNITS_WY","ogr")}
#Here we create the spatial index used by the get_GEOLOGIC_UNITS function.
#Since this function uses a per state shapefile, it is more efficient to create
#the large spatial index once, and search through each time rather than
#re-create the spatial index for each call to get_GEOLOGIC_UNITS
"""
            GEOL_perState_layer    = GEOL_Unit_State_VectorLayer_dict[  str (Source[0] + Source[1]) ]
            GEOL_perState_provider = GEOL_perState_layer.dataProvider()                
            GEOL_State_ROCKTYPE  = GEOL_perState_provider.fieldNameIndex("ROCKTYPE1")
            
            feature  = QgsFeature()
            GEOL_perState_provider.select([GEOL_State_ROCKTYPE])
            
            found = False
            index = QgsSpatialIndex()
            while GEOL_perState_provider.nextFeature(feature):
                index.insertFeature(feature)
"""
t0 = time.time()
GEOL_spatial_index = {}
#iterate over the geology vector layers
for state,layer in GEOL_Unit_State_VectorLayer_dict.items():
    #get the layer's data provider and index of its rocktype field
    provider = layer.dataProvider()
    #ROCKTYPE = provider.fieldNameIndex("ROCKTYPE1")
    #select the rocktype features to work with, QGIS 2 style
    request = QgsFeatureRequest()
    request.setSubsetOfAttributes(['ROCKTYPE1'], provider.fields())
    #provider.select([ROCKTYPE])
    #Initialize the spatial index
    GEOL_spatial_index[state] = QgsSpatialIndex()
    #Build the spatial index
    for feature in layer.getFeatures(request):
        GEOL_spatial_index[state].insertFeature(feature)
#Now we similarly pre-populate the SSURGO spatial index.
#To find the MUKEYS, we first have to look through all the SSURGO features to find
#the AreaSYM that a point lie in.  We can create that spatial index once and reference
#it instead of creating it once for each element.
SSURGO_spatial_index = QgsSpatialIndex()
#feature  = QgsFeature()
#SSURGO_provider.select([SSURGO_AreaSymInd])
request = QgsFeatureRequest()
request.setSubsetOfAttributes([SSURGO_AreaSymInd])
#Populate the spatial index with each feature in the SSURGO provided
#while SSURGO_provider.nextFeature(feature):
for feature in SSURGO_layer.getFeatures(request):
    SSURGO_spatial_index.insertFeature(feature)    

#We also look up county information inside SSURGO, using yet another spatial index
#Store these in a dict the first time and avoid re-creating them every call.  These
#will get populated in the getMUKEY function
SSURGO_county_index = {}

#If using STATSGO, we also use a spatial index, but only one on one layer
#We can initilize this spatial index and save a lot of overhead
#feature  = QgsFeature()
#STATSGO_provider.select([STATSGO_AreaSymInd, STATSGO_MUKEYInd, STATSGO_MUKEYSymInd ])
request = QgsFeatureRequest()
request.setSubsetOfAttributes([STATSGO_AreaSymInd, STATSGO_MUKEYInd, STATSGO_MUKEYSymInd ])
STATSGO_index = QgsSpatialIndex()  
#while STATSGO_provider.nextFeature(feature):
for feature in STATSGO_layer.getFeatures(request):
    STATSGO_index.insertFeature(feature)

t1 = time.time()
print "Time to initialize spatial indicies: "+str(t1 - t0)

def writeSoilFile(s, f, bugFix):
    """
      This function writes a line to the soil output file for each element.
      It is desgned to work the the pandas data frame apply function, which used along the first axis
      of a data frame, this function writes information from each row in the dataframe to an output file.

      Required Arguements:
        s -- pandas series containing element information, including SoilType and HorizonThickness (provided by the apply function)
        f -- file handle to the desired output file
        bugFix -- need to pass a third parameter to bypass a weird file not open for reading bug that appears if s and f are the only args, simply pass None for this arg
    """
    #Write element number 
    f.write('  '+str(s.name)+'  ')
    #Soil type is either an array of soil layers or a scalar float with the value NaN.  If it is NaN write zero to indicate no soil layers
    #isnull might be redundant, since the only thing not an array should be NaN
    if isinstance(s['SoilType'], float) and pd.isnull(s['SoilType']):
        f.write('0')
    else:
        #Write soiltype, horizon thinckness pairs (s,d)
        if isinstance(s['HorizonThickness'], float):
            #Write one to indicate one soil layer.
            f.write('1  ')
            #If the horizon thickness isn't an array (i.e. it is a float) then just write the first soil type and a 0.0 for horizon thickness
            f.write(str(int(s['SoilType'][0]))+',0.0')
        else:
            #Write the number of elements in the soil type array
            f.write(str(len(s['SoilType']))+'  ')
            #Write out the pairs of soil types and thicknesses
            for pair in zip(s['SoilType'], s['HorizonThickness']):
                if (not np.isnan(pair[0])) and (not np.isnan(pair[1])):
                    f.write(str(int(pair[0]))+','+str(pair[1])+'  ')
                elif not np.isnan(pair[0]):
                    f.write(str(int(pair[0]))+','+str(0)+'  ')
                elif not np.isnan(pair[1]):
                    f.write(str(-1)+','+str(pair[1])+'  ')
                else:
                    f.write(str(-1)+','+str(0)+'  ')
    f.write('\n')

#Write a line to the veg file, f, based on information passed in series, s
#Note bugFix is here just to bypass a weird file not open for reading bug that appears
#if s and f are the only args
def writeVegFile(s, f, bugFix):
  """
    This function writes a line to the vegitation output file for each element.
    It is desgned to work the the pandas data frame apply function, which used along the first axis
    of a data frame, this function writes information from each row in the dataframe to an output file.

    Required Arguements:
      s -- pandas series containing element information, including VegParm (provided by the apply function)
      f -- file handle to the desired output file
      bugFix -- need to pass a third parameter to bypass a weird file not open for reading bug that appears if s and f are the only args, simply pass None for this arg

  """
  f.write('  '+str(s.name)+'  '+str(s['VegParm'])+'\n')

#Write a line to the veg file, f, based on information passed in series, s
#Note bugFix is here just to bypass a weird file not open for reading bug that appears
#if s and f are the only args
def writeGeolFile(s, f, bugFix):
  """
    This function writes a line to the Geologic Units output file for each element. 
    First line contain the number of elements. From the second line it contains the element number and
    its geologic type. 1 refers to alluvium, 0 refers to not alluvium.
    It is desgned to work the the pandas data frame apply function, which used along the first axis
    of a data frame, this function writes information from each row in the dataframe to an output file.

    Required Arguements:
      s -- pandas series containing element information, including VegParm (provided by the apply function)
      f -- file handle to the desired output file
      bugFix -- need to pass a third parameter to bypass a weird file not open for reading bug that appears if s and f are the only args, simply pass None for this arg

  """
  f.write('  '+str(s.name)+'  '+str(s['ROCKTYPE'])+'\n')


#Need to make sure latitude and longitude coordinates are projected correctly to match
#the coordinate system used by SSURGO/STATSGO and NLCD (found by using gdalsrsinfo on the .prj file in statsgo dir
STATSGO_PROJ_WKT = 'GEOGCS["GCS_WGS_1984", DATUM["WGS_1984", SPHEROID["WGS_84",6378137,298.257223563, AUTHORITY["EPSG","7030"]], AUTHORITY["EPSG","6326"]], PRIMEM["Greenwich",0, AUTHORITY["EPSG","8901"]], UNIT["degree",0.01745329251994328, AUTHORITY["EPSG","9122"]], AUTHORITY["EPSG","4326"]]'

SSURGO_PROJ_WKT = 'GEOGCS["GCS_WGS_1984", DATUM["WGS_1984", SPHEROID["WGS_84",6378137,298.257223563]], PRIMEM["Greenwich",0], UNIT["Degree",0.017453292519943295]]'
"""

Only difference in the SSURGO and STATSGO projections is that STATSGO provides athority info...I think we can use just one and get proper transformations

"""
AD_HYDRO_PROJ_WKT = 'PROJCS["Sinusoidal", GEOGCS["GCS_WGS_1984", DATUM["WGS_1984", SPHEROID["WGS_84",6378137,298.257223563]], PRIMEM["Greenwich",0], UNIT["Degree",0.017453292519943295]], PROJECTION["Sinusoidal"], PARAMETER["longitude_of_center",-109], PARAMETER["false_easting",20000000], PARAMETER["false_northing",10000000], UNIT["Meter",1]]'

#Create the references for the two different coordinate systems
src_crs = QgsCoordinateReferenceSystem(AD_HYDRO_PROJ_WKT)
dest_crs = QgsCoordinateReferenceSystem(STATSGO_PROJ_WKT)
#Create a transform object to transform between the coordinate systems
xform = QgsCoordinateTransform(src_crs, dest_crs)

def coordTransform(s):
    """
        Helper function to apply a coordinate transformation between the sinusoidal and spheroid projections
        Required Arguements:
             s -- pandas series containing element coordinate information, specficially 'X_center' and 'Y_center'
        Returns:
            Series s with transformed coordinates 'lat_center' and 'long_center' added
    """
    #Transform the ADHydro coordinates since they have the false northing and false easting already
    point = xform.transform(QgsPoint(s['AD_X'], s['AD_Y']))
    s['lat_center'] = point.x()
    s['long_center'] = point.y()
    return s

def who():
    return "Processes "+str(os.getpid())+" "

def getSoilTypDRV(subset):
   """
     This is the user entry point to process the mesh, lookup the soil and vegitation information, and write the output files.
     This process includes reading the element and node files of the mesh and calculating each elements center, ADhydro, and lat/long coordinates.
     
     getSoilTypDRV utilizes pandas data frames to hold this information, and then applies several functions to the data frame to lookup the nessicary information.
     Once all this information is found and added to the dataframe, this function creates the output files specified by these variables:
       output_SoilTyp_file      -- structured output file storing the layers of the soil horizon for each element
       output_VegTyp_file       -- structured output file storing each element's vegitation type
       output_GeolTyp_file      -- structured output file storing each element's Geologic Unit type
       output_element_data_file -- a Pandas dataFrame serialized to file containing all of the computed and looked up information for each element
      
   """
   
   section='.{}-{}'.format(subset[0],subset[-1])
   ELEfilepath           = os.path.join(input_directory_path, 'mesh.1.ele')
   NODEfilepath          = os.path.join(input_directory_path, 'mesh.1.node')
   output_element_data_file = os.path.join(output_directory_path, 'element_data.pkl'+section)

   print '\n\n'+who()+'Reading element file.'
   t0 = time.time()
   elements = pd.read_csv(ELEfilepath, sep=' ', skipinitialspace=True, comment='#', skiprows=1, names=['ID', 'V1', 'V2', 'V3', 'CatchmentNumber'], index_col=0, engine='c').dropna()
   
   elements['V1'] = elements['V1'].astype(int)

   #Only work on our subset of elements
   elements = elements.loc[subset].reset_index()

   print who()+'Reading node file.'

   nodes = pd.read_csv(NODEfilepath, sep=' ', skipinitialspace=True, comment='#', skiprows=1, names=['ID', 'X', 'Y', 'Z'], index_col=0, engine='c').dropna()

   num_elems = len(elements.index)
   num_nodes = len(nodes.index)
   
   print who()+"Processing {} elements from {} nodes".format(num_elems, num_nodes)
   
   print who()+'Calculating element centers. Memory at start: '+str(mem_usage())
   
   ele=1
   num_verticies=3
   R = 6378137.0;		# Sphere model radius in meters from SNYDER - 1987
   pi = acos(-1);
   origin = -109.0*pi/180      # Origin of xy coordinates in degrees

   #Calculate the center of each element in the elements dataframe and store it in columns lat_center and long_center
   #This is FAST as it uses vectorization for element wise computation especially as size of input increases
   #To get everything to line up correctly we have to coerce the index of the pulled data,
   #Since initially nodes are indexed based on thier ID, when they get aligned with the proper element here
   #Their index is off, so when we try to add these values into elements, they must have a corresponding index (1-num_elements)
   v = nodes.loc[elements['V1']].reset_index()
   v.index = range(0, num_elems)
   #Note v index and elements index must be the same!!!
   elements['X_center'] = v.X - 20000000.0
   elements['Y_center'] = v.Y - 10000000.0
   
   
   v = nodes.loc[elements['V2']].reset_index()
   v.index = range(0, num_elems)
   elements['X_center'] = elements['X_center'] + v.X - 20000000.0
   elements['Y_center'] = elements['Y_center'] + v.Y - 10000000.0
   
   v = nodes.loc[elements['V3']].reset_index()
   v.index = range(0, num_elems)
   elements['X_center'] = elements['X_center'] + v.X - 20000000.0
   elements['Y_center'] = elements['Y_center'] + v.Y - 10000000.0
   
   #Center based on number of verticies
   elements['X_center'] = elements['X_center']/num_verticies
   elements['Y_center'] = elements['Y_center']/num_verticies
   
   #ADHydro coordinates for looking up vegitation data
   elements['AD_X'] = elements['X_center'] + 20000000.0
   elements['AD_Y'] = elements['Y_center'] + 10000000.0
   
   """ Incorrect projection!!!
   # Changing from x,y sinusoidal projection to latitude and longitude coordinates
   elements['long_center'] = elements['Y_center']/R
   elements['lat_center'] = elements['X_center'] / (R*np.cos(elements['long_center']))
   elements['lat_center'] = (elements['lat_center'] + origin)*(180/pi)
   elements['long_center'] = elements['long_center']*180/pi
   """
   
   #Apply the coordinate transformations to get Lat/Long in the same CRS as SSURGO/STATSGO
   elements = elements.apply(coordTransform, axis=1) #Slower since it is not vectorizable, but more accurate!
   
   #elements now contains coordinates for the center of each element in three different coordinates:
   #  X, Y sinusoidal projection X_center, Y_center
   #  AdHydro Coordinates AD_X, AD_Y
   #  Lat, Long x,y coordinates lat_center, long_center

   #Using this information we can now lookup various metadata using the MUKEY and AREASYM keys in the dataframe
   
   """
    By getting MUKEY and COKEY piecies here for all elements before any further processing
    we can potentially make this script nicer becuase we can release resources manually
    knowing that we are finished with the shape files and comp.txt files needed to get
    these values.  On really large meshes, this might be required to do.
   """
   
   #be kind, clean up unused memory
   del nodes
   #Since the mukey and areasym are used to lookup different data, find and store them for each element.
   t1 = time.time()
   print who()+"Finished center calculations. Time: "+ str(t1 - t0) + "\tMem usage: "+ str(mem_usage())
   print who()+'Finding MUKEY.'

   
   #find the MUKEY for each element, this adds the following columns to elements:
   #MUKEY, AreaSym, inSSURGO
   t0 = time.time()
   elements = elements.apply(getMUKEY, axis=1)
   t1 = time.time()
   print who()+"Finished finding MUKEY. Time: "+ str(t1 - t0)  + "\tMem usage: "+ str(mem_usage())
   stdout.flush()
   #Write the element data serialized file with the MUKEYS since getMUKEY takes a while.  In case of error, this data can be reloaded
   elements.to_pickle(output_element_data_file)
   
   #If you want to reload the data instead of recomputing it comment out the lines since Finding MUKEY and uncomment this line
   #elements = pd.read_pickle(output_element_data_file)
   
   print who()+'Finding Geologic Units.'

   #find the geologic unit for each element, this adds the following columns to elements:
   #ROCKTYPE
   t0 = time.time()
   elements = elements.apply(get_GEOLOGIC_UNITS, axis=1)
   t1 = time.time()
   print who()+"Finished finding Geologic Units. Time: "+ str(t1 - t0)  + "\tMem usage: "+ str(mem_usage())
   stdout.flush()
   
   #Write the element data serialized file with the geologic units since get_GEOLOGIC_UNITS takes a while.  In case of error, this data can be reloaded
   elements.to_pickle(output_element_data_file)
   
   #If you want to reload the data instead of recomputing it comment out the lines since Finding MUKEY and uncomment this line
   #elements = pd.read_pickle(output_element_data_file)
   
   print who()+'Finding COKEY.'

   #Find the COKEY for each element, must have MUKEY column before calling getCOKEY, this adds the following columns to elements:
   #COKEY, compname
   t0 = time.time()
   elements = elements.apply(getCOKEY, axis=1)
   t1 = time.time()
   print who()+"Finished finding COKEY. Time: "+ str(t1 - t0)  + "\tMem usage: "+ str(mem_usage())
   stdout.flush()

   elements.to_pickle(output_element_data_file)

   print who()+'Finding soil type.'

   #get soil content per element, MUST have COKEY and compname columns before calling getSoilContent, this adds the following columns to elements:
   #SoilType
   t0 = time.time()
   elements = elements.apply(getSoilContent, axis=1)
   t1 = time.time()
   print who()+"Finished finding soil type. Time: "+ str(t1 - t0)  + "\tMem usage: "+ str(mem_usage())
   stdout.flush()
  
   elements.to_pickle(output_element_data_file)

   print who()+'Finding vegetation type.'

   #get vegitation parameters, must have AreaSym column before calling getVegParm, this adds the following columns to elements:
   #VegParm
   t0 = time.time()
   elements = elements.apply(getVegParm, axis=1)
   t1 = time.time()
   print who()+"Finished finding vegetation type. Time: "+ str(t1 - t0)  + "\tMem usage: "+ str(mem_usage()) + "\n\n"
   stdout.flush()

   elements.to_pickle(output_element_data_file)

   return elements


#TODO If really need to specify which dataset to search( SSURGO or STATSGO ) then make two functions, one for each
#Otherwise just search SSURGO first, then STATSGO if SSURGO fails to provide results    
def getMUKEY(s):
    """
      This function searches SSURGO and STATSGO data files for an element's MUKEY using pyQGIS functions.

      This function will attempt to find the MUKEY in SSURGO first, and if it cannot find one it will then repeat
      the processing using STATSGO.

      Since many of these data files get read repeatedly in different calls to getMUKEY, the data is only read
      from the files once and is stored in dictionaries so it can be quickly looked up again in another call needing
      to read the same information.

      It is designed to work with the pandas data frame's apply function, which, when used along axis=1,
      will apply this function to each row of a dataframe, passing the row's information in the series argument.
      This dataframe should have columns for the following variables:
        lat_center
        long_center
  
      These series objects should all have an ID column (s['ID']) which can be used to identify the specific element.
    
      When finished, this function adds the following columns to the series object which is returned to the caller:
        MUKEY    - int scalar found for the given coordinate
        AreaSym  - string with area symbol for the given coordinate
        inSSURGO - Flag: True if MUKEY was found in SSURGO files, False if found in STATSGO file
      
      This function represents missing data using NaN (np.nan)

      Required Args:
        s -- pandas series object containing the forementioned variables
      
      Additional Notes:
        1 - The variables input_SSURGO_file, input_SSURGOperCounty_file, and input_STATSGO_file
        must be CHANGED according to users directory paths for the shape files
        2- For this program, index.shp is the delineation file, containing the border's shape from 
        SSURGO of the region we have data.
    """
    #Get the elements center coordinates
    x = s['lat_center']
    y = s['long_center'] 
    #Get setup for building the spatial index, and initialize the booleans used to indicate if
    #a MUKEY was found in SSURGO or not
    inSSURGO = False
    found = False
    #Select features and build bounding box
    #SSURGO_provider.select([SSURGO_AreaSymInd])
    request = QgsFeatureRequest()
    request.setSubsetOfAttributes([SSURGO_AreaSymInd])
    point = QgsPoint(x, y)
    pointBox = QgsRectangle(point, point)
    #Use the SSURGO spatial index to find features that intersect with the point box build from the elements coordiantes
    ids = SSURGO_spatial_index.intersects(pointBox)
    feature = QgsFeature()
    #Intersects returns a list of features intesecting the point
    #So we still have to search using .contains(), but we search
    #a lot less then using .contains() on all features
    for i in ids:
        #SSURGO_provider.featureAtId(i, feature, True, [SSURGO_AreaSymInd])
        feature = SSURGO_layer.getFeatures(request.setFilterFid(i)).next()
        geom = feature.geometry()
        if geom.contains(point):
            #Found an intersecting shape, get the AreaSym
            #attribute = feature.attributeMap()
            #AreaSym = attribute[SSURGO_AreaSymInd].toString()
            AreaSym = feature[SSURGO_AreaSymInd]
            #See if we have this layer loaded, if not load the layer and create a corresponding spatial index
            if AreaSym not in SSURGO_county_dict:
                input_SSURGOperCounty_file = os.path.join(input_SSURGO_County_prefix, "soilmu_a_") + AreaSym.lower() + ".shp"
                SSURGO_perCounty_layer  = QgsVectorLayer(input_SSURGOperCounty_file, "SSURGOperCounty",  "ogr")
                SSURGO_perCounty_provider = SSURGO_perCounty_layer.dataProvider()
                SSURGO_county_dict[AreaSym] = SSURGO_perCounty_layer
                #To avoid re-creating the per county spatial index, create it the first time we need it and
                #put it into a dict to access later.  The trade-off here is increased memory usage versus time
                #However, the number of these indicies should be relatively small compared to the number of 
                #mesh elements that must be processed, and this should remain some what proportional even as the
                #mesh size increases
                SSURGO_county_index[AreaSym] = QgsSpatialIndex()
                for feature in SSURGO_perCounty_layer.getFeatures(QgsFeatureRequest()):
                    SSURGO_county_index[AreaSym].insertFeature(feature)
            else:
                SSURGO_perCounty_layer = SSURGO_county_dict[AreaSym]
                SSURGO_perCounty_provider = SSURGO_perCounty_layer.dataProvider()
                
            SSURGO_county_AreaSymInd  = SSURGO_perCounty_provider.fieldNameIndex("AREASYMBOL")
            SSURGO_county_MUKEYInd    = SSURGO_perCounty_provider.fieldNameIndex("MUKEY")
            SSURGO_county_MUKEYSymInd = SSURGO_perCounty_provider.fieldNameIndex("MUSYM")

            #SSURGO_perCounty_provider.select([SSURGO_county_AreaSymInd,SSURGO_county_MUKEYInd, SSURGO_county_MUKEYSymInd])
            request = QgsFeatureRequest()
            request.setSubsetOfAttributes([SSURGO_county_AreaSymInd,SSURGO_county_MUKEYInd, SSURGO_county_MUKEYSymInd])
            #Find the intersecting geometries         
            ids = SSURGO_county_index[AreaSym].intersects(pointBox)
            for i in ids:
                #SSURGO_perCounty_provider.featureAtId(i, feature, True, [SSURGO_county_AreaSymInd,SSURGO_county_MUKEYInd, SSURGO_county_MUKEYSymInd])
                feature = SSURGO_perCounty_layer.getFeatures(request.setFilterFid(i)).next()
                geom = feature.geometry()
                if geom.contains(point):
                    inSSURGO = True
                    AreaSym = feature[SSURGO_county_AreaSymInd]
                    MUKEY = feature[SSURGO_county_MUKEYInd]
                    MUSYM   = feature[SSURGO_county_MUKEYSymInd]
                    found = True
                    break
            break #Stop looking if we find a contain

    #If data not found in SSURGO, check STATSGO
    if (inSSURGO == False) or ( MUSYM == 'NOTCOM'):
        inSSURGO = False # In the case MUSYM == NOTCOM, inSSURGO was = True
        found = False
        STATSGOfeature = QgsFeature()
        #STATSGO_provider.select([STATSGO_AreaSymInd, STATSGO_MUKEYInd, STATSGO_MUKEYSymInd ])
        request = QgsFeatureRequest()
        request.setSubsetOfAttributes([STATSGO_AreaSymInd, STATSGO_MUKEYInd, STATSGO_MUKEYSymInd ])
        ids = STATSGO_index.intersects(pointBox)
        for i in ids:
            #STATSGO_provider.featureAtId(i, STATSGOfeature, True, [STATSGO_AreaSymInd, STATSGO_MUKEYInd, STATSGO_MUKEYSymInd ])
            STATSGOfeature = STATSGO_layer.getFeatures(request.setFilterFid(i)).next()
            geom = STATSGOfeature.geometry()
            if geom.contains(point):
                #attribute = STATSGOfeature.attributeMap()
                AreaSym = STATSGOfeature[STATSGO_AreaSymInd]
                MUKEY   = STATSGOfeature[STATSGO_MUKEYInd]
                MUSYM = STATSGOfeature[STATSGO_MUKEYSymInd]
                found = True
                break

    if not found:
        #TODO/FIXME If not found, must handle this differently because the 
        #int cast won't handle nan!!!
        MUKEY = np.nan
        AreaSym = np.nan
        inSSURGO = np.nan
    #Return the input series with MUKEY, AreaSym, and inSSURGO added to the series
    s['MUKEY'] = int(MUKEY)
    s['AreaSym'] = str(AreaSym)
    s['inSSURGO'] = inSSURGO
    return s

def getCOKEY(s):
   """
    This function searches SSURGO and STATSGO data files for an element's COKEY using pyQGIS functions.

    The series, s, should contain a boolean entery s['inSSURGO'] indicating whether or not to look in SSURGO or STATSGO.
    This value is set in the getMUKEY function based on where the MUKEY was found.

    Since many of these data files get read repeatedly in different calls to getCOKEY, the data is only read
    from the files once and is stored in dictionaries so it can be quickly looked up again in another call needing
    to read the same information.

    It is designed to work with the pandas data frame's apply function, which, when used along axis=1,
    will apply this function to each row of a dataframe, passing the row's information in the series argument.
    This dataframe should have columns for the following variables that are set in getMUKEY:
      MUKEY
      AreaSym
      inSSURGO

    These series objects should all have an ID column (s['ID']) which can be used to identify the specific element.
    
    When finished, this function adds the following columns to the series object which is returned to the caller:
      COKEY       - scalor with the dominant COKEY for the given MUKEY
      compname    - descriptor
      
    This function represents missing data using NaN (np.nan)

    Required Args:
      s -- pandas series object containing the forementioned variables

    Additional Notes:
      1 - Component dominance chosen by relative percentage content in map Unit
          Colum 2 (colum index 1) from SSURGO/STASGO Metadata
      2 - tmpCompDesc - Component description: forth colum (index 3) of comp.txt. 
      3 - In case of water or bed rock there is no horizon and getSoilContent (...) takes care of it.
      4 - All indices are taken from the SSURGO metadata rows and colums commented file.
   """

   MUKEY = s['MUKEY']
   AreaSym = s['AreaSym']
   inSSURGO = s['inSSURGO']
      
   #TODO HANDLE NANs
   if (inSSURGO == True):
   #  Find the path for the comp file in SSURGO
      if AreaSym not in SSURGO_comp_dict:
         Compfile = os.path.join(input_SSURGO_unzipped, "wss_SSA_") + str(AreaSym) + "*"
         Compfile_path =  glob.glob(Compfile)                                                        
         # gol.glob(path) -> returns a possibly empty list the auto completing options for the * terminated path given
         Compfile = Compfile_path[0] +'/tabular/comp.txt'
         SSURGO_comp_dict[AreaSym] = pd.read_csv(Compfile, sep='|', names=comp_table_names, usecols=['comppct_r', 'compname', 'mukey', 'cokey']) 
      comp_metadata = SSURGO_comp_dict[AreaSym]
         
   else:
      if AreaSym not in STATSGO_comp_dict:
         #Find the path for the comp file in STATSGO
         Compfile = os.path.join(input_STATSGO_unzipped, "wss_gsmsoil_") + str(AreaSym) + "*" 
         Compfile_path =  glob.glob(Compfile)                                                        
         #gol.glob(path) -> returns a possibly empty list the auto completing options for the * terminated path given
         Compfile = Compfile_path[0] +'/tabular/comp.txt'
         STATSGO_comp_dict[AreaSym] = pd.read_csv(Compfile, sep='|', names=comp_table_names, usecols=['comppct_r', 'compname', 'mukey', 'cokey'])
      comp_metadata = STATSGO_comp_dict[AreaSym]

   mu_key_rows = comp_metadata[comp_metadata['mukey'] == MUKEY]

   if not mu_key_rows.empty:
      #Found a valid cokey based on mukey, get largest percent entry if konwn
      index = mu_key_rows['comppct_r'].idxmax()
      #It is possible that a relative percent doesn't exist.  If it doesn't,
      #we use the first in the list of mukey entries
      if pd.np.isnan(index):
        max_perc = mu_key_rows.iloc[0]
      else:
        max_perc = mu_key_rows.ix[index]
      s['COKEY'] = max_perc['cokey']
      s['compname'] = max_perc['compname']
   else:
      #no cokey found
      s['COKEY'] = np.nan
      s['compname'] = np.nan

   return s

#Helper function to modify totals if rel_percent is not 100
def rel_percent_mod(s):
    """
        This function is a helper function for getSoilContent
        
        When applied to a pandas dataframe containing all of the soil type information,
        this function modifies the totals if the the relative percent is not 100.
        
        Required Arguments:
            s -- pandas series containing sandtotal_r, silttatal_r, claytotal_r, and rel_percent
    """
    if s['rel_percent'] != 100.0 and s['rel_percent'] != 0:
        s['sandtotal_r'] = s['sandtotal_r']*100/s['rel_percent']
        s['silttotal_r'] = s['silttotal_r']*100/s['rel_percent']
        s['claytotal_r'] = s['claytotal_r']*100/s['rel_percent']
    return s
  
def getSoilContent(s):
   """
       This function reads SSURGO or STATSGO files to identify an elements soil conent,
       calculate the relative percentage of the soils, and find the soil type and horizon thickness
       for each layer of soil found.
       
       This function is desgned to work with pandas series objects, and assumes that the passed series
       contains the following columns:
           COKEY - Vector with all the COKEYs for the given MUKEY found in getCOKEY()
           AreaSym   - Qstring with area symbol found in getMUKEY()
           inSSURGO  - Flag: True if MUKEY was found in SSURGO files, False if found in STATSGO file
       
       When finished, this function returns the input series with the addition of an array of soil types for each
       layer in the chorizon file, listed in order of the layer's appearence.
       
       Each line represents a CHorizon key for each COKEY found previously.
       largerRep is the largest number of CHKEYS between all the COKEYS.
       
       Required Arguements:
           s - pandas series object with the forementioned columns
           
       Additional Notes:
          Line one (index 0, all colums) of CHKEY contains the CHKEYs for the first COKEY found.
          Because the metadata files are organized in crescent order of MUKEYs, COKEYs, and
          CHKEYS, the first COKEY is the smallest in value.
   """

   inSSURGO = s['inSSURGO']
   AreaSym = s['AreaSym']
   COKEY = s['COKEY']
   CompDesc = s['compname']
      
   #TODO HANDLE NANs
   if (CompDesc == 'Rock outcrop' or CompDesc == 'Rubble land'):
      s['SoilType'] = np.array([15])
      return s
   else:
      if (CompDesc == 'Water'):
         s['SoilType'] = np.array([14])
         return s

   if (inSSURGO == True):
      if AreaSym not in SSURGO_horizon_dict:
         #Find the path for the chorizon file in SSURGO
         Horizonfile = os.path.join(input_SSURGO_unzipped, "wss_SSA_") + AreaSym + "*"
         Horizonfile_path =  glob.glob(Horizonfile)
         Horizonfile = Horizonfile_path[0] +'/tabular/chorizon.txt'
         SSURGO_horizon_dict[AreaSym] = pd.read_csv(Horizonfile, sep='|', names=horizon_table_names, usecols=['cokey', 'chkey', 'hzdept_r', 'hzdepb_r', 'sandtotal_r', 'silttotal_r', 'claytotal_r'])
      Hrz_metadata = SSURGO_horizon_dict[str(AreaSym)]
   else:
      if AreaSym not in STATSGO_horizon_dict:
         #Find the path for the chorizon file in STATSGO
         Horizonfile = os.path.join(input_STATSGO_unzipped, "wss_gsmsoil_") + AreaSym + "*" 
         Horizonfile_path =  glob.glob(Horizonfile)
         Horizonfile = Horizonfile_path[0] +'/tabular/chorizon.txt'
         STATSGO_horizon_dict[AreaSym] = pd.read_csv(Horizonfile, sep='|', names=horizon_table_names, usecols=['cokey', 'chkey', 'hzdept_r', 'hzdepb_r', 'sandtotal_r', 'silttotal_r', 'claytotal_r'])
      Hrz_metadata = STATSGO_horizon_dict[AreaSym]
   
   #Get all rows with matching COKEY
   co_key_rows = Hrz_metadata[Hrz_metadata['cokey'] == COKEY]
   #Make sure we got at least one row, and sort it based on horizon depth
   if not co_key_rows.empty:
      co_key_rows.sort_values('hzdept_r', axis=0, inplace=True)
   else:
      #No soil data found
      s['SoilType'] = np.nan
      s['HorizonThickness'] = np.nan
      return s
   
   #Calculate relative percent of soil types for each row  
   
   co_key_rows['rel_percent'] = co_key_rows['sandtotal_r'] + co_key_rows['silttotal_r'] + co_key_rows['claytotal_r']
   #Modify if rel_percent != 100
   co_key_rows = co_key_rows.apply(rel_percent_mod, axis=1)
   s['SoilType'] = co_key_rows.apply(getSoilTyp, axis = 1).values
   s['HorizonThickness'] = co_key_rows['hzdepb_r'].values
   s['HorizonThickness'] = s['HorizonThickness']/100.0 # Convert from centimeters to meters.
   return s

def getSoilTyp(s):
   """
       This is a helper function for getSoilContent.
       
       This function maps soil content to a soil type.
       It returns this soil type to the caller.
       
       Required Arguments:
           s -- A pandas series containing the soil content values for an element (sandtotal_r, silttotal_r, claytotal_r)
           
       Additional Notes:
           Returned soil type referenced from SOILPARM.TBL from NoahMP,
           program converted from https://prod.nrcs.usda.gov/Internet/FSE_DOCUMENTS/nrcs142p2_053196.xls
   """

   sand = s['sandtotal_r']
   silt = s['silttotal_r']
   clay = s['claytotal_r']
   
   if np.isnan(sand):
      sand = 0.0
   if np.isnan(silt):
      silt = 0.0
   if np.isnan(clay):
      clay = 0.0
   
   #TODO what should be done when information about soil content isn't available for a particular chkey???
   if ( (sand + silt + clay) == 0 ):
      return -1
   if ((silt + 1.5*clay) < 15):
      return 1 # 'Sand'
   if ( (silt + 1.5*clay) >= 15 and (silt + 2*clay) < 30 ):
      return 2 #'Loamy Sand'
   if ( (clay >= 7) and (clay < 20) and (sand > 52) and ( (silt + 2*clay) >= 30) or ( (clay < 7) and (silt < 50) and ( (silt+2*clay)>=30)  ) ):
      return 3 #'Sandy Loam'
   if ( (silt >= 50 and (clay >= 12 and clay < 27) ) or ( (silt >= 50 and silt < 80) and clay < 12) ):
      return 4 #'Silt Loam'
   if (silt >=  80 and clay < 12):
      return 5 # 'Silt'
   if ( (clay >= 7 and clay < 27) and (silt >= 28 and silt < 50) and (sand <= 52) ):
      return 6 # 'Loam'
   if ((clay >= 20 and clay < 35) and (silt < 28) and (sand > 45) ):
      return 7  #'Sandy Clay Loam'
   if ( (clay >= 27 and clay < 40) and (sand  <= 20) ):
      return 8  #'Silt Clay Loam'
   if ( (clay >= 27 and clay < 40) and (sand > 20 and sand <= 45) ):
      return 9  # 'Clay Loam'
   if (clay >= 35 and sand > 45):
      return 10 # 'Sandy Clay'
   if (clay >= 40 and silt >= 40):
      return 11 # 'Silty Clay'
   if ( (clay >= 40) and (sand <= 45) and (silt < 40) ):
      return 12 # 'Clay'
   return -1 # In case all the above statements fail

def getVegParm(s):
   """
       This function reads the National Land Cover Database (NLCD) to assign a vegitation
       type to each element.

       When finished, this function will add a column to s, VegParm, and return s to the caller.
       
       Missing data is represened as NaN (np.nan)
       
       Required Arguements:
           s -- pandas series object containing the following columns:
               AD_X
               AD_Y
               AreaSym
   """
   #  Find the path for the chorizon file in STATSGO
   x = s['AD_X']
   y = s['AD_Y']
   AreaSym = s['AreaSym']
   if AreaSym not in veg_parm_dict:
      if s['inSSURGO']:
         input_NLCDfile =  input_NLCD_directory_path + AreaSym[0].lower() + AreaSym[1].lower() + ".tif"
      else:
         input_NLCDfile =  input_NLCD_directory_path + AreaSym.lower() + ".tif"
      veg_parm_dict[AreaSym] = QgsRasterLayer(input_NLCDfile)
   
   layer = veg_parm_dict[AreaSym]
   query = layer.dataProvider().identify(QgsPoint(x, y), QgsRaster.IdentifyFormatValue)
   val = -1
   # Map the NLCD2011 20-category classificaiton into USGS 27-category in the VEGPARM.TBL form Noah-MP.
      # NLCD    definition                     USGS         definition
      #	11	open water	               16           Water Bodies   
      #	12	ice/snow	               24           Snow or Ice
      #	21	developed open space	       1            Urban and Built-Up Land
      #	22	developed low intensity	       1            Urban and Built-Up Land
      #	23	developed medium intensity     1            Urban and Built-Up Land
      #	24	developed high intensity       1            Urban and Built-Up Land
      #	31	barren land	               19           Barren or Sparsely Vegetated
      #	41	deciduous forest	       11 and 12    'Deciduous Broadleaf Forest' and "Deciduous Needleleaf Forest"
      #	42	evergreen forest	       13 and 14    'Evergreen Broadleaf Forest' and 'Evergreen Needleleaf Forest'
      #	43	mixed forest	               15           'Mixed Forest' 
      #	51	dwarf scrub-Alaska only	       none/use 22  'Mixed Tundra' 22
      #	52	shrub	                       8            'Shrubland' 
      #	71	grassland	               7            'Grassland' 
      #	72	sedge/herbaceous-Alaska only   none/use 22
      # 73	Lichens-Alaska only	       none/use 22
      #	74	Moss-Alaska only	       none/use 22
      # 81	Pasture	                       2?            'Dryland Cropland and Pasture' 
      # 82	cultivated crops	       3?            'Irrigated Cropland and Pasture' 
      # 90	woody wetland	               18            'Wooded Wetland' 
      # 95	herbaceous wetland	       17            'Herbaceous Wetland' 
   if query.isValid():
      data = str(int(query.results()[1]))
      #TODO/FIXME This is a static mapping!!! Treat it as such and use a dict!
      if (data == '11'):
         val = 16
      elif (data == '12'):
         val = 24
      elif (data == '21'):
         val = 1
      elif (data == '22'):
         val = 1
      elif (data == '23'):
         val = 1
      elif (data == '24'):
         val = 1
      elif (data == '31'):
         val = 19
      elif (data == '41'):
         val = 11
      elif (data == '42'):
         val = 13   
      elif (data == '43'):
         val = 15   
      elif (data == '51'):
         val = 22
      elif (data == '52'):
         val = 8
      elif (data == '71'):
         val = 7   
      elif (data == '72'):
         val = 22
      elif (data == '73'):
         val = 22            
      elif (data == '74'):
         val = 22
      elif (data == '81'):
         val = 2
      elif (data == '82'):
         val = 3   
      elif (data == '90'):
         val = 18   
      elif (data == '95'):
         val = 17      
      else:
         assert (data == '128')
   #set veg type to the value val, or np.nan if not success, then return the series
   s['VegParm'] = val
   return s  

def get_GEOLOGIC_UNITS(s):
            """
      This function searches for the geologic units in the shape files downloaded from http://mrdata.usgs.gov/geology/state/
      
      It is designed to work with the pandas data frame's apply function, which, when used along axis=1,
      will apply this function to each row of a dataframe, passing the row's information in the series argument.
      This dataframe should have columns for the following variables:
        lat_center
        long_center
        AreaSym
      AreaSym is found in getMUKEY() function. Therefore, get_GEOLOGIC_UNITS() can only be run after GetMUKEY().
      
      1 refers to alluvium, 0 refers to not alluvium.     
            """
            Source = s['AreaSym']
            x = s['lat_center']
            y = s['long_center']
            point = QgsPoint(x, y)
            pointBox = QgsRectangle(point, point)
            
            # AreaSym string name starts with the state abbreviation.
            #It is used to access the GEOL_spatial_index dictionary and GEOL_Unit_State_VectorLayer_dict
            #i.e. GEOL_spatial_index[ Source[0:2] ]
            GEOL_perState_layer    = GEOL_Unit_State_VectorLayer_dict[ Source[0:2] ]
            GEOL_perState_provider = GEOL_perState_layer.dataProvider()
            GEOL_State_ROCKTYPE = provider.fieldNameIndex("ROCKTYPE1")
            #select the rocktype features to work with
            #provider.select([GEOL_State_ROCKTYPE]) 
            request = QgsFeatureRequest()
            request.setSubsetOfAttributes([GEOL_State_ROCKTYPE])
            feature = QgsFeature()               
            found = False
            ids = GEOL_spatial_index[Source[0:2]].intersects(pointBox)
            for i in ids:
                #GEOL_perState_provider.featureAtId(i, feature, True, [GEOL_State_ROCKTYPE])
                feature = GEOL_layer.getFeatures(request.setFilterFid(i)).next()
                geom = feature.geometry()
                if geom.contains(point):
                    #attribute = feature.attributeMap()
                    #RockType = str(attribute[GEOL_State_ROCKTYPE].toString())
                    RockType = str(feature[GEOL_State_ROCKTYPE])
                    found = True
                    break
            #break #Stop looking if we find a contain
    
            if not found:
               #print("Geologic type not found. Assuming not Alluvium")
               RockType = "Other"
        
            if (RockType == "alluvium"):
                s['ROCKTYPE'] = int(1)
            else:
                s['ROCKTYPE'] = int(0)
            return s

import multiprocessing as mp
from math import ceil
if __name__ == '__main__':
    print "Initial memory: "+str(mem_usage())
    #Run the getSoilTypDRV function
    ELEfilepath           = os.path.join(input_directory_path, 'mesh.1.ele')
    
    pool = mp.Pool()
    num_elems = 0
    with open(ELEfilepath, 'r') as f:
        num_elems = int(f.readline().split()[0])
    cores = mp.cpu_count() 
    print "Processing "+str(num_elems)+" on "+str(cores)+" cores."
    chunk_size = int( ceil(num_elems/float(cores)) )
    chunks = []
    for i in range(0, cores):
        upper = chunk_size*(i+1)
        upper = upper if num_elems > upper else num_elems
        chunks.append(range(chunk_size*i,upper))
    #print chunks
    #Pass chunks to cores to spread the workload
    t0 = time.time()
    results = pool.map(getSoilTypDRV, chunks)
    t1 = time.time()    
    print "DONE in "+str(t1-t0)+" seconds."
    elements = pd.concat(results, ignore_index=True)
    #print elements
    print 'Writing element data file.'

    #File paths for final outputs
    output_SoilTyp_file   = os.path.join(output_directory_path, 'mesh.1.soilType')
    output_VegTyp_file    = os.path.join(output_directory_path, 'mesh.1.landCover')
    output_GeolTyp_file   = os.path.join(output_directory_path, 'mesh.1.geolType')
    output_element_data_file = os.path.join(output_directory_path, 'element_data.pkl')


    #Write the element data serialized file
    elements.to_pickle(output_element_data_file)
   
     #If you want to reload the data instead of recomputing it comment out the lines since Finding MUKEY and uncomment this line
    #elements = pd.read_pickle(output_element_data_file)
   
    print 'Writing soil file.'
   
    #Get the output file handle for the soil file
    SoilFile = open(output_SoilTyp_file, 'wb')
    #Write soil file header information
    SoilFile.write(str(num_elems))
    SoilFile.write('\n')
    #Write each element
    elements.apply(writeSoilFile, axis=1, args=(SoilFile, None))
    SoilFile.close()
   
    print 'Writing vegetation file.'
   
    #Get the vegetation output file handle
    VegFile = open(output_VegTyp_file, 'wb')
    #Write veg file header information
    VegFile.write(str(num_elems))
    VegFile.write('\n')
    #Write each element
    elements.apply(writeVegFile, axis=1, args=(VegFile, None))
    VegFile.close()
   
    print 'Writing Geologic Unit data file.'
    #Get the output file handle for the soil file
    GeolFile = open(output_GeolTyp_file, 'wb')
    #Write soil file header information
    GeolFile.write(str(num_elems))
    GeolFile.write('\n')
    #Write each element
    elements.apply(writeGeolFile, axis=1, args=(GeolFile, None))
    GeolFile.close()

    #Qgis finalize
    #really should exitQgis...but segfaults for no good reason!!! ignore for now 
    #app.exitQgis()
    #QgsApplication.exit()
    print "Final memory: "+str(mem_usage())
    import sys
    sys.stdout.flush()

