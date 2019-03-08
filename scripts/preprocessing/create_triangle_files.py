#!/usr/bin/env python
# Run the following code on shapefiles that contain all of the catchment,
# waterbody, and stream edges that you want to be in the simulation mesh.
# These files must already have been pre-processed according to the mesh
# creation instructions.  This code will generate the .poly and .node input
# files for triangle and a .link file that contains the mapping from waterbody
# reach codes to link numbers.
import sys
import math
import argparse
import os
import geopandas as gpd

# RCS 5/11/17
# The above code has baked into it the directory structure that we currently use to organize all the different files that are produced when we make a mesh.
# I don't like this for three reasons:
#
# 1) There's no documentation that tells you what directory structure you must use for this script to work.
# 2) I never intended that directory structure to be the one and only one true way that you must organize your files to use ADHydro.
# 3) The way I have had files organized has changed over time as I had better ideas, and it may change in the future.
#
# I would rather have a system of options where you can specify all seven files that need to be specified,
# possibly with a couple groups like an input directory and an output directory where default names will be used.
#
# Here is how I originally specified the directories.  If you want to use this you can uncomment these lines and set file names here.
input_directory_path       = "/project/CI-WATER/data/maps/sugar_creek/ArcGIS"
input_catchment_file       = input_directory_path + "/" + "mesh_catchments.shp"
input_waterbody_file       = input_directory_path + "/" + "mesh_waterbodies.shp"
input_road_file            = input_directory_path + "/" + "mesh_roads.shp"
input_stream_file          = input_directory_path + "/" + "mesh_streams.shp"
input_original_stream_file = input_directory_path + "/" + "projectednet.shp"
output_directory_path      = "/share/CI-WATER_Simulation_Data/sugar_creek/ASCII"
output_node_file           = output_directory_path + "/" + "mesh.node"
output_poly_file           = output_directory_path + "/" + "mesh.poly"
output_link_file           = output_directory_path + "/" + "mesh.1.link"




# Reach codes of waterbodies are 14 digit integers, which can be represented in
# a 64 bit int.  Make sure that a python int is large enough.
assert long(sys.maxint) >= 99999999999999L

# To generate a point in a polygon generate a point on each side of the first
# line segment in the polygon.  One of the points will be in the polygon.
def point_in_polygon(feature):
  geom = feature.geometry()
  polygon = geom.asPolygon()
  assert 0 < len(polygon) # No empty polygons.
  x1 = polygon[0][0].x()
  y1 = polygon[0][0].y()
  x2 = polygon[0][1].x()
  y2 = polygon[0][1].y()
  mx = (x1 + x2) / 2
  my = (y1 + y2) / 2
  dx = (x1 - x2)
  dy = (y1 - y2)
  scale = 1 / (math.sqrt(dx * dx + dy * dy) * 1000)
  testx = mx - dy * scale
  testy = my + dx * scale
  if not geom.contains(QgsPoint(testx, testy)):
    testx = mx + dy * scale
    testy = my - dx * scale
    assert geom.contains(QgsPoint(testx, testy)) # if the first point was not in the polygon, this second one must be.
  return testx, testy

#Densify a multi-line segment by adding n evenly spaced points between two existing nodes
def densify(segments, n):
  if n == 0:
    return segments
  new_segments = []
  #Adding n points means making n+1 intervals
  n=n+1
  #Loop through all points except the last
  #Since range isn't inclusive, use len(segments) to store index
  for p1, index in zip(segments[:-1], range(len(segments))):
    #p1 is the first point on this segment 
    p2 = segments[index+1] #Second point that creates a segment from p1 -> p2
    #Find the x and y interval steps
    x_distance = (p2.x() - p1.x())/float(n)
    y_distance = (p2.y() - p1.y())/float(n)
    new_segments.append(p1)
    for i in range(1,n):
      nextX = p1.x() + i*x_distance
      nextY = p1.y() + i*y_distance
      new_segments.append(QgsPoint(nextX, nextY))
    #end of a given segment is the beginning of the next, so it gets added in the next iteration
  #Finally, add the last point back to the segments
  new_segments.append(segments[-1])
  return new_segments

"""
To handle some geometry roundoff errors in Triangle, we need to align streams and catchments better
when we are densifying one but not the other.  This code densifies the streams then unions the 
points back into the catchment geometry before processing.
"""
import shapely

def densify_shapely_line(line_string, n):
    print line_string
    if n == 0:
        return line_string
    new_lines = []
    #Adding n points by making n+1 intervals
    n = n + 1
    #Loop through all coordinates except the last
    for p1 , index in zip(line_string.coords[:-1], range(len(line_string.coords))):
        p1 = shapely.geometry.Point(p1)
        p2 = shapely.geometry.Point( line_string.coords[index+1] )
        x_distance = (p2.x - p1.x)/float(n)
        y_distance = (p2.y - p1.y)/float(n)
        new_lines.append(p1)
        for i in range(1,n):
            nextX = p1.x + i*x_distance
            nextY = p1.y + i*y_distance
            new_lines.append(shapely.geometry.Point(nextX, nextY))
    new_lines.append( shapely.geometry.Point( line_string.coords[-1]) )
    ret = shapely.geometry.LineString(new_lines)
    return ret

catchments_df = gpd.read_file(input_catchment_file)
streams_df = gpd.read_file(input_stream_file)
streams_df.geometry = streams_df.geometry.apply(densify_shapely_line, args=(1,))

catchments_df.geometry = catchments_df.geometry.union(streams_df.geometry)



#Remove this line, for testing only
#os._exit(1)
with open(output_node_file, "w") as node_file:
  with open(output_poly_file, "w") as poly_file:
    with open(output_link_file, "w") as link_file:
      node    = 0 # Counter for the number of nodes.
      segment = 0 # Counter for the number of segments.
      region  = 0 # Counter for the number of regions.
      hole    = 0 # Counter for the number of holes.
      #
      feature                  = QgsFeature()
      catchment_layer          = QgsVectorLayer(input_catchment_file,       "catchment",       "ogr")
      catchment_provider       = catchment_layer.dataProvider()
      waterbody_layer          = QgsVectorLayer(input_waterbody_file,       "waterbody",       "ogr")
      waterbody_provider       = waterbody_layer.dataProvider()
      road_layer               = QgsVectorLayer(input_road_file,            "road",            "ogr")
      road_provider            = road_layer.dataProvider()
      stream_layer             = QgsVectorLayer(input_stream_file,          "stream",          "ogr")
      stream_provider          = stream_layer.dataProvider()
      original_stream_layer    = QgsVectorLayer(input_original_stream_file, "original_stream", "ogr")
      original_stream_provider = original_stream_layer.dataProvider()
      #
      node_file.write("0                    2 0 0\n") # Leave space for the first zero to be replaced with the number of nodes.
      poly_file.write("0 2 0 0\n")                    # Zero nodes in the .poly file indicates nodes will be read from a separate .node file.
      poly_file.write("0                    1\n")     # Leave space for the first zero to be replaced with the number of line segments.
      #
      # Determine which linknos are used in the original stream file.
      used_linknos = []
      linknoindex = original_stream_provider.fieldNameIndex("LINKNO")
      assert -1 != linknoindex # LINKNO must be found.
      """
      #DEPRICATED IN QGIS 2.0 and above
      original_stream_provider.select([linknoindex])
      while original_stream_provider.nextFeature(feature):
      """
      for feature in original_stream_layer.getFeatures():
        linkno = feature[linknoindex]
        assert isinstance(linkno, (int, long)) # Integer conversion must succeed.
        while linkno >= len(used_linknos):
          used_linknos.append(-1)
        used_linknos[linkno] = linkno
      #
      # Add the catchment nodes and segments to the mesh.
      """
      catchment_provider.select()
      while catchment_provider.nextFeature(feature):
      """
      for feature in catchment_layer.getFeatures():
        if feature.geometry() is not None:
          polygon = feature.geometry().asPolygon()
          assert 0 < len(polygon) # No empty polygons.
          boundary_marker = "0" # Catchment segments have no boundary marker.
          for ring in polygon:
            #ring = densify(ring, args.densify)
            firstnode = node
            for point in ring:
              node_file.write(str(node) + " " + str(point.x()) + " " + str(point.y()) + "\n")
              if node < firstnode + len(ring) - 1:
                # Ordinary line segment from one node to the next.
                poly_file.write(str(segment) + " " + str(node) + " " + str(node + 1)  + " " + boundary_marker + "\n")
              else:
                # Line segment from the last node in a closed polygon to its first node.
                poly_file.write(str(segment) + " " + str(node) + " " + str(firstnode) + " " + boundary_marker + "\n")
              node    += 1
              segment += 1
          region += 1
        else:
          print "Found empty feature in catchment file."
      #
      # Add the waterbody nodes and segments to the mesh.
      reachcodeindex = waterbody_provider.fieldNameIndex("ReachCode")
      assert -1 != reachcodeindex # ReachCode must be found.
      permanentindex = waterbody_provider.fieldNameIndex("Permanent_")
      assert -1 != permanentindex # Permanent_ must be found.
      """
      waterbody_provider.select([reachcodeindex, permanentindex])
      while waterbody_provider.nextFeature(feature):
      """
      for feature in waterbody_layer.getFeatures():
        polygon = feature.geometry().asPolygon()
        assert 0 < len(polygon) # No empty polygons.
        # For some reason toInt rolls over like it is doing a 32 bit conversion even though sys.maxint indicates a 64 bit int. NOT VALID with QGIS > 2.0
        # Deal with this by converting to long first, and then to int.
        # If reach code is not found, use permanent code instead.
        if not feature[reachcodeindex]:
            try:
                reachcodelong = long(feature[permanentindex])
            except ValueError, e:
                #Check if we are trying to convert a hex style ID to long
                result = feature[permanentindex]
                if result[0] == '{':
                    #Slit on hyphen
                    parts = result.split('-')
                    #Take first set of characters without the {
                    first = parts[0][1:]
                    reachcodelong = long(first, 16)
                else:
                    raise e
        else:
            reachcodelong = long(feature[reachcodeindex])
        #
        assert isinstance(reachcodelong, long)  # Integer conversion must succeed.
        reachcode = int(reachcodelong) #TODO just use int conversion in the first place since this is the python impelementation, should use maxint
        # FIXLATER this search could be sped up by storing a map of reachcode -> linkno
        linkno = 0
        while linkno < len(used_linknos) and -1 != used_linknos[linkno] and reachcode != used_linknos[linkno]:
          linkno += 1
        while linkno >= len(used_linknos):
          used_linknos.append(-1)
        used_linknos[linkno] = reachcode
        print "Waterbody number " + str(feature.id()) + " reach code " + str(reachcode) + " assigned to linkno " + str(linkno)
        # The boundary marker indicates which waterbody.  Use linkno + 2 in the poly file because 0 is already used for no boundary and 1 is already used for
        # mesh edge boundary.
        boundary_marker = str(linkno + 2)
        for ring in polygon:
          #ring = densify(ring, args.densify)
          firstnode = node
          for point in ring:
            node_file.write(str(node) + " " + str(point.x()) + " " + str(point.y()) + "\n")
            if node < firstnode + len(ring) - 1:
              # Ordinary line segment from one node to the next.
              poly_file.write(str(segment) + " " + str(node) + " " + str(node + 1)  + " " + boundary_marker + "\n")
            else:
              # Line segment from the last node in a closed polygon to its first node.
              poly_file.write(str(segment) + " " + str(node) + " " + str(firstnode) + " " + boundary_marker + "\n")
            node    += 1
            segment += 1
        hole += 1
      #
      # Add the road nodes and segments to the mesh.
      """
      catchment_provider.select()
      while catchment_provider.nextFeature(feature):
      """
      for feature in road_layer.getFeatures():
        polygon = feature.geometry().asPolygon()
        assert 0 < len(polygon) # No empty polygons.
        boundary_marker = "0" # Road segments have no boundary marker.
        for ring in polygon:
          #ring = densify(ring, args.densify)
          firstnode = node
          for point in ring:
            node_file.write(str(node) + " " + str(point.x()) + " " + str(point.y()) + "\n")
            if node < firstnode + len(ring) - 1:
              # Ordinary line segment from one node to the next.
              poly_file.write(str(segment) + " " + str(node) + " " + str(node + 1)  + " " + boundary_marker + "\n")
            else:
              # Line segment from the last node in a closed polygon to its first node.
              poly_file.write(str(segment) + " " + str(node) + " " + str(firstnode) + " " + boundary_marker + "\n")
            node    += 1
            segment += 1
      #
      # Add the stream nodes and segments to the mesh.
      linknoindex = stream_provider.fieldNameIndex("LINKNO")
      assert -1 != linknoindex # LINKNO must be found.
      """
      stream_provider.select([linknoindex])
      while stream_provider.nextFeature(feature):
      """
      for feature in stream_layer.getFeatures():
        polyline = feature.geometry().asPolyline()
        assert 0 < len(polyline) # No empty polylines.
        linkno = int(feature[linknoindex])
        assert isinstance(linkno, (int,long)) # Integer conversion must succeed.
        assert 0 <= linkno # linkno must be non-negative.
        assert linkno == used_linknos[linkno] # The permanent code of a stream must be its linkno.
        # The boundary marker indicates which stream.  Use linkno + 2 in the poly file because 0 is already used for no boundary and 1 is already used for
        # mesh edge boundary.
        boundary_marker = str(linkno + 2)
        firstnode = node
        #polyline = densify(polyline, args.densify)
        for point in polyline:
          node_file.write(str(node) + " " + str(point.x()) + " " + str(point.y()) + "\n")
          if node < firstnode + len(polyline) - 1:
            # Ordinary line segment from one node to the next.
            poly_file.write(str(segment) + " " + str(node) + " " + str(node + 1) + " " + boundary_marker + "\n")
            segment += 1
          # No else.  No line segment from the last node in an open polyline to its first node.
          node += 1
      #
      # Place a hole in each waterbody polygon.
      # To do this we need to generate a point in each polygon.
      poly_file.write(str(hole) + "\n")
      hole = 0
      """
      waterbody_provider.select()
      while waterbody_provider.nextFeature(feature):
      """
      for feature in waterbody_layer.getFeatures():
        holex, holey = point_in_polygon(feature)
        poly_file.write(str(hole) + " " + str(holex) + " " + str(holey) + "\n")
        hole += 1
      #
      # Place a region attribute and area constraint in each catchment polygon.
      # To do this we need to generate a point in each polygon.
      poly_file.write(str(region) + "\n")
      region = 0
      catchmentindex = catchment_provider.fieldNameIndex("Catchment")
      assert -1 != catchmentindex # Catchment must be found.
      """
      catchment_provider.select([catchmentindex])
      while catchment_provider.nextFeature(feature):
      """
      for feature in catchment_layer.getFeatures():
        if feature.geometry() is not None:
          regionx, regiony = point_in_polygon(feature)
          catchment_number = int(feature[catchmentindex])
          assert isinstance(catchment_number, (int,long)) # Integer conversion must succeed.
          assert 0 <= catchment_number # Catchment number must be non-negative.
          # The region attribute indicates which catchment.  Use catchment_number + 2 in the poly file because 0 is already used for no region attribute and to
          # match stream linkno boundary markers where 1 is already used for mesh edge boundary.
          region_attribute = str(catchment_number + 2)
          area_constraint = "-1" # FIXME calculate a real area constraint.
          poly_file.write(str(region) + " " + str(regionx) + " " + str(regiony) + " " + region_attribute + " " + area_constraint + "\n")
          region += 1
      #
      # Fill in the number of segments and nodes at the beginning of the files.
      node_file.seek(0)
      node_file.write(str(node))
      poly_file.seek(8)
      poly_file.write(str(segment))
      #
      # Write out the mapping from waterbody reach codes to link numbers.
      link_file.write(str(len(used_linknos)) + " 1\n")
      linkno = 0
      while linkno < len(used_linknos):
        link_file.write(str(linkno) + " " + str(used_linknos[linkno]) + "\n")
        linkno += 1


