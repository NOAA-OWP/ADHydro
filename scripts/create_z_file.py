# Once you have the output of triangle or adhydro_channel_preprocessing run this code to extract the z values of each x,y point from the DEM.
import re

# modify these to point to your files
inputMeshFile = "/share/CI-WATER_Simulation_Data/small_green_mesh/mesh.1.node"
inputDEMFile  = "/share/CI-WATER_Simulation_Data/upper_colorado/projected/projected.vrt"
outputZFile   = "/share/CI-WATER_Simulation_Data/small_green_mesh/mesh.1.z"

with open(inputMeshFile, "r") as nodeFile:
  with open(outputZFile, "w") as zFile:
    layer          = QgsRasterLayer(inputDEMFile)
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
        success, data = layer.identify(QgsPoint(x, y))
        if success:
          zFile.write(str(nodeNumber) + " " + str(data.values()[0]) + "\n")
        else:
          print "WARNING: Node " + nodeNumber + " is outside the extent of the DEM.  Using zero for Z coordinate."
          zFile.write(str(nodeNumber) + " 0.0\n")
    assert nodeNumber == numberOfNodes - 1


