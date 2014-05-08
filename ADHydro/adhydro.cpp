#include "adhydro.h"
#include "all.h"
#include <netcdf.h>

ADHydro::ADHydro(CkArgMsg* msg)
{
  delete msg;
  
  // FIXME specify input file
  initializeMesh("/project/CI-WATER/rsteinke/ADHydro/input/mesh_data.nc");
  
  iteration   = 1;
  currentTime = 0.0;
  endTime     = 900.0;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  checkInvariant();
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  
  doTimestep(1.0);
}

ADHydro::ADHydro(CkMigrateMessage* msg)
{
  // Do nothing.
}

void ADHydro::setLoadBalancingToManual(void)
{
  TurnManualLBOn();
}

void ADHydro::initializeMesh(const char* filePath)
{
  bool            error = false;      // Error flag.
  int             retval;             // Library call return value.
  int             ncid;               // NetCDF file ID.
  bool            fileOpened = false; // Flag to indicate the file was opened and must be closed.
  int             grpidMesh;          // NetCDF group ID for Mesh_based group.
  int             dimidMesh;          // NetCDF dimension ID for Mesh_based group.
  unsigned long   dimMesh[1];         // NetCDF dimension array for Mesh_based group.
  unsigned long   indexMesh[1];       // NetCDF index array for Mesh_based group.
  int             varidElementID;     // NetCDF variable ID for Element_ID variable of Mesh_based group.
  int             varidVertex1;       // NetCDF variable ID for Vertex1 variable of Mesh_based group.
  int             varidVertex2;       // NetCDF variable ID for Vertex2 variable of Mesh_based group.
  int             varidVertex3;       // NetCDF variable ID for Vertex3 variable of Mesh_based group.
  int             vertex;             // NetCDF value holder for Vertex variables of Mesh_based group.
  int             varidNeighbor1;     // NetCDF variable ID for Neighbor1 variable of Mesh_based group.
  int             varidNeighbor2;     // NetCDF variable ID for Neighbor2 variable of Mesh_based group.
  int             varidNeighbor3;     // NetCDF variable ID for Neighbor3 variable of Mesh_based group.
  int             neighbor;           // NetCDF value holder for Neighbor variables of Mesh_based group.
  int             grpidNode;          // NetCDF group ID for Node_based group.
  int             dimidNode;          // NetCDF dimension ID for Node_based group.
  unsigned long   dimNode[1];         // NetCDF dimension array for Node_based group.
  unsigned long   indexNode[1];       // NetCDF index array for Node_based group.
  int             varidNodeID;        // NetCDF variable ID for Node_ID variable of Mesh_based group.
  int             varidNodeXCoord;    // NetCDF variable ID for Node_xCoord variable of Mesh_based group.
  int             varidNodeYCoord;    // NetCDF variable ID for Node_yCoord variable of Mesh_based group.
  int             varidNodeZCoord;    // NetCDF variable ID for Node_zCoord variable of Mesh_based group.
  double          vertexXInit[3];
  double          vertexYInit[3];
  double          vertexZSurfaceInit[3];
  double          vertexZBedrockInit[3];
  int             neighborInit[3];
  InteractionEnum interactionInit[3];
  
  retval = nc_open(filePath, NC_NOWRITE, &ncid);
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NC_NOERR == retval))
    {
      CkError("ERROR in ADHydro::initialize: unable to open NetCDF file.  %s.\n", nc_strerror(retval));
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    {
      fileOpened = true;
    }
  
  if (!error)
    {
      nc_inq_ncid(ncid, "Mesh_based", &grpidMesh);
      nc_inq_dimid(ncid, "Element_index", &dimidMesh);
      nc_inq_dimlen(ncid, dimidMesh, dimMesh);
      nc_inq_varid(grpidMesh, "Element_ID", &varidElementID);
      nc_inq_varid(grpidMesh, "Vertex1", &varidVertex1);
      nc_inq_varid(grpidMesh, "Vertex2", &varidVertex2);
      nc_inq_varid(grpidMesh, "Vertex3", &varidVertex3);
      nc_inq_varid(grpidMesh, "Neighbor1", &varidNeighbor1);
      nc_inq_varid(grpidMesh, "Neighbor2", &varidNeighbor2);
      nc_inq_varid(grpidMesh, "Neighbor3", &varidNeighbor3);

      nc_inq_ncid(ncid, "Node_based", &grpidNode);
      nc_inq_dimid(ncid, "Node_index", &dimidNode);
      nc_inq_dimlen(ncid, dimidNode, dimNode);
      nc_inq_varid(grpidNode, "Node_ID", &varidNodeID);
      nc_inq_varid(grpidNode, "Node_xCoord", &varidNodeXCoord);
      nc_inq_varid(grpidNode, "Node_yCoord", &varidNodeYCoord);
      nc_inq_varid(grpidNode, "Node_zCoord", &varidNodeZCoord);

      meshProxySize = dimMesh[0];

      meshProxy = CProxy_MeshElement::ckNew(meshProxySize);

      meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, doTimestep), thisProxy));

      for (indexMesh[0] = 0; indexMesh[0] < dimMesh[0]; indexMesh[0]++)
        {
          nc_get_var1_int(grpidMesh, varidVertex1, indexMesh, &vertex);

          indexNode[0] = vertex % dimNode[0];

          nc_get_var1_double(grpidNode, varidNodeXCoord, indexNode, &vertexXInit[0]);
          nc_get_var1_double(grpidNode, varidNodeYCoord, indexNode, &vertexYInit[0]);
          nc_get_var1_double(grpidNode, varidNodeZCoord, indexNode, &vertexZSurfaceInit[0]);
          vertexZBedrockInit[0] = vertexZSurfaceInit[0] - 1.0; // Soil thickness hardcoded at 1.0 meters.

          nc_get_var1_int(grpidMesh, varidVertex2, indexMesh, &vertex);

          indexNode[0] = vertex % dimNode[0];

          nc_get_var1_double(grpidNode, varidNodeXCoord, indexNode, &vertexXInit[1]);
          nc_get_var1_double(grpidNode, varidNodeYCoord, indexNode, &vertexYInit[1]);
          nc_get_var1_double(grpidNode, varidNodeZCoord, indexNode, &vertexZSurfaceInit[1]);
          vertexZBedrockInit[1] = vertexZSurfaceInit[1] - 1.0; // Soil thickness hardcoded at 1.0 meters.

          nc_get_var1_int(grpidMesh, varidVertex3, indexMesh, &vertex);

          indexNode[0] = vertex % dimNode[0];

          nc_get_var1_double(grpidNode, varidNodeXCoord, indexNode, &vertexXInit[2]);
          nc_get_var1_double(grpidNode, varidNodeYCoord, indexNode, &vertexYInit[2]);
          nc_get_var1_double(grpidNode, varidNodeZCoord, indexNode, &vertexZSurfaceInit[2]);
          vertexZBedrockInit[2] = vertexZSurfaceInit[2] - 1.0; // Soil thickness hardcoded at 1.0 meters.

          nc_get_var1_int(grpidMesh, varidNeighbor1, indexMesh, &neighbor);
          neighborInit[0] = neighbor % meshProxySize;
          nc_get_var1_int(grpidMesh, varidNeighbor2, indexMesh, &neighbor);
          neighborInit[1] = neighbor % meshProxySize;
          nc_get_var1_int(grpidMesh, varidNeighbor3, indexMesh, &neighbor);
          neighborInit[2] = neighbor % meshProxySize;

          interactionInit[0] = BOTH_CALCULATE_FLOW_RATE;
          interactionInit[1] = BOTH_CALCULATE_FLOW_RATE;
          interactionInit[2] = BOTH_CALCULATE_FLOW_RATE;

          meshProxy[indexMesh[0]].sendInitialize(vertexXInit, vertexYInit, vertexZSurfaceInit, vertexZBedrockInit, neighborInit, interactionInit, 0, 1.0e-3,
                                                 0.5, 0.04, 0.1, 0.0, (vertexZSurfaceInit[0] + vertexZSurfaceInit[1] + vertexZSurfaceInit[2]) / 3.0, 0.0, 0.0);
        }
    }
  
  if (fileOpened)
    {
      retval = nc_close(ncid);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == retval))
        {
          CkError("ERROR in ADHydro::initialize: unable to close NetCDF file.  %s.\n", nc_strerror(retval));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (error)
    {
      CkExit();
    }
}

void ADHydro::doTimestep(double dtNew)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < dtNew))
    {
      CkError("ERROR in ADHydro::doTimestep: dtNew must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Load balance once after a few iterations to generate load statistics.
  if (5 == iteration)
    {
      CkStartLB();
    }
  
  if (endTime > currentTime)
    {
      if (endTime - currentTime < dtNew)
        {
          dtNew = endTime - currentTime;
        }
      
      CkPrintf("currentTime = %lf, dtNew = %lf\n", currentTime, dtNew);
      
      iteration++;
      currentTime += dtNew;
      
      meshProxy.sendDoTimestep(iteration, dtNew);
      
      // FIXME remove
      usleep(10000);
    }
  else
    {
      CkPrintf("currentTime = %lf, simulation finished\n", currentTime);
      
      meshProxy.ckSetReductionClient(new CkCallback(CkIndex_ADHydro::outputPartitioning(NULL), thisProxy));
      meshProxy.sendReportPartition();
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  checkInvariant();
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
}

void ADHydro::checkInvariant()
{
  bool error = false; // Error flag.
  
  if (!(0 < meshProxySize))
    {
      CkError("ERROR in ADHydro::checkInvariant: meshProxySize must be greater than zero.\n");
      error = true;
    }
  
  if (!(currentTime <= endTime))
    {
      CkError("ERROR in ADHydro::checkInvariant: currentTime must be less than or equal to endTime.\n");
      error = true;
    }
  
  meshProxy.sendCheckInvariant();
  
  if (error)
    {
      CkExit();
    }
}

void ADHydro::outputPartitioning(CkReductionMsg *msg)
{
  int ii; // Loop counter.
  int partition[meshProxySize];
  
  // Get the initial element in the set.
  CkReduction::setElement* current = (CkReduction::setElement*) msg->getData();
  
  // Loop over elements in set.
  while(current != NULL)
  {
    // Get the pointer to the array of ints.
    int* result = (int*) &current->data;
    
    // The first int is the element number.  The second int is its partition assignment.
    partition[result[0]] = result[1];
    
    // Iterate.
    current = current->next();
  }
  
  // FIXME clean up
  FILE* partitionFile = fopen("/project/CI-WATER/rsteinke/ADHydro/output/mesh.1.part", "w");
  
  fprintf(partitionFile, "%d %d\n", meshProxySize, CkNumPes());
  
  for (ii = 1; ii < meshProxySize; ii++)
    {
      fprintf(partitionFile, "%d %d\n", ii, partition[ii]);
    }
  
  fprintf(partitionFile, "%d %d\n", meshProxySize, partition[0]);
  
  fclose(partitionFile);
  
  CkExit();
}

int ADHydro::meshProxySize;

#include "adhydro.def.h"
