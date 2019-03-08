#!/usr/bin/env python

import os
import netCDF4
import pandas as pd
import geopandas as gpd
from shapely.geometry import Polygon, LineString

import numpy as np

import matplotlib
matplotlib.use('QT4Agg')
import matplotlib.pyplot as plot

from matplotlib.collections import PatchCollection
import argparse 
from datetime import datetime

def parseDate(string):
    try:
       return pd.DatetimeIndex([datetime.strptime(string, '%Y-%m-%d:%H:%M:%S')]).to_julian_date()[0]
    except ValueError:
        pass

    try:
        return float(string)
    except ValueError:
        raise("Cannot convert referenceDate")

parser = argparse.ArgumentParser(description="Generate a max depth map ADHydro simulation input.")
parser.add_argument('depthFile', help = 'file containing max depth from ADHydro Simulation.')
parser.add_argument('geometry', help = 'geometry file, i.e. geometry.nc, used for the simulation.')
parser.add_argument('referenceDate', help = 'reference date of the simulation, as YYYY-MM-DD:H:M:S or julian date', type=parseDate)
parser.add_argument('-o','--output', help='Optional output file name. By default, output is saved as max_depth_<begin time>_<end_time>.csv. If only a directory is provided, the default file name will be output to that directory.')
parser.add_argument('meridian', help='Central meridian of sinusoidal projection used for the mesh.', type=int)
args = parser.parse_args()


output_dir = './'
filename = None
if args.output:
    if os.path.isdir(args.output) and not os.path.isfile(args.output):
        output_dir = os.path.abspath(args.output)
    
geometry = args.geometry #os.path.join(input_dir, 'geometry.nc')
geometry_ncf = netCDF4.Dataset(geometry)
#TODO/FIXME Eventually read proj string from netcdf variable.
proj = '+proj=sinu +lon_0={} +x_0=20000000 +y_0=10000000 +datum=WGS84 +units=m +no_defs'.format(args.meridian)

"""
Plot the given mesh patches, colored by values, to the provided axes.
"""
def plotMesh(ax, patches, values=None, cmap='Set1',  facecolor=None, edgecolor=None, alpha=0.5, linewidth=0.0, **kwargs):
    #Create a PatchCollection from the list of patches that define the mesh
    collection = PatchCollection(patches, facecolor=facecolor, linewidth=linewidth, edgecolor=edgecolor, alpha=alpha, **kwargs)

    if values is not None:
        collection.set_array(values)
        collection.set_cmap(cmap)
    #Plot the collection to the axes
    ax.add_collection(collection, autolim=True)
    ax.autoscale_view()
    ax.set_axis_off()
    #Return the collection
    return collection
"""
Given a list of Shapely geometries (i.e. Polygons) make a matplotlib patch for each one and return the list of these patches
"""
def makePatches(geoms):
    patches = []
    for poly in geoms:
        a = np.asarray(poly.exterior)
        if poly.has_z:
            poly = shapely.geometry.Polygon(zip(*poly.exterior.xy))
            #FIXME what was I doing here?????
        patches.append(matplotlib.patches.Polygon(a))
    return patches


if __name__=='__main__':
    #TODO make file name a CLI input
    max_depth_file = args.depthFile#os.path.join(input_dir, 'state.nc.maxDepth.txt')
    df = pd.read_csv(max_depth_file, sep=' ', usecols=[2, 4, 6], names=['element', 'maxDepth', 'time'], index_col=0)
    print(df)
    print(df.index.is_monotonic)
    df = df.sort_index()
    print(df.index.is_monotonic)
    reference_date = args.referenceDate
    
    df['time'] = df['time'] / 86400 #Partial Day
    df['time'] = df['time'] + reference_date #Julian date from reference
    df['time'] = pd.to_datetime(df['time'], utc=True, unit='D', origin='julian')

    #Get the geometry and create create a geoPandas dataframe for the mesh and parameters
    xs =  geometry_ncf.variables['meshVertexX'][0]
    ys = geometry_ncf.variables['meshVertexY'][0]
    geometry = [ Polygon(v) for v in [zip(vx, vy) for vx,vy in  zip(xs, ys)]] 
    gs = gpd.GeoSeries(geometry, crs=proj)
    gs = gs.to_crs(epsg='4326') #convert to wgs84 lat/lon

    geo_df = gpd.GeoDataFrame(df, geometry=gs)
    geo_df = geo_df[geo_df['maxDepth'] != -1]
    geo_df['center'] = geo_df.geometry.centroid
    """
    stats = geo_df['maxDepth'].describe()
    
    pStats = [['{:.3}'.format(s)] for s in stats.values[1:8]] 
    labels = stats.index[1:8]
    #Make a matplotlib patch for each mesh triangle
    patches = makePatches(geo_df.geometry)
    #start plotting the maps
    fig = plot.figure(figsize=(19,10), dpi=100)
    print("Creating max depth map")
    gs = matplotlib.gridspec.GridSpec(1,1)
    ax = fig.add_subplot(gs[0], aspect='equal', adjustable='box')
    #Add the table for stats 
    table = ax.table(cellText=pStats, cellLoc='center', rowLabels=labels, loc='left', colWidths=[0.5])
    #Plot the geometry collection and color by maxDepth
    mesh = plotMesh(ax, patches, values=geo_df['maxDepth'], linewidth=0, cmap='cool')
    #Clean up formatting of stats table
    table.auto_set_font_size(False)
    table.set_fontsize(8)
    table.scale(1,2)

    ax.set_title('Max Depth')
    #Add color scale bar
    plot.colorbar(mesh, fraction=0.07)
    """    
    print(geo_df.index.is_monotonic)
    #Save data to CSV
    if not filename:
        filename = 'max_depth_{}_{}.csv'.format( df['time'][0].strftime( '%Y-%m-%d:%H' ), df['time'][-1].strftime( '%Y-%m-%d:%H' ) )
    geo_df.to_csv( os.path.join(output_dir, filename) )
    
    plot.show()
    #plot.savefig('max_depth.png', dpi=100)
