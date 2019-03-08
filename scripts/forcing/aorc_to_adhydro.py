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
import matplotlib.pylab as plt
import netCDF4 as nc
import geopandas as gpd
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
import argparse
import traceback
from datetime import datetime
from calendar import monthrange
import re
from qgis.core import *
import subprocess
import traceback

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
        chunks.append(list(range(chunk_size*i, upper)))
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
parser.add_argument('ADHydroMapDir', help='The ADHydro map directory containing the simulation input data.')
parser.add_argument('AORCdir', help='The top level nwm dir containing daily subdirectories')
#parser.add_argument('nexradDir', help='Top level nexrad data directory contain nexrad netcdf files.')
parser.add_argument('-s', '--startTime', help='The year and month to start processing, formatted as YYYY-MM-DD or YYY-MM-DD:H', required=True, type=verify_date)
parser.add_argument('-e', '--endTime', help='The year and month to stop processing (inclusive), formatted as YYYY-MM-DD or YYY-MM-DD:H.', required=True, type=verify_date)
parser.add_argument('-f', '--outputFileName', help='Optional output file name.  By default, the filename will be set based on the output times.')
parser.add_argument('-o', '--outputDir', help='Optional output directory. By default, output will be put in ADHydroMapDir/forcing ')
parser.add_argument('-i', '--inputDir', help='Optional input directory containing geometry.nc to processes.  If not set, looks in ADHydroMapDir/mesh_massage/')
parser.add_argument('-g', '--gridSize', type=checkPositiveInput, default=1000, \
                    help='Optional AORC grid size arguement (in meters).  Defaults to 1000 m.\n\
                    When set, AORC grid cells will be calculated based on the input size.\n\
                    For example, passing 2000 will create grid cells of size 2000m x 2000m by creating a bounding sqaure 1000m around the center point.')
parser.add_argument('-n', '--numSubsets', type=checkPositiveInput, help='Split the processing into N subsets of the mesh.  Useful on large meshes to avoid running out of memory.')
parser.add_argument('-m', '--meridian', help='Centeral meridian to use for ADHydro Sinusoidal projection', required=True, type=float)
args = parser.parse_args()

if not os.path.isdir(args.ADHydroMapDir):
    parser.error("Directory '{}' does not exist".format(args.ADHydroMapDir))

if not os.path.isdir(args.AORCdir):
    parser.error("Directory '{}' does not exist".format(args.AORCdir))
"""
if not os.path.isdir(args.nexradDir):
    parser.error("Directory '{}' does not exist".format(args.nexradDir))
"""
if args.inputDir:
    ADHydroGeometryDir = args.inputDir
else:
    ADHydroGeometryDir = os.path.join(args.ADHydroMapDir, 'mesh_massage')

if not os.path.isdir(ADHydroGeometryDir):
    parser.error("Directory '{}' does not contain the mesh_massage geometry directory".format(args.ADHydroMapDir))

ADHydroGeometryFile = os.path.join(ADHydroGeometryDir, 'geometry.nc')
if not os.path.isfile(ADHydroGeometryFile):
    parser.error("Directory '{}' does not contain the geometry.nc file.".format(ADHydroGeometryDir))

outputDir = ''
if args.outputDir:
    outputDir = args.outputDir
else:
    outputDir = os.path.join(args.ADHydroMapDir, 'forcing')
if not os.path.isdir(outputDir):
    print("{} directory does not exist, creating...".format(outputDir))
    os.mkdir(outputDir)

#if args.gridSize:
#    gridSize = args.gridSize
#else:
#    gridSize = 1000

#Setup the AORC information
#Make sure we work until the last hour of the last day of the input end date if date wasnt provided
if args.endTime < args.startTime:
    args.endTime = args.endTime.replace(hour=23)
#if args.startTime.year != args.endTime.year:
#    parser.error("Script currently only supports working within a single year.\nIf needed, run it multiple times to produce multiple years.")


##TODO/FIXME Eventually read proj string from netcdf variable.
"""
Projection settings for ADHydro and NWM grid.  Currently must manually edit the adhydro_project to change the centeral meridain
to match that of the mesh being processed.  Use the -m command line flag to change the meridian.

 The nwm_projection is the SRID of the projection stored in the database.
"""
#CRS string
adhydro_projection = '+proj=sinu +lon_0={} +x_0=20000000 +y_0=10000000 +datum=WGS84 +units=m +no_defs'.format(args.meridian)
#EPSG CODE? Or srid?
#FIXME set aorc_projection
AORC_projection = '4326'
#EPSG Code for lat/long wgs 84
#FIXME remove or change for other input set as needed
nexrad_projection = '4326'
"""
Globals for use with multiprocessing.
Faster to access globals than trying to pickle large data and pass as args.
"""
initTime0 = time.time()
#Build the time information.  We reset_index to promote to a dataframe so that we can later use Time as the index
#days = pd.date_range(args.startTime, args.endTime, freq='1d')
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
    if year == 2016:
        year = 2015 #FIXME doesn't cross year boundaries well....hacked for now...
    path = os.path.join(args.AORCdir, str(year))
    filename = os.path.join(path , pattern)
    print(filename)
    if not os.path.isfile(filename):
        print(("WARNING: Missing data file for time {}; Ommiting from forcing output.".format(h.strftime('%Y-%m-%d %H:%M:%S'))))
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

outFileName = ''
if args.outputFileName:
    outFileName = os.path.join(outputDir, args.outputFileName)
else:
    name = 'forcing_{}-{:02d}-{}_{}-{:02d}-{}.nc'.format(all_times['Time'].iloc[0].year, all_times['Time'].iloc[0].month, all_times['Time'].iloc[0].day, all_times['Time'].iloc[-1].year, all_times['Time'].iloc[-1].month, all_times['Time'].iloc[-1].day)
    outFileName = os.path.join(outputDir, name)
if os.path.isfile(outFileName):
   raise Exception("wrf_to_adhydro cowardly refusing to overwrite previous forcing file '{}'.\nPlease manually remove this file before running.".format(outFileName))

aorc_vars = ['TMP_2maboveground', 'DLWRF_surface', 'UGRD_10maboveground', 'VGRD_10maboveground', 'SPFH_2maboveground', 'DSWRF_surface', 'PRES_surface', 'SPFH_2maboveground']
precip_var = 'APCP_surface'
aorc_to_adhydro_vars = {'TMP_2maboveground':'T2', 'DLWRF_surface':'GLW', 'UGRD_10maboveground':'U', 'VGRD_10maboveground':'V', 'PRES_surface':'PSFC', 'APCP_surface':'TPREC', 'DSWRF_surface':'SWDOWN', 'SPFH_2maboveground':'QVAPOR'}

adhydro_var_array_index = {'T2':0, 'QVAPOR':1, 'QCLOUD':2, 'PSFC':3, 'U':4, 'V':5, 'TPREC':6, 'SWDOWN':7, 'GLW':8, 'PBLH':9, 'TSLB':10}

#Clean up as much memory as possible before we start forking subprocesses
del ADHydroGeometryDir
del outputDir


"""TESTING INPUT DATA AND TIME INDEX CREATION"""
#os._exit(1)





def mem_usage(GB=False):
    """
        Simple function for estimating processes current memory usage
    """
    process = psutil.Process(os.getpid())
    mem = process.memory_info()[0]/float(2**20)
    if GB:
        mem = mem*0.00104858
    return mem

#TODO/FIXME SHOULD WE ALSO USE AN EXCLUDE
def getADHydroCenterGeometry(geometryFile):
    """
        Function which returns a GeoDataFrame containing the ADHydro geometries as center points
    """
    t0 = time.time()
    #Open the ADHydro geometry data set
    adhydro_geom = nc.Dataset(geometryFile, 'r')
    #Extract mesh element center coordinates in ADHydro Coordinates
    adhydroMesh = {'X':adhydro_geom.variables['meshElementX'][0], 'Y':adhydro_geom.variables['meshElementY'][0]}
    #Extract channel element center coordinates in ADHydro Coordinates
    adhydroChannel = {'X':adhydro_geom.variables['channelElementX'][0], 'Y':adhydro_geom.variables['channelElementY'][0]}

    #Create point geometries and geo series for adhydro mesh and channel elements
    adhydroMeshCenters = [ Point(x, y) for x,y in zip(adhydroMesh['X'], adhydroMesh['Y']) ]
    adhydroMeshGeometry = gpd.GeoSeries(adhydroMeshCenters, crs=adhydro_projection)
    adhydroChannelCenters = [ Point(x, y) for x,y in zip(adhydroChannel['X'], adhydroChannel['Y']) ]
    adhydroChannelGeometry = gpd.GeoSeries(adhydroChannelCenters, crs=adhydro_projection)
    #Create dataframes to contain further attributes
    meshFrame = gpd.GeoDataFrame([], geometry=adhydroMeshGeometry)
    channelFrame = gpd.GeoDataFrame([], geometry=adhydroChannelGeometry)
    t1 = time.time()
    print("Mem after reading ADHydro center point geometries: {} GB".format(mem_usage(True)))
    print("Time to read ADHydro geometry: {}".format(t1-t0))
    """
    #Create a QGSI spatial index since RTREE hangs on moran??? TODO/FIXME
    meshIndex = QgsSpatialIndex()
    for p in meshFrame.geometry:
        feature = QgsFeature()
        feature.setGeometry(QgsGeometry.fromWkt(p.to_wkt()))
        meshIndex.insertFeature(feature) 
    channelIndex = QgsSpatialIndex()
    for c in channelFrame.geometry:
        feature = QgsFeature()
        feature.setGeometry(QgsGeometry.fromWkt(c.to_wkt()))
        channelIndex.insertFeature(feature)
    """
    adhydro_geom.close()
    return meshFrame, channelFrame#, meshIndex, channelIndex

def getADHydroPolyGeometry(geometryFile, meshSubset=None, channelSubset=None):
    """
        Function which returns a GeoDataFrame containing the ADHydro geometries as polygons
    """
    t0 = time.time()
    #Open the ADHydro geometry data set
    adhydro_geom = nc.Dataset(geometryFile, 'r')
    #Extract mesh vertex coordinates in ADHydro Coordinates
    #Extract channel element vertex coordinates in ADHydro Coordinates
    if meshSubset is None or channelSubset is None:
        adhydroMesh = {'X':adhydro_geom.variables['meshVertexX'][0], 'Y':adhydro_geom.variables['meshVertexY'][0]}
        adhydroChannel = {'X':adhydro_geom.variables['channelVertexX'][0], 'Y':adhydro_geom.variables['channelVertexY'][0]}
    else: 
        adhydroMesh = {'X':adhydro_geom.variables['meshVertexX'][0][meshSubset[0]:meshSubset[-1]+1], 'Y':adhydro_geom.variables['meshVertexY'][0][meshSubset[0]:meshSubset[-1]+1]}
        adhydroChannel = {'X':adhydro_geom.variables['channelVertexX'][0][channelSubset[0]:channelSubset[-1]+1], 'Y':adhydro_geom.variables['channelVertexY'][0][channelSubset[0]:channelSubset[-1]+1]}

    #Create polygon and line geometries and geo series for adhydro mesh and channel elements
    adhydroMeshVerticies =[Polygon(p) for p in [ list(zip(xs,ys)) for xs, ys in zip(adhydroMesh['X'], adhydroMesh['Y']) ] ]
    adhydroMeshGeometry = gpd.GeoSeries(adhydroMeshVerticies, crs=adhydro_projection)
    tm = time.time()
    #print "Time to create adhydroMeshSeries: {}".format(tm-t0)
    #print "Mem usage: {}".format(mem_usage(True))
    del adhydroMeshVerticies
    del adhydroMesh
    gc.collect()
    #print "Mem usage after collection: {}".format(mem_usage(True))
    adhydroChannelVerticies = [ LineString(l) for l in [list(zip(xs,ys)) for xs, ys in zip(adhydroChannel['X'], adhydroChannel['Y']) ] ]
    adhydroChannelGeometry = gpd.GeoSeries(adhydroChannelVerticies, crs=adhydro_projection)
    #print "Time to create adhydroChannelSeries: {}".format(time.time()-tm)
    #print "Mem usage: {}".format(mem_usage(True))
    del adhydroChannelVerticies
    del adhydroChannel
    gc.collect()
    #print "Mem usage after collection: {}".format(mem_usage(True))
    #Create dataframes to contain further attributes
    meshFrame = gpd.GeoDataFrame([], geometry=adhydroMeshGeometry)
    channelFrame = gpd.GeoDataFrame([], geometry=adhydroChannelGeometry)
    """
    #Create a QGSI spatial index since RTREE hangs on moran??? TODO/FIXME
    meshIndex = QgsSpatialIndex()
    for p in meshFrame.geometry:
        feature = QgsFeature()
        feature.setGeometry(QgsGeometry.fromWkt(p.to_wkt()))
        meshIndex.insertFeature(feature) 
    channelIndex = QgsSpatialIndex()
    for c in channelFrame.geometry:
        feature = QgsFeature()
        feature.setGeometry(QgsGeometry.fromWkt(c.to_wkt()))
        channelIndex.insertFeature(feature)
    """
    t1 = time.time()
    print("Mem after reading ADHydro polygon geometries: {} GB".format(mem_usage(True)))
    print("Time to read ADHydro geometry: {}".format(t1-t0))
    adhydro_geom.close()
    return meshFrame, channelFrame#, meshIndex, channelIndex

def getAORCCoords(aorc, cellSizeX=1000, cellSizeY=1000):
    """
        Function which returns a GeoDataFrame containing the aorc cells (center of cell)
        as cellSizeX x cellSizeY (meters) rectanular polygons. Each cell center is projected to ADHydro coordinates and then
        a cellSize[X,Y]/2 meter buffer around the center point is used to calculate the polygon.
    """
   
    #The aorc grid cell LAT/LONGS.  These are variables with dimesnions (lat), (lon) respectively
    cols = aorc.variables['longitude'][:]
    rows = aorc.variables['latitude'][:]
    #print rows
    #print cols 
    aorcCells = {'geometry':[], 'row':[], 'col':[]}
    for i in range(len(rows)):
        for j in range(len(cols)):
           #print i, ", ", j
           aorcCells['geometry'].append (Point(cols[j],rows[i]))
           aorcCells['row'].append(i)
           aorcCells['col'].append(j)
    #print(max(aorcCells['row']), max(aorcCells['col']))
    #print(aorc.variables['APCP_surface'].shape)
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
    return aorcFrame, aorcIndex

#TODO/FIXME see abstraction comment below for polygonIntersectAORC.  Same applies here
def centerPointIntersectAORC(s, aorcCoords, aorcIndex, bugFix):
    """
        This function assigns to each adHydro element the grid coordinates
        of the aorc cell that the center lies in.  In the event that the center
        point doesn't lie in a cell, the nearest neighbor is used.
    """
    cells = {}
    #Use QGIS spatial index for now TODO/FIXME
    hits =list(aorcIndex.intersects(QgsRectangle(*(s.geometry.bounds))))
    cX = s.geometry.centroid.x
    cY = s.geometry.centroid.y
    idx = None
    found = False 
    #If only one result, don't need more checking
    if len(hits) == 1:
        idx = hits[0]
        found = True
    else:
        if len(hits) == 0:
            #print "No intersect, try NN"
            #This point doesn't explicity intersect, so try nearest neighbor
            #TODO/FIXME QGIS spatial index
            nn = list(aorcIndex.nearestNeighbor(QgsPoint(cX, cY), 1))
            #Pick the first in case of tie...
            idx = nn[0]
            found = False
        else:
            #print "Multiple Intersects"
            for h in hits:
                geom = aorcCoords.iloc[h].geometry
                if geom.intersects(s.geometry):
                    idx = h
                    found = True
                    break

    cell = aorcCoords.iloc[idx]
    cells['AORCrow'] = cell.name[0]
    cells['AORCcol'] = cell.name[1]
    cells['Found'] = found
    return pd.Series(cells)

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
                        print("FS6") 
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


def getVars(adHydroMeshFrame, adHydroChannelFrame, minY, maxY, minX, maxX):
    """
        Read AORC variables and assign data to each ADHydro element(mesh and channel)
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
    def getData(s, outputFrame, data, var, minRow, minCol, bugFix):
        try:
            for i, var in enumerate(nwm_vars):
                outFrame[var] = data[i, s[rowName] - minRow, s[colName] - minCol ]
                print(s)
                
        except Exception as e:
            print("Error subsetting data")
            print(i, var)
            print(s[rowName], s[colName])
            print(data.shape)
            print(minRow, maxRow+1)
            print(minCol, maxCol+1)
            raise(e)
        return s

    results = {}
    meshVarFrame = adHydroMeshFrame[[rowName, colName]].copy(deep=True)
    channelVarFrame = adHydroChannelFrame[[rowName, colName]].copy(deep=True)

    #Get all variables, but only read data relative to the subset of the mesh we are working with
    data = dataShared[:, (minRow - minY):maxRow+1, (minCol - minX):maxCol+1]
    #Now for every variable, gather the variables information for the current time data in dataShared for each ADHydro element
    for i, var in enumerate(aorc_vars):
        meshVarFrame[var] = data[i, meshVarFrame[rowName] - minRow, meshVarFrame[colName] - minCol]
        channelVarFrame[var] = data[i, channelVarFrame[rowName] - minRow, channelVarFrame[colName] - minCol]
  
    #Now longer need aorc info, clean it up
    meshVarFrame.drop([rowName, colName], axis=1, inplace=True)
    channelVarFrame.drop([rowName, colName], axis=1, inplace=True)
    #Get out of geopandas and to a regular pandas dataframe
    #Convert to float64 type here to minimize possible floating point arithmitic errors later.
    meshVarFrame = pd.DataFrame(meshVarFrame, dtype=pd.np.float64)
    channelVarFrame = pd.DataFrame(channelVarFrame, dtype=pd.np.float64)
    #Take the transpose so we are indexing by variable for each row, element # is each column
    results['mesh'] = meshVarFrame.T
    results['channel'] = channelVarFrame.T
    t1 = time.time()
    return results

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

def initOutput(outputName, meshP, channelP, times):
    """
        Function to inialize the ADHydro netCDF output file.
    """
    #Data variables to put into the output file
    #each variable must define a type and the dimensions.
    #Additional information such as units and fill values can also be set
    #This dictionary is used for convience to create these programatically
    varDict = {
        'T2':{'type':pd.np.float32, 'dims':('Time', 'Mesh_Elements'), 'units':'Celcius', 'fill':-99999},
      'T2_C':{'type':pd.np.float32, 'dims':('Time', 'Channel_Elements'), 'units':'Celcius', 'fill':-99999},
    'QVAPOR':{'type':pd.np.float32, 'dims':('Time', 'Mesh_Elements'), 'units':'kg kg-1', 'fill':-99999},
  'QVAPOR_C':{'type':pd.np.float32, 'dims':('Time', 'Channel_Elements'), 'units':'kg kg-1', 'fill':-99999},
    'QCLOUD':{'type':pd.np.float32, 'dims':('Time', 'Mesh_Elements'), 'units':'kg kg-1', 'fill':-99999},
  'QCLOUD_C':{'type':pd.np.float32, 'dims':('Time', 'Channel_Elements'), 'units':'kg kg-1', 'fill':-99999},
      'PSFC':{'type':pd.np.float32, 'dims':('Time', 'Mesh_Elements'), 'units':'pa', 'fill':-99999},
    'PSFC_C':{'type':pd.np.float32, 'dims':('Time', 'Channel_Elements'), 'units':'pa', 'fill':-99999},
         'U':{'type':pd.np.float32, 'dims':('Time', 'Mesh_Elements'), 'units':'m s-1', 'fill':-99999},
       'U_C':{'type':pd.np.float32, 'dims':('Time', 'Channel_Elements'), 'units':'m s-1', 'fill':-99999},
         'V':{'type':pd.np.float32, 'dims':('Time', 'Mesh_Elements'), 'units':'m s-1', 'fill':-99999},
       'V_C':{'type':pd.np.float32, 'dims':('Time', 'Channel_Elements'), 'units':'m s-1', 'fill':-99999},
     'TPREC':{'type':pd.np.float32, 'dims':('Time', 'Mesh_Elements'), 'units':'m s-1', 'fill':-99999},
   'TPREC_C':{'type':pd.np.float32, 'dims':('Time', 'Channel_Elements'), 'units':'m s-1', 'fill':-99999},
    'SWDOWN':{'type':pd.np.float32, 'dims':('Time', 'Mesh_Elements'), 'units':'W m-2', 'fill':-99999},
  'SWDOWN_C':{'type':pd.np.float32, 'dims':('Time', 'Channel_Elements'), 'units':'W m-2', 'fill':-99999},
       'GLW':{'type':pd.np.float32, 'dims':('Time', 'Mesh_Elements'), 'units':'W m-2', 'fill':-99999},
     'GLW_C':{'type':pd.np.float32, 'dims':('Time', 'Channel_Elements'), 'units':'W m-2', 'fill':-99999},
      'PBLH':{'type':pd.np.float32, 'dims':('Time', 'Mesh_Elements'), 'units':'m', 'fill':-99999},
    'PBLH_C':{'type':pd.np.float32, 'dims':('Time', 'Channel_Elements'), 'units':'m', 'fill':-99999},
      'TSLB':{'type':pd.np.float32, 'dims':('Time', 'Mesh_Elements'), 'units':'Celcius', 'fill':-99999},
    'TSLB_C':{'type':pd.np.float32, 'dims':('Time', 'Channel_Elements'), 'units':'Celcius', 'fill':-99999},
    }
    #Create the netCDF data set in write mode
    dataSet = nc.Dataset(outputName, 'w', format='NETCDF4')
    #Create the dimensions
    dataSet.createDimension('Time', None)
    dataSet.createDimension('Mesh_Elements', len(meshP))
    dataSet.createDimension('Channel_Elements', len(channelP))
    
    #Create and set the time variable
    """
    #Not currently used by ADHydro, remove it to save some space!!
    dataSet.createVariable('Time', int, ('Time'))
    dataSet.variables['Time'].units = 'Hours since {}'.format(times.Time[0])
    dataSet.variables['Time'].long_name = 'Time'
    
    #In theory, this is just the index, but if the interval changes in the future, may no longer be true
    #so we simply subtract each time from the first one and divide by a 1 hour time delta to get the hourse since
    #T[0].  If interval changes later, the timedelta may need to be changed here.
    t = (times.Time-times.Time[0]).divide(pd.np.timedelta64(1, 'h')).astype(int)
    dataSet.variables['Time'] = t.values
    """
    #Set the julian time
    dataSet.createVariable('JULTIME', pd.np.float64, ('Time'), fill_value=-99999)
    dataSet.variables['JULTIME'].units = 'days'
    #Convert the time stamps to julian dates and put in the netCDF data.
    #dataSet.variables['JULTIME'][:] = times.set_index('Time').index.to_julian_date().values
    dataSet.variables['JULTIME'][:] = times.index.to_julian_date().values

    """
    #Not currently used by ADHydro, remove it to save some space
    #Set the mesh_elements index
    dataSet.createVariable('Mesh_Elements', int, ('Mesh_Elements'))
    dataSet.variables['Mesh_Elements'].long_name = 'Mesh_Elements'
    dataSet.variables['Mesh_Elements'] = meshP.index.values

    #Set the channel_elements index
    dataSet.createVariable('Channel_Elements', int, ('Channel_Elements'))
    dataSet.variables['Channel_Elements'].long_name = 'Channel_Elements'
    dataSet.variables['Channel_Elements'] = channelP.index.values
    """

    #Now create each of the data variables that will be filled in by this script
    for name, info in varDict.items():
        dataSet.createVariable(name, info['type'], info['dims'], fill_value=info['fill'])
        dataSet.variables[name].units = info['units']
    #Commit these to the file and close 
    dataSet.close()


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
        raise Exception("Argument minRow must not be None.  Pass the minimum row of the NWM domain for the current mesh")
    if minCol is None:
        raise Exception("Argument minCol must not be None.  Pass the minimum column of the of the NWM domain for the current mesh")
    if maxRow is None:
        raise Exception("Argument maxRow must not be None.  Pass the maximum row of the NWM domain for the current mesh")
    if maxCol is None:
        raise Exception("Argument maxCol must not be None.  Pass the maximum column of the of the NWM domain for the current mesh")
    
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
    for time, precip in precipResult.items():
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

def plotDomain(mesh, nexrad=False):
    """
        Plot the NWM domain that covers the mesh.  Also plot any mesh element that were not
        directly mapped to a NWM cell (i.e. those that required a nearest neighbor search.)
    """
    if nexrad:
        rowName = 'nexRADrow'
        colName = 'nexRADcol'
        found = 'Found_nexRAD'
    else:
        rowName = 'nwmY'
        colName = 'nwmX'
        found = 'Found'
    
    index = pd.Series(list(zip(mesh[rowName], mesh[colName]))).unique()
    if nexrad:
        ax = nexradCoords.plot()#nexradCoords.loc[index].plot()
    else:
        #TODO/FIXME QUERY POSTGIS DB TO GET NWM GEOMETRIES TO PLOT
        #ax = wrfCoords.loc[index].plot()
        ax = None
    mesh[mesh[found] == False].plot(ax=ax)
    plt.show()

def init_shared_arrays(meshShared, channelShared, aorcData, precipData):
    """
        Subprocess initilizer to set up shared memory arrays.
    """
    global meshDataArray
    global channelDataArray
    global dataShared
    global precipShared
    meshDataArray = meshShared
    channelDataArray = channelShared
    dataShared = aorcData
    precipShared = precipData 

all_times['next'] = all_times['Time'].shift(-1)

"""
MUST GET A DATA POINT FOR THE LAST TIME'S NEXT VALUE TO CALCULATE PROPER RATE
"""
last = all_times.iloc[-1].copy()
next_time = last['Time'] + pd.np.timedelta64(15, 'm')
file_path = os.path.join(args.AORCdir, str(next_time.year))
file_path = os.path.join(args.AORCdir, '2015') #HACKED FOR CROSSING YEAR BOUNDARIES FIXME
file_dir = glob.glob(os.path.join(file_path, 'Precip*'))[0] #ASSUME ONLY ONE PRECIP SUB DIR FIXME MAY NEED TO EXTEND THIS LATER
precip_file = os.path.join(file_dir, '{}_{}.nc'.format(mrms_forcing_name, next_time.strftime('%Y%m%d%H%M')))
if os.path.exists(precip_file):
    last['next'] = next_time
    all_times.iloc[-1] = last.copy()
    #Add the last time, not used as a forcing time but required to derive proper forcing rate
    if next_time.minute == 0:
        last['coincident'] = True
        last['Time'] = next_time
        last['next'] = pd.np.nan
        all_times.loc[next_time] = last
else:
    next_time = last['Time'] + pd.np.timedelta64(1, 'h')
    precip_file = os.path.join(file_path, '{}_{}.nc4'.format(forcing_name, next_time.strftime('%Y%m%d%H')) )
    if os.path.exists(precip_file):
        last['next'] = next_time
        all_times.iloc[-1] = last.copy()
        last['coincident'] = False
        last['Time'] = next_time
        last['next'] = pd.np.nan
        all_times.loc[next_time] = last

adhydro_geom = nc.Dataset(ADHydroGeometryFile, 'r')
channelDims = adhydro_geom.variables['channelVertexX'].shape
meshDims = adhydro_geom.variables['meshVertexX'].shape
adhydro_geom.close()
#FIXME remove comments below and comment out thrid line when going to new mesh
#mesh, channel = getADHydroCenterGeometry(ADHydroGeometryFile)

reuse_geom = False

#FIXME remove comment below when not reusing geom
if reuse_geom:
    mesh, channel = (None, None)
else:
    mesh, channel = getADHydroPolyGeometry(ADHydroGeometryFile)

#create output for all_times except the last one, which is needed to derive the output but isn't included in simulation forcing
output = initOutput(outFileName, pd.Series(np.arange(meshDims[1])), pd.Series(np.arange(channelDims[1])), all_times[:-1])
output = nc.Dataset(outFileName, 'a')
#Globals to hold data in later TODO/FIXME if we clean these up in getVars, copy-on-write is useless
intersectMesh = None
intersectChannel = None
initTime1 = time.time()

#Open the first aorc file and use its grid for all intersections.
first_time = all_times.iloc[0]['Time']
file_name = "{}_{:02d}{:02d}{:02d}{:02d}.nc4".format(forcing_name, first_time.year, first_time.month, first_time.day, first_time.hour)
file_path = os.path.join(args.AORCdir, str(first_time.year) )
aorc_file = os.path.join(file_path, file_name)
#Read dataset to get coordinate info
aorcData = nc.Dataset(aorc_file, 'r')
aorcCoords, aorcIndex = getAORCCoords(aorcData)
aorcData.close()

all_times = all_times.reset_index(drop=True)
all_times.reset_index(inplace=True) #We want index by Time but retain interger index position, so reset
all_times = all_times.set_index('Time')
#Process hourly time stamps.  Do not include the last entry to process as it should not be part of the output 
time_groups = all_times[:-1].groupby(pd.TimeGrouper(freq='1H'))

print("Time to initialize: {}".format(initTime1 - initTime0))
#Some simple mass balance accumulators
aorc_accums = []
adhydro_accums = []
if __name__ == '__main__':
    print("Initial memory usage: {} GB".format(mem_usage(True)))
    start_time = time.time()
    """
        Processing subsets of the mesh at a time 
    """
    if args.numSubsets:
        N = args.numSubsets
    else:
        N = 1
    channelSubsets = getChunks(channelDims[1], N)
    meshSubsets = getChunks(meshDims[1], N)
    subsets = list(zip(meshSubsets, channelSubsets))
    num_cores = mp.cpu_count()
    
    for mSubset, cSubset in subsets:
        mesh_chunks = [1]
        channel_chunks = [1]
        pool = mp.Pool(num_cores)
        #TODO/FIXME is this call redundant with above pre-process getADHydroPolyGeometry() ???
        #mesh, channel = getADHydroPolyGeometry(ADHydroGeometryFile, mSubset, cSubset)
        #ax = nexradCoords.plot()
        #mesh.plot(ax=ax)
        #plt.show()
        if reuse_geom:
            t0 = time.time() 
            intersectMesh = pd.read_msgpack('meshIntersect.msg')
            intersectChannel = pd.read_msgpack('channelIntersect.msg')
            mesh_chunks = getChunks(len(intersectMesh), num_cores)
            channel_chunks = getChunks(len(intersectChannel), num_cores)
            t1 = time.time()
        else:
            mesh_chunks = getChunks(len(mesh), num_cores)
            channel_chunks = getChunks(len(channel), num_cores)
            
            print("Processing {} mesh elements, {} channel elements on {} cores.".format(len(mesh), len(channel), num_cores))
            print("Intersecting mesh/channel elements with AORC grid...")
            sys.stdout.flush()
            #Split the processing up across each core as evenly as possible
            t0 = time.time()
            #Mesh and Channel Intersections
            print("Intersecting Mesh")
            results = pool.map(partial(intersectMP, whichADHydro='mesh', how='poly'), mesh_chunks)
            intersectMesh = pd.concat(results)
            print("Intersecting Channel")
            results = pool.map(partial(intersectMP, whichADHydro='channel', how='poly'), channel_chunks)
            intersectChannel = pd.concat(results)
            t1 = time.time()
            
            #SERIALIZE FOR TESTING
            toDrop = ['geometry']
            intersectMesh.drop(toDrop, axis=1, inplace=True)
            intersectChannel.drop(toDrop, axis=1, inplace=True)
        
            pd.DataFrame(intersectMesh).to_msgpack('meshIntersect.msg')        
            pd.DataFrame(intersectChannel).to_msgpack('channelIntersect.msg')
        
        # FIXME UNCOMMENT
        print('{} mesh elements not mapped directly to an AORC cell, used Nearest Neighbor'.format( len( intersectMesh[ intersectMesh['Found'] == False]) ))
        print('{} channel elements not mapped directly to an AORC cell, used Nearest Neighbor'.format( len( intersectChannel[ intersectChannel['Found'] == False]) ))
        #print  intersectChannel[intersectChannel['Found'] == False]
        #intersectChannel[intersectChannel['Found'] == False].plot()
        #plt.show()
        
        toDrop = ['Found']
        intersectMesh.drop(toDrop, axis=1, inplace=True)
        intersectChannel.drop(toDrop, axis=1, inplace=True)
        print("Time to intersect AORC Grid: {}".format(t1-t0))
        print("Mem after intersecting AORC Grid: {} GB".format(mem_usage(True)))
         
        #gather some information about the mesh/channel domain for reading AORC
        minX = min(intersectMesh['AORCcol'].min(), intersectChannel['AORCcol'].min())
        maxX = max(intersectMesh['AORCcol'].max(), intersectChannel['AORCcol'].max())
        
        minY = min(intersectMesh['AORCrow'].min(), intersectChannel['AORCrow'].min())
        maxY = max(intersectMesh['AORCrow'].max(), intersectChannel['AORCrow'].max())
        #print minX, minY
        #print maxX, maxY
        adhydro_to_aorc_unique = intersectMesh.drop_duplicates(['AORCrow', 'AORCcol'])
        ad_aorc_rows = adhydro_to_aorc_unique['AORCrow']
        ad_aorc_cols = adhydro_to_aorc_unique['AORCcol'] 
        pool.close()
        pool.join()
        
        #Close the old pool and re create so new processes get proper global data.  Also helps mem usage since
        #now new forked processes will get copy-on-write (on most systems, i.e. *nix) memory pages for these large data sets
        chunks = list(zip(mesh_chunks, channel_chunks))
        
        """
            Important!!!
            We are reusing the mesh and channel shared memory arrays for each variable.
            This only works because all variables are of the same shape/dimensions (time, meshElements) or (time, channelElements)
            If any variables are ever used which have different dimensions, a new shared memory array of the correct size must be
            created!
        """
        
        #for each variable of interest, read the wrf data at the correct cell and fill in the shared
        #memory array with approrpiate data.  This array is then put into the output netcdf file and
        #synced to the disk
        #TODO/FIXME Work on more than one hour at a time?  Maybe a day at a time??? Pass the workers
        #The day and and let them open up each file for that day and build a daily output???
        
        for hour, times in time_groups:
            t0 = time.time()
        
            print("Processing {} with times\n{}".format(hour, '\n'.join( [ '\t{}'.format(t.strftime('%Y-%m-%d %H:%M:%S')) for t in times.index] ) ))
            """
            For a given hour, we either have dissagregate data or we do not.
            This means that for groups of hourly frequency, it is either 1 (no dissagregate)
            or it is more (dissagregate between hours).  We always have to read the top of the hour
            in order to get the non-dissagregated variables, so find that first, then decide if we need
            additional files for dissagregated precip.
            """
            file_path = os.path.join(args.AORCdir, str(hour.year))
            file_path = os.path.join(args.AORCdir, '2015') #FIXME for crossing year boundaries str(hour.year))
            file_name = "{}_{:02d}{:02d}{:02d}{:02d}.nc4".format(forcing_name, hour.year, hour.month, hour.day, hour.hour)
            nc_file = os.path.join(file_path, file_name)
            aorc_dataset = nc.Dataset(nc_file)
            #Now gather nessicary data
            #times are the AORC times AND precip_times between AORC hourly forcing (downscaled using MRMS) that we also want to include for precip
            #it may also be the case that there is precip data for the hour mark as well, in which case we need
            #to check for its existence and add that data as well.  
            #needed_precip = times
            #if not times.iloc[0]['coincident']:
            #   needed_precip = needed_precip[1:]
        
            #Since we have to convert mass to rate, we always need at least the next precip value to get the rate for interval between the two times
            precip_times = times.index
            precip_times = precip_times.append(pd.DatetimeIndex([times['next'][-1]]))
            #precip_data will hold precipitation data for dissagregated data as well as data for the next time needed for building rates ^^^
            precip_data = np.ndarray((len(precip_times), maxY - minY + 1, maxX - minX + 1))
            #aorc_data will hold the rest of the forcing data, as well as the precip for the top of the hour
            aorc_data = np.ndarray((len(aorc_vars), maxY - minY + 1, maxX - minX + 1))
            #Read and fill in the top of the hour AORC data
            for i, var in enumerate(aorc_vars):
               #print i, var
               aorc_data[i] = aorc_dataset[var][0, minY:maxY+1,minX:maxX+1]

            local_accum = 0.0
            for i,t in enumerate(precip_times):
                if t.minute == 0 and not all_times.loc[t]['coincident']:
                    #READ AORC
                    precip_file = os.path.join(file_path, '{}_{}.nc4'.format(forcing_name, t.strftime('%Y%m%d%H')))
                    precip_dataset = nc.Dataset(precip_file, 'r')
                    precip_data[i] = precip_dataset['APCP_surface'][0, minY:maxY+1, minX:maxX+1]
                    if i != 0:
                        local_accum += sum([precip_dataset['APCP_surface'][0, row, col] for row, col in zip(ad_aorc_rows, ad_aorc_cols) ]) 
                else:
                    #Read this precip from dissagregated data
                    precip_dir = glob.glob(os.path.join(file_path, 'Precip*'))[0] #ASSUME ONLY ONE PRECIP SUB DIR FIXME MAY NEED TO EXTEND THIS LATER
                    precip_file = os.path.join(precip_dir, '{}_{}.nc'.format(mrms_forcing_name, t.strftime('%Y%m%d%H%M')))
                    precip_dataset = nc.Dataset(precip_file, 'r')
                    precip_data[i] = precip_dataset['APCP_surface'][ minY:maxY+1, minX:maxX+1]
                    if i != 0:
                        #Don't accumulate the first one since it really doesn't belong to the total accumulated, it accumulated before time t0
                        local_accum += sum([precip_dataset['APCP_surface'][row, col] for row, col in zip(ad_aorc_rows, ad_aorc_cols) ])
                #FIXME testing for neg input, remove
                neg = precip_data[i][precip_data[i] < 0]
                if len(neg) > 0:
                    print("FOUND NEG AORC input in {}".format(precip_file))
                    print(i)
                    print(neg)
                    print(len(neg)) 
                    os._exit(1)
           
            aorc_accums.append( local_accum/999.0/( (precip_times[-1]-precip_times[0])/pd.np.timedelta64(1, 's') ) )
            print("AORC ACCUM {}".format(aorc_accums[-1]))
            #Create a shared memory array for each process to put data into, then the master
            #process can write this data to the output file
            #lock=False since by design all processes are writing independent segments of the 
            #output array
            #Since precip_data is temporaly variable, must recreate "output" arrays mesh/channelShared
            #11 variables for mesh and 11 for channel.  So make an len(precip_times)*11*mesh/channel array

            #Create the readonly shared mem array for the variable data
            dataBase = mp.Array(ctypes.c_double, aorc_data.size, lock=False)
            dataShared = np.ctypeslib.as_array(dataBase).reshape(aorc_data.shape)
            np.copyto(dataShared, aorc_data)
            dataBasePrecip = mp.Array(ctypes.c_double, precip_data.size, lock=False)
            dataSharedPrecip = np.ctypeslib.as_array(dataBasePrecip).reshape(precip_data.shape)
            np.copyto(dataSharedPrecip, precip_data)
            
            #Init the "output" shared memory arrays
            outputSize = len(times)
            
            meshBase = mp.Array(ctypes.c_double, outputSize*11*len(mSubset), lock=False)
            meshShared = np.frombuffer(meshBase).reshape(outputSize, 11, len(mSubset))
         
            channelBase = mp.Array(ctypes.c_double, outputSize*11*len(cSubset), lock=False)
            channelShared = np.ctypeslib.as_array(channelBase).reshape(outputSize, 11, len(cSubset))

            del aorc_data
            del precip_data
            readTime = time.time()
            #init the processes with the data arrays
            pool = mp.Pool(num_cores, initializer=init_shared_arrays,initargs=(meshShared, channelShared, dataShared, dataSharedPrecip))

            #print "Time to read timestamp {}: {}".format(hour, readTime - t0)
            #To be safe, fill the arrays with fill_value first
            meshShared.fill(-99999)
            channelShared.fill(-99999)
            results = pool.map(partial(getVarMP, times=times, minRow=minY, maxRow=maxY, minCol=minX, maxCol=maxX), chunks )
            pool.close()
            pool.join()
            #print times 
            adhydro_local_accum = 0.0
            for i, t_index in enumerate(times['index']):
                #print i, t_index
                for var in aorc_vars:
                    ad_hydro_var =  aorc_to_adhydro_vars[var]
                    var_index = adhydro_var_array_index[ad_hydro_var]
                    output.variables[ad_hydro_var][t_index,mSubset[0]:mSubset[-1]+1] = meshShared[i, var_index,:]
                    output.variables[ad_hydro_var+'_C'][t_index,cSubset[0]:cSubset[-1]+1] = channelShared[i, var_index,:]
                for var in ['TSLB', 'QCLOUD', 'QVAPOR', 'PBLH']:
                    var_index = adhydro_var_array_index[var]
                    output.variables[var][t_index,mSubset[0]:mSubset[-1]+1] = meshShared[i, var_index,:]
                    output.variables[var+'_C'][t_index,cSubset[0]:cSubset[-1]+1] = channelShared[i, var_index,:]
                #Set precip
                var_index = adhydro_var_array_index['TPREC']
                output.variables['TPREC'][t_index,mSubset[0]:mSubset[-1]+1] = meshShared[i, var_index,:]
                output.variables['TPREC'+'_C'][t_index,cSubset[0]:cSubset[-1]+1] = channelShared[i, var_index,:]
                adhydro_local_accum += pd.np.sum(output.variables['TPREC'][t_index ,adhydro_to_aorc_unique.index])
            
            adhydro_accums.append( adhydro_local_accum )
            print("ADHYDRO ACCUM {}".format(adhydro_accums[-1])) 
            output.sync()
            t1 = time.time()
            print("Time to read/write timestamp {}: {}".format(hour, t1-t0))
    #TODO track down this mass balance difference, testing on 2008-08-17:20
    #adhydro_accum = pd.np.sum(output.variables['TPREC'][:,adhydro_to_aorc_unique.index])
    #print all_times
    #accum_time = (all_times.index[-1] - all_times.index[0])/pd.np.timedelta64(1,'s')
    #print "ACCUM TIME {}".format(accum_time)
    print("meter/second total over ADHYDRO domain {}".format(sum(adhydro_accums)))
    #print "kg/m^2 total over AORC domain {}".format(sum(aorc_accums))
    #aorc_accums = [ a/999.0 for a in aorc_accums ]
    print("Approximate m/second over AORC domain {}".format((sum(aorc_accums))))
    #print aorc_accums
    #print len(aorc_accums)
    #print len(adhydro_accums)
    output.close()
    end_time = time.time()
    print("Total time: {}".format(end_time-start_time))
    print("Final mem usage: {}".format(mem_usage(True)))
