#!/usr/bin/env python
import geopandas as gpd
import os
import argparse

parser = argparse.ArgumentParser(description="Create a single vector polygon defining the perimeter of the mesh.\nBy default, generates an outline which includes holes for water bodies.")
parser.add_argument('meshDirectory', help='Directory containing mesh data, specifically mesh_catchments.shp file.\nBy default this directory will also be used for all output.')
parser.add_argument('-o','--output', help='Optional output directory.')
parser.add_argument('-n','--name', help='Name of the mesh, defaults to the input directory basename.')
parser.add_argument('-b','--boundary', help='Dissolve the original catchements to generate a single polygon defining the boundary of the mesh (no holes).', action='store_true')
parser.add_argument('-x','--extent', help='Print the extents of the existing mesh outline in its original CRS as well as WSG84 lat/long coordinates.  (minX, minY, maxX, maxY)', action='store_true')

args = parser.parse_args()

# input_directory_path
input_directory_path =  os.path.abspath(args.meshDirectory)

# This script will write its output to this directory
# the files it will write are:
#
# output_directory_path/outline.shp
output_directory_path = os.path.abspath(args.output) if args.output else input_directory_path
mesh_name = args.name if args.name else os.path.basename(input_directory_path)

def dissolve_mesh(inputShape):
    """
        Function for dissolving the peremiter of the mesh to generate an outline polygon.
        Params:
            inputShape      The base name of the shapefile to dissolve
    """
    #Suprisingly, geopandas (using shapely) can dissolve the mesh triangles (elements.shp)
    #relatively quickly (a little over an hour on the Upper Colorado River (~9,000,000 elements)
    #To increase speed, can union the geometry of mesh_catchments.shp, takes about 20 minutes,
    #and provides an outline polygon almost identical (difference is 0.359375 square meters!) 
    #df = gpd.read_file(os.path.join(input_directory_path, 'elements.shp'))
    print "Dissolving mesh"
    shp = inputShape+'.shp'
    prj = inputShape+'.prj'
    #Have to get devel version of geopandas, or bad geometries cause read to fail
    df = gpd.read_file(os.path.join(input_directory_path, shp))
    g = df.geometry
    #Deal with bad geometries, happens with catchements_validated, but oddly not mesh_catchments
    g = g[ g.notnull() ]
    #Prevents self intersection topolgy exception
    g = g.buffer(0)
    g = gpd.GeoSeries([g.unary_union])
    g.to_file(os.path.join(output_directory_path, 'outline.shp'))
    from shutil import copy
    copy(os.path.join(input_directory_path, prj), os.path.join(output_directory_path, 'outline.prj'))
    return

def printExtent(shapeName):
    from osgeo import ogr, osr, gdal
    #gdal.UseExceptions()
    shpFile = shapeName+'.shp'
    shp = ogr.Open(os.path.join(input_directory_path, shpFile))
    layer = shp.GetLayer()
    source_srs = layer.GetSpatialRef()
    #Get the extents of the outline as close as possible to shape of the mesh
    extent = layer.GetExtent(force=True)
    #lat/long wkt
    latlong = 'GEOGCS["GCS_WGS_1984",\
        DATUM["WGS_1984",\
                SPHEROID["WGS_84",6378137,298.257223563]],\
                    PRIMEM["Greenwich",0],\
                        UNIT["Degree",0.017453292519943295]]'

    dest_srs = osr.SpatialReference(latlong)
    #Now transform extent coordinates to lat/long
    xform = osr.CoordinateTransformation(source_srs, dest_srs)
    min = ogr.Geometry(ogr.wkbPoint)
    min.AddPoint(extent[0], extent[2])
    max = ogr.Geometry(ogr.wkbPoint)
    max.AddPoint(extent[1], extent[3])
    min.Transform(xform)
    max.Transform(xform)
    print "ADHydro: {}, {}, {}, {}".format(extent[0], extent[1], extent[2], extent[3])
    print "Lat/Long: {}, {}, {}, {}".format(min.GetX(), max.GetX(), min.GetY(), max.GetY()) 
 
if __name__ == "__main__":
  if args.extent:
    outline = os.path.join(input_directory_path, 'outline.shp')
    if(os.path.isfile(outline)):
         print "Determining extents from outline shape."
         printExtent('outline')
    else:
        print "Outline doesn't exist, determining extents from catchments"
        printExtent('catchments_validated')

  elif args.boundary:
    dissolve_mesh('catchments_validated')
  else:
    dissolve_mesh('mesh_catchments')
