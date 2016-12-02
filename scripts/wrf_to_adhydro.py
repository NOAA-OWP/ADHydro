#!/usr/bin/env python
"""
Author: Nels Frazier nfrazie1@uwyo.edu
Date: September 8, 2016

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
import numpy as np
import ctypes
from functools import partial
import argparse
import traceback
from datetime import datetime
from calendar import monthrange
import re
from qgis.core import *

#Since Rtree spatial index doesn't want to work on moran, need qgis for 
#spatial index
qgishome = '/project/CI-WATER/tools/CI-WATER-tools'
app = QgsApplication([], True)
app.setPrefixPath(qgishome, True)
app.initQgis()

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
        return datetime.strptime(s, '%Y-%m')

def checkPositiveInput(i):
    value = int(i)
    if value < 1:
        raise argparse.ArguementTypeError('{} is not a valid input, must be >= 1'.format(value))
    return value

parser = argparse.ArgumentParser(description="Create an ADHydro forcing file from WRF data.")
parser.add_argument('ADHydroMapDir', help='The ADHydro map directory containing the simulation input data.')
parser.add_argument('WRFdir', help='The top level wrf dir containing yearly subdirectories')
parser.add_argument('-s', '--startTime', help='The year and month to start processing, formatted as YYYY-MM.', required=True, type=verify_date)
parser.add_argument('-e', '--endTime', help='The year and month to stop processing (inclusive), formatted as YYYY-MM.', required=True, type=verify_date)
parser.add_argument('-f', '--outputFileName', help='Optional output file name.  By default, the filename will be set based on the output times.')
parser.add_argument('-o', '--outputDir', help='Optional output directory. By default, output will be put in ADHydroMapDir/forcing ')
parser.add_argument('-a', '--adjustPrecip', nargs='?', const=1, type=checkPositiveInput, \
                    help='Optional flag for dropping WRF boundaries for precipitation calculation.\n\
                          An optional integer value, N, can be passed with this parameter as well. By default, N=1.\n\
                          When set, the first and last N rows and columns of WRF data are dropped before intersecting with mesh geometries.')
parser.add_argument('-i', '--inputDir', help='Optional input directory containing geometry.nc to processes.  If not set, looks in ADHydroMapDir/mesh_massage/')
parser.add_argument('-g', '--gridSize', type=checkPositiveInput, \
                    help='Optional WRF grid size arguement (in meters).  Defaults to 4000 m.\n\
                    When set, WRF grid cells will be calculated based on the input size.\n\
                    For example, passing 2000 will create WRF grid cells of size 2000m x 2000m by creating a bounding sqaure 1000m around the center point.')
parser.add_argument('-n', '--numSubsets', type=checkPositiveInput, help='Split the processing into N subsets of the mesh.  Useful on large meshes to avoid running out of memory.')
args = parser.parse_args()
#'/project/CI-WATER/data/maps/co_district_58/drain_down/geometry.nc'
if not os.path.isdir(args.ADHydroMapDir):
    parser.error("Directory '{}' does not exist".format(args.ADHydroMapDir))

if not os.path.isdir(args.WRFdir):
    parser.error("Directory '{}' does not exist".format(args.WRFdir))

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

if args.gridSize:
    gridSize = args.gridSize
else:
    gridSize = 4000
#Setup the WRF information
#Make sure we work until the last hour of the last day of the input end month
args.endTime = args.endTime.replace(hour=23, day=monthrange(args.endTime.year, args.endTime.month)[1])
outFileName = ''
if args.outputFileName:
    outFileName = os.path.join(outputDir, args.outputFileName)
else:
    #calculate output name
    name = 'forcing_{}-{:02d}_{}-{:02d}.nc'.format(args.startTime.year, args.startTime.month, args.endTime.year, args.endTime.month)
    outFileName = os.path.join(outputDir, name)
if os.path.isfile(outFileName):
   raise Exception("wrf_to_adhydro cowardly refusing to overwrite previous forcing file '{}'.\nPlease manually remove this file before running.".format(outFileName))
if args.startTime.year != args.endTime.year:
    parser.error("Script currently only supports working within a single year.\nIf needed, run it multiple times to produce multiple years.")

#Clean up as much memory as possible before we start forking subprocesses
del ADHydroGeometryDir
del outputDir

#TODO/FIXME Eventually read proj string from netcdf variable.
proj = '+proj=sinu +lon_0=-109 +x_0=20000000 +y_0=10000000 +datum=WGS84 +units=m +no_defs' 

"""
Globals for use with multiprocessing.
Faster to access globals than trying to pickle large data and pass as args.
"""
initTime0 = time.time()
#TODO/FIXME work on a range of years
endYear = '{}/'.format(args.startTime.year)
yearDir = os.path.join(args.WRFdir, endYear)
#Figure out which wrf files for the given year we need to work with
#Get a list of all files
files = os.listdir(os.path.join(args.WRFdir,endYear))
#Figure out which months the user is interested in
months = range(args.startTime.month, args.endTime.month+1)
#grab the month before and after as long as it isn't the begining or end of the year
#TODO/FIXME Is there a better way to do this?  In some cases it really is nessicare (i.e. February)
#But for other cases, we are reading in 2 extra months of data that we don't want to use!!!
if args.startTime.month > 1:
    months.append(args.startTime.month - 1)
if args.endTime.month < 12:
    months.append(args.endTime.month + 1)
#Format these months correctly to match the file names 
monthStrings = ['{0:02d}'.format(m) for m in months]
#Build a regex OR
monthRegex = '|'.join(monthStrings)
#Grab the subset of needed files
needed = [f for f in files if re.search(r'wrfout_d03_{}-({})'.format(args.endTime.year, monthRegex), f)]
needed = [ os.path.join(yearDir, x) for x in needed ]
#Need to sort the file names so that they are passed in chronological order to netcdf, otherwise they get read in in the wrong time order
needed.sort()
#print monthRegex
#print needed
#os._exit(1)
#In case the wrf file naming tries to give us too much info, build a proper time index to cover the input range
#tIndex = pd.Series(pd.date_range(args.startTime, args.endTime, freq='H'))
#use exclude=[list_of_unneeded_vars] for speed/resource management???
exclude_list = [u'LU_INDEX', u'ZNU', u'ZNW', u'ZS', u'DZS', u'W', u'PH', u'PHB', u'T', u'HFX_FORCE', u'LH_FORCE', u'TSK_FORCE', u'HFX_FORCE_TEND', u'LH_FORCE_TEND', u'TSK_FORCE_TEND', u'MU', u'MUB', u'NEST_POS', u'P', u'PB', u'FNM', u'FNP', u'RDNW', u'RDN', u'DNW', u'DN', u'CFN', u'CFN1', u'P_HYD', u'Q2', u'TH2', u'U10', u'V10', u'RDX', u'RDY', u'RESM', u'ZETATOP', u'CF1', u'CF2', u'CF3', u'ITIMESTEP', u'XTIME', u'QRAIN', u'QICE', u'QSNOW', u'QGRAUP', u'QNICE', u'QNRAIN', u'LANDMASK', u'SMOIS', u'SH2O', u'SMCREL', u'SEAICE', u'XICEM', u'SFROFF', u'UDROFF', u'IVGTYP', u'ISLTYP', u'VEGFRA', u'GRDFLX', u'ACGRDFLX', u'SNOW', u'SNOWH', u'CANWAT', u'SST', u'SSTSK', u'LAI', u'TKE_PBL', u'EL_PBL', u'H_DIABATIC', u'MAPFAC_M', u'MAPFAC_U', u'MAPFAC_V', u'MAPFAC_MX', u'MAPFAC_MY', u'MAPFAC_UX', u'MAPFAC_UY', u'MAPFAC_VX', u'MF_VX_INV', u'MAPFAC_VY', u'F', u'E', u'SINALPHA', u'COSALPHA', u'HGT', u'TSK', u'P_TOP', u'T00', u'P00', u'TLP', u'TISO', u'MAX_MSTFX', u'MAX_MSTFY', u'SNOWNC', u'GRAUPELNC', u'HAILNC', u'STEPAVE_COUNT', u'CLDFRA', u'GSW', u'SWNORM', u'ACSWUPT', u'ACSWUPTC', u'ACSWDNT', u'ACSWDNTC', u'ACSWUPB', u'ACSWUPBC', u'ACSWDNB', u'ACSWDNBC', u'ACLWUPT', u'ACLWUPTC', u'ACLWDNT', u'ACLWDNTC', u'ACLWUPB', u'ACLWUPBC', u'ACLWDNB', u'ACLWDNBC', u'I_ACSWUPT', u'I_ACSWUPTC', u'I_ACSWDNT', u'I_ACSWDNTC', u'I_ACSWUPB', u'I_ACSWUPBC', u'I_ACSWDNB', u'I_ACSWDNBC', u'I_ACLWUPT', u'I_ACLWUPTC', u'I_ACLWDNT', u'I_ACLWDNTC', u'I_ACLWUPB', u'I_ACLWUPBC', u'I_ACLWDNB', u'I_ACLWDNBC', u'SWUPT', u'SWUPTC', u'SWDNT', u'SWDNTC', u'SWUPB', u'SWUPBC', u'SWDNB', u'SWDNBC', u'LWUPT', u'LWUPTC', u'LWDNT', u'LWDNTC', u'LWUPB', u'LWUPBC', u'LWDNB', u'LWDNBC', u'OLR', u'ALBEDO', u'ALBBCK', u'EMISS', u'NOAHRES', u'TMN', u'XLAND', u'UST', u'HFX', u'QFX', u'LH', u'ACHFX', u'ACLHF', u'SNOWC', u'SR', u'POTEVP', u'SNOPCX', u'SOILTB', u'SAVE_TOPO_FROM_REAL', u'AVG_FUEL_FRAC', u'UAH', u'VAH', u'SEED1', u'SEED2']

wrf = nc.MFDataset(needed, 'r', exclude=exclude_list)
t0 = time.time()

def getTimes(subset):
    wrfData = nc.MFDataset(needed, 'r', exclude=exclude_list)
    ts = pd.to_datetime(pd.Series((''.join(ca) for ca in wrfData.variables['Times'][subset[0]:subset[-1]+1]), index=subset), format='%Y-%m-%d_%H:%M:%S').to_frame(name='Time')
    wrfData.close()
    return ts

pool = mp.Pool()
chunks = getChunks(wrf.variables['Times'].shape[0], mp.cpu_count())
results = pool.map(getTimes, chunks)
pool.close()
pool.join()
#times = pd.to_datetime(pd.Series((''.join(ca) for ca in wrf.variables['Times'][0:len(tIndex)])), format='%Y-%m-%d_%H:%M:%S').to_frame(name='Time')
times = pd.concat(results)
t1 = time.time()
print 'Time to get times: {}'.format(t1-t0)
times = times.reset_index().set_index('Time')[args.startTime:args.endTime]
times = times.reset_index().set_index('index')
"""
print times
tmp = times
tmp['Time'] = tmp.set_index('Time').index.to_julian_date()
pd.set_option('display.float_format', lambda x: '{:.4f}'.format(x))
print tmp
os._exit(1)
"""

v_theta = ["T2", "QVAPOR","QCLOUD","PSFC","SWDOWN","GLW","PBLH","TSLB"]
v_theta_precip = ['RAINC', 'RAINSH', 'RAINNC']
v_u = ['U']
v_v = ['V']

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
    adhydroMeshGeometry = gpd.GeoSeries(adhydroMeshCenters, crs=proj)
    adhydroChannelCenters = [ Point(x, y) for x,y in zip(adhydroChannel['X'], adhydroChannel['Y']) ]
    adhydroChannelGeometry = gpd.GeoSeries(adhydroChannelCenters, crs=proj)
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
    adhydroMeshGeometry = gpd.GeoSeries(adhydroMeshVerticies, crs=proj)
    tm = time.time()
    #print "Time to create adhydroMeshSeries: {}".format(tm-t0)
    #print "Mem usage: {}".format(mem_usage(True))
    del adhydroMeshVerticies
    del adhydroMesh
    gc.collect()
    #print "Mem usage after collection: {}".format(mem_usage(True))
    adhydroChannelVerticies = [ LineString(l) for l in [zip(xs,ys) for xs, ys in zip(adhydroChannel['X'], adhydroChannel['Y']) ] ]
    adhydroChannelGeometry = gpd.GeoSeries(adhydroChannelVerticies, crs=proj)
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

def getWRFcoords(wrf, precipAdjust=False, dropCount=None, gridSize=4000):
    """
        Function which returns a GeoDataFrame containing the WRF Theta cells (center of cell)
        as gridSize sqaure polygons. Each cell center is projected to ADHydro coordinates and then
        a gridSize/2 meter square buffer around the center point is used to calculate the polygon.
        If adjusting precipitation bounds, dropCount can be used to specify how many cells to drop.
    """
    #The WRF grid cell LAT/LONGS.  These are variables with dimesnions (time, south_north, west_east)
    wrfCoords = {'LONG':wrf.variables['XLONG'][0], 'LAT':wrf.variables['XLAT'][0]}

    #The length of the first dimension is time (TIME), second is south_north (ROWS), third is west_east (COLS)
    #Create a list of point geometries for wrf coordinates
    #ATTENTION: THIS SLICE EXISTS BECAUSE SOME WRF DATA IS 0's ON THE BOUNDARY
    #It has been recommended to not use 5 cells on the boundary, but an intuitive look at co_dist_58
    #indicates that the data is good enough in all but the last cells on the boundary of the WRF
    #data set.
    #Additionally, it appears that only precip data in this last cell is lacking, the other vars
    #used by ADHydro seem to be OK, so really only want to drop this for precip data !!!FIXME!!!
    if precipAdjust:
        if dropCount == None or dropCount <= 0:
            wrfRows = wrfCoords['LONG'][1:-1,1:-1]
            wrfCols = wrfCoords['LAT'][1:-1,1:-1]
        else:
            drop = -1*dropCount
            wrfRows = wrfCoords['LONG'][1:drop,1:drop]
            wrfCols = wrfCoords['LAT'][1:drop,1:drop]
    else:
        wrfRows = wrfCoords['LONG'][:,:]
        wrfCols = wrfCoords['LAT'][:,:]

    wrfCells = {'geometry':[], 'row':[], 'col':[]}
    for i in range(len(wrfRows)):
        row = wrfRows[i]
        for j in range(len(wrfCols[i])):
           #print i, ", ", j
           wrfCells['geometry'].append (Point(wrfRows[i][j],wrfCols[i][j]))
           wrfCells['row'].append(i)
           wrfCells['col'].append(j)
    #Create a wrf data frame and project it to ADHydro coordinates
    wrfFrame = gpd.GeoDataFrame(wrfCells, crs={'init' :'epsg:4326'})
    wrfFrame = wrfFrame.to_crs(proj)
    wrfFrame['point'] = wrfFrame.geometry
    #Apply a gridSize/2 meter buffer to each cell center point, making the geometry a gridSize x gridSize KM Square (cap_style=3)
    buffSize = gridSize/2
    wrfFrame['poly'] = wrfFrame.geometry.apply(lambda p: p.buffer(buffSize, cap_style=3))#wrfFrame.geometry.apply(lambda p: box(p.x - 2000, p.y - 2000, p.x + 2000, p.y + 2000))
    wrfFrame.geometry = wrfFrame['poly']
    
    #Make QGIS spatial index #TODO/FIXME
    wrfIndex = QgsSpatialIndex() 
    def buildIndex(s):
        feature = QgsFeature(s.name)
        feature.setGeometry(QgsGeometry.fromWkt(s.geometry.to_wkt()))
        wrfIndex.insertFeature(feature)
    wrfFrame.apply(buildIndex, axis=1)
    #Now index the frame by the original row, col index
    wrfFrame.set_index(['row', 'col'], inplace=True)
    #print wrfFrame
    return wrfFrame, wrfIndex

def getWRFstaggeredXcoords(wrf, gridSize=4000):
    """
        Function which returns a GeoDataFrame containing the WRF staggerd cells (right of cell, X)
        as gridSize KM sqaure polygons. Each cell center is projected to ADHydro coordinates and then
        a gridSize/2 meter square buffer around the center point is used to calculate the polygon.
    """
    #The WRF grid cell LAT/LONGS.  These are variables with dimesnions (time, south_north, west_east)
    wrfCoords = {'LONG':wrf.variables['XLONG_U'][0], 'LAT':wrf.variables['XLAT_U'][0]}

    #The length of the first dimension is time (TIME), second is south_north (ROWS), third is west_east (COLS)
    #Create a list of point geometries for wrf coordinates
    wrfRows = wrfCoords['LONG']
    wrfCols = wrfCoords['LAT'][:,:]

    wrfCells = {'geometry':[], 'row':[], 'col':[]}
    for i in range(len(wrfRows)):
        row = wrfRows[i]
        for j in range(len(wrfCols[i])):
           #print i, ", ", j
           wrfCells['geometry'].append (Point(wrfRows[i][j],wrfCols[i][j]))
           wrfCells['row'].append(i)
           wrfCells['col'].append(j)

    #Create a wrf data frame and project it to ADHydro coordinates
    wrfFrame = gpd.GeoDataFrame(wrfCells, crs={'init' :'epsg:4326'})
    wrfFrame = wrfFrame.to_crs(proj)
    wrfFrame['point_U'] = wrfFrame.geometry
    #Apply a gridSize/2 meter buffer to each cell center point, making the geometry a gridSize KM Square (cap_style=3)
    buffSize = gridSize/2
    wrfFrame['poly_U'] = wrfFrame.geometry.apply(lambda p: p.buffer(buffSize, cap_style=3))#wrfFrame.geometry.apply(lambda p: box(p.x - 2000, p.y - 2000, p.x + 2000, p.y + 2000))
    wrfFrame.geometry = wrfFrame['poly_U']

    #Make QGIS spatial index #TODO/FIXME
    wrfIndex = QgsSpatialIndex() 
    def buildIndex(s):
        feature = QgsFeature(s.name)
        feature.setGeometry(QgsGeometry.fromWkt(s.geometry.to_wkt()))
        wrfIndex.insertFeature(feature)
    wrfFrame.apply(buildIndex, axis=1)
    #print wrfFrame
    #Now index the frame by the original row, col index
    wrfFrame.set_index(['row', 'col'], inplace=True)
    #print wrfFrame
    
    return wrfFrame, wrfIndex

def getWRFstaggeredYcoords(wrf, gridSize=4000):
    """
        Function which returns a GeoDataFrame containing the WRF staggerd cells (top of cell, Y)
        as gridSize KM sqaure polygons. Each cell center is projected to ADHydro coordinates and then
        a gridSize/2 meter square buffer around the center point is used to calculate the polygon.
    """
    #The WRF grid cell LAT/LONGS.  These are variables with dimesnions (time, south_north, west_east)
    wrfCoords = {'LONG':wrf.variables['XLONG_V'][0], 'LAT':wrf.variables['XLAT_V'][0]}

    #The length of the first dimension is time (TIME), second is south_north (ROWS), third is west_east (COLS)
    #Create a list of point geometries for wrf coordinates
    wrfRows = wrfCoords['LONG']
    wrfCols = wrfCoords['LAT'][:,:]
    #print wrfCols.shape
    #print wrfCoords

    wrfCells = {'geometry':[], 'row':[], 'col':[]}
    for i in range(len(wrfRows)):
        row = wrfRows[i]
        for j in range(len(wrfCols[i])):
           #print i, ", ", j
           wrfCells['geometry'].append (Point(wrfRows[i][j],wrfCols[i][j]))
           wrfCells['row'].append(i)
           wrfCells['col'].append(j)

    #Create a wrf data frame and project it to ADHydro coordinates
    wrfFrame = gpd.GeoDataFrame(wrfCells, crs={'init' :'epsg:4326'})
    wrfFrame = wrfFrame.to_crs(proj)
    wrfFrame['point_V'] = wrfFrame.geometry
    #Apply a 2000 meter buffer to each cell center point, making the geometry a 4KM Square (cap_style=3)
    buffSize = gridSize/2
    wrfFrame['poly_V'] = wrfFrame.geometry.apply(lambda p: p.buffer(buffSize, cap_style=3))#wrfFrame.geometry.apply(lambda p: box(p.x - 2000, p.y - 2000, p.x + 2000, p.y + 2000))
    wrfFrame.geometry = wrfFrame['poly_V']
    
    #Make QGIS spatial index #TODO/FIXME
    wrfIndex = QgsSpatialIndex() 
    def buildIndex(s):
        feature = QgsFeature(s.name)
        feature.setGeometry(QgsGeometry.fromWkt(s.geometry.to_wkt()))
        wrfIndex.insertFeature(feature)
    wrfFrame.apply(buildIndex, axis=1)
    #Now index the frame by the original row, col index
    wrfFrame.set_index(['row', 'col'], inplace=True)
    #print wrfFrame
    return wrfFrame, wrfIndex


def centerPointIntersect(s, wrfCoords, wrfIndex, bugFix):
    """
        This function assigns to each adHydro element the grid coordinates
        of the WRF cell that the center lies in.  In the event that the center
        point doesn't lie in a 4x4km cell, the nearest neighbor is used.
    """
    cells = {}
    for k, v in wrfData.iteritems():
        wrfCoords = v[0]
        wrfIndex  = v[1]
        #Get a list of all wrfCoord indexes that intersect the series geometry
        #hits = list( wrfCoords.sindex.intersection(s.geometry.bounds) )
        #Use QGIS spatial index for now TODO/FIXME
        hits =list(wrfIndex.intersects(QgsRectangle(*(s.geometry.bounds))))
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
                #nn = list(wrfCoords.sindex.nearest(s.geometry.bounds, 1))
                #TODO/FIXME QGIS spatial index
                nn = list(wrfIndex.nearestNeighbor(QgsPoint(cX, cY), 1))
                #Pick the first in case of tie...
                idx = nn[0]
                found = False
            else:
                #print "Multiple Intersects"
                for h in hits:
                    geom = wrfCoords.iloc[h].geometry
                    if geom.intersects(s.geometry):
                        idx = h
                        found = True
                        break

        cell = wrfCoords.iloc[idx]
        cells['WRFrow_{}'.format(k)] = cell.name[0]
        cells['WRFcol_{}'.format(k)] = cell.name[1]
        cells['Found_{}'.format(k)] = found
    return pd.Series(cells)
 
#Since we have to do several intersections over the SAME mesh/channel elements
#we speed things up by using a single function applied to these which does all intersections
#This reduces the need to iterate over large numbers of elements 3-4 times!!!
def polygonIntersect(s, wrfData, bugFix):
    """
        This function assigns to each adHydro element the grid coordinates
        of the WRF cell that contains the largest portion of the element.
        In the event that an element intersects no WRF cells, the nearest 
        neighbor is used.
    """
    cells = {}
    #Get a list of all wrfCoord indexes that intersect the series geometry
    #hits = list( wrfCoords.sindex.intersection(s.geometry.bounds) )
    #Use QGIS spatial index for now TODO/FIXME
    for k, v in wrfData.iteritems():
        wrfCoords = v[0]
        wrfIndex = v[1]
        hits =list(wrfIndex.intersects(QgsRectangle(*(s.geometry.bounds))))
        cX = s.geometry.centroid.x
        cY = s.geometry.centroid.y
        idx = None
        found = False 
        #If only one result, don't need more checking
        if len(hits) == 1:
            geom = wrfCoords.iloc[hits[0]].geometry
            #If hit really intersects, great
            if geom.intersects(s.geometry):
                idx = hits[0]
                if idx == None:
                    print "FS1"
                found = True
            else:
                #False positive, find nearest neighbor
                #nn = list(wrfCoords.sindex.nearest(s.geometry.bounds, 1))
                #TODO/FIXME QGIS spatial index
                nn = list(wrfIndex.nearestNeighbor(QgsPoint(cX, cY), 1))
                idx = nn[0]
                if idx == None:
                    print "FS2"
                found = False
        else:
            if len(hits) == 0:
                #print "No intersect, try NN"
                #This polygon doesn't explicity intersect, so try nearest neighbor
                #nn = list(wrfCoords.sindex.nearest(s.geometry.bounds, 1))
                #TODO/FIXME QGIS spatial index
                nn = list(wrfIndex.nearestNeighbor(QgsPoint(cX, cY), 1))
                #Pick the first in case of tie...
                idx = nn[0]
                if idx == None:
                    print "FS3"
                found = False
            else:
                #print "Multiple Intersects"
                #Eliminate false positives and get a list of WRFcells that the
                #element intersects
                exact_intersects = []
                for h in hits:
                    geom = wrfCoords.iloc[h].geometry
                    if geom.intersects(s.geometry):
                        exact_intersects.append((h,geom))
                if len(exact_intersects) == 0:
                    #False positive, find nearest neighbor
                    #nn = list(wrfCoords.sindex.nearest(s.geometry.bounds, 1))
                    #TODO/FIXME QGIS spatial index
                    nn = list(wrfIndex.nearestNeighbor(QgsPoint(cX, cY), 1))
                    idx = nn[0]
                    if idx == None:
                        print "FS4"
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
                            print "FS5"
                    else:
                        maxIntersectingArea = 0.0
                        for i, geom in exact_intersects:
                            area = geom.intersection(s.geometry).area
                            if area > maxIntersectingArea:
                                maxIntersectingArea = area
                                idx = i
                        if idx == None:
                            print "FS6" 
                    if idx == None:
                        #On occasion, an intersections occurs with 0 length,
                        #Still need to assign an idx. Pick nearest neighbor
                        nn = list(wrfIndex.nearestNeighbor(QgsPoint(cX, cY), 1))
                        idx = nn[0]
                     
        cell = wrfCoords.iloc[idx]
        cells['WRFrow_{}'.format(k)] = cell.name[0]
        cells['WRFcol_{}'.format(k)] = cell.name[1]
        cells['Found_{}'.format(k)] = found
    return pd.Series(cells)
     
def intersectWRFgrid(adHydroFrame, wrfData, how='center'):
    """
        This function assigns each ADHydro element in adHydroFrame to a WRF grid cell.
        This can be done by simply matching the center of the cell to a cell it lies in,
        or by more complexly finding the largest intersecting cell of the element.
     """  
    #t0 = time.time()
    #print list(coords.sindex.intersection(mesh.loc[0].geometry.bounds, objects='raw'))
    if adHydroFrame.geom_type.eq('Point').all():
        if how != 'center':
            print "Reverting to center point calculations since geometries are all points."
        
        adHydroFrame = pd.concat([adHydroFrame, adHydroFrame.apply(centerPointIntersect, args=(wrfData, None), axis=1)], axis=1)
    elif adHydroFrame.geom_type.eq('Polygon').all() or adHydroFrame.geom_type.eq('LineString').all():
        if how=='center':
            print "Changing geometry to centroids for center calculations"
            adHydroFrame['Poly'] = adHydroFrame.geometry
            adHydroFrame.geometry = adHydroFrame.centroid
            adHydroFrame = pd.concat([adHydroFrame, adHydroFrame.apply(centerPointIntersect, args=(wrfData, None), axis=1)], axis=1)
            adHydroFrame.geometry = adHydroFrame['Poly']
            del adHydroFrame['Poly']
        elif how=='poly':
            adHydroFrame = pd.concat([adHydroFrame, adHydroFrame.apply(polygonIntersect, args=(wrfData, None), axis=1)], axis=1)
        else:
            raise Exception("Unsupported intersection type {}. Supported options are [center, poly].".format(how))  
    else:
        print adHydroFrame.geom_type
        raise Exception("All adhydro geometries must be homogeneous.") 
    #t1 = time.time()
    #print "Mem after intersecting: {} GB".format(mem_usage(True))
    #print "Time to intersect: {}".format(t1-t0)
    
    return adHydroFrame

#TODO/FIXME make sure this now works with redesigned code
def plotWRFvar(domain, wrf, var):
    """
        Function for reading a subset of WRF data and plotting it
    """
    domain = domain.reset_index()
    #Get min/max indicies
    minRow = domain.row.min()
    maxRow = domain.row.max()
    minCol = domain.col.min()
    maxCol = domain.col.max()
    #Get a handle for the data, note this doesn't read until sliced
    data = wrf.variables[var]
    #Based on dimensions, slice a subset of data
    if len(data.shape) == 4:
        data = data[:,0,minRow:maxRow+1,minCol:maxCol+1]
    elif len(data.shape) == 3:
        data = data[:,minRow:maxRow+1,minCol:maxCol+1]
    else:
        raise Exception("Variable {} is not 2 or 3 dimensional.")
    #Take the mean of the read data 
    data = pd.np.mean(data, axis=0)
    #Put the data into the domain GeoDataFrame
    domain[var] = data[domain.row - minRow, domain.col - minCol]
    #Plot the domain based on the variable colum
    ax = domain.plot(column=var, cmap='cool')
    ax.set_title('{} ({})'.format(var, wrf.variables[var].units))
    fig = ax.get_figure()
    vmin = domain[var].min()
    vmax = domain[var].max()
    cax = fig.add_axes([0.7, 0.1, 0.03, 0.8])
    #Add a color bar
    sm = plt.cm.ScalarMappable(cmap='cool', norm=plt.Normalize(vmin=vmin, vmax=vmax))
    # fake up the array of the scalar mappable. Urgh...
    sm._A = []
    fig.colorbar(sm, cax=cax)


def getVars(adHydroMeshFrame, adHydroChannelFrame, wrfVars, columnSuffix, minWRFrow, maxWRFrow, minWRFcol, maxWRFcol, times):
    """
        Read multiple WRF variables and assign data to each ADHydro element(mesh and channel)
        for each time in times for each variable in wrfVars
    """
    t0 = time.time()
    rowName = 'WRFrow_{}'.format(columnSuffix)
    colName = 'WRFcol_{}'.format(columnSuffix)
    #Find the min and max WRF cells that we are going to work with
    minRow = min(adHydroMeshFrame[rowName].min(), adHydroChannelFrame[rowName].min())
    maxRow = max(adHydroMeshFrame[rowName].max(), adHydroChannelFrame[rowName].max())
    minCol = min(adHydroMeshFrame[colName].min(), adHydroChannelFrame[colName].min())
    maxCol = max(adHydroMeshFrame[colName].max(), adHydroChannelFrame[colName].max())
    #print minRow, maxRow
    #print minCol, maxCol
    def getData(s, varFrame, data, minRow, minCol, var, bugFix):
        try:
            varFrame[s.Time] = data[ s.name, varFrame[rowName] - minRow, varFrame[colName] - minCol ]
        except Exception, e:
            print "Error subsetting data"
            print varFrame[rowName], varFrame[colName]
            print data.shape
            print minRow, maxRow+1
            print minCol, maxCol+1
            raise(e)

    #Get the wrf variables of interest
    results = {}
    #FIXME with latest optimization, dataShared must be reset between each var read, so no use looping here
    #Fix all function calls to pass single var and rename getVar 
    for var in wrfVars:
        #Get all times, but only read data relative to the subset of the mesh we are working with
        data = dataShared[:, (minRow - minWRFrow):maxRow+1, (minCol - minWRFcol):maxCol+1]
        #print "Finished reading WRF {} data".format(var)
        #sys.stdout.flush()
         
        #Now for every time of interest, gather the variables information for that time for each ADHydro element
        meshVarFrame = adHydroMeshFrame[[rowName, colName]].copy(deep=True)
        times.apply(getData, axis=1, args=(meshVarFrame, data, minRow, minCol, var, None))
        #meshVarFrame.to_csv('enhanced_{}_mesh_frame_{}_{}.csv'.format(var, meshVarFrame.index[0], meshVarFrame.index[-1]))
        channelVarFrame = adHydroChannelFrame[[rowName, colName]].copy(deep=True)
        times.apply(getData, axis=1, args=(channelVarFrame, data, minRow, minCol, var, None))
        #Now longer need WRF info, clean it up
        meshVarFrame.drop([rowName, colName], axis=1, inplace=True)
        channelVarFrame.drop([rowName, colName], axis=1, inplace=True)
        #Get out of geopandas and to a regular pandas dataframe
        #Convert to float64 type here to minimize possible floating point arithmitic errors later.
        meshVarFrame = pd.DataFrame(meshVarFrame, dtype=pd.np.float64)
        channelVarFrame = pd.DataFrame(channelVarFrame, dtype=pd.np.float64)
        #Take the transpose so we are indexing by time for each row, element # is each column
        results[var] = meshVarFrame.T
        results[var+'_C'] = channelVarFrame.T
        #Uncomment for possible degubbing enhancements
        #results[var].to_msgpack('{}_{}.msg'.format(var, meshVarFrame.index[0]))
        #results[var+'_C'].to_msgpack('{}_C_{}.msg'.format(var, channelVarFrame.index[0]))
    t1 = time.time()
    #print "Time to gather variables {}: {}".format(wrfVars, t1-t0)
    #print "Mem after gathering variables {}: {}".format(wrfVars, mem_usage(True))
    return results

def getWRFgeometry(wrf, coordType='theta', precipAdjust=False, dropCount=None, gridSize=None):
    """
        Helper function for getting various WRF geometries.  coordType indicates which type of geometry
        to return, theta = center, u = staggeredX, v = staggeredY
    """
    types = ['theta', 'u', 'v']
    if coordType not in types:
        raise Exception('Unknown WRF coordinate type.  Supported options are {}'.format(types))
    if gridSize is None or gridSize < 2:
        raise Exception('Invalid gridSize {}. gridSize must be >= 2'.format(gridSize))
    t0 = time.time()
    #Get the wrf cell coordinates in a GEOdataframe with multiindex (row,column)
    if coordType == 'theta':
        coords, index = getWRFcoords(wrf, precipAdjust, dropCount, gridSize)
    elif coordType == 'u':
        coords, index = getWRFstaggeredXcoords(wrf, gridSize)
    else:
        coords, index = getWRFstaggeredYcoords(wrf, gridSize)
    t1 = time.time()
    #print "Mem after reading WRF {} Coords: {} GB".format(coordType, mem_usage(True))
    #print "Time to read {} coords: {}".format(coordType, t1-t0)
    return coords, index
     
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
    'SWDOWN':{'type':pd.np.float32, 'dims':('Time', 'Mesh_Elements'), 'units':'W s-2', 'fill':-99999},
  'SWDOWN_C':{'type':pd.np.float32, 'dims':('Time', 'Channel_Elements'), 'units':'W s-2', 'fill':-99999},
       'GLW':{'type':pd.np.float32, 'dims':('Time', 'Mesh_Elements'), 'units':'W s-2', 'fill':-99999},
     'GLW_C':{'type':pd.np.float32, 'dims':('Time', 'Channel_Elements'), 'units':'W s-2', 'fill':-99999},
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

#TODO/FIXME make sure this works with new code
def plotWRF(wrf, mesh):
    """
        Diagnostic/Visulization function which color plots
        WRF variables across the ADHydro domain for center point
        variables given in v_theta, and staggered X and Y variables
        given in v_u and v_v respectively.
    """
    rowName = 'WRFrow_theta'
    colName = 'WRFcol_theta'
    for v in v_theta:
        index = pd.Series(zip(mesh[rowName], mesh[colName])).unique()
        domain = tCoords.loc[index]
        plotWRFvar(domain, wrf, v)
    if args.adjustPrecip:
        rowName = 'WRFrow_theta_precip'
        colName = 'WRFcol_theta_precip'
    for v in v_theta_precip:
        index = pd.Series(zip(mesh[rowName], mesh[colName])).unique()
        domain = tCoords_precip.loc[index]
        plotWRFvar(domain, wrf, v)
    
    rowName = 'WRFrow_u'
    colName = 'WRFcol_u'
    for v in v_u:
        index = pd.Series(zip(mesh[rowName], mesh[colName])).unique()
        domain = uCoords.loc[index]
        plotWRFvar(domain, wrf, v)
    
    rowName = 'WRFrow_v'
    colName = 'WRFcol_v'
    for v in v_v:
        index = pd.Series(zip(mesh[rowName], mesh[colName])).unique()
        domain = vCoords.loc[index]
        plotWRFvar(domain, wrf, v)
    plt.show()


def intersectMP(subset, whichADHydro=None, whichWRF=None, how=None):
    """
        Wrapper for multiprocessing to run intersection on a subset of the mesh dataframe.
        Note: This may not produce exact same results as calling intersectWRFgrid on the entire mesh
        dataframe since the function looks at the passed dataframe geometry types and decides on 
        possible different ways to intersect the mesh and wrf coords.
    """
    if whichADHydro is None or whichADHydro not in ['mesh', 'channel']:
        raise Exception("Unknown option {}\nwhichADhydro must be one of ['mesh', 'channel']".format(whichADHydro))
    
    if whichWRF is None or any( [w not in ['tCoords', 'tCoords_precip', 'uCoords', 'vCoords'] for w in whichWRF]):
        raise Exception("Unknown option {}\nwhichWRF may only contain ['tCoords', 'tCoords_precip', 'uCoords', 'vCoords']".format(whichWRF))
    
    if how is None or how not in ['poly', 'center']:
        raise Exception("Unknown option {}\nhow must be one of ['poly', 'center']".format(how))

    #print "Processes {} processing {}  elements".format(os.getpid(), len(subset))
    adHydro_frame = None
    try:
        if whichADHydro == 'mesh':
            adHydro_frame = mesh.iloc[subset].copy()
        elif whichADHydro == 'channel':
            adHydro_frame = channel.iloc[subset].copy()
        wrfData = {}
        if 'tCoords' in whichWRF:
            wrfData['theta'] = (tCoords, wrfIndex_t)
        if 'tCoords_precip' in whichWRF:
            wrfData['theta_precip'] = (tCoords_precip, wrfIndex_t_precip)
        if 'uCoords' in whichWRF:
            wrfData['u'] = (uCoords, wrfIndex_u)
        if 'vCoords' in whichWRF:
            wrfData['v'] = (vCoords, wrfIndex_v)
        result = intersectWRFgrid(adHydro_frame, wrfData, how)
    except Exception, e:
        print "ERROR intersecting wrf grid"
        print e
        raise(e)
    #print "Process {} finished processing {} elements".format(os.getpid(), len(result))
    #print "Returning {}".format(result.shape)
    return result
    """
        !!!!!!!!!!!!!!!!!BROKE ON MT MORAN COMPUTE NODES using RTREE, so using qgis spatial index instead.
    """

#Let main processes read in the wrf data variable into a shared memory array
#This will prevent conflicting reads at different positions by each process
#at the disk level, then only have cache misses to slow us down.
def getVarMP(subset, var=None, minRow=None, maxRow=None, minCol=None, maxCol=None):
    """
        Wrapper for multiprocessing to gather wrf data in parallel on subsets of mesh and channel.
        subset should be a tuple of (meshSubset, channelSubset). We gather one variable at a time,
        as defined by var.
    """
    if var is None:
        raise(Exception("Argument var must not be None"))
    if minRow is None:
        raise(Exception("Argument minRow must not be None.  Pass the minimum row of the WRF domain for the current mesh"))
    if minCol is None:
        raise(Exception("Argument minCol must not be None.  Pass the minimum column of the of the WRF domain for the current mesh"))
    if maxRow is None:
        raise(Exception("Argument maxRow must not be None.  Pass the maximum row of the WRF domain for the current mesh"))
    if maxCol is None:
        raise(Exception("Argument maxCol must not be None.  Pass the maximum column of the of the WRF domain for the current mesh"))

    #Have to re-open wrf data on each processes so we don't get conflicting lseek() calls.
    #myWrf = nc.MFDataset(needed, 'r', exclude=exclude_list)
    #if var is None or var not in myWrf.variables.keys():
    #    if var != 'TPREC': #TPREC is the only var name we use that isn't a WRF variable name
    #        raise Exception("Uknown variable '{}' for dataset.".format(var))
    #    #v_theta = ["T2", "QVAPOR","QCLOUD","PSFC","SWDOWN","GLW","PBLH","TSLB"]
    if var in v_theta:
        #Subset the mesh and get variable info for var for this subset
        mesh = intersectMesh.iloc[subset[0]]
        channel = intersectChannel.iloc[subset[1]]
        #Map mesh/channel elements to appropriate WRF values
        result = getVars(mesh, channel, (var,), 'theta', minRow, maxRow, minCol, maxCol, times)
        if var == 'T2' or var == 'TSLB':
            try:
                #Temperature var, apply data transform to all values before adding to output
                meshDataArray[:,mesh.index[0]:mesh.index[-1]+1] = result[var].applymap(lambda x: x - 272.15).astype(pd.np.float32).values
                channelDataArray[:,channel.index[0]:channel.index[-1]+1] = result[var+'_C'].applymap(lambda x: x - 272.15).astype(pd.np.float32).values
            except Exception, e:
                print "WTF"
                raise(e)   
        elif var == 'GLW':
            """
                Similar to Hernan's R script, for each GLW on the first hour of the first day of each year,
                copy the next hours value since these are always 0.0
            """
            #Get a time based index
            fix = result['GLW']
            fix_c = result['GLW_C']
            fix.index = fix.index.to_datetime()
            #Time index is the same in both channel and mesh
            fix_c.index = fix.index
            #Find all first entries for each year
            firsts = fix[ (fix.index.month == 1) & (fix.index.day == 1) & (fix.index.hour == 0) ].index.values
            for f in firsts:
                #Copy the next hour to each of these
                fix.loc[f] = fix.loc[f+pd.np.timedelta64(1,'h')]
                fix_c.loc[f] = fix_c.loc[f+pd.np.timedelta64(1,'h')]
            meshDataArray[:,mesh.index[0]:mesh.index[-1]+1] = fix.astype(pd.np.float32).values
            channelDataArray[:,channel.index[0]:channel.index[-1]+1] = fix_c.astype(pd.np.float32).values
        else:
            #QVAPOR, QCLOUD, PSFC, SWDOWN, and PBLH require no transformations
            meshDataArray[:,mesh.index[0]:mesh.index[-1]+1] = result[var].astype(pd.np.float32).values
            channelDataArray[:,channel.index[0]:channel.index[-1]+1] = result[var+'_C'].astype(pd.np.float32).values
    elif var in v_u:
        #Do the same thing, only use the staggered X intersection data
        mesh = intersectMesh.iloc[subset[0]]
        channel = intersectChannel.iloc[subset[1]]
        result = getVars(mesh, channel, (var,), 'u', minRow, maxRow, minCol, maxCol, times)
        meshDataArray[:,mesh.index[0]:mesh.index[-1]+1] = result[var].astype(pd.np.float32).values
        channelDataArray[:,channel.index[0]:channel.index[-1]+1] = result[var+'_C'].astype(pd.np.float32).values
    elif var in v_v:
        #Use staggered Y intersection data
        mesh = intersectMesh.iloc[subset[0]]
        channel = intersectChannel.iloc[subset[1]]
        result = getVars(mesh, channel, (var,), 'v', minRow, maxRow, minCol, maxCol, times)
        meshDataArray[:,mesh.index[0]:mesh.index[-1]+1] = result[var].astype(pd.np.float32).values
        channelDataArray[:,channel.index[0]:channel.index[-1]+1] = result[var+'_C'].astype(pd.np.float32).values

    elif var == 'TPREC':
        global dataShared 
        #Calculate precipitation (TPREC) from WRF variables RAINC, RAINSH, and RAINNC
        mesh = intersectMesh.iloc[subset[0]]
        channel = intersectChannel.iloc[subset[1]]
        if args.adjustPrecip:
            suffix = 'theta_precip'
        else:
            suffix = 'theta'
        #Here is the somewhat dirty hack to get sharedData correct for reading multiple wrf variables before
        #computing output.
        dataShared = raincShared
        rainc = getVars(mesh, channel, ('RAINC',), suffix, minRow, maxRow, minCol, maxCol, times)
        dataShared = rainshShared
        rainsh= getVars(mesh, channel, ('RAINSH',), suffix, minRow, maxRow, minCol, maxCol, times)
        dataShared = rainncShared
        rainnc = getVars(mesh, channel, ('RAINNC',), suffix, minRow, maxRow, minCol, maxCol, times)
        #print "Calculating hourly precipitation"
        rc = rainc['RAINC']
        rc_c = rainc['RAINC_C']
        rsh = rainsh['RAINSH']
        rsh_c = rainsh['RAINSH_C']
        rnc = rainnc['RAINNC']
        rnc_c = rainnc['RAINNC_C']
        index = rc.index.to_datetime()
        index_c = rc_c.index.to_datetime()
        #Grab the time stamp for the first hour of each year
        #firsts = index [(index.month==1) & (index.day == 1) & (index.hour == 0)]
        #Also catch the first of each year if working on multiple years
        firsts = index [ ( (index.year == args.startTime.year) & (index.month == args.startTime.month) & (index.day == 1) & (index.hour==0) ) | ( (index.month == 1) & (index.day == 1) & (index.hour == 0) ) ]
        #FIXME if not reading from files, probably no required 
        rc.index = index
        rsh.index = index
        rnc.index = index
        rc_c.index = index_c
        rsh_c.index = index_c
        rnc_c.index = index_c
        #Sum the precipitation
        total = rc + rsh + rnc
        total_c = rc_c + rsh_c + rnc_c
        #Precip accumulates each year, so we group each year
        #And calculate hourly preceip for each year
        hourly = total.groupby(total.index.year).diff()
        hourly_c = total_c.groupby(total_c.index.year).diff()
        #hourly.to_csv('enhanced_hourly_2.csv')
        #print firsts
        #Add in the appropriate values for the first entry of each year
        #If we are in january, this is fine...otherwise we need to either look back a month
        #to get the appropriate diff, or just copy the second hour into the first (This is the current implementation).
        nexts = firsts + pd.np.timedelta64(1,'h')
        for f,n in zip(firsts, nexts):
            hourly.loc[f] = hourly.loc[n]
            hourly_c.loc[f] = hourly_c.loc[n]
        #THIS DOESN"T WORK RIGHT SINCE THE INDEXING RETURNS A COPY NOT A VIEW, SO ^^^ LOOP IS NEEDED
        #hourly.loc[firsts] = hourly.loc[nexts].copy()
        #hourly_c.loc[firsts] = hourly_c.loc[nexts].copy()
        #print hourly
        #convert mm/hr to m/s then put in output
        meshDataArray[:,mesh.index[0]:mesh.index[-1]+1] = hourly.applymap(lambda x: x/3600000).astype(pd.np.float32).values
        channelDataArray[:,channel.index[0]:channel.index[-1]+1] = hourly_c.applymap(lambda x: x/3600000).astype(pd.np.float32).values

def plotDomain(mesh, whichWRF=None):
    """
        Plot the WRF domain that covers the mesh.  Also plot any mesh element that were not
        directly mapped to a WRF cell (i.e. those that required a nearest neighbor search.)
    """
    if whichWRF is None or whichWRF not in ['theta', 'theta_precip', 'u', 'v']:
        raise Exception("Unknown option {}\nwhichWRF may only contain  ['theta', 'theta_precip', 'u', 'v']".format(whichWRF))
    rowName = 'WRFrow_{}'.format(whichWRF)
    colName = 'WRFcol_{}'.format(whichWRF)
    wrfCoords = None
    if whichWRF == 'theta':
        wrfCoords = tCoords
    elif whichWRF == 'theta_precip':
        wrfCoords = tCoords_precip
    elif wichWRF == 'u':
        wrfCoords = uCoords
    elif whichWRF == 'v':
        wrfCoords = vCoords

    index = pd.Series(zip(mesh[rowName], mesh[colName])).unique()
    ax = wrfCoords.loc[index].plot()
    mesh[mesh['Found_{}'.format(whichWRF)] == False].plot(ax=ax)
    #plt.show()

def init_shared_arrays(meshShared, channelShared, wrfData):
    """
        Subprocess initilizer to set up shared memory arrays.
    """
    global meshDataArray
    global channelDataArray
    global dataShared
    meshDataArray = meshShared
    channelDataArray = channelShared
    dataShared = wrfData

def init_shared_precip_arrays(meshShared, channelShared, precipArrays):
    """
        Subprocess initilizer to set up shared memory arrays.
    """
    global meshDataArray
    global channelDataArray
    global dataShared
    global raincShared
    global rainncShared
    global rainshShared
    meshDataArray = meshShared
    channelDataArray = channelShared
    raincShared = precipArrays['RAINC']
    rainncShared = precipArrays['RAINNC']
    rainshShared = precipArrays['RAINSH']

adhydro_geom = nc.Dataset(ADHydroGeometryFile, 'r')
channelDims = adhydro_geom.variables['channelVertexX'].shape
meshDims = adhydro_geom.variables['meshVertexX'].shape
adhydro_geom.close()

#mesh, channel = getADHydroCenterGeometry(ADHydroGeometryFile)
#mesh, channel = getADHydroPolyGeometry(ADHydroGeometryFile)
mesh, channel = (None, None)
output = initOutput(outFileName, pd.Series(np.arange(meshDims[1])), pd.Series(np.arange(channelDims[1])), times)
output = nc.Dataset(outFileName, 'a')

tCoords, wrfIndex_t = getWRFgeometry(wrf, 'theta', gridSize=gridSize)

if args.adjustPrecip:
    tCoords_precip, wrfIndex_t_precip = getWRFgeometry(wrf, 'theta', True, dropCount=args.adjustPrecip, gridSize=gridSize)
else:
    tCoords_precip = tCoords
    wrfIndex_t = wrfIndex_t

uCoords, wrfIndex_u = getWRFgeometry(wrf, 'u', gridSize=gridSize)
vCoords, wrfIndex_v = getWRFgeometry(wrf, 'v', gridSize=gridSize)

#Globals to hold data in later TODO/FIXME if we clean these up in getVars, copy-on-write is useless
intersectMesh = None
intersectChannel = None
initTime1 = time.time()

print "Time to initialize: {}".format(initTime1 - initTime0)
if __name__ == '__main__':
    print "Initial memory usage: {} GB".format(mem_usage(True))
    start_time = time.time()
    """
        The large mesh (Upper Colorado River) is too large.
        Channel geometries especially use a lot of memory.
        Processing the entire mesh in one shot is impossible in this case.
        So we will processing subsets of the mesh at a time 
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
        mesh, channel = getADHydroPolyGeometry(ADHydroGeometryFile, mSubset, cSubset)
        mesh_chunks = getChunks(len(mesh), num_cores)
        channel_chunks = getChunks(len(channel), num_cores)
        print "Processing {} mesh elements, {} channel elements on {} cores.".format(len(mesh), len(channel), num_cores)
        print "Intersecting mesh/channel elements with WRF grid..."
        sys.stdout.flush()
        #Mesh and Channel Theta coordinae intersections (tCoords)
        #Mesh and Channel Staggered X coordinate intersections (uCoords)
        #Mesh and Channel Staggered Y coordinate intersections (vCoords)
        wrfCoordsNeeded = ['tCoords', 'uCoords', 'vCoords']
        #TODO For meshes internal to the domain, this is not needed.  Can save some processing by trying to predetermine if this is nessicary or not.
        #For now, leave it up to the user to decide
        if args.adjustPrecip:
            wrfCoordsNeeded.append('tCoords_precip')
        #Split the processing up across each core as evenly as possible
        t0 = time.time()
        pool = mp.Pool()
        #Mesh and Channel Intersections
        results = pool.map(partial(intersectMP, whichADHydro='mesh', whichWRF=wrfCoordsNeeded, how='poly'), mesh_chunks)
        intersectMesh = pd.concat(results)
        results = pool.map(partial(intersectMP, whichADHydro='channel', whichWRF=wrfCoordsNeeded, how='poly'), channel_chunks)
        intersectChannel = pd.concat(results)
        t1 = time.time()
        #gather some information about the mesh/channel domain for reading WRF
        minWRFrowTheta = min(intersectMesh['WRFrow_theta'].min(), intersectChannel['WRFrow_theta'].min())
        maxWRFrowTheta = max(intersectMesh['WRFrow_theta'].max(), intersectChannel['WRFrow_theta'].max())
        minWRFrowU = min(intersectMesh['WRFrow_u'].min(), intersectChannel['WRFrow_u'].min())
        maxWRFrowU = max(intersectMesh['WRFrow_u'].max(), intersectChannel['WRFrow_u'].max())
        minWRFrowV = min(intersectMesh['WRFrow_v'].min(), intersectChannel['WRFrow_v'].min())
        maxWRFrowV = max(intersectMesh['WRFrow_v'].max(), intersectChannel['WRFrow_v'].max())
        
        minWRFcolTheta = min(intersectMesh['WRFcol_theta'].min(), intersectChannel['WRFcol_theta'].min())
        maxWRFcolTheta = max(intersectMesh['WRFcol_theta'].max(), intersectChannel['WRFcol_theta'].max())
        minWRFcolU = min(intersectMesh['WRFcol_u'].min(), intersectChannel['WRFcol_u'].min())
        maxWRFcolU = max(intersectMesh['WRFcol_u'].max(), intersectChannel['WRFcol_u'].max())
        minWRFcolV = min(intersectMesh['WRFcol_v'].min(), intersectChannel['WRFcol_v'].min())
        maxWRFcolV = max(intersectMesh['WRFcol_v'].max(), intersectChannel['WRFcol_v'].max())

        if args.adjustPrecip:
            minWRFrowTheta_precip = min(intersectMesh['WRFrow_theta_precip'].min(), intersectChannel['WRFrow_theta_precip'].min())
            maxWRFrowTheta_precip = max(intersectMesh['WRFrow_theta_precip'].max(), intersectChannel['WRFrow_theta_precip'].max())
            minWRFcolTheta_precip = min(intersectMesh['WRFcol_theta_precip'].min(), intersectChannel['WRFcol_theta_precip'].min())
            maxWRFcolTheta_precip = max(intersectMesh['WRFcol_theta_precip'].max(), intersectChannel['WRFcol_theta_precip'].max())
        else:
            minWRFrowTheta_precip = minWRFrowTheta
            maxWRFrowTheta_precip = maxWRFrowTheta
            minWRFcolTheta_precip = minWRFcolTheta
            maxWRFcolTheta_precip = maxWRFcolTheta
        
        #If you want to plot the domain or the average wrf var color maps, do it before the follow del commands which remove some cell data.
        #Plots each wrf variables average as a color map over the mesh domain
        #plotWRF(wrf, intersectMesh)
        #Plots the mesh domain and WRF grids, showing all mesh elements not intersecting a WRF grid cell
        #plotDomain(intersectMesh, 'theta')
        #plt.show()
        print '{} mesh elements not mapped directly to a theta cell'.format( len( intersectMesh[ intersectMesh['Found_theta'] == False]) )
        print '{} channel elements not mapped directly to a theta cell'.format( len( intersectChannel[ intersectChannel['Found_theta'] == False]) )
        toDrop = ['Found_theta', 'Found_u', 'Found_v', 'geometry']
        if args.adjustPrecip:
            toDrop.append('Found_theta_precip') 
        intersectMesh.drop(toDrop, axis=1, inplace=True)
        intersectChannel.drop(toDrop, axis=1, inplace=True)
        print "Time to intersect {} : {}".format(wrfCoordsNeeded, t1-t0)
        print "Mem after intersecting theta: {} GB".format(mem_usage(True))
        pool.close()
        pool.join()
        #wrf.close()
        #Close the old pool and re create so new processes get proper global data.  Also helps mem usage since
        #now new forked processes will get copy-on-write (on must systems, i.e. *nix) memory pages for these large data sets
        #lock = mp.Lock()
        #Create a shared memory array for each process to put data into, then the master
        #process can write this data to the output file
        #lock=False since by design all processes are writing independent segments of the 
        #output array
        meshBase = mp.Array(ctypes.c_double, len(times)*len(mSubset), lock=False)
        meshShared = np.frombuffer(meshBase).reshape((len(times), len(mSubset)))
     
        channelBase = mp.Array(ctypes.c_double, len(times)*len(cSubset), lock=False)
        channelShared = np.ctypeslib.as_array(channelBase).reshape((len(times), len(cSubset))) 
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
        for var in v_theta:
            print "Reading var {}".format(var)
            t0 = time.time()
            if len(wrf.variables[var].shape) == 4:
                #print '{} has 4 dimensions'.format(var)
                #Subset the data based on the domian.  For 4D, layered data, pick the first layer
                data = wrf.variables[var][:,0,minWRFrowTheta:maxWRFrowTheta+1, minWRFcolTheta:maxWRFcolTheta+1]
            elif len(wrf.variables[var].shape) == 3:
                #Subset the data based on domain
                data = wrf.variables[var][:,minWRFrowTheta:maxWRFrowTheta+1, minWRFcolTheta:maxWRFcolTheta+1]
            else:
                raise Exception("Variable {} is not 3 or 4 dimensional.".format(var))
            #Create the readonly shared mem array for the variable data
            dataBase = mp.Array(ctypes.c_double, data.size, lock=False)
            dataShared = np.ctypeslib.as_array(dataBase).reshape(data.shape)
            np.copyto(dataShared, data)
            del data
            readTime = time.time()
            #init the processes with the data arrays
            pool = mp.Pool(initializer=init_shared_arrays,initargs=(meshShared, channelShared, dataShared))

            print "Time to read all {}: {}".format(var, readTime - t0)
            #To be safe, fill the arrays with fill_value first
            meshShared.fill(-99999)
            channelShared.fill(-99999)
            results = pool.map(partial(getVarMP, var=var, minRow=minWRFrowTheta, maxRow=maxWRFrowTheta, minCol=minWRFcolTheta, maxCol=maxWRFcolTheta), chunks )
            pool.close()
            pool.join()
            output.variables[var][:,mSubset[0]:mSubset[-1]+1] = meshShared[:]
            output.variables[var+'_C'][:,cSubset[0]:cSubset[-1]+1] = channelShared[:]
            output.sync()
            t1 = time.time()
            print "Time to read/write {}: {}".format(var, t1-t0)
        
        for var in v_u:
            print "Reading var {}".format(var)
            t0 = time.time()
            if len(wrf.variables[var].shape) == 4:
                #print '{} has 4 dimensions'.format(var)
                #Subset the data based on the domian.  For 4D, layered data, pick the first layer
                data = wrf.variables[var][:,0,minWRFrowU:maxWRFrowU+1, minWRFcolU:maxWRFcolU+1]
            elif len(wrf.variables[var].shape) == 3:
                #Subset the data based on domain
                data = wrf.variables[var][:,minWRFrowTheta:maxWRFrowU+1, minWRFcolU:maxWRFcolU+1]
            else:
                raise Exception("Variable {} is not 3 or 4 dimensional.".format(var))
            #Create the readonly shared mem array for the variable data
            dataBase = mp.Array(ctypes.c_double, data.size, lock=False)
            dataShared = np.ctypeslib.as_array(dataBase).reshape(data.shape)
            np.copyto(dataShared, data)
            del data
            readTime = time.time()
            #init the processes with the data arrays
            pool = mp.Pool(initializer=init_shared_arrays,initargs=(meshShared, channelShared, dataShared))

            print "Time to read all {}: {}".format(var, readTime - t0)
           
            #To be safe, fill the arrays with fill_value first
            meshShared.fill(-99999)
            channelShared.fill(-99999)
            results = pool.map(partial(getVarMP, var=var, minRow=minWRFrowU, maxRow=maxWRFrowU, minCol=minWRFcolU, maxCol=maxWRFcolU), chunks )
            pool.close()
            pool.join()
            output.variables[var][:,mSubset[0]:mSubset[-1]+1] = meshShared[:]
            output.variables[var+'_C'][:,cSubset[0]:cSubset[-1]+1] = channelShared[:]
            output.sync()
            t1 = time.time()
            print "Time to read/write {}: {}".format(var, t1-t0)
        
        for var in v_v:
            print "Reading var {}".format(var)
            t0 = time.time()
            if len(wrf.variables[var].shape) == 4:
                #print '{} has 4 dimensions'.format(var)
                #Subset the data based on the domian.  For 4D, layered data, pick the first layer
                data = wrf.variables[var][:,0,minWRFrowV:maxWRFrowV+1, minWRFcolV:maxWRFcolV+1]
            elif len(wrf.variables[var].shape) == 3:
                #Subset the data based on domain
                data = wrf.variables[var][:,minWRFrowV:maxWRFrowV+1, minWRFcolV:maxWRFcolV+1]
            else:
                raise Exception("Variable {} is not 3 or 4 dimensional.".format(var))
            #Create the readonly shared mem array for the variable data
            dataBase = mp.Array(ctypes.c_double, data.size, lock=False)
            dataShared = np.ctypeslib.as_array(dataBase).reshape(data.shape)
            np.copyto(dataShared, data)
            del data
            readTime = time.time()
            #init the processes with the data arrays
            pool = mp.Pool(initializer=init_shared_arrays,initargs=(meshShared, channelShared, dataShared))

            print "Time to read all {}: {}".format(var, readTime - t0)
           
            #To be safe, fill the arrays with fill_value first
            meshShared.fill(-99999)
            channelShared.fill(-99999)
            results = pool.map(partial(getVarMP, var=var, minRow=minWRFrowV, maxRow=maxWRFrowV, minCol=minWRFcolV, maxCol=maxWRFcolV), chunks )
            pool.close()
            pool.join()
            output.variables[var][:,mSubset[0]:mSubset[-1]+1] = meshShared[:]
            output.variables[var+'_C'][:,cSubset[0]:cSubset[-1]+1] = channelShared[:]
            output.sync()
            t1 = time.time()
            print "Time to read/write {}: {}".format(var, t1-t0)
        
        #Handle 'TPREC'
        #Really have to hack this to save memory using shared mem arrays
        t0 = time.time()
        sharedPrecipBase = {}
        sharedPrecip = {}
     
        print "Reading vars {}".format(v_theta_precip)
        for var in v_theta_precip:
            if len(wrf.variables[var].shape) == 4:
                #print '{} has 4 dimensions'.format(var)
                #Subset the data based on the domian.  For 4D, layered data, pick the first layer
                data = wrf.variables[var][:,0,minWRFrowTheta_precip:maxWRFrowTheta_precip+1, minWRFcolTheta_precip:maxWRFcolTheta_precip+1]
            elif len(wrf.variables[var].shape) == 3:
                #Subset the data based on domain
                data = wrf.variables[var][:,minWRFrowTheta_precip:maxWRFrowTheta_precip+1, minWRFcolTheta_precip:maxWRFcolTheta_precip+1]
            else:
                raise Exception("Variable {} is not 3 or 4 dimensional.".format(var))
            sharedPrecipBase[var] = mp.Array(ctypes.c_double, data.size, lock=False)
            sharedPrecip[var] = np.ctypeslib.as_array(sharedPrecipBase[var]).reshape(data.shape)
            np.copyto(sharedPrecip[var], data)
            del data
        #init the processes with the data arrays
        pool = mp.Pool(initializer=init_shared_precip_arrays,initargs=(meshShared, channelShared, sharedPrecip)) 
        readTime = time.time()
        print "Time to read all {}: {}".format(v_theta_precip, readTime - t0)
        
        #To be safe, fill the arrays with fill_value first
        meshShared.fill(-99999)
        channelShared.fill(-99999)
        #Create a readable shared mem array for the WRF data and read in the data before forking
        results = pool.map(partial(getVarMP, var='TPREC', minRow=minWRFrowTheta_precip, maxRow=maxWRFrowTheta_precip, minCol=minWRFcolTheta_precip, maxCol=maxWRFcolTheta_precip), chunks )
        output.variables['TPREC'][:,mSubset[0]:mSubset[-1]+1] = meshShared[:]
        output.variables['TPREC_C'][:,cSubset[0]:cSubset[-1]+1] = channelShared[:]
        output.sync()
        t1 = time.time()
        print "Time to read/write {}: {}".format(var, t1-t0)
      
        pool.close()
        pool.join()
    output.close()
    end_time = time.time()
    print "Total time: {}".format(end_time-start_time)
    print "Final mem usage: {}".format(mem_usage(True))
    
    """
    #Taken from GeoPandas base on creating spatial index for GeoDataFrame.  Not needed now but good reference.
    stream = ((i, item.bounds, idx) for i, (idx, item) in enumerate(coords.geometry.iteritems()) if pd.notnull(item) and not item.is_empty and len(item.bounds) == 4)
    geo = coords.geometry.loc[(0,0)]
    print geo
    print geo.bounds
    print stream.next()
    idx = rtree.index.Index(stream)
    print idx
    """
