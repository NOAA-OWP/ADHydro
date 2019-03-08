#!/usr/bin/env python
"""
Note, on Mt Moran, need to load 
"""

import os
import netCDF4
import pandas as pd
import geopandas as gpd
from shapely.geometry import Polygon

import matplotlib
matplotlib.use('QT4Agg')
import matplotlib.pyplot as plot
from mpl_toolkits import axes_grid1
import argparse 
from math import ceil
#import sys
#input_dir = sys.argv[1]

parser = argparse.ArgumentParser(description="Generate a parameter map and statistics for an ADHydro simulation input.")
parser.add_argument('inputDirectory', help='Directory containing ADHydro simulation input, specifically parameter.nc and geometry.nc.')
parser.add_argument('-o','--output', help='Optional output file (png). By default, results are only displayed to the screen and not saved.')
parser.add_argument('-p','--parameters', help='A list of ADHydro parameters to map and analyze.', nargs='+', type=str, default=[])
parser.add_argument('meridian', help='Central meridian of sinusoidal projection used for the mesh.', type=int)
args = parser.parse_args()

input_dir = os.path.abspath(args.inputDirectory)
output_dir = os.path.abspath(args.output) if args.output else None

parameter = os.path.join(input_dir, 'parameter.nc')
geometry = os.path.join(input_dir, 'geometry.nc')

parameter_ncf = netCDF4.Dataset(parameter)
geometry_ncf = netCDF4.Dataset(geometry)
#TODO/FIXME Eventually read proj string from netcdf variable.
proj = '+proj=sinu +lon_0={} +x_0=20000000 +y_0=10000000 +datum=WGS84 +units=m +no_defs'.format(args.meridian)

"""
http://stackoverflow.com/questions/33714050/geopandas-plotting-any-way-to-speed-things-up
to try to make plotting faster until geopandas updates their plotting functions (possible in versino 0.3)
NJF updated colormap=cmap
"""
from matplotlib.collections import PatchCollection
import numpy as np
def plot_polygon_collection(ax, geoms, values=None, cmap='Set1',  facecolor=None, edgecolor=None,
                                    alpha=0.5, linewidth=1.0, **kwargs):
    """ Plot a collection of Polygon geometries """
    patches = []

    for poly in geoms:
        a = np.asarray(poly.exterior)
        if poly.has_z:
            poly = shapely.geometry.Polygon(zip(*poly.exterior.xy))

        patches.append(matplotlib.patches.Polygon(a))

    patches = PatchCollection(patches, facecolor=facecolor, linewidth=linewidth, edgecolor=edgecolor, alpha=alpha, **kwargs)

    if values is not None:
        patches.set_array(values)
        patches.set_cmap(cmap)

    ax.add_collection(patches, autolim=True)
    ax.autoscale_view()
    return patches

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

        patches.append(matplotlib.patches.Polygon(a))
    return patches

"""
http://stackoverflow.com/questions/18195758/set-matplotlib-colorbar-size-to-match-graph

Didn't work quite right...
"""
def add_colorbar(patch, aspect=20, pad_fraction=0.5, **kwargs):
    divider = axes_grid1.make_axes_locatable(patch.axes)
    width = axes_grid1.axes_size.AxesY(patch.axes, aspect=1/aspect)
    pad = axes_grid1.axes_size.Fraction(pad_fraction, width)
    current_ax = plot.gca()
    cax = divider.append_axes("right", size=width, pad=pad)
    plot.sca(current_ax)
    return patch.axes.figure.colorbar(patch, cax=cax, **kwargs)

def statString(s):
    s = """\
    \\begin{{table}}[]
    \\centering
    \\caption{{My caption}}
    \\label{{my-label}}
    \\begin{{tabular}}{{lllllll}}
    Mean   & Std    & Min    & Max    & 25%    & 50%    & 75%    \\\\
{:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f}
\\end{{tabular}}
\\end{{table}}
   """.format(s['mean'], s['std'], s['min'], s['max'], s['25%'], s['50%'], s['75%'])

    print s
    return s

    #return "Mean\tStd\tMin\tMax\n{:.2f}\t{:.2f}\t{:.2f}\t{:.2f}\n25%\t50%\t75%\n{:.2f}\t{:.2f}\t{:.2f}".format(s['mean'], s['std'], s['min'], s['max'], s['25%'], s['50%'], s['75%'])
if __name__=='__main__':
    
    if not args.parameters:
        parser.error('No parameters provided, please provide one or more parameters using the -p flag.')
    params = {}
    for p in args.parameters:
        if p not in parameter_ncf.variables:
            print 'Parameter {} does not exist in {}'.format(p, parameter)
            continue
        params[p] = parameter_ncf.variables[p][0]
    param_df = pd.DataFrame(params)
    
    #print "Creating map for parameter {}".format(p)
    #Get the geometry and create create a geoPandas dataframe for the mesh and parameters
    xs =  geometry_ncf.variables['meshVertexX'][0]
    ys = geometry_ncf.variables['meshVertexY'][0]
    geometry = [ Polygon(v) for v in [zip(vx, vy) for vx,vy in  zip(xs, ys)]] 
    gs = gpd.GeoSeries(geometry, crs=proj)
    df = gpd.GeoDataFrame(param_df, geometry=gs)
    if 'meshConductivity' in df.columns:
        df['meshConductivity'].replace(0.000141, pd.np.nan)
        #TODO/FIXME Finish cutting max from meshConductivity and meshPorosity for bedrock types 
    #Make a matplotlib patch for each mesh triangle
    patches = makePatches(df.geometry)
    #Define the subplot grid based on the number of passed parameters
    N = len(params.keys())
    ncols = 1
    if N >= 1 and N <= 4:
        ncols = 2
    else:
        ncols = 3 
    nrows = N/ncols if N > ncols else 1
    if N%ncols != 0 and N != 1:
        nrows = nrows + 1
    gs = matplotlib.gridspec.GridSpec(nrows, ncols)
    #print "Num rows {}, num cols {}, N {}".format(nrows, ncols, N)
    #start plotting the maps
    fig = plot.figure(figsize=(19,10), dpi=100)
    stats =  df.describe()
    #Each parameter is its own subplot
    for n, p in zip(range(N), params.keys()):
        print "Creating map for {}".format(p) 
        ax = fig.add_subplot(gs[n], aspect='equal', adjustable='box')
        #Plot the map of the given parameter using the geometry patches
        pStats = [['{:.3}'.format(s)] for s in stats[p].values[1:8]]
        #pStats.append('{:.2f}'.format(stats[p].values[7]))
        labels = stats.index[1:8]
        #labels.append(stats.index[7])
        table = ax.table(cellText=pStats, cellLoc='center', rowLabels=labels, loc='left', colWidths=[0.5]) #bottom bbox=[0.0, -0.05, 1.0, 0.08]) #for top bbox=[0.0,0.93,1.0,0.08])# colLabels=statHeader, loc='bottom')
        mesh = plotMesh(ax, patches, values=df[p], linewidth=0, cmap='cool')
        table.auto_set_font_size(False)
        table.set_fontsize(8)
        table.scale(1,2)

        ax.set_title('{}'.format(p))
        #Add the colorbar scale to the plot
        fig.colorbar(mesh, ax=ax, fraction=0.07)
    #gs.tight_layout(fig)
    #fig.tight_layout()
    #figManager = plot.get_current_fig_manager()
    #figManager.window.showMaximized()
    #plot.subplots_adjust(wspace=0.33, hspace=0.50)
    gs.update(left=0.01, right=0.95, wspace=0.1, hspace=0.2, bottom=0.05, top=0.95)
    if args.output:
            if args.output[-4:] != '.png':
                plot.savefig(args.output+'.png', dpi=100)
            else:
                plot.savefig(args.output, dpi=100)
    plot.show()
    #plot.savefig('paramters.png', dpi=100)
