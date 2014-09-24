#!/usr/bin/env python
import os
import pandas as pd
import numpy as np
import glob
from math import acos, cos
from qgis.core import * #VERY BAD PRACTICE...SHOULD IMPORT QGIS.CORE AND FIND ALL REFERENCES AND PREPEND QGIS.CORE.*
#import cython #Maybe try to cythonize some functions for better performance???
#To run this scrip outside of the Qgis Console, make sure you run

#$export PYTHONPATH=/opt/share/qgis/python:$PYTHONPATH
#From the command line in the terminal you will run python in #TODO can do this programatically in the script if needed...
#Qgis setup, here so GLOBALS get setup right
qgishome = '/opt'
app = QgsApplication([], True)
QgsApplication.setPrefixPath(qgishome, True)
QgsApplication.initQgis()

#GLOBALS
#These are reused each time getMUKEY is called, so create them once and save the overhead
#Can even put into an init function if the need arises
input_SSURGO_file  = "/share/CI-WATER Third Party Files/SSURGO/index.shp"
SSURGO_layer       = QgsVectorLayer(input_SSURGO_file, "SSURGO",  "ogr")
SSURGO_provider    = SSURGO_layer.dataProvider()
SSURGO_AreaSymInd = SSURGO_provider.fieldNameIndex("AREASYMBOL")

input_STATSGO_file  = "/share/CI-WATER Third Party Files/STATSGO/all.shp"
STATSGO_layer     = QgsVectorLayer(input_STATSGO_file, "STATSGO",  "ogr")
STATSGO_provider  = STATSGO_layer.dataProvider()
STATSGO_AreaSymInd        = STATSGO_provider.fieldNameIndex("AREASYMBOL")
STATSGO_MUKEYInd          = STATSGO_provider.fieldNameIndex("MUKEY")
STATSGO_MUKEYSymInd       = STATSGO_provider.fieldNameIndex("MUSYM")

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

#Write a line to the soil file, f, based on information passed in series, s
def writeSoilFile(s, f, bugFix):
    f.write('  '+str(s.name)+'  ')
    #isnull might be redundant, since the only thing not an array should be NaN
    if isinstance(s['SoilType'], float) and pd.isnull(s['SoilType']):
        f.write('-1')
    else:
	f.write(str(len(s['SoilType']))+'  ')
	#Write soiltype, horizon thinckness pairs (s,d)
	if isinstance(s['HorizonThickness'], float):
		f.write(str(s['SoilType'][0])+', -1')
	else:
	    for pair in zip(s['SoilType'], s['HorizonThickness']):
                f.write(str(pair[0])+','+str(pair[1])+'  ')
    f.write('\n')

#Write a line to the veg file, f, based on information passed in series, s
#Note bugFix is here just to bypass a weird file not open for reading bug that appears
#if s and f are the only args
def writeVegFile(s, f, bugFix):
    f.write('  '+str(s.name)+'  '+str(s['VegParm'])+'\n')

def getSoilTypDRV():
# INPUT:
# bool variable layered - True if the information requested is for all soil horizons
#                       - False if the information requested is only for the top layer.
# Output:
#  If layered = False:
#     A file mesh.1.soil with the soil type for each element of mesh.1.ele. 
#  If layered = True:
#     A file mesh.1.soil with the soil type for each horizon of each element of mesh.1.ele.
#     Horizons are in order of depth of top to the surface. 
#  *********************************
#  ****** IMPORTANT ****************
#  *********************************
#  If needed, change the variables:
#  sufixPath           -> location of the ADHydro repository
#  ELEfilepath         -> element file path and name
#  NODEfilepath        -> node file path and name
#  output_SoilTyp_file -> output: soil type file path and name
   sufixPath             = os.path.join(os.path.join(os.path.split(os.getcwd())[0],'input'), 'small_green_mesh')
   ELEfilepath           = os.path.join(sufixPath, 'mesh.1.ele')
   NODEfilepath          = os.path.join(sufixPath, 'mesh.1.node')
   output_SoilTyp_file   = os.path.join(sufixPath, 'mesh.1.soilType')
   output_VegTyp_file    = os.path.join(sufixPath, 'mesh.1.LandCover')
   
   elements = pd.read_csv(ELEfilepath, sep=' ', skipinitialspace=True, comment='#', skiprows=1, names=['ID', 'V1', 'V2', 'V3', 'V4'], index_col=0, engine='c').dropna()
   
   elements['V1'] = elements['V1'].astype(int)

   nodes = pd.read_csv(NODEfilepath, sep=' ', skipinitialspace=True, comment='#', skiprows=1, names=['ID', 'X', 'Y', 'Z'], index_col=0, engine='c').dropna()

   num_elems = len(elements.index)
   num_nodes = len(nodes.index)
   
  
   
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
   v.index = range(1, num_elems+1)
   
   elements['X_center'] = v.X - 20000000.0
   elements['Y_center'] = v.Y - 10000000.0
   
   v = nodes.loc[elements['V2']].reset_index()
   v.index = range(1, num_elems+1)
   elements['X_center'] = elements['X_center'] + v.X - 20000000.0
   elements['Y_center'] = elements['Y_center'] + v.Y - 10000000.0
   
   v = nodes.loc[elements['V3']].reset_index()
   v.index = range(1, num_elems+1)
   elements['X_center'] = elements['X_center'] + v.X - 20000000.0
   elements['Y_center'] = elements['Y_center'] + v.Y - 10000000.0
   
   #Center based on number of verticies
   elements['X_center'] = elements['X_center']/num_verticies
   elements['Y_center'] = elements['Y_center']/num_verticies
   
   #ADHydro coordinates for looking up vegitation data
   elements['AD_X'] = elements['X_center'] + 20000000.0
   elements['AD_Y'] = elements['Y_center'] + 10000000.0
   
   # Changing from x,y sinusoidal projection to latitude and longitude coordinates
   elements['long_center'] = elements['Y_center']/R
   elements['lat_center'] = elements['X_center'] / (R*np.cos(elements['long_center']))
   elements['lat_center'] = (elements['lat_center'] + origin)*(180/pi)
   elements['long_center'] = elements['long_center']*180/pi
   
   #TODO remove, for debugging only
   #elements = elements.head()
   
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
   #Since the mukey and areasym are used to lookup different data, find and store them for each element.
   
   #find the MUKEY for each element, this adds the following columns to elements:
   #MUKEY, AreaSym, inSSURGO
   elements = elements.apply(getMUKEY, axis=1)
   #Find the COKEY for each element, must have MUKEY column before calling getCOKEY, this adds the following columns to elements:
   #COKEY, compname
   elements = elements.apply(getCOKEY, axis=1)
   #get soil content per element, MUST have COKEY and compname columns before calling getSoilContent, this adds the following columns to elements:
   #SoilType
   elements = elements.apply(getSoilContent, axis=1)
   #print elements
   #get vegitation parameters, must have AreaSym column before calling getVegParm, this adds the following columns to elements:
   #VegParm
   elements = elements.apply(getVegParm, axis=1)
   
   elements.to_csv('element_data.csv', na_rep='NaN')
   
   SoilFile = open(output_SoilTyp_file, 'wb')
   #Write soil file header information
   SoilFile.write(str(num_elems))
   SoilFile.write('  ')
   
   SoilFile.write('\n')
   #Write each element
   elements.apply(writeSoilFile, axis=1, args=(SoilFile, None))
   SoilFile.close()
   
   VegFile = open(output_VegTyp_file, 'wb')
   #Write veg file header information
   VegFile.write(str(num_elems)+'  1\n')
   #Write each element
   elements.apply(writeVegFile, axis=1, args=(VegFile, None))
   VegFile.close()
   
   
#TODO If really need to specify which dataset to search( SSURGO or STATSGO ) then make two functions, one for each
#Otherwise just search SSURGO first, then STATSGO if SSURGO fails to provide results    
def getMUKEY(s):
# in:
# s series containing information for the given element (identifiable by s['ID'])
# out: 
# Series containtining all the previous element information given in s plus the following
    # MUKEY    - int scalar found for the given coordinate
    # AreaSym  - string with area symbol for the given coordinate
    # inSSURGO - Flag: True if MUKEY was found in SSURGO files, False if found in STATSGO file
    # reports values as NaN (np.nan) for missing data
# *************** IMPORTANT ******************
# 1- The variables input_SSURGO_file, input_SSURGOperCounty_file, and input_STATSGO_file
# must be CHANGED according to users directory paths for the shape files
# 2- For this program, index.shp is the delineation file, containing the border's shape from 
# SSURGO of the region we have data.
    x = s['lat_center']
    y = s['long_center']
    feature  = QgsFeature()
    inSSURGO = False
    found = False
    SSURGO_provider.select([SSURGO_AreaSymInd])

    index = QgsSpatialIndex()
    while SSURGO_provider.nextFeature(feature):
        index.insertFeature(feature)
    point = QgsPoint(x, y)
    pointBox = QgsRectangle(point, point)
    ids = index.intersects(pointBox)
    
    #Intersects returns a list of features intesecting the point
    #So we still have to search using .contains(), but we search
    #a lot less then using .contains() on all features
    for i in ids:
        SSURGO_provider.featureAtId(i, feature, True, [SSURGO_AreaSymInd])
        geom = feature.geometry()
        if geom.contains(point):
            #This should really only get set if the next shape file is found to contain the coordinates as well
            #inSSURGO = True # True if the x,y coordinates are inside any shape in SSURGO. False otherwise
            attribute = feature.attributeMap()
            AreaSym = attribute[SSURGO_AreaSymInd].toString()

            #TODO Maybe load all these into memory initally then lookup from dict???
            if AreaSym not in SSURGO_county_dict:
                input_SSURGOperCounty_file = "/share/CI-WATER Third Party Files/SSURGO/all/soilmu_a_"+ AreaSym.toLower() + ".shp"
                SSURGO_perCounty_layer  = QgsVectorLayer(input_SSURGOperCounty_file, "SSURGOperCounty",  "ogr")
                SSURGO_perCounty_provider = SSURGO_perCounty_layer.dataProvider()
                SSURGO_county_dict[AreaSym] = SSURGO_perCounty_layer
            else:
                SSURGO_perCounty_layer = SSURGO_county_dict[AreaSym]
                SSURGO_perCounty_provider = SSURGO_perCounty_layer.dataProvider()
                
            SSURGO_county_AreaSymInd  = SSURGO_perCounty_provider.fieldNameIndex("AREASYMBOL")
            SSURGO_county_MUKEYInd    = SSURGO_perCounty_provider.fieldNameIndex("MUKEY")
            SSURGO_county_MUKEYSymInd = SSURGO_perCounty_provider.fieldNameIndex("MUSYM")
            feature  = QgsFeature()
            SSURGO_perCounty_provider.select([SSURGO_county_AreaSymInd,SSURGO_county_MUKEYInd, SSURGO_county_MUKEYSymInd])
            
            index = QgsSpatialIndex()
            while SSURGO_perCounty_provider.nextFeature(feature):
                index.insertFeature(feature)
            ids = index.intersects(pointBox)
            for i in ids:
                SSURGO_perCounty_provider.featureAtId(i, feature, True, [SSURGO_county_AreaSymInd,SSURGO_county_MUKEYInd, SSURGO_county_MUKEYSymInd])
                geom = feature.geometry()
                if geom.contains(point):
                    inSSURGO = True
                    attribute = feature.attributeMap()
                    AreaSym = attribute[SSURGO_county_AreaSymInd].toString()
                    MUKEY = attribute[SSURGO_county_MUKEYInd].toString()
                    MUSYM   = attribute[SSURGO_county_MUKEYSymInd].toString()
                    found = True
                    break
            break #Stop looking if we find a contain

    #If data not found in SSURGO, check STATSGO
    if (inSSURGO == False) or ( MUSYM == 'NOTCOM'):
        inSSURGO = False # In the case MUSYM == NOTCOM, inSSURGO was = True
        found = False

        STATSGOfeature  = QgsFeature()
        STATSGO_provider.select([STATSGO_AreaSymInd, STATSGO_MUKEYInd, STATSGO_MUKEYSymInd ])
        index = QgsSpatialIndex()  
        while STATSGO_provider.nextFeature(STATSGOfeature):
            index.insertFeature(STATSGOfeature)
        ids = index.intersects(pointBox)
        for i in ids:
            STATSGO_provider.featureAtId(i, STATSGOfeature, True, [STATSGO_AreaSymInd, STATSGO_MUKEYInd, STATSGO_MUKEYSymInd ])
            geom = STATSGOfeature.geometry()
            if geom.contains(point):
                attribute = STATSGOfeature.attributeMap()
                AreaSym = attribute[STATSGO_AreaSymInd].toString() 
                MUKEY   = attribute[STATSGO_MUKEYInd].toString()
                MUSYM = attribute[STATSGO_MUKEYSymInd].toString()
                found = True
                break

    if not found:
        MUKEY = np.nan
        AreaSym = np.nan
        inSSURGO = np.nan

    #Return the input series with MUKEY, AreaSym, and inSSURGO added to the series
    s['MUKEY'] = int(MUKEY)
    s['AreaSym'] = str(AreaSym)
    s['inSSURGO'] = inSSURGO
    return s

def getCOKEY(s):
# in:
# series containing data for givin element (s['ID']) with the following values accessible by s['<key>']
    # MUKEY       - int scalar found in getMUKEY()
    # AreaSym     - Qstring with area symbol found in getMUKEY()
    # inSSURGO    - Flag: True if MUKEY was found in SSURGO files, False if found in STATSGO file
# out:
#series containing element information plus
    # COKEY       - scalor with the dominant COKEY for the given MUKEY
    # compname    - descriptor
    # reports np.nan for missing data
# NOTE: Component dominance chosen by relative percentage content in map Unit
#       Colum 2 (colum index 1) from SSURGO/STASGO Metadata
# tmpCompDesc - Component description: forth colum (index 3) of comp.txt. 
# In case of water or bed rock there is no horizon and getSoilContent (...) takes care of it.
# NOTE: All indices are taken from the SSURGO metadata rows and colums commented file.
#

   MUKEY = s['MUKEY']
   AreaSym = s['AreaSym']
   inSSURGO = s['inSSURGO']
      
   #TODO HANDLE NANs
   if (inSSURGO == True):
   #  Find the path for the comp file in SSURGO
      if AreaSym not in SSURGO_comp_dict:
         Compfile = "/share/CI-WATER Third Party Files/SSURGO/unzipped/wss_SSA_" + str(AreaSym) + "*" 
         Compfile_path =  glob.glob(Compfile)                                                        
         # gol.glob(path) -> returns a possibly empty list the auto completing options for the * terminated path given
         Compfile = Compfile_path[0] +'/tabular/comp.txt'
         SSURGO_comp_dict[AreaSym] = pd.read_csv(Compfile, sep='|', names=comp_table_names, usecols=['comppct_r', 'compname', 'mukey', 'cokey']) 
      comp_metadata = SSURGO_comp_dict[AreaSym]
         
   else:
      if AreaSym not in STATSGO_comp_dict:
         #Find the path for the comp file in STATSGO
         Compfile = "/share/CI-WATER Third Party Files/STATSGO/unzipped/wss_gsmsoil_" + str(AreaSym) + "*" 
         Compfile_path =  glob.glob(Compfile)                                                        
         #gol.glob(path) -> returns a possibly empty list the auto completing options for the * terminated path given
         Compfile = Compfile_path[0] +'/tabular/comp.txt'
         STATSGO_comp_dict[AreaSym] = pd.read_csv(Compfile, sep='|', names=comp_table_names, usecols=['comppct_r', 'compname', 'mukey', 'cokey'])
      comp_metadata = STATSGO_comp_dict[AreaSym]

   mu_key_rows = comp_metadata[comp_metadata['mukey'] == MUKEY]
   if not mu_key_rows.empty:
      #Found a valid cokey based on mukey, get largest percent entry
      max_perc = mu_key_rows.ix[mu_key_rows['comppct_r'].idxmax()]
      s['COKEY'] = max_perc['cokey']
      s['compname'] = max_perc['compname']
   else:
      #no cokey found
      s['COKEY'] = np.nan
      s['compname'] = np.nan
   return s

#Helper function to modify totals if rel_percent is not 100
def rel_percent_mod(s):
    if s['rel_percent'] != 100.0:
        s['sandtotal_r'] = s['sandtotal_r']*100/s['rel_percent']
        s['silttotal_r'] = s['silttotal_r']*100/s['rel_percent']
        s['claytotal_r'] = s['claytotal_r']*100/s['rel_percent']
    return s
  
def getSoilContent(s):
# in:
# s series containing element information such as MUKEY, COKEY, AreaSym, InSSURGO, and coordinates
    # COKEY[]   - Vector with all the COKEYs for the given MUKEY found in getCOKEY()
    # AreaSym   - Qstring with area symbol found in getMUKEY()
    # inSSURGO  - Flag: True if MUKEY was found in SSURGO files, False if found in STATSGO file
# out:
# series containing previous data plus an array of soil types for each layer in the chorizon file, listed in order
#        of the layer's appearence
# Each line represents a CHorizon key for each COKEY found previously.
# largerRep is the largest number of CHKEYS between all the COKEYS.
# Note: Line one (index 0, all colums) of CHKEY contains the CHKEYs for the first COKEY found.
#       Because the metadata files are organized in crescent order of MUKEYs, COKEYs, and
#       CHKEYS, the first COKEY is the smallest in value.            
# AreaSym is of type Qstring and will be need as type str
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
         Horizonfile = "/share/CI-WATER Third Party Files/SSURGO/unzipped/wss_SSA_" + AreaSym + "*"
         Horizonfile_path =  glob.glob(Horizonfile)
         Horizonfile = Horizonfile_path[0] +'/tabular/chorizon.txt'
         SSURGO_horizon_dict[AreaSym] = pd.read_csv(Horizonfile, sep='|', names=horizon_table_names, usecols=['cokey', 'chkey', 'hzdept_r', 'hzdepb_r', 'sandtotal_r', 'silttotal_r', 'claytotal_r'])
      Hrz_metadata = SSURGO_horizon_dict[str(AreaSym)]
   else:
      if AreaSym not in STATSGO_horizon_dict:
         #Find the path for the chorizon file in STATSGO
         Horizonfile = "/share/CI-WATER Third Party Files/STATSGO/unzipped/wss_gsmsoil_" + AreaSym + "*" 
         Horizonfile_path =  glob.glob(Horizonfile)
         Horizonfile = Horizonfile_path[0] +'/tabular/chorizon.txt'
         STATSGO_horizon_dict[AreaSym] = pd.read_csv(Horizonfile, sep='|', names=horizon_table_names, usecols=['cokey', 'chkey', 'hzdept_r', 'hzdepb_r', 'sandtotal_r', 'silttotal_r', 'claytotal_r'])
      Hrz_metadata = STATSGO_horizon_dict[AreaSym]
   
   #Get all rows with matching COKEY
   co_key_rows = Hrz_metadata[Hrz_metadata['cokey'] == COKEY]
   #Make sure we got at least one row
   if not co_key_rows.empty:
      co_key_rows = co_key_rows.sort('hzdept_r', axis=0)
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
   return s

def getSoilTyp(s):
# Inputs:
# s -> a series containing soil types for a row from the horizon table
# Output:
# int -> Soil type referenced from SOILPARM.TBL from NoahMP
# Program converted from https://prod.nrcs.usda.gov/Internet/FSE_DOCUMENTS/nrcs142p2_053196.xls

   sand = s['sandtotal_r']
   silt = s['silttotal_r']
   clay = s['claytotal_r']
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

def getVegParm(s):
# in:
# takes a series with nessicary data to look up vegitation parameters, must contain
    #AD_X
    #AD_Y
    #AreaSym
# out:
# series containing the additional vegitation data or np.nan if not found
# the adhydro coordinates of the given element and its area sym (found in getMUKEY)
    #  Find the path for the chorizon file in STATSGO
   x = s['AD_X']
   y = s['AD_Y']
   AreaSym = s['AreaSym']
   if AreaSym not in veg_parm_dict:
      if s['inSSURGO']:
         input_NLCDfile =  "/share/CI-WATER Third Party Files/NLCD/all_projected/" + AreaSym[0].lower() + AreaSym[1].lower() + ".tif"
      else:
         input_NLCDfile =  "/share/CI-WATER Third Party Files/NLCD/all_projected/" + AreaSym.lower() + ".tif"
      veg_parm_dict[AreaSym] = QgsRasterLayer(input_NLCDfile)
   
   layer = veg_parm_dict[AreaSym]
   success, data = layer.identify(QgsPoint(x, y))
   data = str(data.values()[0])
   val = np.nan
   if success:
      if (data == '11'):
         val = 21
      elif (data == '12'):
         val = 22
      elif (data == '21'):
         val = 23
      elif (data == '22'):
         val = 24
      elif (data == '23'):
         val = 25
      elif (data == '24'):
         val = 26
      elif (data == '31'):
         val = 27
      elif (data == '41'):
         val = 28
      elif (data == '42'):
         val = 29   
      elif (data == '43'):
         val = 30   
      elif (data == '51'):
         val = 31
      elif (data == '52'):
         val = 32
      elif (data == '71'):
         val = 33   
      elif (data == '72'):
         val = 34
      elif (data == '73'):
         val = 35            
      elif (data == '74'):
         val = 36
      elif (data == '81'):
         val = 37
      elif (data == '82'):
         val = 38   
      elif (data == '90'):
         val = 39   
      elif (data == '95'):
         val = 40      
      else:
         assert (data == '128')
   #set veg type to the value val, or np.nan if not success, then return the series
   s['VegParm'] = val
   return s  

if __name__ == '__main__':
    import cProfile
    import timeit
    #cProfile.run('getSoilTypDRV()', sort='time')
    #numRuns = 10
    #print "New Code Average Time Over "+str(numRuns)+" Runs:"
    #print(timeit.timeit("getSoilTypDRV()", setup="from __main__ import getSoilTypDRV", number = numRuns))
    getSoilTypDRV()
    #Qgis finalize
    QgsApplication.exitQgis()