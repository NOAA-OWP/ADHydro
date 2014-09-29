# Once you have the output of triangle or adhydro_channel_preprocessing run this code to extract the z values of each x,y point from the DEM.
import re

# modify these to point to your files
inputMeshFile = "/share/CI-WATER Simulation Data/small_green_mesh/mesh.1.chan.node"
inputDEMFile  = "/share/CI-WATER Simulation Data/upper_colorado/projected/projected.vrt"
outputZFile   = "/share/CI-WATER Simulation Data/small_green_mesh/mesh.1.chan.z"

with open(inputMeshFile, "r") as nodeFile:
  with open(outputZFile, "w") as zFile:
    layer              = QgsRasterLayer(inputDEMFile)
    firstLine          = True
    firstNode          = True
    numberOfNodes      = 0
    nodeNumber         = 0
    firstNodeNumber    = 0
    previousNodeNumber = 0
    for line in nodeFile:
      if firstLine:
        # Process header line.
        firstLine = False
        match = re.match("\s*([-+]?\d+)", line)
        if match:
          numberOfNodes = int(match.group(1))
          zFile.write(str(numberOfNodes) + " 1\n")
        else:
          print "ERROR: Invalid header."
          break
      else:
        # Process node line.
        match = re.match("\s*([-+]?\d+)\s*([-+]?(\d+(\.\d*)?|\.\d+)([eE][-+]?\d+)?)\s*([-+]?(\d+(\.\d*)?|\.\d+)([eE][-+]?\d+)?)", line)
        if match:
          nodeNumber = int(match.group(1))
          if firstNode:
            firstNode = False
            if 0 == nodeNumber or 1 == nodeNumber:
              firstNodeNumber    = nodeNumber
              previousNodeNumber = nodeNumber - 1
            else:
              print "ERROR: Nodes must be numbered starting at zero or one."
              break
          if nodeNumber == previousNodeNumber + 1:
            previousNodeNumber = nodeNumber
            x = float(match.group(2))
            y = float(match.group(6))
            success, data = layer.identify(QgsPoint(x, y))
            if success:
              zFile.write(str(nodeNumber) + " " + str(data.values()[0]) + "\n")
            else:
              print "WARNING: Node " + nodeNumber + " is outside the extent of the DEM."
              zFile.write(str(nodeNumber) + " 0.0\n")
          else:
            print "ERROR: Invalid node number " + nodeNumber + " should be " + (previousNodeNumber + 1)
            break
        else:
          print "WARNING: No match found on line:"
          print line
    if nodeNumber != numberOfNodes - 1 + firstNodeNumber:
      print "WARNING: Number of nodes in header does not match number of nodes in file."


