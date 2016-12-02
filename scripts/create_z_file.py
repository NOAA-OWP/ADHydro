#!/usr/bin/env python
# Once you have the output of triangle or adhydro_channel_preprocessing run this code to extract the z values of each x,y point from the DEM.
import re
import argparse
import os

from qgis.core import *
qgishome = '/project/CI-WATER/tools/CI-WATER-tools'
app = QgsApplication([], True)
app.setPrefixPath(qgishome, True)
app.initQgis()

parser = argparse.ArgumentParser(description="Create ADHydro z file from the results of triangle or adhydro_channel_preprocessing.")
parser.add_argument('ADHydroMapDir', help='The ADHydro map directory containing sub-directories ASCII and TauDEM.')
parser.add_argument('-o', '--outputDir', help='Optional output directory. By default, output will be put in ADHydroMapDir/ASCII')
args = parser.parse_args()

if not os.path.isdir(args.ADHydroMapDir):
    parser.error("Directory '{}' does not exist.".format(args.ADHydroMapDir))

meshPath = os.path.join(args.ADHydroMapDir, 'ASCII')
demPath = os.path.join(args.ADHydroMapDir, 'TauDEM')

if not os.path.isdir(meshPath):
    parser.error("Directory '{}' does not exist.".format(meshPath))
if not os.path.isdir(demPath):
    parser.error("Directory '{}' does not exist.".format(netPath))

inputMeshFile = os.path.join(meshPath, "mesh.1.node")
if not os.path.isfile(inputMeshFile):
    parser.error("Cannot find file '{}'".format(inputMeshFile))

taudemPath = os.path.join(args.ADHydroMapDir, 'TauDEM')
if not os.path.isdir(taudemPath):
    parser.error("Cannot find TauDEM directory in '{}'".format(args.ADHydroMapDir))
taudemPath = os.path.join(taudemPath, 'projected')
#Check for either a single .tif or a .vrt
inputDEMFile = os.path.join(taudemPath, 'projected.tif')
if not os.path.isfile(inputDEMFile):
    print "No .tif found, looking for .vrt"
    inputDEMFile = os.path.join(taudemPath, 'projected.vrt')
    if not os.path.isfile(inputDEMFile):
        parser.error("Could not find appropriate DEM, neither projected.tif or projected.vrt exists in '{}'".format(taudemPath))
print "Using '{}' for DEM".format(inputDEMFile)

output_directory_path = os.path.join(args.ADHydroMapDir, 'ASCII')
if args.outputDir:
    output_directory_path = args.outputDir
if not os.path.isdir(output_directory_path):
    print "'{}' directory does not exist, creating...".format(output_directory_path)
    os.mkdir(output_directory_path)

outputZFile   = os.path.join(output_directory_path, "mesh.1.z")

with open(inputMeshFile, "r") as nodeFile:
  with open(outputZFile, "w") as zFile:
    layer          = QgsRasterLayer(inputDEMFile)
    provider       = layer.dataProvider()
    firstLine      = True
    numberOfNodes  = 0
    nodeNumber     = 0
    nextNodeNumber = 0
    #
    for line in nodeFile:
      if line.strip().startswith("#") or 0 == len(line.strip()):
        print "Ignoring comment line: " + line
      elif firstLine:
        # Process header line.
        firstLine = False
        match = re.match("\s*([-+]?\d+)", line)
        assert match
        numberOfNodes = int(match.group(1))
        zFile.write(str(numberOfNodes) + " 1\n")
      else:
        # Process node line.
        match = re.match("\s*([-+]?\d+)\s*([-+]?(\d+(\.\d*)?|\.\d+)([eE][-+]?\d+)?)\s*([-+]?(\d+(\.\d*)?|\.\d+)([eE][-+]?\d+)?)", line)
        assert match
        nodeNumber = int(match.group(1))
        assert nextNodeNumber == nodeNumber
        nextNodeNumber += 1
        x = float(match.group(2))
        y = float(match.group(6))
        #success, data = layer.identify(QgsPoint(x, y))
        data = provider.identify(QgsPoint(x, y), QgsRaster.IdentifyFormatValue)
        if data.isValid():
          #Write the value of the first raster band
          #To get exact same results as old method, have to stip trailing 0's and then possibly trailing .
          #zFile.write("{} {}\n".format(nodeNumber, '{:.2f}'.format(data.results()[1]).rstrip('0').rstrip('.')))
          zFile.write("{} {}\n".format(nodeNumber, '{:.2f}'.format(data.results()[1])))
        else:
          print "WARNING: Node " + nodeNumber + " is outside the extent of the DEM.  Using zero for Z coordinate."
          zFile.write(str(nodeNumber) + " 0.0\n")
    assert nodeNumber == numberOfNodes - 1


