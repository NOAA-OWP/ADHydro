#!/usr/bin/env python
"""
Author: Nels Frazier nfrazie1@uwyo.edu
Date: June 6, 2017

IMPORTANT NOTE:
This script runs with geopandas 0.2+5.gb048d13 installed from GitHub.
The next official geopandas release should include spatial index support
which is the feature from this branch that is required for this script.
EDIT: Broke on mtmoran compute cluster, falling back to qgis spatial index
A bug in the Rtree bulk insert in version 0.8.2 requires that Rtree version 0.8.0
be used.
"""
import matplotlib.pylab as plt
import netCDF4 as nc
import geopandas as gpd
import pandas as pd
import os
import gc
import sys
import psutil
import time
import multiprocessing as mp
from math import ceil
from shapely.geometry import Point, Polygon, LineString#, box
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
import psycopg2
import subprocess


#FIXME really should mov epostgres database info to command line args...another day...

"""CURSOR AND CONNECTION TESTING
geometry = "ST_GeomFromText('{}')".format('POLYGON((20189392.1587 14459559.1203, 20189773.656 14459492.7524, 20189603.0267 14459871.087, 20189392.1587 14459559.1203))')
t_from =  "'+proj=sinu +lon_0={} +x_0=20000000 +y_0=10000000 +datum=WGS84 +units=m +no_defs'".format(-109)
t_to = "900914"
transform = "ST_Transform({}, {}, {})".format(geometry, t_from, t_to)
cursor.execute("SELECT * FROM fishnet_nwm_1km "+\
"WHERE ST_Intersects({}, wkb_geometry);".format(transform) )

for result in cursor:
    print result

connection.commit()
connection.close()
status = subprocess.call(['pg_ctl', 'stop', '-w', '-D', '/project/CI-WATER/data/postgres_data'])
if status != 0:
    print "Failed to shutdown postgres server."
else:
    print "Postgres server shutdown."
os._exit(1)
"""
#Since Rtree spatial index doesn't want to work on moran, need qgis for 
#spatial index
#qgishome = '/project/CI-WATER/tools/CI-WATER-tools'
#app = QgsApplication([], True)
#app.setPrefixPath(qgishome, True)
#app.initQgis()

"""
NOTE:
NWM grid is rather large, and reading the entire thing into memory only to subset a small portion seems like a poor idea.
Instead, the gdb should be preloaded into a postgres sql PostGIS database:
mkdir data/postgres_data
cd data/postgres_data
initdb ./
postgres -D ./ &
createdb nwm
psql -d nwm -c "CREATE EXTENSION postgis;"
###I DONT THINK THIS ONE IS NESSICARY###psql -d nwm -c "CREATE EXTENSION postgis_topology;"
cd <nwm.gdb containing dir>
ogr2ogr -progress -overwrite -f "PostgreSQL" PG:"host=localhost user=nfrazie1 dbname=nwm" "nwm.gdb"

Wait a few minuts for the db to be created, then verify that it contains a spatial index for the wkb_geometry column
psql -d nwm -c "\d+ fishnet_nwm_1km;"
Update db stats using
psql -d nwm -c "VACUUM ANALYZE;"
"""
postgres_dir = '/project/CI-WATER/data/postgres_data'
def pg_init():
    """
    Function for starting the PostGres server process.  If the NWM database exists in a different directory,
    the the postgres_dir global must be updated appropriately.
    """
    status = subprocess.call(['pg_ctl', 'status', '-D', postgres_dir])
    print "pg_ctl return status: {}".format(status)
    if status == 3:
        print "STARTING POSTGRES SERVER"
        #subprocess.call(['pg_ctl', 'start','-l', '/dev/null', '-w', '-D', '/project/CI-WATER/data/postgres_data'])
        subprocess.call(['pg_ctl', 'start', '-w', '-D', postgres_dir])

    elif status == 0:
        print "POSTGRES SERVER RUNNING, PREPARING CONNECTION"
    else:
        print "POSTGRES SERVER ERROR: pg_ctl status returned: {}".format(status)
    
def pg_finalize(connection=None, cursor=None):
    """
    Function for shutting down the PostGres processes.  Additionally can close cursor and connection
    if they are passed as variables.  This is useful for graceful exit on errors.
    """
    if cursor != None:
        cursor.close() 
    if connection != None:
        connection.commit()
        connection.close()
    status = subprocess.call(['pg_ctl', 'stop', '-m', 'fast', '-w', '-D', postgres_dir])
    if status != 0:
        print "Failed to shutdown postgres server."
    else:
        print "Postgres server shutdown."

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
        return datetime.strptime(s, '%Y-%m-%d')

def checkPositiveInput(i):
    value = int(i)
    if value < 1:
        raise argparse.ArguementTypeError('{} is not a valid input, must be >= 1'.format(value))
    return value

parser = argparse.ArgumentParser(description="Create an ADHydro forcing file from NWM data.")
parser.add_argument('ADHydroMapDir', help='The ADHydro map directory containing the simulation input data.')
parser.add_argument('NWMdir', help='The top level nwm dir containing daily subdirectories')
parser.add_argument('-s', '--startTime', help='The year and month to start processing, formatted as YYYY-MM.', required=True, type=verify_date)
parser.add_argument('-e', '--endTime', help='The year and month to stop processing (inclusive), formatted as YYYY-MM.', required=True, type=verify_date)
parser.add_argument('-f', '--outputFileName', help='Optional output file name.  By default, the filename will be set based on the output times.')
parser.add_argument('-o', '--outputDir', help='Optional output directory. By default, output will be put in ADHydroMapDir/forcing ')
#parser.add_argument('-a', '--adjustPrecip', nargs='?', const=1, type=checkPositiveInput, \
#                    help='Optional flag for dropping WRF boundaries for precipitation calculation.\n\
#                          An optional integer value, N, can be passed with this parameter as well. By default, N=1.\n\
#                          When set, the first and last N rows and columns of WRF data are dropped before intersecting with mesh geometries.')
parser.add_argument('-i', '--inputDir', help='Optional input directory containing geometry.nc to processes.  If not set, looks in ADHydroMapDir/mesh_massage/')
#parser.add_argument('-g', '--gridSize', type=checkPositiveInput, \
#                    help='Optional NWM grid size arguement (in meters).  Defaults to 1000 m.\n\
#                    When set, NWM grid cells will be calculated based on the input size.\n\
#                    For example, passing 2000 will create NWM grid cells of size 2000m x 2000m by creating a bounding sqaure 1000m around the center point.')
parser.add_argument('-n', '--numSubsets', type=checkPositiveInput, help='Split the processing into N subsets of the mesh.  Useful on large meshes to avoid running out of memory.')
parser.add_argument('-m', '--meridian', help='Centeral meridian to use for ADHydro Sinusoidal projection', required=True, type=float)
args = parser.parse_args()
#'/project/CI-WATER/data/maps/co_district_58/drain_down/geometry.nc'
if not os.path.isdir(args.ADHydroMapDir):
    parser.error("Directory '{}' does not exist".format(args.ADHydroMapDir))

if not os.path.isdir(args.NWMdir):
    parser.error("Directory '{}' does not exist".format(args.NWMdir))

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
    print "{} directory does not exist, creating...".format(outputDir)
    os.mkdir(outputDir)

#if args.gridSize:
#    gridSize = args.gridSize
#else:
#    gridSize = 1000

#Setup the NWM information
#Make sure we work until the last hour of the last day of the input end date
args.endTime = args.endTime.replace(hour=23)

#if args.startTime.year != args.endTime.year:
#    parser.error("Script currently only supports working within a single year.\nIf needed, run it multiple times to produce multiple years.")


#TODO/FIXME Eventually read proj string from netcdf variable.
"""
Projection settings for ADHydro and NWM grid.  Currently must manually edit the adhydro_project to change the centeral meridain
to match that of the mesh being processed.  Use the -m command line flag to change the meridian.

 The nwm_projection is the SRID of the projection stored in the database.
"""
adhydro_projection = '+proj=sinu +lon_0={} +x_0=20000000 +y_0=10000000 +datum=WGS84 +units=m +no_defs'.format(args.meridian)
nwm_projection = "900914"

"""
Globals for use with multiprocessing.
Faster to access globals than trying to pickle large data and pass as args.
"""
initTime0 = time.time()
#Build the time information.  We reset_index to promote to a dataframe so that we can later use Time as the index
days = pd.date_range(args.startTime, args.endTime, freq='1d')
"""
Check for existence of data
"""
dropDays = []
for d in days:
    if not os.path.isdir( os.path.join( args.NWMdir, d.strftime('%Y%m%d'))):
        print("WARNING: Missing data directory for day {}; Ommiting from forcing output.".format(d.strftime('%Y-%m-%d')))
        dropDays.append(d)
days = days.drop(dropDays)
#Resample days to hourly, and handle any missing data days in the process
start = days.min()
stop = days.max() + pd.DateOffset(days=1)
#Build an hourly index of all possible times from min to max+1, then slice off the extra
index = pd.date_range(start, stop, freq='H')[:-1]
#Build a series indexed by the daily floor of the above hourly index, then grab only the
#rows which are in our good days
times = pd.Series(index, index.floor('D'), name='Time').loc[days]
#Convert this series to dataframe useable later by initOutput
times = pd.Series(times.values, name='Time').reset_index()

outFileName = ''
if args.outputFileName:
    outFileName = os.path.join(outputDir, args.outputFileName)
else:
    name = 'forcing_{}-{:02d}-{}_{}-{:02d}-{}.nc'.format(times['Time'].iloc[0].year, times['Time'].iloc[0].month, times['Time'].iloc[0].day, times['Time'].iloc[-1].year, times['Time'].iloc[-1].month, times['Time'].iloc[-1].day)
    outFileName = os.path.join(outputDir, name)
if os.path.isfile(outFileName):
   raise Exception("wrf_to_adhydro cowardly refusing to overwrite previous forcing file '{}'.\nPlease manually remove this file before running.".format(outFileName))

nwm_vars = ['T2D', 'LWDOWN', 'Q2D', 'U2D', 'V2D', 'PSFC', 'RAINRATE', 'SWDOWN']
nwm_to_adhydro_vars = {'T2D':'T2', 'LWDOWN':'GLW', 'Q2D':'QVAPOR', 'U2D':'U', 'V2D':'V', 'PSFC':'PSFC', 'RAINRATE':'TPREC', 'SWDOWN':'SWDOWN'}
adhydro_var_array_index = {'T2':0, 'QVAPOR':1, 'QCLOUD':2, 'PSFC':3, 'U':4, 'V':5, 'TPREC':6, 'SWDOWN':7, 'GLW':8, 'PBLH':9, 'TSLB':10}

#Clean up as much memory as possible before we start forking subprocesses
del ADHydroGeometryDir
del outputDir

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
    print "Mem after reading ADHydro center point geometries: {} GB".format(mem_usage(True))
    print "Time to read ADHydro geometry: {}".format(t1-t0)
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
    adhydroMeshVerticies =[Polygon(p) for p in [ zip(xs,ys) for xs, ys in zip(adhydroMesh['X'], adhydroMesh['Y']) ] ]
    adhydroMeshGeometry = gpd.GeoSeries(adhydroMeshVerticies, crs=adhydro_projection)
    tm = time.time()
    #print "Time to create adhydroMeshSeries: {}".format(tm-t0)
    #print "Mem usage: {}".format(mem_usage(True))
    del adhydroMeshVerticies
    del adhydroMesh
    gc.collect()
    #print "Mem usage after collection: {}".format(mem_usage(True))
    adhydroChannelVerticies = [ LineString(l) for l in [zip(xs,ys) for xs, ys in zip(adhydroChannel['X'], adhydroChannel['Y']) ] ]
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
    print "Mem after reading ADHydro polygon geometries: {} GB".format(mem_usage(True))
    print "Time to read ADHydro geometry: {}".format(t1-t0)
    adhydro_geom.close()
    return meshFrame, channelFrame#, meshIndex, channelIndex

def centerPointIntersect(s, cursor, bugFix):
    """
        This function assigns to each adHydro element the grid coordinates
        of the NWM cell that the center lies in.  In the event that the center
        point doesn't lie in a cell, the nearest neighbor is used.
    """
    
    cells = {}
    xIndex = None
    yIndex = None
    nwmID = None
    #Get a list of all nwmCoord indexes that intersect the series geometry
    geometry = "ST_GeomFromText('{}')".format(s.geometry)
    transform = "ST_Transform({}, '{}', {})".format(geometry, adhydro_projection, nwm_projection)
    #The following columns will be returned form the query.  Right now we only need the indicies for south_north and west_east
    #to index the netcdf files correctly, but other information may be valuable? All possible columns currently are
    #objectid, south_north, west_east, xlong_m, xlat_m, shape_length, shape_area, wkb_geometry
    #We let PostGIS take care of projecting the ADHydro geometry to the NWM CRS, and similarly, resulting NWM intersecting 
    #geometries will be transformed back to adhydro projection coordinates.
    columns = "objectid, south_north, west_east, ST_Transform(wkb_geometry, '{}')".format(adhydro_projection)
    statement = "SELECT {} FROM fishnet_nwm_1km ".format(columns)+\
                "WHERE ST_Intersects({}, wkb_geometry);".format(transform) 
    #print statement
    cursor.execute(statement)
        
    if(cursor.rowcount != 0):
        #Only one grid cell intersects, or more than one does, in this case just use the first one anyways
        #ST_Intersects gives exact intersects, so no false positives to worry about
        nwmID, yIndex, xIndex, geometry = cursor.fetchone()
        found = True
    elif cursor.rowcount == 0:
        #No results
        #TODO/FIXME try to do NN search??? If truely CONUS this shouldn't happen
        print('WARNING: A mesh element center was not directly mapped to a NWM grid cell!')
        found = False 

    if xIndex == None or yIndex == None:
        print "Not able to georeference ADHydro Geometry:"
        #TODO do NN search in this case
    cells['nwmID'] = nwmID
    cells['nwmX'] = xIndex
    cells['nwmY'] = yIndex
    cells['Found'] = found
    return pd.Series(cells)

 
#Since we have to do several intersections over the SAME mesh/channel elements
#we speed things up by using a single function applied to these which does all intersections
#This reduces the need to iterate over large numbers of elements 3-4 times!!!
def polygonIntersect(s, cursor, bugFix):
    """
        This function assigns to each adHydro element the grid coordinates
        of the nwm cell that contains the largest portion of the element.
        In the event that an element intersects no NWM cells, the nearest 
        neighbor is used.
    """
    cells = {}
    xIndex = None
    yIndex = None
    nwmID = None
    #Get a list of all nwmCoord indexes that intersect the series geometry
    geometry = "ST_GeomFromText('{}')".format(s.geometry)
    transform = "ST_Transform({}, '{}', {})".format(geometry, adhydro_projection, nwm_projection)
    #The following columns will be returned form the query.  Right now we only need the indicies for south_north and west_east
    #to index the netcdf files correctly, but other information may be valuable? All possible columns currently are
    #objectid, south_north, west_east, xlong_m, xlat_m, shape_length, shape_area, wkb_geometry
    #We let PostGIS take care of projecting the ADHydro geometry to the NWM CRS, and similarly, resulting NWM intersecting 
    #geometries will be transformed back to adhydro projection coordinates.
    columns = "objectid, south_north, west_east, ST_Transform(wkb_geometry, '{}')".format(adhydro_projection)
    statement = "SELECT {} FROM fishnet_nwm_1km ".format(columns)+\
                "WHERE ST_Intersects({}, wkb_geometry);".format(transform) 
    #print statement
    cursor.execute(statement)
        
    if(cursor.rowcount == 1):
        #Only one grid cell intersects, easy enough.
        #ST_Intersects gives exact intersects, so no false positives to worry about
        nwmID, yIndex, xIndex, geometry = cursor.fetchone()
        found = True
    elif cursor.rowcount == 0:
        #No results
        #TODO/FIXME try to do NN search??? If truely CONUS this shouldn't happen
        found = False
        if s.geometry.geom_type == 'LineString':
            print('WARNING: Channel element {} was not directly mapped to a NWM grid cell!'.format(s.name))
            if s.geometry.length < 0.0001:
                print('WARNING: Channel element {} has zero length. Reverting to center point intersection.'.format(s.name))
                center_s = s.copy()
                center_s.geometry = s.geometry.centroid
                return centerPointIntersect(center_s, cursor, None)

        else:
            print('WARNING: Mesh element {} was not directly mapped to a NWM grid cell!'.format(s.name))
            if s.geometry.area < 0.001:
                print('WARNING: Mesh element {} has zero area. Reverting to center point intersection.'.format(s.name))
                center_s = s.copy()
                center_s.geometry = s.geometry.centroid
                return centerPointIntersect(center_s, cursor, None)
    else:
        #Multiple results, find the one with the largest overlapping area
        if s.geometry.geom_type == 'LineString':
            maxLength = 0.0
            for row, y_index, x_index, wkb_geometry in cursor:
                geom = wkb.loads(wkb_geometry, hex=True)
                length = geom.intersection(s.geometry).length
                if length > maxLength:
                    maxLength = length
                    xIndex = x_index
                    yIndex = y_index
                    nwmID = row
                    found = True       
        else:
           maxIntersectingArea = 0.0
           for row, y_index, x_index, wkb_geometry in cursor:
                geom = wkb.loads(wkb_geometry, hex=True)
                area = geom.intersection(s.geometry).area
                if area > maxIntersectingArea:
                    maxIntersectingArea = area
                    xIndex = x_index
                    yIndex = y_index
                    nwmID = row
                    found = True

    if xIndex == None or yIndex == None:
        print "Not able to georeference ADHydro Geometry:"
        #TODO do NN search in this case
    cells['nwmID'] = nwmID
    cells['nwmX'] = xIndex
    cells['nwmY'] = yIndex
    cells['Found'] = found
    return pd.Series(cells)
     
def intersectNWMgrid(adHydroFrame, nwmData, how='center'):
    """
        This function assigns each ADHydro element in adHydroFrame to a NWM grid cell.
        This can be done by simply matching the center of the cell to a cell it lies in,
        or by more complexly finding the largest intersecting cell of the element.
     """ 
    
    #If running in parallel on multiple cores, each core gets its own connection to the database
    #If on a single core, then just one connection
    connection = psycopg2.connect(database="nwm", user="nfrazie1")
    cursor = connection.cursor()
    try:
        #t0 = time.time()
        #print list(coords.sindex.intersection(mesh.loc[0].geometry.bounds, objects='raw'))
        if adHydroFrame.geom_type.eq('Point').all():
            if how != 'center':
                print "Reverting to center point calculations since geometries are all points."
            
            adHydroFrame = pd.concat([adHydroFrame, adHydroFrame.apply(centerPointIntersect, args=(cursor, None), axis=1)], axis=1)
        elif adHydroFrame.geom_type.eq('Polygon').all() or adHydroFrame.geom_type.eq('LineString').all():
            if how=='center':
                print "Changing geometry to centroids for center calculations"
                adHydroFrame['Poly'] = adHydroFrame.geometry
                adHydroFrame.geometry = adHydroFrame.centroid
                adHydroFrame = pd.concat([adHydroFrame, adHydroFrame.apply(centerPointIntersect, args=(cursor, None), axis=1)], axis=1)
                adHydroFrame.geometry = adHydroFrame['Poly']
                del adHydroFrame['Poly']
            elif how=='poly':
                adHydroFrame = pd.concat([adHydroFrame, adHydroFrame.apply(polygonIntersect, args=(cursor, None), axis=1)], axis=1)
            else:
                pg_finalize(cursor, connection)
                raise Exception("Unsupported intersection type {}. Supported options are [center, poly].".format(how))  
        else:
            print adHydroFrame.geom_type
            pg_finalize(cursor, connection)
            raise Exception("All adhydro geometries must be homogeneous.") 
        #t1 = time.time()
        #print "Mem after intersecting: {} GB".format(mem_usage(True))
        #print "Time to intersect: {}".format(t1-t0)
    except Exception, e:
        pg_finalize(connection, cursor) 
    return adHydroFrame

def getVars(adHydroMeshFrame, adHydroChannelFrame, minY, maxY, minX, maxX):
    """
        Read NWM variables and assign data to each ADHydro element(mesh and channel)
    """
    t0 = time.time()
    rowName = 'nwmY'
    colName = 'nwmX'
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
                print s
                
        except Exception, e:
            print "Error subsetting data"
            print i, var
            print s[rowName], s[colName]
            print data.shape
            print minRow, maxRow+1
            print minCol, maxCol+1
            raise(e)
        return s

    results = {}
    meshVarFrame = adHydroMeshFrame[[rowName, colName]].copy(deep=True)
    channelVarFrame = adHydroChannelFrame[[rowName, colName]].copy(deep=True)

    #Get all variables, but only read data relative to the subset of the mesh we are working with
    data = dataShared[:, (minRow - minY):maxRow+1, (minCol - minX):maxCol+1]
    #Now for every variable, gather the variables information for the current time data in dataShared for each ADHydro element
    for i, var in enumerate(nwm_vars):
        meshVarFrame[var] = data[i, meshVarFrame[rowName] - minRow, meshVarFrame[colName] - minCol]
        channelVarFrame[var] = data[i, channelVarFrame[rowName] - minRow, channelVarFrame[colName] - minCol]
  
    #Now longer need NWM info, clean it up
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
    dataSet.variables['JULTIME'][:] = times.set_index('Time').index.to_julian_date().values

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
    for name, info in varDict.iteritems():
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
    try:
        if whichADHydro == 'mesh':
            adHydro_frame = mesh.iloc[subset].copy()
        elif whichADHydro == 'channel':
            adHydro_frame = channel.iloc[subset].copy()
        nwmData = {}
        result = intersectNWMgrid(adHydro_frame, nwmData, how)
    except Exception, e:
        print "ERROR intersecting nwm grid"
        print e
        raise(e)
    #print "Process {} finished processing {} elements".format(os.getpid(), len(result))
    #print "Returning {}".format(result.shape)
    return result

#Let main processes read in the wrf data variable into a shared memory array
#This will prevent conflicting reads at different positions by each process
#at the disk level, then only have cache misses to slow us down.
def getVarMP(subset, minRow=None, maxRow=None, minCol=None, maxCol=None):
    """
        Wrapper for multiprocessing to gather NWM data in parallel on subsets of mesh and channel.
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
    #Map mesh/channel elements to appropriate NWM values
    result = getVars(mesh, channel, minRow, maxRow, minCol, maxCol)
    #Temperature var, apply data transform to all values before adding to output
    #print result['mesh']
    #print result['channel']
    result['mesh'].loc['T2D'] = result['mesh'].loc['T2D'].map(lambda x: x - 272.15)
    result['channel'].loc['T2D'] = result['channel'].loc['T2D'].map(lambda x: x - 272.15)
    result['mesh'].loc['RAINRATE'] = result['mesh'].loc['RAINRATE'].map(lambda x: x*.001) #Convert mm/s to m/s
    result['channel'].loc['RAINRATE'] = result['channel'].loc['RAINRATE'].map(lambda x: x*.001) #Convert mm/s to m/s

    for i, var in enumerate(nwm_vars):
        var_index = adhydro_var_array_index[ nwm_to_adhydro_vars[var]]
        meshDataArray[ var_index , mesh.index[0]:mesh.index[-1]+1] = result['mesh'].loc[var].values
        channelDataArray[ var_index, channel.index[0]:channel.index[-1]+1] = result['channel'].loc[var].values

    #Now we need to add data for the other vars not included in NWM forcing:
    #TSLB, QVAPOR, PBLH
    maxTemp = result['mesh'].loc['T2D'].max()
    minTemp = result['mesh'].loc['T2D'].min()
    meshDataArray[ adhydro_var_array_index['TSLB'], mesh.index[0]:mesh.index[-1]+1] = (maxTemp+minTemp)/2
    #Safe to assume min and max temp of mesh is OK since channel overlaps mesh???
    channelDataArray[ adhydro_var_array_index['TSLB'], channel.index[0]:channel.index[-1]+1] = (maxTemp+minTemp)/2
    #No cloud water
    meshDataArray[ adhydro_var_array_index['QCLOUD'], mesh.index[0]:mesh.index[-1]+1] = 0 
    channelDataArray[ adhydro_var_array_index['QCLOUD'], channel.index[0]:channel.index[-1]+1] = 0
    #Const PBLH.  This is probably a bad idea, but for lack of better data, it will do for now.
    #The PBLH is the hieght of the "active" atmosphere.  However, ADHydro passes this to NoamMP, which only uses it 
    #For certain options, which ADHydro doesn't use.  So the value of PBLH has zero effect on the results of ADHydro
    meshDataArray[ adhydro_var_array_index['PBLH'], mesh.index[0]:mesh.index[-1]+1] = 500
    channelDataArray[ adhydro_var_array_index['PBLH'], channel.index[0]:channel.index[-1]+1] = 500

def plotDomain(mesh):
    """
        Plot the NWM domain that covers the mesh.  Also plot any mesh element that were not
        directly mapped to a NWM cell (i.e. those that required a nearest neighbor search.)
    """
    rowName = 'nwmY'
    colName = 'nwmX'

    index = pd.Series(zip(mesh[rowName], mesh[colName])).unique()
    #TODO/FIXME QUERY POSTGIS DB TO GET NWM GEOMETRIES TO PLOT
    #ax = wrfCoords.loc[index].plot()
    mesh[mesh['Found'] == False].plot(ax=ax)
    #plt.show()

def init_shared_arrays(meshShared, channelShared, nwmData):
    """
        Subprocess initilizer to set up shared memory arrays.
    """
    global meshDataArray
    global channelDataArray
    global dataShared
    meshDataArray = meshShared
    channelDataArray = channelShared
    dataShared = nwmData

adhydro_geom = nc.Dataset(ADHydroGeometryFile, 'r')
channelDims = adhydro_geom.variables['channelVertexX'].shape
meshDims = adhydro_geom.variables['meshVertexX'].shape
adhydro_geom.close()

#mesh, channel = getADHydroCenterGeometry(ADHydroGeometryFile)
#mesh, channel = getADHydroPolyGeometry(ADHydroGeometryFile)
mesh, channel = (None, None)
output = initOutput(outFileName, pd.Series(np.arange(meshDims[1])), pd.Series(np.arange(channelDims[1])), times)
output = nc.Dataset(outFileName, 'a')
#Globals to hold data in later TODO/FIXME if we clean these up in getVars, copy-on-write is useless
intersectMesh = None
intersectChannel = None
initTime1 = time.time()

print "Time to initialize: {}".format(initTime1 - initTime0)

if __name__ == '__main__':
    #Initialize database daemon
    pg_init()
    print "Initial memory usage: {} GB".format(mem_usage(True))
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
    subsets = zip(meshSubsets, channelSubsets)
    num_cores = mp.cpu_count()

    for mSubset, cSubset in subsets:
        mesh_chunks = [1]
        channel_chunks = [1]
         
        mesh, channel = getADHydroPolyGeometry(ADHydroGeometryFile, mSubset, cSubset)
        mesh_chunks = getChunks(len(mesh), num_cores)
        channel_chunks = getChunks(len(channel), num_cores)
        print "Processing {} mesh elements, {} channel elements on {} cores.".format(len(mesh), len(channel), num_cores)
        print "Intersecting mesh/channel elements with NWM grid..."
        sys.stdout.flush()
        #Split the processing up across each core as evenly as possible
        t0 = time.time()
        pool = mp.Pool(num_cores)
        #Mesh and Channel Intersections
        results = pool.map(partial(intersectMP, whichADHydro='mesh', how='poly'), mesh_chunks)
        intersectMesh = pd.concat(results)
        results = pool.map(partial(intersectMP, whichADHydro='channel', how='poly'), channel_chunks)
        intersectChannel = pd.concat(results)
        t1 = time.time()
        #gather some information about the mesh/channel domain for readingNWM
        minX = min(intersectMesh['nwmX'].min(), intersectChannel['nwmX'].min())
        maxX = max(intersectMesh['nwmX'].max(), intersectChannel['nwmX'].max())
        
        minY = min(intersectMesh['nwmY'].min(), intersectChannel['nwmY'].min())
        maxY = max(intersectMesh['nwmY'].max(), intersectChannel['nwmY'].max())
 
        print '{} mesh elements not mapped directly to a NWM cell'.format( len( intersectMesh[ intersectMesh['Found'] == False]) )
        print '{} channel elements not mapped directly to a NWM cell'.format( len( intersectChannel[ intersectChannel['Found'] == False]) )
        print  intersectChannel[intersectChannel['Found'] == False]
        intersectChannel[intersectChannel['Found'] == False].plot()

        toDrop = ['Found', 'geometry']
        intersectMesh.drop(toDrop, axis=1, inplace=True)
        intersectChannel.drop(toDrop, axis=1, inplace=True)
        print "Time to intersect NWM Grid: {}".format(t1-t0)
        print "Mem after intersecting theta: {} GB".format(mem_usage(True))
        pool.close()
        pool.join()
         
        #Close the old pool and re create so new processes get proper global data.  Also helps mem usage since
        #now new forked processes will get copy-on-write (on most systems, i.e. *nix) memory pages for these large data sets
        #Create a shared memory array for each process to put data into, then the master
        #process can write this data to the output file
        #lock=False since by design all processes are writing independent segments of the 
        #output array
        #Since NWM data is already broken up by time in the file/directory structure, we will read one file
        #at a time and let each processes process all variables for that time.  ADHydro forcing using 
        #11 variables for mesh and 11 for channel.  So make an 11*mesh array
        meshBase = mp.Array(ctypes.c_double, 11*len(mSubset), lock=False)
        meshShared = np.frombuffer(meshBase).reshape(11, len(mSubset))
     
        channelBase = mp.Array(ctypes.c_double, 11*len(cSubset), lock=False)
        channelShared = np.ctypeslib.as_array(channelBase).reshape(11, len(cSubset))
        chunks = zip(mesh_chunks, channel_chunks)
        
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
        for t_index, t in enumerate(times['Time']):
            print t
            day_dir = t.strftime('%Y%m%d')
            nc_dir = os.path.join( os.path.join(args.NWMdir,  day_dir), 'fe_analysis_assim')
            nc_file = os.path.join(nc_dir, 'nwm.t{}z.fe_analysis_assim.tm00.conus.nc'.format(t.strftime('%H')))
            print "Processing timestamp {}".format(t)
            nwmData = nc.Dataset(nc_file)
            data = np.ndarray((len(nwm_vars), maxY - minY + 1, maxX - minX + 1))
            #print data.shape
            for i, var in enumerate(nwm_vars):
               #print i, var
               data[i] = nwmData[var][minY:maxY+1,minX:maxX+1] 
            t0 = time.time()
            #Create the readonly shared mem array for the variable data
            dataBase = mp.Array(ctypes.c_double, data.size, lock=False)
            dataShared = np.ctypeslib.as_array(dataBase).reshape(data.shape)
            np.copyto(dataShared, data)
            del data
            readTime = time.time()
            #init the processes with the data arrays
            pool = mp.Pool(num_cores, initializer=init_shared_arrays,initargs=(meshShared, channelShared, dataShared))

            print "Time to read timestamp {}: {}".format(t, readTime - t0)
            #To be safe, fill the arrays with fill_value first
            meshShared.fill(-99999)
            channelShared.fill(-99999)
            results = pool.map(partial(getVarMP, minRow=minY, maxRow=maxY, minCol=minX, maxCol=maxX), chunks )
            pool.close()
            pool.join()
            for var in nwm_vars:
                ad_hydro_var =  nwm_to_adhydro_vars[var]
                var_index = adhydro_var_array_index[ad_hydro_var]
                output.variables[ad_hydro_var][t_index,mSubset[0]:mSubset[-1]+1] = meshShared[var_index,:]
                output.variables[ad_hydro_var+'_C'][t_index,cSubset[0]:cSubset[-1]+1] = channelShared[var_index,:]
            for var in ['TSLB', 'QCLOUD', 'PBLH']:
                var_index = adhydro_var_array_index[var]
                output.variables[var][t_index,mSubset[0]:mSubset[-1]+1] = meshShared[var_index,:]
                output.variables[var+'_C'][t_index,cSubset[0]:cSubset[-1]+1] = channelShared[var_index,:]

            output.sync()
            t1 = time.time()
            print "Time to read/write timestamp {}: {}".format(t, t1-t0)
        
    output.close()
    pg_finalize()
    end_time = time.time()
    print "Total time: {}".format(end_time-start_time)
    print "Final mem usage: {}".format(mem_usage(True))
