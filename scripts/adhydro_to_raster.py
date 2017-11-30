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
from shapely import speedups
speedups.enable()
from shapely.geometry import Point, Polygon, box
from shapely.ops import transform
#from qgis.core import QgsSpatialIndex, QgsFeature, QgsGeometry, QgsRectangle, QgsPoint, QgsFeatureIterator
from functools import partial
from math import ceil
import pyproj
#from osgeo import gdal, osr
#gdal.UseExceptions()
import rasterio
import rasterio.features
from datetime import datetime

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
    if value < 1:
        raise argparse.ArgumentTypeError('{} is not a valid input, must be >= 1'.format(value))
    return value

def verify_date(s):
    try:
      return datetime.strptime(s, '%Y-%m-%d:%H')
    except:
      return datetime.strptime(s, '%Y-%m-%d')

def test_bounds(bounds):
  minX, minY, maxX, maxY = bounds
  if minX >= maxX:
    parser.error('Boundary Error: minLong ({}) must be < maxLong ({})'.format(minX, maxX))
  if minY <= maxY: #raster Y axis is inverted, so maxY > minY world coordinate
    parser.error('Boundary Error: minLat ({}) must be < maxLat ({})'.format(maxY, minY))


"""
  Set up command line args and parser
"""
parser = argparse.ArgumentParser(description="Create a rasterized inundation map based on original DEM cells.")
parser.add_argument('ADHydroGeometry', help='The ADHydro geometry.nc file containing the mesh geometry.')
parser.add_argument('ADHydroOutput', help='The ADHydro output netCDF file containing <variable>.')
#parser.add_argument('DEM', help='The original DEM file used to create the mesh in geometry.nc.')
parser.add_argument('-v', '--variable', type=str, help='The adhydro variable to transform to raster form, must a variable in ADHydroOutput.', required=True)
parser.add_argument('-t', '--time', help='Time to generate output for, accepted inputs are YYYY-MM-DD or YYY-MM-DD:H.', type=verify_date, required=True)
#parser.add_argument('-n', '--numSubsets', type=checkPositiveInput, help='Split the processing into N subsets of the raster grid.  Useful on large meshes to avoid running out of memory.')
parser.add_argument('-m', '--meridian', help='Centeral meridian to use for ADHydro Sinusoidal projection', required=True, type=float)
parser.add_argument('-o', '--outputFileName', help='Optional output file name.  By default, the filename will be <time>_<variable>_adhydro.tif .')
parser.add_argument('-a', '--aggregate', help='Aggregate results daily using [min, max, mean].', required=False, choices=['min', 'max', 'mean'])
parser.add_argument('-p', '--period', help='Periods to produce output.  One file per period will be produced (daily frequency), with the time appended to outputFileName if specified. Defaults to 1.', type=checkPositiveInput, default=1)
parser.add_argument('-b', '--bounds', help="Bounding box to limit processing. Coordinates in lat/long (WGS84).  Input as minLong, minLat, maxLong, maxLat.  Default uses mesh extent.", nargs=4, type=float, metavar=('minLong', 'minLat', 'maxLong', 'maxLat'))
args = parser.parse_args()
test_bounds(args.bounds)
"""
  Verify inputs exist as files
"""
if not os.path.isfile(args.ADHydroGeometry):
    parser.error("Geometry file '{}' does not exist".format(args.ADHydroGeometry))

if not os.path.isfile(args.ADHydroOutput):
    parser.error("Data file '{}' does not exist".format(args.ADHydroOutput))

#Open the adhydro output file and verify the time and variable to process
adhydro_output = nc.Dataset(args.ADHydroOutput, 'r')

ref_date = adhydro_output.variables['referenceDate'][0]
adhydro_times = adhydro_output.variables['currentTime'][:]/86400.0 + ref_date
times = pd.to_datetime(adhydro_times, unit='D', origin='julian').round('H') #Round to nearest hour to clean up small rounding erros in julian conversions
time_df = pd.DataFrame({'i':range(len(times))}, index=times)
#Close and reopen dataset per process later
adhydro_output.close()

if args.time not in times:
  parser.error( "Time {} does not exist in adhydro output file {}. Time range is: {} - {}".format(args.time, args.ADHydroOutput, times[0], times[-1]) )

if args.variable not in adhydro_output.variables.keys():
    parser.error("Variable {} not found in {}".format(args.variable, args.ADHydroOutput))


#set ADHydro projection string based on passed meridian
adhydro_projection = '+proj=sinu +lon_0={} +x_0=20000000 +y_0=0 +datum=WGS84 +units=m +no_defs'.format(args.meridian)

#Albers Equal Area Conic on NAD83 datum
output_projection = '+proj=aea +lat_1=29.5 +lat_2=45.5 +lat_0=37.5 +lon_0=-96 +x_0=0 +y_0=0 +ellps=GRS80 +datum=NAD83 +units=m +no_defs'


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
    """ 
    #Create a QGSI spatial index since RTREE hangs on moran??? TODO/FIXME
    meshIndex = QgsSpatialIndex()
    for i, triangle in meshFrame.itertuples():
        feature = QgsFeature()
        feature.setFeatureId(i)
        feature.setGeometry(QgsGeometry.fromWkt(triangle.to_wkt()))
        meshIndex.insertFeature(feature) 
    """
    t1 = time.time()
    print "Mem after reading ADHydro mesh polygon geometries: {} GB".format(mem_usage(True))
    print "Time to read ADHydro mesh geometry: {}".format(t1-t0)
    adhydro_geom.close()
    return meshFrame#, meshIndex


"""
BEGIN processing beyond user control!
"""

adhydro_to_output = partial(
   pyproj.transform,
   pyproj.Proj( adhydro_projection ),
   pyproj.Proj( output_projection ) )

bounds = gpd.GeoSeries([])
if args.bounds:
  #Limit processing based on input bounding box
  bounds = gpd.GeoSeries( [Point(args.bounds[0], args.bounds[1]), Point(args.bounds[2], args.bounds[3])], crs="+init=epsg:4326")
#Use the bounds of the mesh geometry to limit the raster processing
#mesh_minX, mesh_minY, mesh_maxX, mesh_maxY = mesh.total_bounds
#bounds = gpd.GeoSeries( [ Point(mesh_minX, mesh_minY), Point(mesh_maxX, mesh_maxY) ], crs=adhydro_projection)
#Pre defined bounding box
#in lat/long coords (WGS84)
#mesh_minX, mesh_maxX, mesh_minY, mesh_maxY = (-95.8939, -95.2916, 29.9744, 29.5541)
#bounds = gpd.GeoSeries( [ Point(mesh_minX, mesh_minY), Point(mesh_maxX, mesh_maxY) ], crs="+init=epsg:4326" )

bounding_box = None

if not bounds.empty:
  mesh_bounds = bounds.to_crs(adhydro_projection)
  bounding_box = box(mesh_bounds[0].x, mesh_bounds[0].y, mesh_bounds[1].x, mesh_bounds[1].y)

#Get the mesh into geometries for operating on
mesh = getADHydroMeshPolyGeometry(args.ADHydroGeometry, bounding_box)

if bounds.empty:
  mesh_minX, mesh_minY, mesh_maxX, mesh_maxY = mesh.total_bounds
  mesh_bounds = gpd.GeoSeries( [ Point(mesh_minX, mesh_minY), Point(mesh_maxX, mesh_maxY) ], crs=adhydro_projection)

#print mesh
#Can save mesh shape file for debuggin if needed, uncomment the next line
#mesh.to_file('/scratch/houston_mesh_small.shp', 'ESRI Shapefile')
#os._exit(1)
#print mesh_bounds
#The parameters for the new raster will based on these bounds
#We need to convert them to the output_projection and create the output raster
output_xMin, output_yMin = adhydro_to_output( mesh_bounds[0].x, mesh_bounds[0].y )
output_xMax, output_yMax = adhydro_to_output( mesh_bounds[1].x, mesh_bounds[1].y )
output_xRes = 10
output_yRes = -10
#Build the geo transform for pixels->coords
transform = rasterio.transform.from_origin(output_xMin, output_yMin, output_xRes, -output_yRes)
#Invert transform for coords->pixels
inverse_transform = ~transform

index_xMin, index_yMin = map(int, inverse_transform * (output_xMin, output_yMin) )
index_xMax, index_yMax = map(int, inverse_transform * (output_xMax, output_yMax) )

#Not supported in this version of rasterio... so using manual method above
#index_xMin, index_xMax, index_yMin, index_yMax = rasterio.transform.rowcol(transform, [output_xMin, output_xMax], [output_yMin, output_yMax] )

#Find raster size in number of pixels per column and row
col_size = index_xMax - index_xMin + 1 #For 0 based index
row_size = index_yMax - index_yMin + 1 #For 0 based index

print "Projecting mesh coordinates"
mesh = mesh.to_crs(output_projection)

def process(which):

  def read_data(s):
    return output_variable[s.name]

  def aggregate_data(s, agg):
    return agg( output_variable[:, s.name] )

  #Read the dataset into memory
  adhydro_output = nc.Dataset(args.ADHydroOutput, 'r')
  output_variable = None
  start_time = args.time + pd.Timedelta('{} day'.format(which))
  if start_time > time_df.index[-1]:
    print "WARNING: Time {} does not exist in {}.  Skipping rasterization.".format(start_time, args.ADHydroOutput)
    return
  if args.aggregate:
    end_time = start_time + pd.Timedelta('23 hours')
    if end_time > time_df.index[-1]:
        end_time = time_df.index[-1]
        print "WARNING: Cannot aggregate full day, result for {} aggregates to {}".format(start_time, end_time)
    indicies = time_df['i'][start_time:end_time]
    #print time_df.loc[start_time:end_time] 
    output_variable = adhydro_output.variables[args.variable][ indicies.values ][:]
  else:
    #If no aggregation, can read a single time
    output_variable = adhydro_output.variables[args.variable][time_df['i'][start_time]][:]
  
  #Set the output file name
  time_string = start_time.strftime('%Y-%m-%d:%H:%M:%S')
  output_file = '{}_{}_adhydro.tif'.format(time_string, args.variable)
  if args.aggregate:
    output_file = '{}_{}_adhydro_daily_{}.tif'.format(time_string, args.variable, args.aggregate)

  if args.outputFileName:
    output_file = time_string+'_'+args.outputFileName

  if args.aggregate:
    op = None
    if args.aggregate == 'min':
      op = pd.np.min
    elif args.aggregate == 'max':
      op = pd.np.max
    else:
      op = pd.np.mean
    mesh['data'] = mesh.apply(aggregate_data, axis=1, args=(op,))
  else:
      mesh['data'] = mesh.apply(read_data, axis=1)
  mesh['data'].fillna(-9999, inplace=True)

  print "Creating {} with {} rows, {} cols, starting at {}, {} with xRes {}, yRes {}".format(output_file, row_size, col_size, output_xMin, output_yMin, output_xRes, output_yRes)
  #NOTE cannot use proj4 string output_projection for albers equal area...it hangs and segfaults the raster open...so for now use the epsg/seri code 102003
  output_crs = rasterio.crs.CRS.from_string("epsg:102003")
  with rasterio.open(output_file, 'w', driver='GTiff', width = col_size, height=row_size, count=1, dtype = rasterio.float64, crs=output_crs, transform=transform, nodata=-9999) as out:
    #print "Starting rasterization"
    raster_start = time.time()
    img = rasterio.features.rasterize( ( (geom, data) for data,geom in mesh[['data', 'geometry']].itertuples(index=False) ), 
                                       out_shape=(row_size, col_size), fill=-9999, transform=transform, dtype=rasterio.float64 )
    raster_end = time.time()
    print "Time to rasterize {} was {} seconds.".format(output_file, raster_end - raster_start)
    out.write_band(1, img)

if __name__ == '__main__':
  num_cores = mp.cpu_count()
  pool = mp.Pool(num_cores)
  pool.map(process, range(args.period))
