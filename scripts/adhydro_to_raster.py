#!/usr/bin/env python

import netCDF4 as nc
import geopandas as gpd
import pandas as pd
import os
import gc
import sys
import psutil
import time
import argparse
import multiprocessing as mp
from collections import defaultdict
from shapely import speedups
speedups.enable()
from shapely.geometry import Point, Polygon, box
from shapely.ops import transform
from qgis.core import QgsSpatialIndex, QgsFeature, QgsGeometry, QgsRectangle, QgsPoint, QgsFeatureIterator
from functools import partial
from math import ceil
import pyproj
from osgeo import gdal, osr
gdal.UseExceptions()


def mem_usage(GB=False):
    """
        Simple function for estimating processes current memory usage
    """
    process = psutil.Process(os.getpid())
    mem = process.memory_info()[0]/float(2**20)
    if GB:
        mem = mem*0.00104858
    return mem

def checkPositiveInput(i):
    """
      Utility for checking a postive input value passed as i in command line options
    """
    value = int(i)
    if value < 0:
        raise argparse.ArgumentTypeError('{} is not a valid input, must be >= 0'.format(value))
    return value

"""
  Set up command line args and parser
"""
parser = argparse.ArgumentParser(description="Create a rasterized inundation map based on original DEM cells.")
parser.add_argument('ADHydroGeometry', help='The ADHydro geometry.nc file containing the mesh geometry.')
parser.add_argument('ADHydroOutput', help='The ADHydro output netCDF file containing <variable>.')
parser.add_argument('DEM', help='The original DEM file used to create the mesh in geometry.nc.')
parser.add_argument('-v', '--variable', type=str, help='The adhydro variable to transform to raster form, must a variable in ADHydroOutput.', required=True)
parser.add_argument('-t', '--time', help='Time instance in ADHydroOutput to generate the raster for.  Defaults to 0.', type=checkPositiveInput)
parser.add_argument('-n', '--numSubsets', type=checkPositiveInput, help='Split the processing into N subsets of the raster grid.  Useful on large meshes to avoid running out of memory.')
parser.add_argument('-m', '--meridian', help='Centeral meridian to use for ADHydro Sinusoidal projection', required=True, type=float)
parser.add_argument('-f', '--outputFileName', help='Optional output file name.  By default, the filename will be gridded_output.tif .')
args = parser.parse_args()

"""
  Verify inputs exist as files
"""
if not os.path.isfile(args.ADHydroGeometry):
    parser.error("Geometry file '{}' does not exist".format(args.ADHydroGeometry))

if not os.path.isfile(args.ADHydroOutput):
    parser.error("Data file '{}' does not exist".format(args.ADHydroOutput))

if not os.path.isfile(args.DEM):
    parser.error("DEM file '{}' does not exist".format(args.DEM))

#Open the adhydro output file and verify the variable to process
adhydro_output = nc.Dataset(args.ADHydroOutput, 'r')
if args.variable not in adhydro_output.variables.keys():
    parser.error("Variable {} not found in {}".format(args.variable, args.ADHydroOutput))

#Otherwise, get the output dataset if time is in proper range
adhydro_time = 0
if args.time:
  if args.time < len(adhydro_output.variables[args.variable]):
    adhydro_time = args.time
  else:
    print("WARNING: time arguement out of range for variable {} in {}.  Setting time to last instance.".format(args.variable, args.ADHydroOutput))
    adhydro_time = -1
#Read the dataset into memory
output_variable = adhydro_output.variables[args.variable][adhydro_time][:]

#Set the output file name
output_file = 'gridded_output.tif'
if args.outputFileName:
  output_file = args.outputFileName

#set ADHydro projection string based on passed meridian
adhydro_projection = '+proj=sinu +lon_0={} +x_0=20000000 +y_0=0 +datum=WGS84 +units=m +no_defs'.format(args.meridian)

#Open up raster as global variable
raster = gdal.Open(args.DEM)
raster_crs = osr.SpatialReference()
raster_crs.ImportFromWkt(raster.GetProjection())
#NOTE Can modify the bounds if a differnt box is desired, or set up for the entire mesh.  TODO make bounding box an arg to command line
#Use the bounds of the mesh geometry to limit the raster processing
#mesh_minX, mesh_minY, mesh_maxX, mesh_maxY = mesh.total_bounds
#bounds = gpd.GeoSeries( [ Point(mesh_minX, mesh_minY), Point(mesh_maxX, mesh_maxY) ], crs=adhydro_projection)
#Pre defined bounding box
#in lat/long coords
mesh_minX, mesh_maxX, mesh_minY, mesh_maxY = (-95.8939, -95.2916, 29.9744, 29.5541)
bounds = gpd.GeoSeries( [ Point(mesh_minX, mesh_minY), Point(mesh_maxX, mesh_maxY) ], crs=raster_crs.ExportToProj4() )


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


def getADHydroMeshPolyGeometry(geometryFile, bounding_box = None):
    """
        Function which returns a GeoDataFrame containing the ADHydro geometries as polygons
        as well as a QgsSpatialIndex that can be used to quicly search the mesh geometrically
        Optionally filters mesh elements by bounding box if provided.
    """
    t0 = time.time()
    #Open the ADHydro geometry data set
    adhydro_geom = nc.Dataset(geometryFile, 'r')
    #Extract mesh vertex coordinates in ADHydro Coordinates
    #if meshSubset is None:
    adhydroMesh = {'X':adhydro_geom.variables['meshVertexX'][0], 'Y':adhydro_geom.variables['meshVertexY'][0]}
    #else: 
    #    adhydroMesh = {'X':adhydro_geom.variables['meshVertexX'][0][meshSubset[0]:meshSubset[-1]+1], 'Y':adhydro_geom.variables['meshVertexY'][0][meshSubset[0]:meshSubset[-1]+1]}

    #Create polygon geometries and geo series for adhydro mesh elements
    adhydroMeshVerticies =[Polygon(p) for p in [ zip(xs,ys) for xs, ys in zip(adhydroMesh['X'], adhydroMesh['Y']) ] ]
    adhydroMeshGeometry = gpd.GeoSeries(adhydroMeshVerticies, crs=adhydro_projection)
    if bounding_box:
      mask = adhydroMeshGeometry.intersects( bounding_box )
      adhydroMeshGeometry = adhydroMeshGeometry[mask]
    tm = time.time()
    #print "Time to create adhydroMeshSeries: {}".format(tm-t0)
    #print "Mem usage: {}".format(mem_usage(True))
    del adhydroMeshVerticies
    del adhydroMesh
    gc.collect()
    #print "Mem usage after collection: {}".format(mem_usage(True))
    #Create dataframe to contain further attributes
    meshFrame = gpd.GeoDataFrame([], geometry=adhydroMeshGeometry)
    
    #Create a QGSI spatial index since RTREE hangs on moran??? TODO/FIXME
    meshIndex = QgsSpatialIndex()
    for i, triangle in meshFrame.itertuples():
        feature = QgsFeature()
        feature.setFeatureId(i)
        feature.setGeometry(QgsGeometry.fromWkt(triangle.to_wkt()))
        meshIndex.insertFeature(feature) 
    t1 = time.time()
    print "Mem after reading ADHydro mesh polygon geometries: {} GB".format(mem_usage(True))
    print "Time to read ADHydro mesh geometry: {}".format(t1-t0)
    adhydro_geom.close()
    return meshFrame, meshIndex

def pixels_to_points(geoTransform, source_crs, target_crs, xMin = 0, xMax = 1, yMin = 0, yMax = 1 ):
  """
    Function that takes a range of x and y values (Pixel Values)
    and returns a series of points based on the provided geoTransform
    Note:  This can generate multiple rows worth, but they will be
           returned in a linear series!

  """
  upperLeftX = geoTransform[0]
  upperLeftY = geoTransform[3]
  #xSkew = geoTransform[2]
  #ySkew = geoTransform[4]
  xResolution = geoTransform[1]
  yResolution = geoTransform[5]
  projection = partial(
       pyproj.transform,
       pyproj.Proj(source_crs),
       pyproj.Proj(target_crs))
  xs = pd.np.arange(xMin, xMax+1)
  ys = pd.np.arange(yMin, yMax+1)
  xs = xs*xResolution + upperLeftX + (xResolution / 2.0)
  ys = ys*yResolution + upperLeftY + (yResolution / 2.0)
  xxs, yys = projection( *(pd.np.meshgrid(xs, ys)) )
  """
  print xxs.shape
  print xxs
  print yys.shape
  print yys
  """
  points = [ [ (x,y)for x in xxs[0] ] for y in yys[:,0] ] 
  """
  print len(points)
  print len(points[0])
  print points[0][0]
  """
  return points

def points_to_pixels(geoTransform, points):
  upperLeftX = geoTransform[0]
  upperLeftY = geoTransform[3]
  xSkew = geoTransform[2]
  ySkew = geoTransform[4]
  xResolution = geoTransform[1]
  yResolution = geoTransform[5]
  indicies = [ ]
  #print upperLeftX, upperLeftY
  #print xResolution, yResolution
  for point in points:
    col = int( (point.x - upperLeftX ) / xResolution )
    row = int( (point.y - upperLeftY ) / yResolution )
    indicies.append( (col, row) )
  return indicies

def data_at_points(points):
  result = pd.np.full(len(points), pd.np.nan)
  for i, point in enumerate(points):
    search_start = time.time()
    hits = list(meshIndex.intersects(QgsRectangle(point[0],point[1],point[0],point[1])))
    for h in hits:
      geom = mesh.loc[h].geometry
      if geom.contains(Point(point[0], point[1])):
        result[i] = output_variable[h]
        break
  return result

def data_at_point(point):
  hits = list(meshIndex.intersects(QgsRectangle(*(point.bounds))))

  for h in hits:
    geom = mesh.loc[h].geometry
    if geom.contains(point):
      return output_variable[h]
  return pd.np.nan


def process_rows(chunks):
  try:
    setup_start = time.time()
    my_subset = row_points[chunks[0]:chunks[-1]]
    offset =chunks[0]
    output_dataset = gdal.Open(output_file, gdal.GA_Update)
    band = output_dataset.GetRasterBand(1)
    setup_end = time.time()
    dataTimeStart = time.time()
    dataTimes = []
    writeTimes = []
    for i, row in enumerate(my_subset):
        #Use these geometries to create a dataframe
        data_start = time.time()
        data = pd.DataFrame()
        #Add depth info to dataframe
        data['output']  = data_at_points(row)# pixelGeom.geometry.apply(data_at_point)
        #print data['depth'].dropna()
        data['output'].fillna(-9999, inplace=True)
        write_data = pd.np.expand_dims(data['output'].values, axis=0)
        dataTimes.append(time.time() - data_start)
        #print write_data.shape
        write_start = time.time()
        lock.acquire()
        fs_time = time.time()
        band.WriteArray(write_data, 0, offset+i)
        lock.release()
        writeTimes.append(time.time() - write_start)
        fs_time2 = time.time()
    dataTimeEnd = time.time()
    flush_time = time.time()
    lock.acquire()
    band.FlushCache()
    del output_dataset
    flush_time_end = time.time()
    print "Time to process {} rows: {}. dataTime {}, writeTime {}, flushTime {}".format(len(chunks), dataTimeEnd - dataTimeStart, sum(dataTimes), sum(writeTimes), flush_time - flush_time_end)
    lock.release()
  except Exception, e:
    print e
    raise(e)

def mp_init(init_lock):
  global lock
  lock = init_lock

"""
BEGIN processing beyond user control!

"""
#Figure out which indicies these points correspond to in the DEM
raster_bounds = points_to_pixels( raster.GetGeoTransform(), bounds )
raster_xMin, raster_yMin = raster_bounds[0]
raster_xMax, raster_yMax = raster_bounds[1]
col_size = raster_xMax - raster_xMin + 1 #For 0 based index
row_size = raster_yMax - raster_yMin + 1 #For 0 based index
mesh_bounds = bounds.to_crs(adhydro_projection)
bounding_box = box(mesh_bounds[0].x, mesh_bounds[0].y, mesh_bounds[1].x, mesh_bounds[1].y)

#Get the mesh into geometries for operating on, including a spatial index
mesh, meshIndex = getADHydroMeshPolyGeometry(args.ADHydroGeometry, bounding_box)
#print mesh
#Can save mesh shape file for debuggin if needed, uncomment the next line
#mesh.to_file('/scratch/houston_mesh_small.shp', 'ESRI Shapefile')
#os._exit(1)

#Set up the output raster
driver = gdal.GetDriverByName('GTiff')
output_dataset = driver.Create(output_file, col_size, row_size, 1, gdal.GDT_Float64)
output_originX = bounds[0].x 
output_originY = bounds[0].y
output_xResolution = raster.GetGeoTransform()[1]
output_yResolution = raster.GetGeoTransform()[5]
print "Creating raster with {} rows, {} cols, starting at {}, {} with xRes {}, yRes {}".format(row_size, col_size, output_originX, output_originY, output_xResolution, output_yResolution)
output_dataset.SetGeoTransform( (output_originX, output_xResolution, 0, output_originY, 0, output_yResolution) )
output_dataset.SetProjection( raster_crs.ExportToWkt() )
band = output_dataset.GetRasterBand(1)
band.SetNoDataValue(-9999)
del output_dataset #reopen in each process
print "Bounded Grid Size: {} rows, {} cols".format(row_size, col_size)
percent_done = 0
#update_at = int(row_size*0.01) #update progress every 10%
    
#Generate points for all
#Let the generator project the coordinates from raster to adhydro coords
pointTimeStart = time.time()
row_points = pixels_to_points(raster.GetGeoTransform(), raster_crs.ExportToProj4(), adhydro_projection, raster_xMin, raster_xMax, raster_yMin, raster_yMax)
pointTimeEnd = time.time()
print "Created all points in {} seconds".format(pointTimeEnd - pointTimeStart)

if __name__ == '__main__':
    print "Initial memory usage: {} GB".format(mem_usage(True))
    start_time = time.time()
       
    #total = len(row_points)
    #update_at = int(total*0.1)
    num_cores = mp.cpu_count()
    chunks = getChunks(len(row_points), num_cores)
    lock = mp.Lock()
    pool = mp.Pool(num_cores, initializer=mp_init, initargs=(lock,))
    pool.map(process_rows, chunks)
    pool.close()
    pool.join()
