#!/usr/bin/env python

import os
import glob
import subprocess
import zipfile
import argparse

parser = argparse.ArgumentParser(description="This script extracts prepares SSURGO and STATSO data files.")
parser.add_argument('dataDirectory', help='Directory containing input data, specifically directories SSURGO and STATSGO.\n')
parser.add_argument('-x','--extract', help='Flag to indicate if SSURGO/STATSGO files need to be extracted.', action='store_true')

args = parser.parse_args()

# input_directory_path
input_directory_path =  os.path.abspath(args.dataDirectory)
SSURGO_DIR = os.path.join(input_directory_path, 'SSURGO')
STATSGO_DIR = os.path.join(input_directory_path, 'STATSGO')

def extractSSURGO():
    g = os.path.join(SSURGO_DIR, '*.zip')
    ssurgo_zip = glob.glob(g)
    #Should now have the top level SSURGO zip file name.  Could possibly be more than one...
    for z in ssurgo_zip:
        with zipfile.ZipFile(z, 'r') as zf:
            zf.extractall(SSURGO_DIR)
    #Once extracted, we now have a set of zip files for the ssurgo areas
    #where the names look like soils\wss_SSA*.zip
    #Glob these and extract each one, then remove the archive
    g = os.path.join(SSURGO_DIR, '*wss_SSA*.zip')
    soils = glob.glob(g)
    for s in soils:
        with zipfile.ZipFile(s, 'r') as zf:
            zf.extractall(SSURGO_DIR)
        os.remove(s)
    
def prepareSSURGO():
    #build the soilmu_a__merge by combining spatial data for each area
    areas = os.path.join(SSURGO_DIR,'*')
    spatial = os.path.join(areas,'spatial')
    soilmu = os.path.join(spatial, 'soilmu_a*.shp')
    shapes = glob.glob(soilmu)
    #Use gdal ogr2ogr to merge the set of shape files.  Should provide comeplete coverage of mesh area!.  Can verify with QGIS.
    merged = os.path.join(SSURGO_DIR, 'soilmu_a_merged.shp')
    for s in shapes:
        cmd = ['ogr2ogr', '-update', '-append', merged, s, '-f', 'ESRI Shapefile']
        subprocess.call(cmd)
        
def extractSTATSGO():
    g = os.path.join(STATSGO_DIR, '*.zip')
    statsgo_zip = glob.glob(g)
    #Should now have the top level STATSGO zip file name.  Could possibly be more than one...
    for z in statsgo_zip:
        with zipfile.ZipFile(z, 'r') as zf:
            zf.extractall(STATSGO_DIR)
    #Once extracted, we now have a set of zip files for each state
    #where the names look like soils\wss_gsmsoil*.zip
    #Glob these and extract each one, then remove the archive
    g = os.path.join(STATSGO_DIR, '*wss_gsmsoil*.zip')
    soils = glob.glob(g)
    for s in soils:
        with zipfile.ZipFile(s, 'r') as zf:
            zf.extractall(STATSGO_DIR)
        os.remove(s)

def prepareSTATSGO():
    #build the gsmsoilmu_a_merge by combining spatial data for each state
    areas = os.path.join(STATSGO_DIR,'*')
    spatial = os.path.join(areas,'spatial')
    soilmu = os.path.join(spatial, 'gsmsoilmu_a*.shp')
    shapes = glob.glob(soilmu)
    #Use gdal ogr2ogr to merge the set of shape files.  Should provide comeplete coverage of mesh area!.  Can verify with QGIS.
    merged = os.path.join(STATSGO_DIR, 'gsmsoilmu_a_merged.shp')
    for s in shapes:
        cmd = ['ogr2ogr', '-update', '-append', merged, s, '-f', 'ESRI Shapefile']
        subprocess.call(cmd)


if __name__ == '__main__':
    #Test to see if SSURGO/STATSGO directories exist in provided directory
    if not os.path.isdir(SSURGO_DIR):
        print input_directory_path+" does not contain a SSURGO directory."
    else:
        if args.extract:
            extractSSURGO()
        prepareSSURGO()
    if not os.path.isdir(STATSGO_DIR):
        print input_directory_path+" does not contain a STATSGO directory."
    else:
        if args.extract:
            extractSTATSGO()
        prepareSTATSGO()
