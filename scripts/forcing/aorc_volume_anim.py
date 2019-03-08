#!/usr/bin/env python
"""
Author: Nels Frazier nfrazie1@uwyo.edu
Date: June 30, 2017

IMPORTANT NOTE:
This script runs with geopandas 0.2+5.gb048d13 installed from GitHub.
The next official geopandas release should include spatial index support
which is the feature from this branch that is required for this script.
EDIT: Broke on mtmoran compute cluster, falling back to qgis spatial index
A bug in the Rtree bulk insert in version 0.8.2 requires that Rtree version 0.8.0
be used.
"""
import matplotlib
matplotlib.use('Qt5Agg')
from matplotlib.colors import Normalize
from matplotlib import cm
import matplotlib.pyplot as plt
import matplotlib.animation as ani
import netCDF4 as nc
import geopandas as gpd
from geopandas.plotting import plot_polygon_collection
import pandas as pd
import os
import glob
import gc
import sys
import psutil
import time
import multiprocessing as mp
from math import ceil
from shapely.geometry import Point, Polygon, LineString, box
from shapely import wkb
import numpy as np
import ctypes
from functools import partial
from itertools import product
from collections import namedtuple
import argparse
import traceback
from datetime import datetime
from calendar import monthrange
import re
from qgis.core import *
import subprocess
import traceback
from math import sqrt

def getChunks(size, num_chunks):
    """
        Helper function for getting chunks based on a given size.
        These chunks can be used to distrubute the workload of
        a function as evenly as possible.
    """
    
    chunk_size = int( ceil(size/float(num_chunks)) )
    chunks = []
    for i in range(0, num_chunks):
        upper = chunk_size*(i+1)
        upper = upper if size > upper else size
        chunks.append(range(chunk_size*i, upper))
    return chunks

def verify_date(s):
    try:
        return datetime.strptime(s, '%Y-%m-%d')
    except ValueError:
        pass
    return datetime.strptime(s, '%Y-%m-%d:%H')

def checkPositiveInput(i):
    value = int(i)
    if value < 1:
        raise argparse.ArguementTypeError('{} is not a valid input, must be >= 1'.format(value))
    return value

parser = argparse.ArgumentParser(description="Create an ADHydro forcing file from NWM data.")
#parser.add_argument('ADHydroMapDir', help='The ADHydro map directory containing the simulation input data.')
parser.add_argument('AORCdir', help='The top level nwm dir containing daily subdirectories')
#parser.add_argument('nexradDir', help='Top level nexrad data directory contain nexrad netcdf files.')
parser.add_argument('-s', '--startTime', help='The year and month to start processing, formatted as YYYY-MM-DD or YYY-MM-DD:H', required=True, type=verify_date)
parser.add_argument('-e', '--endTime', help='The year and month to stop processing (inclusive), formatted as YYYY-MM-DD or YYY-MM-DD:H.', required=True, type=verify_date)
parser.add_argument('-m', '--meridian', help='Centeral meridian to use for ADHydro Sinusoidal projection', required=True, type=float)
args = parser.parse_args()

#if not os.path.isdir(args.ADHydroMapDir):
#    parser.error("Directory '{}' does not exist".format(args.ADHydroMapDir))

if not os.path.isdir(args.AORCdir):
    parser.error("Directory '{}' does not exist".format(args.AORCdir))


#Setup the AORC information
#Make sure we work until the last hour of the last day of the input end date if date wasnt provided
if args.endTime < args.startTime:
	args.endTime = args.endTime.replace(hour=23)
#if args.startTime.year != args.endTime.year:
#    parser.error("Script currently only supports working within a single year.\nIf needed, run it multiple times to produce multiple years.")


#CRS string
adhydro_projection = '+proj=sinu +lon_0={} +x_0=20000000 +y_0=10000000 +datum=WGS84 +units=m +no_defs'.format(args.meridian)
#EPSG CODE? Or srid?
#FIXME set aorc_projection
AORC_projection = '4326'
#EPSG Code for lat/long wgs 84
#FIXME remove or change for other input set as needed
nexrad_projection = '4326'
#Build the time information.  We reset_index to promote to a dataframe so that we can later use Time as the index
"""
Check for existence of AORC data FIXME
"""
forcing_name = 'AORC_Charlotte'
mrms_forcing_name = 'AORC_MRMS_Charlotte'
if args.startTime != args.endTime:
	hours = pd.date_range(args.startTime, args.endTime, freq='1h')
else:
	hours = pd.DatetimeIndex([args.startTime])
years = hours.groupby(hours.year)
dropTimes = []
precip_times = []

for year in years:
  for h in hours[ hours.year == year ]:
    pattern = '{}_{}.nc4'.format( forcing_name, h.strftime('%Y%m%d%H') )
    #if year == 2016:
    #    year = 2015 #FIXME doesn't cross year boundaries well....hacked for now...
    path = os.path.join(args.AORCdir, str(year))
    filename = os.path.join(path , pattern)
    print( filename )
    if not os.path.isfile(filename):
        print("WARNING: Missing data file for time {}; Ommiting from forcing output.".format(h.strftime('%Y-%m-%d %H:%M:%S')))
        dropTimes.append(h)
    #Now look for additional dissagregated 15 minute data and add those to the time if they exist
    precip_dirs = glob.glob(os.path.join(path, 'Precip*'))
    for d in precip_dirs:
        precip_pattern = os.path.join(d, "{}_{}{:02d}{:02d}{:02d}*".format(mrms_forcing_name, h.year, h.month, h.day, h.hour))
        precip_files = glob.glob(precip_pattern)
    	#Now add these times to the final time index
        t = pd.to_datetime(pd.Series([ os.path.split(f)[-1] for f in precip_files]), format="{}_%Y%m%d%H%M.nc".format(mrms_forcing_name)).sort_values().reset_index(drop=True)
        precip_times.extend(t)
hours.frequency = None
#All the precip times as a datetime index
precip_times = pd.DatetimeIndex(precip_times)
#Find all the coincident times (i.e. we have AORC data AND dissagregated precip data
coincident = precip_times.intersection(hours)
#Drop the coincident times from the AORC times so we have unique index when we combine
hours = hours.drop(coincident)
hours = pd.Series(hours, index=hours, name='Time')
all_times = pd.Series(precip_times, index=precip_times, name='Time')
all_times = all_times.append(hours)
all_times.drop_duplicates(inplace=True)
all_times.sort_values(inplace=True)

#Convert this series to dataframe useable later by initOutput
all_times = pd.DataFrame({'Time':all_times})
all_times['coincident'] = False
#Mark all overlapping times
all_times.loc[coincident, 'coincident'] = True
#print(all_times)
precip_var = 'APCP_surface'
aorc_to_adhydro_vars = {'TMP_2maboveground':'T2', 'DLWRF_surface':'GLW', 'UGRD_10maboveground':'U', 'VGRD_10maboveground':'V', 'PRES_surface':'PSFC', 'APCP_surface':'TPREC', 'DSWRF_surface':'SWDOWN', 'SPFH_2maboveground':'QVAPOR'}
adhydro_var_array_index = {'T2':0, 'QVAPOR':1, 'QCLOUD':2, 'PSFC':3, 'U':4, 'V':5, 'TPREC':6, 'SWDOWN':7, 'GLW':8, 'PBLH':9, 'TSLB':10}

#Clean up as much memory as possible before we start forking subprocesses


def mem_usage(GB=False):
    """
        Simple function for estimating processes current memory usage
    """
    process = psutil.Process(os.getpid())
    mem = process.memory_info()[0]/float(2**20)
    if GB:
        mem = mem*0.00104858
    return mem


def getAORCCoords(aorc, cellSizeX=1000, cellSizeY=1000):
    """
        Function which returns a GeoDataFrame containing the aorc cells (center of cell)
        as cellSizeX x cellSizeY (meters) rectanular polygons. Each cell center is projected to ADHydro coordinates and then
        a cellSize[X,Y]/2 meter buffer around the center point is used to calculate the polygon.
    """
    cols = aorc.variables['longitude'][:]
    rows = aorc.variables['latitude'][:]
    #print rows
    #print cols
    aorcCells = {'geometry':[], 'row':[], 'col':[]}
    for i in range(len(rows)):
        for j in range(len(cols)):
           #print i, ", ", j
           aorcCells['geometry'].append (Point(cols[j],rows[i]))
           aorcCells['row'].append(len(rows)-i-1)
           aorcCells['col'].append(j)
    #Create a data frame and project it to ADHydro coordinates
    #aorc data is in lat/long WGS84 coordinates (epsg:4326)
    aorcFrame = gpd.GeoDataFrame(aorcCells, crs={'init' :'epsg:4326'})
    aorcFrame = aorcFrame.to_crs(crs=adhydro_projection)
    aorcFrame['point'] = aorcFrame.geometry
    #Calculate cell size since AORC cells are degree based
    #Find the corner, the east-west neighbor, and north-south neighbor of the corner
    size_points = gpd.GeoSeries( [Point(cols[0], rows[0]), Point(cols[1], rows[0]), Point(cols[0], rows[1]) ], crs={'init' : 'epsg:4326'})
    #Preoject these points to our coordinate system
    size_points = size_points.to_crs(crs=adhydro_projection)
    #Calculate the distance between cell centers
    cellSizeX = size_points[0].distance(size_points[1])
    cellSizeY = size_points[0].distance(size_points[2])
    print(cellSizeX, cellSizeY)
    #os._exit(1)
   
    #Apply a cellSize/2 meter buffer to each cell center point, making the geometry a cellSizeX x cellSizeY meter rectangle
    aorcFrame['poly'] = aorcFrame.geometry.apply(lambda p: box(p.x - cellSizeX/2, p.y - cellSizeY/2, p.x + cellSizeX/2, p.y + cellSizeY/2))

    aorcFrame.geometry = aorcFrame['poly']
    
    #Make QGIS spatial index #TODO/FIXME
    aorcIndex = QgsSpatialIndex() 
    def buildIndex(s):
        feature = QgsFeature(s.name)
        feature.setGeometry(QgsGeometry.fromWkt(s.geometry.to_wkt()))
        aorcIndex.insertFeature(feature)
    aorcFrame.apply(buildIndex, axis=1)
    #Now index the frame by the original row, col index
    aorcFrame.set_index(['row', 'col'], inplace=True)
    #print aorcFrame
    return aorcFrame, aorcIndex

#TODO/FIXME this is really a generic geo-dataframe/spatial index intersection.  It can be reused
#For any data source, not specific to AORC/NEXRAD/WRF...it was specialized for intersecting the NWM geo-database
#grid, but these others are really just syntactic sugar to name variables appropriate to the dataset being operated
#on.  This should be set in a module and the dataset name passed as arg to set <dataset>Row, <dataset>Col, Found_<dataset>
#Since we have to do several intersections over the SAME mesh/channel elements
#we speed things up by using a single function applied to these which does all intersections
#This reduces the need to iterate over large numbers of elements 3-4 times!!!
def polygonIntersectAORC(s, AORCCoords, AORCIndex, bugFix):
    """
        This function assigns to each adHydro element the grid coordinates
        of the AORC cell that contains the largest portion of the element.
        In the event that an element intersects no AORC cells, the nearest 
        neighbor is used.
    """
    cells = {}
    #Get a list of all aorcCoord indexes that intersect the series geometry
    #Use QGIS spatial index for now TODO/FIXME
    hits =list(aorcIndex.intersects(QgsRectangle(*(s.geometry.bounds))))
    cX = s.geometry.centroid.x
    cY = s.geometry.centroid.y
    idx = None
    found = False 
    #If only one result, don't need more checking
    if len(hits) == 1:
        geom = aorcCoords.iloc[hits[0]].geometry
        #If hit really intersects, great
        if geom.intersects(s.geometry):
            idx = hits[0]
            if idx == None:
                print("FS1")
            found = True
        else:
            #False positive, find nearest neighbor
            #TODO/FIXME QGIS spatial index
            nn = list(aorcIndex.nearestNeighbor(QgsPoint(cX, cY), 1))
            idx = nn[0]
            if idx == None:
                print("FS2")
            found = False
    else:
        if len(hits) == 0:
            #print "No intersect, try NN"
            #This polygon doesn't explicity intersect, so try nearest neighbor
            #TODO/FIXME QGIS spatial index
            nn = list(aorcIndex.nearestNeighbor(QgsPoint(cX, cY), 1))
            #Pick the first in case of tie...
            idx = nn[0]
            if idx == None:
                print("FS3")
            found = False
        else:
            #print "Multiple Intersects"
            #Eliminate false positives and get a list of cells that the
            #element intersects
            exact_intersects = []
            for h in hits:
                geom = aorcCoords.iloc[h].geometry
                if geom.intersects(s.geometry):
                    exact_intersects.append((h,geom))
            if len(exact_intersects) == 0:
                #False positive, find nearest neighbor
                #nn = list(wrfCoords.sindex.nearest(s.geometry.bounds, 1))
                #TODO/FIXME QGIS spatial index
                nn = list(aorcIndex.nearestNeighbor(QgsPoint(cX, cY), 1))
                idx = nn[0]
                if idx == None:
                    print("FS4")
                found = False
            else: 
                #Now we can find the exact intersecting amount, and use the largest to assign cell to element
                found = True
                if s.geometry.geom_type == 'LineString':
                    maxLength = 0.0
                    for i, geom in exact_intersects:
                        length = geom.intersection(s.geometry).length
                        if length > maxLength:
                            maxLength = length
                            idx = i
                    if idx == None:
                        print("FS5")
                        print(length, maxLength)
                else:
                    maxIntersectingArea = 0.0
                    for i, geom in exact_intersects:
                        area = geom.intersection(s.geometry).area
                        if area > maxIntersectingArea:
                            maxIntersectingArea = area
                            idx = i
                    if idx == None:
                        print("FS6" )
                if idx == None:
                    #On occasion, an intersections occurs with 0 length,
                    #Still need to assign an idx. Pick nearest neighbor
                    nn = list(aorcIndex.nearestNeighbor(QgsPoint(cX, cY), 1))
                    idx = nn[0]
                 
    cell = aorcCoords.iloc[idx]
    cells['AORCrow'] = cell.name[0]
    cells['AORCcol'] = cell.name[1]
    cells['Found'] = found
    return pd.Series(cells)

def intersectAORCgrid(adHydroFrame, aorcIndex, aorcCoords, how='center'):
    """
        This function assigns each ADHydro element in adHydroFrame to an AORC grid cell.
        This can be done by simply matching the center of the cell to a cell it lies in,
        or by more complexly finding the largest intersecting cell of the element.
     """
    #aorcIndex = aorcTuple[0]
    #aorcCoords = aorcTuple[1]
    #t0 = time.time()
    #print list(coords.sindex.intersection(mesh.loc[0].geometry.bounds, objects='raw'))
    if adHydroFrame.geom_type.eq('Point').all():
        if how != 'center':
            print("Reverting to center point calculations since geometries are all points.")
        
        adHydroFrame = pd.concat([adHydroFrame, adHydroFrame.apply(centerPointIntersectAORC, args=(aorcCoords, aorcIndex, None), axis=1)], axis=1)
    elif adHydroFrame.geom_type.eq('Polygon').all() or adHydroFrame.geom_type.eq('LineString').all():
        if how=='center':
            print("Changing geometry to centroids for center calculations")
            adHydroFrame['Poly'] = adHydroFrame.geometry
            adHydroFrame.geometry = adHydroFrame.centroid
            adHydroFrame = pd.concat([adHydroFrame, adHydroFrame.apply(centerPointIntersectAORC, args=(aorcCoords, aorcIndex, None), axis=1)], axis=1)
            adHydroFrame.geometry = adHydroFrame['Poly']
            del adHydroFrame['Poly']
        elif how=='poly':
            try:
                aorcFrame = adHydroFrame.apply(polygonIntersectAORC, args=(aorcCoords, aorcIndex, None), axis=1)
                adHydroFrame = pd.concat([adHydroFrame, aorcFrame], axis=1)
            except Exception as e:
                print("Error running polygonIntersectAORC")
                print(e)

        else:
            raise Exception("Unsupported intersection type {}. Supported options are [center, poly].".format(how))  
    else:
        print(adHydroFrame.geom_type)
        raise Exception("All adhydro geometries must be homogeneous.") 
    #t1 = time.time()
    #print "Mem after intersecting: {} GB".format(mem_usage(True))
    #print "Time to intersect: {}".format(t1-t0)
    return adHydroFrame


def getPrecip(adHydroMeshFrame, adHydroChannelFrame, minY, maxY, minX, maxX):
    """
        Read AORC precip variables and assign data to each ADHydro element(mesh and channel)
    """
    t0 = time.time()
    rowName = 'AORCrow'
    colName = 'AORCcol'
    #Find the min and max cells that we are going to work with
    minRow = min(adHydroMeshFrame[rowName].min(), adHydroChannelFrame[rowName].min())
    maxRow = max(adHydroMeshFrame[rowName].max(), adHydroChannelFrame[rowName].max())
    minCol = min(adHydroMeshFrame[colName].min(), adHydroChannelFrame[colName].min())
    maxCol = max(adHydroMeshFrame[colName].max(), adHydroChannelFrame[colName].max())
    #print minRow, maxRow
    #print minCol, maxCol
    def getData(s, data, minRow, minCol, i, bugFix):
        try:
                return data[i, s[rowName] - minRow, s[colName] - minCol ]
        except Exception as e:
            print("Error subsetting precip data")
            print(i, var)
            print(s)
            print(data.shape)
            print(minRow, maxRow+1)
            print(minCol, maxCol+1)
            traceback.print_exc(e)
            raise(e)
        return s

    results = {}
    #this loop handles sub precip times...NOTE ALL DATA ARRAYS AND THEIR ALIGNMENT IN TIME MUST BE DONE PRIOR TO CALLING
    data = precipShared[:, (minRow - minY):maxRow+1, (minCol - minX):maxCol+1]
    for i in range(0, precipShared.shape[0]):
	#read data for each stored precip time
    	meshVarFrame = adHydroMeshFrame[[rowName, colName]].copy(deep=True)
    	channelVarFrame = adHydroChannelFrame[[rowName, colName]].copy(deep=True)
    	#Get all data relative to the subset of the mesh we are working with
	#Now for every  time, gather the variable information for the data in precipShared for each ADHydro element
    meshVarFrame['Precip'] = data[i, (meshVarFrame['AORCrow'] - minRow).values, (meshVarFrame['AORCcol'] - minCol).values]	
    channelVarFrame['Precip'] = data[i, (channelVarFrame['AORCrow'] - minRow).values, (channelVarFrame['AORCcol'] - minCol).values]	
	#meshVarFrame['Precip'] = meshVarFrame.apply(getData, axis=1, args=(data, minRow, minCol, i, None))
	#channelVarFrame['Precip'] = channelVarFrame.apply(getData, axis=1, args=(data, minRow, minCol, i, None))
      
	#Now longer need AORC info, clean it up
    meshVarFrame.drop([rowName, colName], axis=1, inplace=True)
    channelVarFrame.drop([rowName, colName], axis=1, inplace=True)
    #Get out of geopandas and to a regular pandas dataframe
    #Convert to float64 type here to minimize possible floating point arithmitic errors later.
    meshVarFrame = pd.DataFrame(meshVarFrame, dtype=pd.np.float64)
    channelVarFrame = pd.DataFrame(channelVarFrame, dtype=pd.np.float64)
    #Take the transpose so we are indexing by variable for each row, element # is each column
    results[i] = {'mesh':meshVarFrame.T, 'channel':channelVarFrame.T}
    t1 = time.time()
    return results

def intersectMP(subset, whichADHydro=None,  how=None):
    """
        Wrapper for multiprocessing to run intersection on a subset of the mesh dataframe.
        Note: This may not produce exact same results as calling intersectNWMgrid on the entire mesh
        dataframe since the function looks at the passed dataframe geometry types and decides on 
        possible different ways to intersect the mesh and wrf coords.
    """
    
    if whichADHydro is None or whichADHydro not in ['mesh', 'channel']:
        raise Exception("Unknown option {}\nwhichADhydro must be one of ['mesh', 'channel']".format(whichADHydro))
    
    if how is None or how not in ['poly', 'center']:
        raise Exception("Unknown option {}\nhow must be one of ['poly', 'center']".format(how))

    #print "Processes {} processing {}  elements".format(os.getpid(), len(subset))
    adHydro_frame = None
    if whichADHydro == 'mesh':
        adHydro_frame = mesh.iloc[subset].copy()
    elif whichADHydro == 'channel':
        adHydro_frame = channel.iloc[subset].copy()

    try:
        #NOTE Have to pass global aorcIndex here!!! if passed as arg via multi-processing call, super-class pointer
        #in qgsspatialindex is lost during pickling!  If global, a complete copy is provided to each forked processes and it works.
        result = intersectAORCgrid(adHydro_frame, aorcIndex, aorcCoords, how)
    except Exception as e:
        print("Error intersecting AORC grid")
        print(e)
        raise(e)
    #print "Process {} finished processing {} elements".format(os.getpid(), len(result))
    #print "Returning {}".format(result.shape)
    return result

def rho(temp):
	"""
		Calculate water density at temperature
	"""
	return 999.99399 + 0.04216485*temp - 0.007097451*(temp**2) + 0.00003509571*(temp**3) - 9.9037785E-8*(temp**4)

#Let main processes read in the aorc data variable into a shared memory array
#This will prevent conflicting reads at different positions by each process
#at the disk level, then only have cache misses to slow us down.
def getVarMP(subset, times=[], minRow=None, maxRow=None, minCol=None, maxCol=None ):
    """
        Wrapper for multiprocessing to gather aorc data in parallel on subsets of mesh and channel.
        subset should be a tuple of (meshSubset, channelSubset). We gather all variables for each timestamp,
        as defined by the time variable
    """
    if minRow is None:
        raise(Exception("Argument minRow must not be None.  Pass the minimum row of the NWM domain for the current mesh"))
    if minCol is None:
        raise(Exception("Argument minCol must not be None.  Pass the minimum column of the of the NWM domain for the current mesh"))
    if maxRow is None:
        raise(Exception("Argument maxRow must not be None.  Pass the maximum row of the NWM domain for the current mesh"))
    if maxCol is None:
        raise(Exception("Argument maxCol must not be None.  Pass the maximum column of the of the NWM domain for the current mesh"))
    
    #Subset the mesh and get variable info for var for this subset
    mesh = intersectMesh.iloc[subset[0]]
    channel = intersectChannel.iloc[subset[1]]
    #Map mesh/channel elements to appropriate aorc values
    result = getVars(mesh, channel, minRow, maxRow, minCol, maxCol)
    """
	For NoahMP QVAPOR = water vapor mixing ration, AORC provides specific humidty
        So we convert with QVAPOR = SPCIFIC_HUMIDITY/(1-SPECIFIC_HUMIDITY)
	Qv = 0.622*e/p
	w = 0.662*3/(p-e)

	Qv = specific humidity
	w = water vapor mixing ratio
	e = partial pressure of water vapor < 4kPa
	p = atmospheric pressure (1011.3 kPa)

	We can approximate e = 1.5 for CONUS basins.
    	
	For lack of better representation, assume QCLOUD = QVAPOR

	An empirical equation for the density of water as a function of temperature is: 
	rho (kg/m^3) = 999.99399 + 0.04216485 T - 0.007097451 T^2 + 0.00003509571 T^3 - 9.9037785 x 10^-8 T^4
	for temperature T

	So to convert ACPC_surface (kg/m^2) to m/s
	
	(precip / rho) / time = ( (kg/m^2) / (kg/m^3) ) / second
			      = ( (kg/m^2) * (m^3/kg) ) / second
			      = m/second
    """

    #Kelvin to celcious for temp
    result['mesh'].loc['TMP_2maboveground'] = result['mesh'].loc['TMP_2maboveground'].map(lambda x: x - 272.15)
    result['channel'].loc['TMP_2maboveground'] = result['channel'].loc['TMP_2maboveground'].map(lambda x: x - 272.15)
    
    #FIXME check out all unit conversions
    maxTemp = result['mesh'].loc['TMP_2maboveground'].max()
    minTemp = result['mesh'].loc['TMP_2maboveground'].min()
    
    result['mesh'].loc['SPFH_2maboveground'] = result['mesh'].loc['SPFH_2maboveground'].map(lambda x: x/(1-x)) #Convert specific humidity to mixing ratio 
    result['channel'].loc['SPFH_2maboveground'] = result['channel'].loc['SPFH_2maboveground'].map(lambda x: x/(1-x)) #Convert specific humidty to mixing ratio
    
    #QCLOUD = QVAPOR = SPFH_2maboveground/(1-SPFH_2maboveground)
    meshDataArray[:, adhydro_var_array_index['QCLOUD'], mesh.index[0]:mesh.index[-1]+1] = result['mesh'].loc['SPFH_2maboveground'].values 
    channelDataArray[:, adhydro_var_array_index['QCLOUD'], channel.index[0]:channel.index[-1]+1] = result['channel'].loc['SPFH_2maboveground'].values

    #TODO/FIXME get TSLB done 
    #Const PBLH. 
    #The PBLH is the hieght of the "active" atmosphere.  However, ADHydro passes this to NoamMP, which only uses it 
    #For certain options, which ADHydro doesn't use.  So the value of PBLH has zero effect on the results of ADHydro
    meshDataArray[:, adhydro_var_array_index['PBLH'], mesh.index[0]:mesh.index[-1]+1] = 0
    channelDataArray[:, adhydro_var_array_index['PBLH'], channel.index[0]:channel.index[-1]+1] = 0
    #Temp at bottom of soil layer.  FIXME use the NWM TSLB data???
    #15.2 was taken as the area average of the long term mean provided by Mike Smith
    #"used in our modeling with the Sac-HTET model. It was derived from the long term mean annual air temperature and developed at NCEP years ago"
    #Safe to assume min and max temp of mesh is OK since channel overlaps mesh???
    meshDataArray[:, adhydro_var_array_index['TSLB'], mesh.index[0]:mesh.index[-1]+1] = 15.2
    channelDataArray[:, adhydro_var_array_index['TSLB'], channel.index[0]:channel.index[-1]+1] = 15.2
 
    #Handle precip including any hourly dissagregated data
    precipResult = getPrecip(mesh, channel, minRow, maxRow, minCol, maxCol)
    #Precip result contains maps of precip to mesh and channel elements for each time available in sharedPrecip (as established by main driver loop)
    #It also contains the nessicary offset time to calculate rates, and the interval can be found from the times dataframe passed to the funtion.
    times = times.reset_index()
    tprec_index = adhydro_var_array_index['TPREC'] 
    #Step through the times and use precipResult to derive the rate at the current time that is valid until the next time.
    #Should always have at least two precip times to deal with, current and the next so we can derive a rate...
    for time, precip in precipResult.iteritems():
        #Don't process the last time, it is only there for rate calc
        if time < len(times):
            #Get the precip total for next time
            nextPrecip = precipResult[time+1]
            interval = times.iloc[time]['next'] - times.iloc[time]['Time']
            #Get the interval in seconds...
            interval = interval / pd.np.timedelta64(1, 's')
            #Do the above noted conversion.....(precip/rho ) / time (s)
            precip['mesh'].loc['Precip'] = ( precipResult[time+1]['mesh'].loc['Precip'] / result['mesh'].loc['TMP_2maboveground'].apply(rho) ) / interval
            #precip['mesh'].loc['Precip'] = ( precipResult[time+1]['mesh'].loc['Precip'] / 999.0 / interval ) #FIXME revert to ^^^ testing mass balance
            precip['channel'].loc['Precip'] = ( precipResult[time+1]['channel'].loc['Precip'] / result['channel'].loc['TMP_2maboveground'].apply(rho) ) / interval
            meshDataArray[time, tprec_index , mesh.index[0]:mesh.index[-1]+1] = precip['mesh'].loc['Precip'].values
            channelDataArray[time, tprec_index , channel.index[0]:channel.index[-1]+1] = precip['channel'].loc['Precip'].values
    #Copy the reset of the variables that map directly.  They are valid for all times during this processing step
    for i, var in enumerate(aorc_vars):
        var_index = adhydro_var_array_index[ aorc_to_adhydro_vars[var]]
        meshDataArray[:, var_index , mesh.index[0]:mesh.index[-1]+1] = result['mesh'].loc[var].values
        channelDataArray[:, var_index, channel.index[0]:channel.index[-1]+1] = result['channel'].loc[var].values

all_times['next'] = all_times['Time'].shift(-1)

"""
MUST GET A DATA POINT FOR THE LAST TIME'S NEXT VALUE TO CALCULATE PROPER RATE
"""
last = all_times.iloc[-1].copy()
next = last['Time'] + pd.np.timedelta64(15, 'm')
file_path = os.path.join(args.AORCdir, str(next.year))
#file_path = os.path.join(args.AORCdir, '2015') #HACKED FOR CROSSING YEAR BOUNDARIES FIXME
file_dir = glob.glob(os.path.join(file_path, 'Precip*'))[0] #ASSUME ONLY ONE PRECIP SUB DIR FIXME MAY NEED TO EXTEND THIS LATER
precip_file = os.path.join(file_dir, '{}_{}.nc'.format(mrms_forcing_name, next.strftime('%Y%m%d%H%M')))
if os.path.exists(precip_file):
	last['next'] = next
	all_times.iloc[-1] = last.copy()
	#Add the last time, not used as a forcing time but required to derive proper forcing rate
	if next.minute == 0:
		last['coincident'] = True
		last['Time'] = next
		last['next'] = pd.np.nan
		all_times.loc[next] = last
else:
	next = last['Time'] + pd.np.timedelta64(1, 'h')
	precip_file = os.path.join(file_path, '{}_{}.nc4'.format(forcing_name, next.strftime('%Y%m%d%H')) )
	if os.path.exists(precip_file):
		last['next'] = next
		all_times.iloc[-1] = last.copy()
		last['coincident'] = False
		last['Time'] = next
		last['next'] = pd.np.nan
		all_times.loc[next] = last

#mesh, channel = getADHydroPolyGeometry(ADHydroGeometryFile)

#Open the first aorc file and use its grid for all intersections.
first_time = all_times.iloc[0]['Time']
file_name = "{}_{:02d}{:02d}{:02d}{:02d}.nc4".format(forcing_name, first_time.year, first_time.month, first_time.day, first_time.hour)
file_path = os.path.join(args.AORCdir, str(first_time.year) )
aorc_file = os.path.join(file_path, file_name)
#Read dataset to get coordinate info
aorcData = nc.Dataset(aorc_file, 'r')
aorcCoords, aorcIndex = getAORCCoords(aorcData)
cell_area = aorcCoords.loc[ (0,0) ].geometry.area 
#sub_basin = gpd.read_file('sub9_catchment.shp', crs={'init' :'epsg:4326'})
sub_basin = gpd.read_file('adhydro_extent.shp')
#sub_basin = sub_basin.to_crs(crs=adhydro_projection)
sub_basin_area = sub_basin.iloc[0].geometry.area

#print( aorcCoords )
#print( sub_basin )
aorcData.close()
aorcCoords['percent_intersecting'] = 0.0

aorcCoords['precip'] = 0.0
#intersection = gpd.overlay(sub_basin, aorcCoords, how='intersection')
#print( intersection )
def intersectAORC(s, AORCCoords, AORCIndex, bugFix):
    """
        This function assigns to each adHydro element the grid coordinates
        of the AORC cell that contains the largest portion of the element.
        In the event that an element intersects no AORC cells, the nearest 
        neighbor is used.
    """
    cells = {}
    #Get a list of all aorcCoord indexes that intersect the series geometry
    #Use QGIS spatial index for now TODO/FIXME
    hits =list(aorcIndex.intersects(QgsRectangle(*(s.geometry.bounds))))
    cX = s.geometry.centroid.x
    cY = s.geometry.centroid.y
    ids = [] 
    found = False 
    #Eliminate false positives and get a list of cells that the
    #s intersects
    exact_intersects = []
    for h in hits:
        geom = aorcCoords.iloc[h].geometry
        if geom.intersects(s.geometry):
            exact_intersects.append((h,geom))
    ids.extend(exact_intersects)

    rows = []
    for i,p in ids:
        cell = aorcCoords.iloc[i]
        intersection = gpd.overlay(gpd.GeoDataFrame(geometry=[s.geometry]), gpd.GeoDataFrame(geometry=[cell.geometry]),how='intersection')
        percent_intersecting = intersection.iloc[0].geometry.area/cell.geometry.area
        aorcCoords.loc[cell.name, 'percent_intersecting'] = percent_intersecting
        cells['AORCrow'] = int(cell.name[0])
        cells['AORCcol'] = int(cell.name[1])
        rows.append( pd.Series( cells ) )
    df = pd.DataFrame(rows)
    df = df.astype({'AORCrow':int, 'AORCcol':int})
    return df

intersecting_ids = intersectAORC(sub_basin.iloc[0], aorcCoords, aorcIndex, None)
#aorcCoords['percent_intersecting'] = 0.0
#index = list(zip(intersecting_ids['AORCrow'], intersecting_ids['AORCcol']))
#print(aorcCoords.loc[index])
#aorcCoords.loc[ index , 'percent_intersecting'] = intersecting_ids['percent_intersecting'].values
aorc_rows = intersecting_ids['AORCrow'].unique()
aorc_cols = intersecting_ids['AORCcol'].unique()
minX = aorc_cols.min()
minY = aorc_rows.min()
maxX = aorc_cols.max()
maxY = aorc_rows.max()
print("SUB row bounds: ", minY, maxY)
print("SUB col bounds: ", minX, maxX)
sub_index = pd.MultiIndex.from_product((aorc_rows, aorc_cols))
sub_aorc = aorcCoords#aorcCoords.loc[sub_index]
aorc_rows = aorcCoords.index.get_level_values('row').unique()
aorc_cols = aorcCoords.index.get_level_values('col').unique()
minX, minY = 0,0
#print(aorcCoords[ aorcCoords['percent_intersecting'] > 0 ] )
#os._exit(1)
"""
plt.figure()
ax = plt.gca()
print(aorcCoords)
#sub = aorcCoords[ aorcCoords['AORCrow'].isin(aorc_rows) & aorcCoords['AORCcol'].isin(aorc_cols) ]
aorcCoords.plot(ax=ax, facecolor='none', edgecolor='black')
sub_basin.plot(ax=ax, facecolor='none', edgecolor='red')
plt.show()
"""

all_times = all_times.reset_index(drop=True)
all_times.reset_index(inplace=True) #We want index by Time but retain interger index position, so reset
all_times = all_times.set_index('Time')
#Process hourly time stamps.  Do not include the last entry to process as it should not be part of the output 
time_groups = all_times[:-1].groupby(pd.TimeGrouper(freq='1H'))
#Some simple mass balance accumulators
aorc_accums = []
aorc_accums_time = []
adhydro_accums = []
mass_accum = []
def animation_init(fig, ax, artists):
    sub_basin.plot(ax=ax, facecolor='none', edgecolor='red')
    return artists

def sample_precip(s, data):
    row = s.name[0] - minY
    col = s.name[1] - minX
    return data[row, col]

def readAORC(time_data, axes, artists, cax):
    #print(time_data)
    time = time_data.name
    next_time = time_data['next']
    print("Processing {}".format(time))#, '\n'.join( [ '\t{}'.format(t.strftime('%Y-%m-%d %H:%M:%S')) for t in times.index] ) ))
    """
    For a given time, we either have dissagregate data or we do not.
    This means that for groups of hourly frequency, it is either 1 (no dissagregate)
    or it is more (dissagregate between hours).  We always have to read the top of the hour
    in order to get the non-dissagregated variables, so find that first, then decide if we need
    additional files for dissagregated precip.
    """
    
    #Since we have to convert mass to rate, we always need at least the next precip value to get the rate for interval between the two times
    precip_times = [time, next_time]
    precip_data = np.ndarray((len(precip_times), len(aorc_rows), len(aorc_cols) ))
    local_accum = 0.0
    file_path = os.path.join(args.AORCdir, str(time.year))
    fs_array_debug = []
    for i,t in enumerate(precip_times):
        #print("FINDING PRECIP FOR TIME {}".format(t))
        if t.minute == 0 and not all_times.loc[t]['coincident']:
            #READ AORC
            #file_path = os.path.join(args.AORCdir, '2015') #FIXME for crossing year boundaries str(hour.year))
            precip_file = os.path.join(file_path, '{}_{}.nc4'.format(forcing_name, t.strftime('%Y%m%d%H')))
            precip_dataset = nc.Dataset(precip_file, 'r')
            precip_data[i] = precip_dataset['APCP_surface'][0, aorc_rows, aorc_cols] #This is in kg/m^2
            fs_array_debug.append(precip_dataset['APCP_surface'][0][:,:])
        else:
            #Read this precip from dissagregated data
            precip_dir = glob.glob(os.path.join(file_path, 'Precip*'))[0] #ASSUME ONLY ONE PRECIP SUB DIR FIXME MAY NEED TO EXTEND THIS LATER
            precip_file = os.path.join(precip_dir, '{}_{}.nc'.format(mrms_forcing_name, t.strftime('%Y%m%d%H%M')))
            precip_dataset = nc.Dataset(precip_file, 'r')
            fs_array_debug.append(precip_dataset['APCP_surface'][:,:])
            precip_data[i] = precip_dataset['APCP_surface'][ aorc_rows, aorc_cols] #kg/m^2
        if i != 0:
            #Don't accumulate the first one since it really doesn't belong to the total accumulated, it accumulated before time t0
            for r in aorc_rows:
                for c in aorc_cols:
                    precip_data[i,r-minY,c-minX] = precip_data[i,r-minY,c-minX] * aorcCoords.loc[(r,c)]['percent_intersecting']
            local_accum = (cell_area*precip_data[i]/999.0).sum() #Totol m^3 over domain (aorc_rows*aorc_cols) each cell is cell_area km (so kg/m^2 / (1kg/999.0m) * 1000m)
        #FIXME testing for neg input, remove
        neg = precip_data[i][precip_data[i] < 0]
        if len(neg) > 0:
            print("FOUND NEG AORC input in {}".format(precip_file))
            print(i)
            print(neg)
            print(len(neg) )
            os._exit(1)
        #Here is aorc precip
        if i != 0:
            mass_accum.append(local_accum)
            aorc_accums.append( local_accum/( (precip_times[i]-precip_times[i-1])/pd.np.timedelta64(1, 's') ) ) #m^3/second
            aorc_accums_time.append(precip_times[i-1])
            #if local_accum > 0:
            #    print("{} precip at time: {}".format(local_accum, precip_times[i-1]))

    #sub_aorc['precip'] = 5.0 #sub_aorc.apply(sample_precip, args=(precip_data[i],), axis=1)
    sub_aorc['precip'] = sub_aorc.apply(sample_precip, args=(fs_array_debug[1],), axis=1)
    #sub_aorc.plot('precip', ax=axes, legend=False, edgecolor='none')
    mn = sub_aorc['precip'].min()
    mx = sub_aorc['precip'].max()
    #print(t, '\t', mx)
    artists.precip.set_array(sub_aorc['precip'])
    
    if mx > v_max:
        artists.precip.set_clim(v_min, mx)
        norm = Normalize(v_min, mx)
        n_cmap = cm.ScalarMappable(norm=norm, cmap=c_map)
        n_cmap.set_array([])
        cax.clear()
        fig.colorbar(n_cmap,cax=cax)
    artists.title.set_text(precip_times[1])
    """
    p_data2 = fs_array_debug[i].flatten()
    fs2 = aorcCoords
    fs2['precip'] = p_data2
    #print(fs2)
    #plt.figure()
    ax = fs2.plot('precip', legend=True, edgecolor='green')
    #fs2[fs2['precip'] > 0].apply(lambda x: ax.annotate(x.name, xy=x.geometry.centroid.coords[0], ha='center'),axis=1)
    plt.title('Precip at time {}'.format(t))
    axes.append(ax)
    artists.precip.set_data(p_data2)
    """

    print("AORC ACCUM {}".format(pd.Series(aorc_accums).sum()))
    print("MASS ACCUM {}".format(pd.Series(mass_accum).sum()))
    t1 = time.time()
    return artists.precip, artists.title
frame = 0
def on_press_single(event):
    global frame
    if event.key == 'left':
        if frame > 0:
            frame  -= 1
        else:
            frame = len(all_times) - 1
    elif event.key == 'right':
        if frame < len(all_times) - 1:
            frame += 1
        else:
            frame = 0
    if event.key in ['left', 'right']:
        update(all_times.iloc[frame])
        plt.draw()

def on_press_animate(event):
    if event.key.isspace():
        if animation.running:
            animation.event_source.stop()
        else:
            animation.event_source.start()
        animation.running ^= True
    if event.key == 'left':
        animation.direction = -1
    elif event.key == 'right':
        animation.direction = +1
    if event.key in ['left', 'right']:
        t = animation.frame_seq.__next__()
        update(t)
        plt.draw()

def get_frames(start=0):
    frame = start
    while True:
        yield all_times.iloc[frame]
        frame += animation.direction
        if frame < 0:
            frame = len(all_times) - 2 
        elif frame > len(all_times) - 2:
            frame = 0

fig = plt.figure()
ax = fig.gca()
#fig.canvas.mpl_connect('key_press_event', on_press_single)
fig.canvas.mpl_connect('key_press_event', on_press_animate)
Artists = namedtuple("Artists", ("precip","title"))
v_min = sub_aorc['precip'].min()
v_max = sub_aorc['precip'].max()
v_min = 0
v_max = 15 
c_map = None 
polys = plot_polygon_collection(ax, sub_aorc.geometry, sub_aorc['precip'].values, vmin=v_min, vmax=v_max, cmap=c_map)
norm = Normalize(v_min, v_max)
n_cmap = cm.ScalarMappable(norm=norm, cmap=c_map)
n_cmap.set_array([])
colorbar = fig.colorbar(n_cmap)

#ax = sub_aorc.plot('precip', ax=fig.gca(), legend=True, edgecolor='none')
#print( '\n'.join( dir(ax )))
#print(ax.get_children())
#os._exit(1)
artists = Artists(polys, ax.set_title(''))

update = partial(readAORC, axes=ax, artists=artists, cax=fig.axes[-1]) 
init = partial(animation_init, fig=fig, ax=ax, artists=artists)

if __name__ == '__main__':
    print("Initial memory usage: {} GB".format(mem_usage(True)))
    start_time = time.time()
    """
        Processing subsets of the mesh at a time 
    """
    #init()
    #print(ax.artists)
    #os._exit(1)
    #for u in list(all_times[0:2].itertuples()):
    #    new = update(u)
    #    plt.show()
    #os._exit(1)
    
    animation = ani.FuncAnimation(fig, update, init_func=init, interval=200, frames=get_frames, blit=False, repeat=True)
    animation.running = True 
    animation.event_source.stop()
    animation.direction = +1
    
    #init()
    plt.show()
    #for g in all_times[:-1].itertuples():#time_groups:
    #    readAORC(g) 
    #TODO track down this mass balance difference, testing on 2008-08-17:20
    #adhydro_accum = pd.np.sum(output.variables['TPREC'][:,adhydro_to_aorc_unique.index])
    #print all_times
    #accum_time = (all_times.index[-1] - all_times.index[0])/pd.np.timedelta64(1,'s')
    #print "ACCUM TIME {}".format(accum_time)
    #print "kg/m^2 total over AORC domain {}".format(sum(aorc_accums))
    #aorc_accums = [ a/999.0 for a in aorc_accums ]
    """
    print("Approximate m/second over AORC domain {}".format((sum(aorc_accums))))
    print("Number of rows {}, number of columns {}".format(len(aorc_rows), len(aorc_cols)))
    """
    #print aorc_accums
    #print len(aorc_accums)
    #print len(adhydro_accums)
    end_time = time.time()
    """
    accum_series = pd.Series(aorc_accums, index=aorc_accums_time)
    accum_series.to_csv('domain_precip_rates.csv')
    accum_series.plot()
    plt.show()
    print(sub_basin.iloc[0].geometry.area)
    """

