#include "OutputManager.h"
#include "../adhydro_netcdf.h"
#include <fstream> //for printing tests
namespace nc = adhydro::io::netcdf;

//Construct the OutputManager.  interperter is initialized with non-default constructor
OutputManager::OutputManager():numberOfOutputElements(0),outputInterval(0),
			      //Initialize the interperter with the interface variables
			      interperter("../output_spec", meshSpatialIndex, channelSpatialIndex, meshOutputElements, channelOutputElements)
{
  CkPrintf("Building output manager\n");
  size_t meshMeshNeighborsSize;
  size_t channelVerticesSize;
  size_t instance;
  int geometryID;
  int numberOfMeshElements;
  int numberOfChannelElements;
  int* intVariable;
  std::vector<value> indexable;
  double* meshVertexX = NULL;
  double* meshVertexY = NULL;
  double* channelVertexX = NULL;
  double* channelVertexY = NULL;
  
  cout << "READING NETCDF\n";    
  //TODO/FIXME read from ADHYDRO SUPERFILE
  bool error = nc::NetCDFOpenForRead("/localstore/runs/open_book_bedrock/20_meter/geometry.nc", &geometryID);
  if(error)
  {
      CkError("ERROR OutputManager::OutputManager() : Failed to open geometry.nc\n");
  }
  //To read the mesh elements and their coordinates, need:
  //meshMeshNeighborSize to determine the number of verticies of each mesh element (DIMENSION)
  //channelVerticesSize to determine the number of verticies of each channel element (DIMENSION)
  //instance to determine the last instance in the geometry file (DIMENSION)
  
  //read verticies from mesh/channelVertexX (ARRAY(instance#, #elements, #verticies))
  //			mesh/channelVertexY

  //numberOfMeshElements to determine how many elements to read from netcdf (SCALAR AT INSTANCE)
  //numberOfChannelElements to determine how many channel elements to read from netcdf (SCALAR AT INSTANCE)
  
  //On adaption: TODO/FIXME would we want to then update all "wheres"??? Assume if old element
  //is in our list of output elements then new ones shoudl be as well???
  if(!error)
  {
      //Read mesh vertex size dimension
      error = nc::NetCDFReadDimensionSize(geometryID, "meshMeshNeighborsSize", &meshMeshNeighborsSize);
  }
  if(!error)
  {
      //Read channel vertex size dimension
      error = nc::NetCDFReadDimensionSize(geometryID, "channelVerticesSize", &channelVerticesSize);
  }
  if(!error)
  {
      //read the instance size
      error = nc::NetCDFReadDimensionSize(geometryID, "instances", &instance);
      instance -= 1; //To use as 0-based index
  }
  if(!error)
  {
      //Read the nubmer of mesh elements of the last geometry instance (found by reading the instancd size)
      intVariable = &numberOfMeshElements;
      error       = nc::NetCDFReadVariable(geometryID, "numberOfMeshElements", instance, 0, 1, 1, 1, true, 0, true, &intVariable);
  }
  if(!error)
  {
      //Read the nubmer of channel elements of the last geometry instance (found by reading the instancd size)
      intVariable = &numberOfChannelElements;
      error       = nc::NetCDFReadVariable(geometryID, "numberOfChannelElements", instance, 0, 1, 1, 1, true, 0, true, &intVariable);
  }
  if(!error)
  {
      //Read the entire mesh vertex geometry into memory.  let the function allocate the memory, and clean it up when done in this constructor
      error = nc::NetCDFReadVariable(geometryID, "meshVertexX", instance, 0, numberOfMeshElements, meshMeshNeighborsSize,
                                     meshMeshNeighborsSize, true, 0.0, false, &meshVertexX);
  }
  if(!error)
  {
      //Read the entire mesh vertex geometry into memory.  let the function allocate the memory, and clean it up when done in this constructor
      error = nc::NetCDFReadVariable(geometryID, "meshVertexY", instance, 0, numberOfMeshElements, meshMeshNeighborsSize,
                                     meshMeshNeighborsSize, true, 0.0, false, &meshVertexY);
  }
  if (!error)
  {
      error = nc::NetCDFReadVariable(geometryID, "channelVertexX", instance, 0, numberOfChannelElements, channelVerticesSize,
				channelVerticesSize, true, 0.0, false, &channelVertexX);
  }
  if (!error)
  {
      error = nc::NetCDFReadVariable(geometryID, "channelVertexY", instance, 0, numberOfChannelElements, channelVerticesSize,
				channelVerticesSize, true, 0.0, false, &channelVertexY);
  }
  if (!error)
  {   cout<<"Building indicies\n";
      //std::ofstream ofs;
      //ofs.open("geometry_wkt.csv", std::ofstream::out);
      //Resize vector for efficiency
      indexable.resize(numberOfMeshElements);
      //Should now have all nessicary vertex data
      //temporary box for reuse during loop
      box bounding_box;
      for(int i = 0; i < numberOfMeshElements; i++)
      {
	polygon p;
	for(int j = 0; j < meshMeshNeighborsSize; j++)
	{
	    //TODO verify these are correctly indexed
	    p.outer().push_back(point(meshVertexX[i*meshMeshNeighborsSize+j], meshVertexY[i*meshMeshNeighborsSize+j]));
	}
	p.outer().push_back(p.outer()[0]); //Make it closed
	//Find the bounding box for the mesh element
	bounding_box = bg::return_envelope<box>(p);
	//Create the value to be inserted into the rtree.
	//To create a more balanced tree, the packing version is used which is why all values are
	//created then "bulk loaded" into the rtree" instead of inserting one at a time
	indexable[i] = std::make_pair( bounding_box, std::make_pair(i, geometry(p)) );
	//ofs<<std::setprecision(12)<<i<<"\t"<<bg::wkt<polygon>(p)<<std::endl;
	//CkPrintf("\n");
      }
      //TODO/FIXME add channel elements to polygons and rtree? Or keep a seperate rtree for channel elements?
      //Now instansiate the rtree using the polygons with a max of 16 elemetns per node, and min of 4
      meshSpatialIndex = rtree(indexable);
      //now create the channel spatial index
      //Reuse the indexable container, resize it to hold numberOfChannelElements
      indexable.resize(numberOfChannelElements);
      for(int i = 0; i < numberOfChannelElements; i++)
      {
	linestring line;
	for(int j = 0; j < channelVerticesSize; j++)
	{
	    line.push_back(point(channelVertexX[i*channelVerticesSize+j], channelVertexY[i*channelVerticesSize+j]));
	}
	bounding_box = bg::return_envelope<box>(line);
	indexable[i] = std::make_pair( bounding_box, std::make_pair(i, geometry(line)));
      }
      
      channelSpatialIndex = rtree(indexable);
        
      //TODO build confluence map here: map all confluence elements based on the stream orders of the confluence
      //i.e. 1 -> all elements of stream order 1 contributing to a confluence
      //     2 -> all elements of stream order 2 contributing to a confluence
      
      //Indexes are built, we can now use the interperter.
      //TODO should we only do this in a function after the outputmanager is fully constructed?
      //It won't hurt the interperter, just nothing will be found in the intersections 
      //if the rtrees are not populated.
      CkPrintf("OutputManager preparing to interpert\n"); 
      interperter.interpert();

      cout<<"OutputManager results from maps:\n";
      int outCount = 0;
      for(auto it : meshOutputElements)//std::unordered_map<int, bool>::iterator it = meshOutputElements.begin(); it != meshOutputElements.end(); ++it)
      {
	if(it.second)
	  outCount++;
	  //cout<<"Output element "<<it.first<<"\n";
      }
      cout<<"Outputing "<<outCount<<" mesh elements\n";
      outCount = 0;
      for(auto it : channelOutputElements)
      {
	if(it.second)
	  outCount++;
      }
      cout<<"Outputing "<<outCount<<" channel elements\n";
      
      //TODO Set numberOfOutputElements, create seperate var for mesh and channel  DO IN REGISTER/UNREGISTER, remove previous few lines of reporting
  }
  
  //Clean up and then exit on error
  if(channelVertexX != NULL) delete[] channelVertexX;
  if(channelVertexY != NULL) delete[] channelVertexY;
  if(meshVertexX != NULL) delete[] meshVertexX;
  if(meshVertexY != NULL) delete[] meshVertexY;
  if(error)
    CkExit();
  
}

OutputManager::~OutputManager()
{
 //delete interperter;
 //delete rtree;
}

void OutputManager::registerRegion(Region& region)
{
  CkPrintf("Proc %d Registering region %d\n", CkMyPe(), region.thisIndex);
  std::map<int, Element>::iterator it;
  for(it = region.meshElements.begin(); it != region.meshElements.end(); it++)
  {
      CkPrintf("Registering Mesh element id: %d\n", it->first);
      meshElements[it->first] = &it->second;
      //Const pointer allows the compiler to throw a warning of outputmanager tries to change data!!!
      //i.e. meshElements[it->first]->data1 = 100; is a compiler error
      if(meshOutputElements[it->first])
      {
	numberOfOutputElements++;
      }
  }
  for(it = region.channelElements.begin(); it != region.channelElements.end(); it++)
  {
      CkPrintf("Registering Channel element id: %d\n", it->first);
      channelElements[it->first] = &it->second;
      //Const pointer allows the compiler to throw a warning of outputmanager tries to change data!!!
      //i.e. meshElements[it->first]->data1 = 100; is a compiler error
      if(channelOutputElements[it->first])
      {
	numberOfOutputElements++;
      }
  }
}
void OutputManager::unregisterRegion(Region& region)
{
  CkPrintf("Proc %d Unregistering region %d\n", CkMyPe(), region.thisIndex);
  std::map<int, Element>::iterator it;
  for(it = region.meshElements.begin(); it != region.meshElements.end(); it++)
  {
      meshElements.erase(it->first);
      if(meshOutputElements[it->first])
      {
	numberOfOutputElements--;
      }
  }
  for(it = region.channelElements.begin(); it != region.channelElements.end(); it++)
  {
      channelElements.erase(it->first);
      if(channelOutputElements[it->first])
      {
	numberOfOutputElements--;
      }
  }
}

double OutputManager::synchronizeOutputTime()
{
    if(numberOfOutputElements > 0)
    {
	return INFINITY;
    }
    else
    {
	return outputInterval;
    }
}
#include "OutputManager.def.h"