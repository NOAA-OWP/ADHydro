#include "all.h"
#include <shapefil.h>
#include <assert.h>

// FIXME put in .h file
#define SHAPES_SIZE   (2) // Size of array of shapes in ChannelLinkStruct.
#define UPSTREAM_SIZE (4) // Size of array of upstream links in ChannelLinkStruct.

typedef struct
{
  ChannelTypeEnum type;                       // The type of the link.
  int             permanent;                  // For waterbodies, permanent code.  For streams, original link number.  If a stream is split this will indicate
                                              // the original link number of the unsplit stream.  For unused -1.
  SHPObject*      shapes[SHAPES_SIZE];        // Shape object(s) of the link.  Streams will have only one.  Waterbodies may have more than one if they are
                                              // multipart.
  double          length;                     // Meters.
  double          baseWidth;                  // Width of channel base in meters.
  double          sideSlope;                  // Widening of each side of the channel for each unit increase in water depth.  It is delta-x over delta-y, the
                                              // inverse of the traditional definition of slope, unitless.
  double          upstreamContributingArea;   // For unmoved streams, contributing area in square meters at   upstream end of shape.  For others, 0.0.
  double          downstreamContributingArea; // For unmoved streams, contributing area in square meters at downstream end of shape.  For others, 0.0.
  int             upstream[UPSTREAM_SIZE];    // Array indices of links upstream from this link or a boundary condition code.
  int             downstream;                 // Array index of link downstream from this link or a boundary condition code.
} ChannelLinkStruct;

// Connect upstreamLinkNo to flow into downstreamLinkNo.  If upstreamLinkNo
// already has a downstream connection it is broken.  If downstreamLinkNo
// already has upstream connections they are left connected and it is an error
// if there is no space in the upstream connections array.
// 
// Either upstreamLinkNo or downstreamLinkNo can be a boundary condition code
// in which case the other link is set to have that boundary upstream or
// downstream of it.  If upstreamLinkNo is NOFLOW then no upstream link is
// added to downstreamLinkNo.  If both are boundary condition codes nothing is
// done.
//
// Returns: true if there is an error, false otherwise.  If there is an error
// no connections are modified.
// 
// Parameters:
//
// channels         - The channel network as a 1D array of ChannelLinkStruct.
// size             - The number of elements in channels.
// upstreamLinkNo   - The link to make flow into downstreamLinkNo or a boundary
//                    condition code.
// downstreamLinkNo - The link that upstreamLinkNo will flow into or a boundary
//                    condition code.
bool makeChannelConnection(ChannelLinkStruct* channels, int size, int upstreamLinkNo, int downstreamLinkNo)
{
  bool error = false; // Error flag.
  int  ii;            // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && (isBoundary(upstreamLinkNo) || (0 <= upstreamLinkNo && upstreamLinkNo < size)) &&
         (isBoundary(downstreamLinkNo) || (0 <= downstreamLinkNo && downstreamLinkNo < size)));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Add upstreamLinkNo to downstreamLinkNo.
  if (!isBoundary(downstreamLinkNo) && NOFLOW != upstreamLinkNo)
    {
      // Find the first empty upstream connection.
      ii = 0;
      
      while (UPSTREAM_SIZE > ii && NOFLOW != channels[downstreamLinkNo].upstream[ii])
        {
          ii++;
        }
      
      if (UPSTREAM_SIZE > ii)
        {
          channels[downstreamLinkNo].upstream[ii] = upstreamLinkNo;
        }
      else
        {
          fprintf(stderr, "ERROR in makeChannelConnection: Channel link %d has more than the maximum allowable %d upstream neighbors.\n",
                  downstreamLinkNo, UPSTREAM_SIZE);
          error = true;
        }
    }
  
  // Add downstreamLinkNo to UpstreamLinkNo.
  if (!error && !isBoundary(upstreamLinkNo))
    {
      // Break downstream connection of upstreamLinkNo.
      if (!isBoundary(channels[upstreamLinkNo].downstream))
        {
          // Find UpstreamLinkNo in the upstream connection list of its downstream neighbor.
          ii = 0;
          
          while (UPSTREAM_SIZE > ii && upstreamLinkNo != channels[channels[upstreamLinkNo].downstream].upstream[ii])
            {
              ii++;
            }
          
          // Remove UpstreamLinkNo from the list and move all other upstream connections forward to fill the gap.
          while (UPSTREAM_SIZE - 1 > ii && NOFLOW != channels[channels[upstreamLinkNo].downstream].upstream[ii])
            {
              channels[channels[upstreamLinkNo].downstream].upstream[ii] = channels[channels[upstreamLinkNo].downstream].upstream[ii + 1];
              ii++;
            }
          
          // Fill in the last connection with NOFLOW.
          if (UPSTREAM_SIZE - 1 == ii)
            {
              channels[channels[upstreamLinkNo].downstream].upstream[ii] = NOFLOW;
            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
          else if (UPSTREAM_SIZE == ii)
            {
              // Just a warning because we are breaking the connection anyway.
              fprintf(stderr, "WARNING in makeChannelConnection: Breaking downstream connection of channel link %d and there is no reciprocal connection from "
                              "its downstream neighbor %d.\n", upstreamLinkNo, channels[upstreamLinkNo].downstream);
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
        }
      
      // Make new downstream connection.
      channels[upstreamLinkNo].downstream = downstreamLinkNo;
    }
  
  return error;
}

// Prune linkNo if it is unneeded.  If linkNo is pruned, recursively prune
// downstream because linkNo might have been the last upstream link of its
// downstream neighbor.
// 
// Parameters:
//
// channels - The channel network as a 1D array of ChannelLinkStruct.
// size     - The number of elements in channels.
// linkno   - The link to try to prune.
void tryToPruneLink(ChannelLinkStruct* channels, int size, int linkNo)
{
  bool error = false;       // Error flag.
  int  oldDownstreamLinkNo; // For pruning recursively.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= linkNo && linkNo < size);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Prune streams if there are no upstream links and all elements are unassociated.
  if (STREAM == channels[linkNo].type && NOFLOW == channels[linkNo].upstream[0])
    {
      // FIXME check that all elements are unassociated.
      
      oldDownstreamLinkNo = channels[linkNo].downstream;
      
      error = makeChannelConnection(channels, size, linkNo, NOFLOW);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Since we are only breaking a connection makeChannelConnection cannot return an error.
      assert(!error);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      channels[linkNo].type = PRUNED_STREAM;
      
      tryToPruneLink(channels, size, oldDownstreamLinkNo);
    }
}

// Read a .link file and allocate and initialize an array of ChannelLinkStruct.
// The caller takes ownership of this array and is responsible for deleting it.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels - A pointer passed by reference which will be assigned to point to
//            the newly allocated array or NULL if there is an error.
// size     - A scalar passed by reference which will be assigned to the number
//            of elements in the newly allocated array or zero if there is an
//            error.
// filename - The name of the .link file to read.
bool readLink(ChannelLinkStruct** channels, int* size, const char* filename)
{
  bool  error = false; // Error flag.
  int   ii, jj;        // Loop counters.
  FILE* linkFile;      // The link file to read from.
  int   numScanned;    // Used to check that fscanf scanned all of the requested values.
  int   linkNo;        // Used to read the link numbers in the file and check that they are sequential.
  int   firstLinkNo;   // Used to store whether the file is zero based or one based.
  int   dimension;     // Used to check that the dimension is 1.
  int   permanent;     // Used to read the permanent codes of the links.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && NULL != size && NULL != filename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Open the file.
  linkFile = fopen(filename, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NULL != linkFile))
    {
      fprintf(stderr, "ERROR in readLink: Could not open link file %s.\n", filename);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

  // Read the number of links from the file.
  if (!error)
    {
      numScanned = fscanf(linkFile, "%d %d", size, &dimension);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          fprintf(stderr, "ERROR in readLink: Unable to read header from link file %s.\n", filename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < *size && 1 == dimension))
        {
          fprintf(stderr, "ERROR in readLink: Invalid header in link file %s.\n", filename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Allocate the array.
  if (!error)
    {
      *channels = new ChannelLinkStruct[*size];
    }
  else
    {
      *channels = NULL;
    }
  
  for (ii = 0; !error && ii < *size; ii++)
    {
      // Initialize fixed values.
      (*channels)[ii].type = NOT_USED;
      
      for (jj = 0; jj < SHAPES_SIZE; jj++)
        {
          (*channels)[ii].shapes[jj] = NULL;
        }
      
      (*channels)[ii].length                     = 0.0;
      (*channels)[ii].baseWidth                  = 0.0;
      (*channels)[ii].sideSlope                  = 0.0;
      (*channels)[ii].upstreamContributingArea   = 0.0;
      (*channels)[ii].downstreamContributingArea = 0.0;
      
      for (jj = 0; jj < UPSTREAM_SIZE; jj++)
        {
          (*channels)[ii].upstream[jj] = NOFLOW;
        }
      
      (*channels)[ii].downstream = NOFLOW;
      
      // Fill in the permanent codes from the file.
      numScanned = fscanf(linkFile, "%d %d", &linkNo, &permanent);
      
      // Set zero based or one based link numbers.
      if (0 == ii)
        {
          if (0 == linkNo)
            {
              firstLinkNo = 0;
            }
          else
            {
              firstLinkNo = 1;
            }
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          fprintf(stderr, "ERROR in readLink: Unable to read entry %d from link file %s.\n", ii + firstLinkNo, filename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii + firstLinkNo == linkNo))
        {
          fprintf(stderr, "ERROR in readLink: Invalid link number in link file %s.  %d should be %d.\n", filename, linkNo, ii + firstLinkNo);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      // The array is zero based, but the link file might be one based.  Use mod rather than minus so that only one link gets renumbered, the last one.
      (*channels)[linkNo % *size].permanent = permanent;
    }
  
  // Deallocate channels and set size to zero on error.
  if (error)
    {
      if (NULL != *channels)
        {
          delete[] *channels;
          *channels = NULL;
        }

      *size = 0;
    }

  // Close the file.
  if (NULL != linkFile)
    {
      fclose(linkFile);
    }

  return error;
}

// Read a waterbodies shapefile.  Tag the links in the channel network as
// WATERBODY or ICEMASS.  Save the polygons from the shapefile.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels     - The channel network as a 1D array of ChannelLinkStruct.
// size         - The number of elements in channels.
// fileBasename - The basename of the waterbody shapefile to read.
//                readWaterbodies will read the following files:
//                fileBasename.shp, and fileBasename.dbf.
bool readWaterbodies(ChannelLinkStruct* channels, int size, const char* fileBasename)
{
  bool            error = false;  // Error flag.
  int             ii, jj;         // Loop counters.
  SHPHandle       shpFile;        // Geometry  part of the shapefile.
  DBFHandle       dbfFile;        // Attribute part of the shapefile.
  int             numberOfShapes; // Number of shapes in the shapefile.
  int             permanentIndex; // Index of metadata field.
  int             ftypeIndex;     // Index of metadata field.
  int             permanent;      // The permanent code of the waterbody.
  const char*     ftype;          // The type of the waterbody as a string.
  int             linkNo;         // The link number of the waterbody.
  ChannelTypeEnum linkType;       // The type of the waterbody as an enum.
  SHPObject*      shape;          // The waterbody polygon.
  double          xMin;           // For computing bounding box of waterbody.
  double          xMax;           // For computing bounding box of waterbody.
  double          yMin;           // For computing bounding box of waterbody.
  double          yMax;           // For computing bounding box of waterbody.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && NULL != fileBasename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Open the geometry and attribute files.
  shpFile = SHPOpen(fileBasename, "rb");
  dbfFile = DBFOpen(fileBasename, "rb");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NULL != shpFile))
    {
      fprintf(stderr, "ERROR in readWaterbodies: Could not open shp file %s.\n", fileBasename);
      error = true;
    }

  if (!(NULL != dbfFile))
    {
      fprintf(stderr, "ERROR in readWaterbodies: Could not open dbf file %s.\n", fileBasename);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

  // Get the number of shapes and attribute indices
  if (!error)
    {
      numberOfShapes = DBFGetRecordCount(dbfFile);
      permanentIndex = DBFGetFieldIndex(dbfFile, "Permanent_");
      ftypeIndex     = DBFGetFieldIndex(dbfFile, "FTYPE");
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfShapes))
        {
          // Only warn because you could have zero waterbodies.
          fprintf(stderr, "WARNING in readWaterbodies: Zero shapes in dbf file %s.\n", fileBasename);
        }
      
      if (!(-1 != permanentIndex))
        {
          fprintf(stderr, "ERROR in readWaterbodies: Could not find field Permanent_ in dbf file %s.\n", fileBasename);
          error = true;
        }
      
      if (!(-1 != ftypeIndex))
        {
          fprintf(stderr, "ERROR in readWaterbodies: Could not find field FTYPE in dbf file %s.\n", fileBasename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Read the shapes.
  for (ii = 0; !error && ii < numberOfShapes; ii++)
    {
      permanent = DBFReadIntegerAttribute(dbfFile, ii, permanentIndex);
      ftype     = DBFReadStringAttribute(dbfFile, ii, ftypeIndex);
      linkNo    = 0;

      // Get link number.
      while(linkNo < size && permanent != channels[linkNo].permanent)
        {
          linkNo++;
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(linkNo < size))
        {
          fprintf(stderr, "ERROR in readWaterbodies: Could not find waterbody permanent code %d in channel network.\n", permanent);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      // Get link type.
      if (!error)
        {
          if (0 == strcmp("Ice Mass", ftype))
            {
              linkType = ICEMASS;
            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          else if (!(0 == strcmp("LakePond", ftype) || 0 == strcmp("SwampMarsh", ftype)))
            {
              fprintf(stderr, "ERROR in readWaterbodies: Waterbody permanent code %d has unknown type %s.\n", permanent, ftype);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          else
            {
              linkType = WATERBODY;
            }
        }
      
      if (!error)
        {
          // Because of the multipart to singlepart operation there can be duplicate permanent codes in the shapefile.  Only tag the link if it has not already
          // been tagged.
          if (NOT_USED == channels[linkNo].type)
            {
              channels[linkNo].type = linkType;
            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          else if (!(linkType == channels[linkNo].type))
            {
              fprintf(stderr, "ERROR in readWaterbodies: Waterbody permanent code %d occurs multiple times with different link types.\n", permanent);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
      
      // Get polygon.
      if (!error)
        {
          shape = SHPReadObject(shpFile, ii);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NULL != shape))
            {
              fprintf(stderr, "ERROR in readWaterbodies: Failed to read shape %d from shp file %s.\n", ii, fileBasename);
              error = true;
            }
          else if (!((5 == shape->nSHPType || 15 == shape->nSHPType || 25 == shape->nSHPType) && 1 < shape->nVertices))
            {
              // Even after multipart to singlepart waterbody polygons may have multiple parts if they have holes so don't check nParts.
              fprintf(stderr, "ERROR in readWaterbodies: Invalid shape %d from shp file %s.\n", ii, fileBasename);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Get bounding box and save polygon.
      if (!error)
        {
          SHPComputeExtents(shape);
          xMin = shape->dfXMin;
          xMax = shape->dfXMax;
          yMin = shape->dfYMin;
          yMax = shape->dfYMax;
          
          for (jj = 0; jj < SHAPES_SIZE && NULL != channels[linkNo].shapes[jj]; jj++)
            {
              if (xMin > channels[linkNo].shapes[jj]->dfXMin)
                {
                  xMin = channels[linkNo].shapes[jj]->dfXMin;
                }
              
              if (xMax < channels[linkNo].shapes[jj]->dfXMax)
                {
                  xMax = channels[linkNo].shapes[jj]->dfXMax;
                }
              
              if (yMin > channels[linkNo].shapes[jj]->dfYMin)
                {
                  yMin = channels[linkNo].shapes[jj]->dfYMin;
                }
              
              if (yMax < channels[linkNo].shapes[jj]->dfYMax)
                {
                  yMax = channels[linkNo].shapes[jj]->dfYMax;
                }
            }
          
          if (xMax - xMin > yMax - yMin)
            {
              channels[linkNo].length    = xMax - xMin;
              channels[linkNo].baseWidth = yMax = yMin;
            }
          else
            {
              channels[linkNo].length    = yMax - yMin;
              channels[linkNo].baseWidth = xMax = xMin;
            }
          
          if (jj < SHAPES_SIZE)
            {
              channels[linkNo].shapes[jj] = shape;
            }
          else
            {
              SHPDestroyObject(shape);
              
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
              fprintf(stderr, "ERROR in readWaterbodies: Waterbody permanent code %d has more than the maximum allowable %d shapes.\n",
                      permanent, SHAPES_SIZE);
              error = true;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
            }
        }
    } // End read the shapes.

  // Close the files.
  if (NULL != shpFile)
    {
      SHPClose(shpFile);
    }
  
  if (NULL != dbfFile)
    {
      DBFClose(dbfFile);
    }
  
  return error;
}

// Read a TauDEM stream network shapefile and fill in the stream links into the
// channel network.  Does not make any connections to the stream edges in the
// mesh.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels     - The channel network as a 1D array of ChannelLinkStruct.
// size         - The number of elements in channels.
// fileBasename - The basename of the TauDEM stream network shapefile to read.
//                readTaudemStreamnet will read the following files:
//                fileBasename.shp and fileBasename.dbf.
bool readTaudemStreamnet(ChannelLinkStruct* channels, int size, const char* fileBasename)
{
  bool      error = false;   // Error flag.
  int       ii;              // Loop counter.
  SHPHandle shpFile;         // Geometry  part of the shapefile.
  DBFHandle dbfFile;         // Attribute part of the shapefile.
  int       numberOfShapes;  // Number of shapes in the shapefile.
  int       linknoIndex;     // Index of metadata field.
  int       uslinkno1Index;  // Index of metadata field.
  int       uslinkno2Index;  // Index of metadata field.
  int       dslinknoIndex;   // Index of metadata field.
  int       lengthIndex;     // Index of metadata field.
  int       us_cont_arIndex; // Index of metadata field.
  int       ds_cont_arIndex; // Index of metadata field.
  int       linkNo;          // The link number of the stream.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && NULL != fileBasename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Open the geometry and attribute files.
  shpFile = SHPOpen(fileBasename, "rb");
  dbfFile = DBFOpen(fileBasename, "rb");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NULL != shpFile))
    {
      fprintf(stderr, "ERROR in readTaudemStreamnet: Could not open shp file %s.\n", fileBasename);
      error = true;
    }

  if (!(NULL != dbfFile))
    {
      fprintf(stderr, "ERROR in readTaudemStreamnet: Could not open dbf file %s.\n", fileBasename);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

  // Get the number of shapes and attribute indices.
  if (!error)
    {
      numberOfShapes  = DBFGetRecordCount(dbfFile);
      linknoIndex     = DBFGetFieldIndex(dbfFile, "LINKNO");
      uslinkno1Index  = DBFGetFieldIndex(dbfFile, "USLINKNO1");
      uslinkno2Index  = DBFGetFieldIndex(dbfFile, "USLINKNO2");
      dslinknoIndex   = DBFGetFieldIndex(dbfFile, "DSLINKNO");
      lengthIndex     = DBFGetFieldIndex(dbfFile, "Length");
      us_cont_arIndex = DBFGetFieldIndex(dbfFile, "US_Cont_Ar");
      ds_cont_arIndex = DBFGetFieldIndex(dbfFile, "DS_Cont_Ar");
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfShapes))
        {
          fprintf(stderr, "ERROR in readTaudemStreamnet: Zero shapes in dbf file %s.\n", fileBasename);
          error = true;
        }
      
      if (!(-1 != linknoIndex))
        {
          fprintf(stderr, "ERROR in readTaudemStreamnet: Could not find field LINKNO in dbf file %s.\n", fileBasename);
          error = true;
        }
      
      if (!(-1 != uslinkno1Index))
        {
          fprintf(stderr, "ERROR in readTaudemStreamnet: Could not find field USLINKNO1 in dbf file %s.\n", fileBasename);
          error = true;
        }
      
      if (!(-1 != uslinkno2Index))
        {
          fprintf(stderr, "ERROR in readTaudemStreamnet: Could not find field USLINKNO2 in dbf file %s.\n", fileBasename);
          error = true;
        }
      
      if (!(-1 != dslinknoIndex))
        {
          fprintf(stderr, "ERROR in readTaudemStreamnet: Could not find field DSLINKNO in dbf file %s.\n", fileBasename);
          error = true;
        }
      
      if (!(-1 != lengthIndex))
        {
          fprintf(stderr, "ERROR in readTaudemStreamnet: Could not find field Length in dbf file %s.\n", fileBasename);
          error = true;
        }
      
      if (!(-1 != us_cont_arIndex))
        {
          fprintf(stderr, "ERROR in readTaudemStreamnet: Could not find field US_Cont_Ar in dbf file %s.\n", fileBasename);
          error = true;
        }
      
      if (!(-1 != ds_cont_arIndex))
        {
          fprintf(stderr, "ERROR in readTaudemStreamnet: Could not find field DS_Cont_Ar in dbf file %s.\n", fileBasename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    } // End get the number of shapes and attribute indices.

  // Fill in the links.
  for (ii = 0; !error && ii < numberOfShapes; ii++)
    {
      // Get the index of where to put this link in the array.
      linkNo = DBFReadIntegerAttribute(dbfFile, ii, linknoIndex);

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < linkNo && linkNo <= size))
        {
          fprintf(stderr, "ERROR in readTaudemStreamnet: Invalid link number %d in TauDEM stream network.\n", linkNo);
          error = true;
        }
      else
        {
          if (!(NOT_USED == channels[linkNo % size].type))
            {
              fprintf(stderr, "ERROR in readTaudemStreamnet: Link number %d used twice in TauDEM stream network.\n", linkNo);
              error = true;
            }

          if (!(linkNo == channels[linkNo % size].permanent))
            {
              fprintf(stderr, "ERROR in readTaudemStreamnet: Link number %d does not match permanent code in TauDEM stream network.\n", linkNo);
              error = true;
            }
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      if (!error)
        {
          // The array is zero based, but the shapefile might be one based.  Use mod rather than minus so that only one link gets renumbered, the last one.
          linkNo %= size;

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // The following code requires at least two upstream links.
          assert(2 <= UPSTREAM_SIZE);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          // Fill in the ChannelLinkStruct.
          channels[linkNo].type                       = STREAM;
          channels[linkNo].shapes[0]                  = SHPReadObject(shpFile, ii);
          channels[linkNo].length                     = DBFReadDoubleAttribute(dbfFile, ii, lengthIndex);
          channels[linkNo].upstreamContributingArea   = DBFReadDoubleAttribute(dbfFile, ii, us_cont_arIndex);
          channels[linkNo].downstreamContributingArea = DBFReadDoubleAttribute(dbfFile, ii, ds_cont_arIndex);
          channels[linkNo].upstream[0]                = DBFReadIntegerAttribute(dbfFile, ii, uslinkno1Index);
          channels[linkNo].upstream[1]                = DBFReadIntegerAttribute(dbfFile, ii, uslinkno2Index);
          channels[linkNo].downstream                 = DBFReadIntegerAttribute(dbfFile, ii, dslinknoIndex);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NULL != channels[linkNo].shapes[0]))
            {
              fprintf(stderr, "ERROR in readTaudemStreamnet: Failed to create shape from shp file %s.\n", fileBasename);
              error = true;
            }
          else if (!((3 == channels[linkNo].shapes[0]->nSHPType || 13 == channels[linkNo].shapes[0]->nSHPType || 23 == channels[linkNo].shapes[0]->nSHPType) &&
                     1 == channels[linkNo].shapes[0]->nParts && 1 < channels[linkNo].shapes[0]->nVertices))
            {
              fprintf(stderr, "ERROR in readTaudemStreamnet: Invalid shape %d from shp file %s.\n", ii, fileBasename);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          
          // FIXME error check these values, maybe do it in invariant?

          // The array is zero based, but the shapefile might be one based.  Use mod rather than minus so that only one link gets renumbered, the last one.
          if (!isBoundary(channels[linkNo].upstream[0]))
            {
              channels[linkNo].upstream[0] %= size;
            }
          
          if (!isBoundary(channels[linkNo].upstream[1]))
            {
              channels[linkNo].upstream[1] %= size;
            }
          
          if (!isBoundary(channels[linkNo].downstream))
            {
              channels[linkNo].downstream %= size;
            }
          else if (NOFLOW == channels[linkNo].downstream)
            {
              channels[linkNo].downstream = OUTFLOW;
            }
          
          // FIXME create link element
        }
    } // End fill in the links.
  
  // Eliminate links of length zero.
  for (ii = 0; !error && ii < size; ii++)
    {
      while (!error && 0.0 != channels[ii].length && !isBoundary(channels[ii].downstream) && 0.0 == channels[channels[ii].downstream].length)
        {
          linkNo = channels[ii].downstream;
          error  = makeChannelConnection(channels, size, ii, channels[channels[ii].downstream].downstream);
          
          if (!error)
            {
              tryToPruneLink(channels, size, linkNo);
            }
        }
    }

  // Close the files.
  if (NULL != shpFile)
    {
      SHPClose(shpFile);
    }
  
  if (NULL != dbfFile)
    {
      DBFClose(dbfFile);
    }
  
  return error;
}

// FIXME remove
int main(void)
{
  bool               error = false;
  int                ii, jj;
  ChannelLinkStruct* channels;
  int                size;
  
  error = readLink(&channels, &size, "/share/CI-WATER Simulation Data/small_green_mesh/mesh.1.link");
  
  if (!error)
    {
      error = readWaterbodies(channels, size, "/share/CI-WATER Simulation Data/small_green_mesh/mesh_waterbodies");
    }
  
  if (!error)
    {
      error = readTaudemStreamnet(channels, size, "/share/CI-WATER Simulation Data/small_green_mesh/projectednet");
    }
  
  if (NULL != channels)
    {
      for (ii = 0; ii < size; ii++)
        {
          for (jj = 0; jj < SHAPES_SIZE; jj++)
            {
              if (NULL != channels[ii].shapes[jj])
                {
                  SHPDestroyObject(channels[ii].shapes[jj]);
                }
            }
        }
      
      delete[] channels;
    }

  return 0;
}
