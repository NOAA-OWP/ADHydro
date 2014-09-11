#include "all.h"
#include <shapefil.h>
#include <assert.h>

// FIXME put in .h file
#define SHAPES_SIZE   (2) // Size of array of shapes in ChannelLinkStruct.
#define UPSTREAM_SIZE (4) // Size of array of upstream links in ChannelLinkStruct.

// This code is a separate executable used to pre-process the channel network
// into a .chan file and a .chanxy file that are read into the adhydro
// executable by the file managers.  In this code the channel network is
// represented as a tree of links including TauDEM stream links and NHD
// waterbodies.  The tree of links is stored as an array of ChannelLinkStruct
// with array indices instead of pointers for the tree connections.  The link
// number of each link is the array index where it is stored.  Link numbers are
// not contiguous in the TauDEM output so there may be some elements in the
// array that are unused.  The gaps in the TauDEM link numbers are used for
// waterbodies.

// A linkElementStruct represents a section of a stream link.  The section
// goes from prev->endLocation to endLocation along the 1D length of the link
// or from zero to endLocation if prev is NULL.  For the last unmoved element
// in the list endLocation should always be the length of the link.  The
// element is associated with an edge number or is unassociated if edge is -1.
// The edge numbers are from the triangle .edge file and in the adhydro
// executable they are further associated with mesh element edges.  movedTo
// indicates if this link was split and part of the link was moved to a new
// link number.  For a description of how splitting and moving links works see
// the comment of the function splitLink.
typedef struct LinkElementStruct LinkElementStruct;
struct LinkElementStruct
{
  LinkElementStruct* prev;        // The previous element of this link or NULL if this is the first element.
  LinkElementStruct* next;        // The next     element of this link or NULL if this is the last  element.
  double             endLocation; // The 1D location in meters along the link of the end of this element.
  int                edge;        // The mesh edge associated with this link element or -1 if unassociated.
  int                movedTo;     // The new link number that this section of this link has been moved to or -1 if unmoved.
};

// Returns: The begin location of a link element.
inline double beginLocation(LinkElementStruct* element)
{
  double location; // The begin location.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != element);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  if (NULL == element->prev)
    {
      location = 0.0;
    }
  else
    {
      location = element->prev->endLocation;
    }
  
  return location;
}

// The channel network is stored as a 1D array of ChannelLinkStruct.  It is
// actually a tree stored in an array using array indices instead of pointers
// for the tree connections.  The reason it is done this way is because we know
// the size of the array and an index number for each link from the input
// files, and references to other links are stored as index numbers in the
// input files so it is just easier to do it this way.
typedef struct
{
  ChannelTypeEnum    type;                       // The type of the link.
  int                permanent;                  // For waterbodies, permanent code.  For streams, original link number.  If a stream is split this will
                                                 // indicate the original link number of the unsplit stream.  For unused -1.
  SHPObject*         shapes[SHAPES_SIZE];        // Shape object(s) of the link.  Streams will have only one.  Waterbodies may have more than one if they are
                                                 // multipart.
  double             length;                     // Meters.
  double             baseWidth;                  // Width of channel base in meters.
  double             sideSlope;                  // Widening of each side of the channel for each unit increase in water depth.  It is delta-x over delta-y,
                                                 // the inverse of the traditional definition of slope, unitless.
  double             upstreamContributingArea;   // For unmoved streams, contributing area in square meters at   upstream end of shape.  For others, 0.0.
  double             downstreamContributingArea; // For unmoved streams, contributing area in square meters at downstream end of shape.  For others, 0.0.
  int                upstream[UPSTREAM_SIZE];    // Array indices of links upstream from this link or a boundary condition code.
  int                downstream;                 // Array index of link downstream from this link or a boundary condition code.
  LinkElementStruct* firstElement;               // Sections of the link associated with mesh edges or NULL if no elements.
  LinkElementStruct* lastElement;                // Sections of the link associated with mesh edges or NULL if no elements.
} ChannelLinkStruct;

// Create a new LinkElementStruct in the channel network in the given link
// number.  The LinkElementStruct is placed after prevLinkElement in the linked
// list or at the head of the linked list if prevLinkElement is NULL.
//
// Returns: true if there is an error, false otherwise.  If there is an error
// no LinkElementStruct is created.
// 
// Parameters:
//
// channels        - The channel network as a 1D array of ChannelLinkStruct.
// size            - The number of elements in channels.
// linkNo          - The link to add the LinkElementStruct to.
// prevLinkElement - The new LinkElementStruct will be placed after this
//                   LinkElementStruct in the linked list.  Pass in NULL to
//                   place the new LinkElementStruct at the head of the linked
//                   list or if the linked list is empty.
// endLocation     - The 1D location in meters along the link of the end of the
//                   new LinkElementStruct.
// edge            - The edge number associated with this LinkElementStruct or
//                   -1 if unassociated.
// movedTo         - The new link number that this section of this link has
//                   been moved to.
bool createLinkElementAfter(ChannelLinkStruct* channels, int size, int linkNo, LinkElementStruct* prevLinkElement, double endLocation, int edge, int movedTo)
{
  bool               error = false;  // Error flag.
  LinkElementStruct* newLinkElement; // To point to the created LinkElementStruct.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= linkNo && linkNo < size && STREAM == channels[linkNo].type);
  
  if (NULL == prevLinkElement)
    {
      assert(0.0 < endLocation);
      
      if (NULL != channels[linkNo].firstElement)
        {
          assert(endLocation < channels[linkNo].firstElement->endLocation);
        }
      else
        {
          assert(endLocation == channels[linkNo].length);
        }
    }
  else
    {
      assert(prevLinkElement->endLocation < endLocation);
      
      if (NULL != prevLinkElement->next)
        {
          assert(endLocation < prevLinkElement->next->endLocation);
        }
      else
        {
          assert(endLocation == channels[linkNo].length);
        }
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)

  // Allocate the new LinkElementStruct.
  newLinkElement = new LinkElementStruct;
  
  newLinkElement->endLocation = endLocation;
  newLinkElement->edge        = edge;
  newLinkElement->movedTo     = movedTo;
  newLinkElement->prev        = prevLinkElement; // If prevLinkElement is NULL newLinkElement->prev should be NULL.

  if (NULL == newLinkElement->prev)
    {
      // Insert at the head of the list.
      newLinkElement->next          = channels[linkNo].firstElement;
      channels[linkNo].firstElement = newLinkElement;
    }
  else
    {
      // Insert not at the head of the list.
      newLinkElement->next       = newLinkElement->prev->next;
      newLinkElement->prev->next = newLinkElement;
    }

  if (NULL == newLinkElement->next)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      assert(channels[linkNo].lastElement == newLinkElement->prev); // lastElement should be newLinkElement->prev or NULL if newLinkElement->prev is NULL.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)

      // Insert at the tail of the list.
      channels[linkNo].lastElement = newLinkElement;
    }
  else
    {
      // Insert not at the tail of the list.
      newLinkElement->next->prev = newLinkElement;
    }

  return error;
}

// Remove a LinkElementStruct from its linked list and deallocate its memory.
//
// Returns: true if there is an error, false otherwise.  If there is an error
// the LinkElementStruct is not removed or deallocated.
//
// Parameters:
//
// channels          - The channel network as a 1D array of ChannelLinkStruct.
// size              - The number of elements in channels.
// linkNo            - The link to remove the LinkElementStruct from.
// linkElementToKill - The LinkElementStruct that will be removed from its
//                     linked list and deallocated.
bool killLinkElement(ChannelLinkStruct* channels, int size, int linkNo, LinkElementStruct* linkElementToKill)
{
  int error = false; // Error flag.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= linkNo && linkNo < size && STREAM == channels[linkNo].type && NULL != linkElementToKill);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Remove from the list.
  if (NULL == linkElementToKill->prev)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      assert(channels[linkNo].firstElement == linkElementToKill); // Remove from the head of the list.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)

      channels[linkNo].firstElement = linkElementToKill->next;
    }
  else
    {
      // Remove not from the head of the list.
      linkElementToKill->prev->next = linkElementToKill->next;
    }

  if (NULL == linkElementToKill->next)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      assert(channels[linkNo].lastElement == linkElementToKill); // Remove from the tail of the list.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)

      channels[linkNo].lastElement = linkElementToKill->prev;
    }
  else
    {
      // Remove not from the tail of the list.
      linkElementToKill->next->prev = linkElementToKill->prev;
    }

  // Deallocate.
  delete linkElementToKill;
  
  return error;
}

// Connect upstreamLinkNo to flow into downstreamLinkNo.  If upstreamLinkNo
// already has a downstream connection it is broken.  If downstreamLinkNo
// already has upstream connections they are left connected and it is an error
// if there is no more space in its upstream connections array.
// 
// Either upstreamLinkNo or downstreamLinkNo can be a boundary condition code
// in which case the other link is set to have that boundary upstream or
// downstream of it.  If upstreamLinkNo is NOFLOW then it is not added to
// downstreamLinkNo.  If both are boundary condition codes nothing is done.
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
// linkNo   - The link to try to prune.
void tryToPruneLink(ChannelLinkStruct* channels, int size, int linkNo)
{
  bool               error = false;       // Error flag.
  bool               prune;               // Whether to prune.
  LinkElementStruct* element;             // For checking if elements are unassociated.
  int                oldDownstreamLinkNo; // For pruning recursively.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= linkNo && linkNo < size);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Prune streams if there are no upstream links and all elements are unassociated.
  if (STREAM == channels[linkNo].type && NOFLOW == channels[linkNo].upstream[0])
    {
      // Check that all elements are unassociated.
      prune   = true;
      element = channels[linkNo].firstElement;
      
      while (prune && NULL != element)
        {
          if (-1 != element->edge)
            {
              prune = false;
            }
          
          element = element->next;
        }

      if (prune)
        {
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
}

// Return the 1D distance in meters along a stream link polyline of the point
// on the polyline closest to the given point (x, y).
// 
// Parameters:
// 
// link       - The stream link polyline.
// linkLength - The length of the stream link in meters.
// x(y)       - The X and Y coordinates in meters of the point to find the
//              closest location to on the polyline.
double getLocation(SHPObject* link, double linkLength, double x, double y)
{
  int    ii;              // Loop counter.
  double minDistance;     // The closest distance found so far from (x, y) to link.
  double location;        // The 1D distance along link where minDistance occurs.
  double length;          // The length of line segments processed so far.
  double segmentLength;   // The length of a line segment in the polyline.
  double closestLocation; // The closest location along the current line segment.
  double x_prime;         // The point on the line segment closest to (x, y).
  double y_prime;         // The point on the line segment closest to (x, y).
  double distance;        // The distance from (x', y') to (x, y).
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != link && 0.0 < linkLength);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Start with the most upstream vertex.  In the TauDEM stream network shapefile the beginning
  // of the polyline is the downstream end so start with the last vertex in the polyline.
  ii          = link->nVertices - 1;
  minDistance = sqrt((x - link->padfX[ii]) * (x - link->padfX[ii]) + (y - link->padfY[ii]) * (y - link->padfY[ii]));
  location    = 0.0;
  length      = 0.0;

  // Check every other line segment and vertex in link.
  for (ii = link->nVertices - 2; ii >= 0; ii--)
    {
      // Check the line segment from vertex ii + 1 to vertex ii.
      segmentLength   = sqrt((link->padfX[ii] - link->padfX[ii + 1]) * (link->padfX[ii] - link->padfX[ii + 1]) +
                             (link->padfY[ii] - link->padfY[ii + 1]) * (link->padfY[ii] - link->padfY[ii + 1]));
      closestLocation =    (((              x - link->padfX[ii + 1]) * (link->padfX[ii] - link->padfX[ii + 1]) +
                             (              y - link->padfY[ii + 1]) * (link->padfY[ii] - link->padfY[ii + 1])) / segmentLength);

      // Is the closest location between the endpoints of the line segment?
      if (0.0 < closestLocation && closestLocation < segmentLength)
        {
          // (x', y') is the point on the line segment closest to (x, y)
          x_prime  = (link->padfX[ii + 1] + (closestLocation / segmentLength) * (link->padfX[ii] - link->padfX[ii + 1]));
          y_prime  = (link->padfY[ii + 1] + (closestLocation / segmentLength) * (link->padfY[ii] - link->padfY[ii + 1]));
          distance = sqrt((x - x_prime) * (x - x_prime) + (y - y_prime) * (y - y_prime));

          if (minDistance > distance)
            {
              minDistance = distance;
              location    = length + closestLocation;
            }
        }

      // Add the line segment to the length processed so far.
      length += segmentLength;

      // Check the vertex ii.
      distance = sqrt((x - link->padfX[ii]) * (x - link->padfX[ii]) + (y - link->padfY[ii]) * (y - link->padfY[ii]));

      if (minDistance > distance)
        {
          minDistance = distance;
          location    = length;
        }
    }

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Length in the TauDEM shapefile is rounded off to the nearest decimeter so length could be up to a half a decimeter different from linkLength.
  assert(0.05 > fabs(length - linkLength));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  // The length of the polyline can be half a decimeter longer or shorter than linkLength.
  // If the location is near or beyond the end of the polyline set it to linkLength.
  if (0.05 > linkLength - location)
    {
      location = linkLength;
    }

  return location;
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
      
      (*channels)[ii].downstream   = NOFLOW;
      (*channels)[ii].firstElement = NULL;
      (*channels)[ii].lastElement  = NULL;
      
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
          // FIXME error check these values, maybe do it in invariant?
          channels[linkNo].type                       = STREAM;
          channels[linkNo].shapes[0]                  = SHPReadObject(shpFile, ii);
          channels[linkNo].length                     = DBFReadDoubleAttribute(dbfFile, ii, lengthIndex);
          channels[linkNo].upstreamContributingArea   = DBFReadDoubleAttribute(dbfFile, ii, us_cont_arIndex);
          channels[linkNo].downstreamContributingArea = DBFReadDoubleAttribute(dbfFile, ii, ds_cont_arIndex);
          channels[linkNo].upstream[0]                = DBFReadIntegerAttribute(dbfFile, ii, uslinkno1Index);
          channels[linkNo].upstream[1]                = DBFReadIntegerAttribute(dbfFile, ii, uslinkno2Index);
          channels[linkNo].downstream                 = DBFReadIntegerAttribute(dbfFile, ii, dslinknoIndex);
          channels[linkNo].firstElement               = new LinkElementStruct;
          channels[linkNo].lastElement                = channels[linkNo].firstElement;
          channels[linkNo].firstElement->prev         = NULL;
          channels[linkNo].firstElement->next         = NULL;
          channels[linkNo].firstElement->endLocation  = channels[linkNo].length;
          channels[linkNo].firstElement->edge         = -1;
          channels[linkNo].firstElement->movedTo      = -1;
          
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

// Assign an edge in the mesh to an element on a link in the channel network.
// This function  assumes that no links have been split and moved yet so
// movedTo should be -1 for all elements.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels     - The channel network as a 1D array of ChannelLinkStruct.
// size         - The number of elements in channels.
// linkNo       - The link that the mesh edge is on.
// location1    - The distance in meters along the link at one end of the mesh
//                edge.
// location2    - The distance in meters along the link at the other end of the
//                mesh edge.
// edge         - The edge number of the mesh edge.
bool addStreamMeshEdge(ChannelLinkStruct* channels, int size, int linkNo, double location1, double location2, int edge)
{
  bool               error = false;  // Error flag.
  double             tempLocation;   // For swapping locations.
  LinkElementStruct* element1;       // The element at location1.
  LinkElementStruct* element2;       // The element at location2.
  LinkElementStruct* tempElement;    // For looping through elements between element1 and element2.
  LinkElementStruct* nextElement;    // For looping through elements between element1 and element2.
  LinkElementStruct* elementToUse;   // The LinkElementStruct that will be used for an added edge.
  double             element1Length; // Length of element1.
  double             element2Length; // Length of element2.
  double             length;         // Length of the element being added.
  double             afterGap;       // Used to calculate gaps when overlapping elements have to be shifted.
  double             beforeGap;      // Used to calculate gaps when overlapping elements have to be shifted.
  double             newLocation;    // Used for moving location1 or location2 when overlapping elements have to be shifted.
  double             oldEndLocation; // Used for remembering the old end location of an element that is being shifted.
  bool               killElement2;   // Flag to indicate delayed removal of element2.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= linkNo && linkNo < size && STREAM == channels[linkNo].type && 0.0 <= location1 && location1 <= channels[linkNo].length &&
         0.0 <= location2 && location2 <= channels[linkNo].length);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)

  // Ensure that location1 is strictly less than location2.
  if (location1 > location2)
    {
      tempLocation = location1;
      location1    = location2;
      location2    = tempLocation;
    }
  else if (location1 == location2)
    {
      // Move the locations a nominal 2 meters apart, but not past either end of the stream link.
      location1 -= 1.0;
      
      if (0.0 > location1)
        {
          location1 = 0.0;
        }
      
      location2 += 1.0;
      
      if (location2 > channels[linkNo].length)
        {
          location2 = channels[linkNo].length;
        }
    }
  
  // Find the element at location1.
  element1 = channels[linkNo].firstElement;

  while (epsilonLess(element1->endLocation, location1))
    {
      element1 = element1->next; // Move on to the next element.
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // The last element's end location should never be less than location1 so we should be guaranteed that there is another element.
      assert(NULL != element1);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }

  // Find the element at location2.
  element2 = element1;

  while (epsilonLess(element2->endLocation, location2))
    {
      element2 = element2->next; // Move on to the next element.
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // The last element's end location should never be less than location2 so we should be guaranteed that there is another element.
      assert(NULL != element2);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  assert(-1 == element1->movedTo && -1 == element2->movedTo);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  if (element1 == element2)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // The new element fits entirely within a single existing element.
     assert(beginLocation(element1) <= location1 && epsilonGreaterOrEqual(element1->endLocation, location2));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      if (-1 != element1->edge)
        {
          // This element is already associated.  Put the new element at either end so that you don't split the existing element.
          afterGap       = element1->endLocation - location2;
          beforeGap      = location1             - beginLocation(element1);
          element1Length = element1->endLocation - beginLocation(element1);
          length         = location2             - location1;

          if (beforeGap < afterGap)
            {
              // Put the new element at the beginning of the existing element.
              // Calculate the split point that shortens both elements by the same ratio.  For example, each 1/3 shorter.
              newLocation = (beginLocation(element1) * element1Length + element1->endLocation * length) / (element1Length + length);

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              assert(beginLocation(element1) < newLocation && newLocation < element1->endLocation);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              fprintf(stderr, "WARNING in addStreamMeshEdge: Overlap on channel link %d.  Mesh edge %d moved from (%lf to %lf) to (%lf to %lf).  Mesh edge %d "
                              "moved from (%lf to %lf) to (%lf to %lf).\n", linkNo, element1->edge, beginLocation(element1), element1->endLocation,
                      newLocation, element1->endLocation, edge, location1, location2, beginLocation(element1), newLocation);

              error = createLinkElementAfter(channels, size, linkNo, element1->prev, newLocation, edge, -1);
            }
          else
            {
              // Put the new element at the end of the existing element.
              // Calculate the split point that shortens both elements by the same ratio.  For example, each 1/3 shorter.
              newLocation = (element1->endLocation * element1Length + beginLocation(element1) * length) / (element1Length + length);

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              assert(beginLocation(element1) < newLocation && newLocation < element1->endLocation);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              fprintf(stderr, "WARNING in addStreamMeshEdge: Overlap on channel link %d.  Mesh edge %d moved from (%lf to %lf) to (%lf to %lf).  Mesh edge %d "
                              "moved from (%lf to %lf) to (%lf to %lf).\n", linkNo, element1->edge, beginLocation(element1), element1->endLocation,
                      beginLocation(element1), newLocation, edge, location1, location2, newLocation, element1->endLocation);

              oldEndLocation        = element1->endLocation;
              element1->endLocation = newLocation;
              error                 = createLinkElementAfter(channels, size, linkNo, element1, oldEndLocation, edge, -1);
            }
        }
      else // if (-1 == element1->edge)
        {
          // This element is unassociated.
          
          if (epsilonEqual(location1, beginLocation(element1)) && epsilonEqual(location2, element1->endLocation))
            {
              // The new element takes up all of the unassociated space.
              element1->edge = edge;
            }
          else if (epsilonEqual(location1, beginLocation(element1)))
            {
              // The new element is at the beginning of the unassociated space.
              error = createLinkElementAfter(channels, size, linkNo, element1->prev, location2, edge, -1);
            }
          else if (epsilonEqual(location2, element1->endLocation))
            {
              // The new element is at the end of the unassociated space.
              location2             = element1->endLocation;
              element1->endLocation = location1;
              error                 = createLinkElementAfter(channels, size, linkNo, element1, location2, edge, -1);
            }
          else
            {
              // The new element is in the middle of the unassociated space.
              error = createLinkElementAfter(channels, size, linkNo, element1->prev, location1, element1->edge, element1->movedTo);
              
              if (!error)
                {
                  error = createLinkElementAfter(channels, size, linkNo, element1->prev, location2, edge, -1);
                }
            }
        }
    }
  else // if (element1 != element2)
    {
      // The new element spans across multiple existing elements.
      
      // Deal with occluded elements.
      tempElement = element1->next;

      while (tempElement != element2)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          assert(-1 == tempElement->movedTo);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          if (-1 != tempElement->edge)
            {
              // tempElement is an associated occluded element.
              fprintf(stderr, "WARNING in addStreamMeshEdge: Occlusion on channel link %d.  Mesh edge %d from %lf to %lf occluded by mesh edge %d from %lf to "
                              "%lf.\n", linkNo, tempElement->edge, beginLocation(tempElement), tempElement->endLocation, edge, location1, location2);
              
              // FIXLATER We could deal with the occluded element in a more sophisticated way.  For example, we could find the largest unallocated gap.
              // But for now, just move the insertion region past the occluded element by setting element1 to the occluded element.
              element1 = tempElement;
              location1 = beginLocation(element1);
            }

          tempElement = tempElement->next; // Move on to the next element.
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // We should eventually reach element2 so we should be guaranteed that there is another element.
          assert(NULL != tempElement);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

      // At this point any elements between element1 and element2 are unassociated.  element1 and element2 themselves may be associated.
      
      elementToUse = NULL;  // The LinkElementStruct that will be used for the added edge.  NULL until that struct is found.
      killElement2 = false; // Flag to indicate delayed removal of element2.
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      assert(beginLocation(element1) <= location1 && epsilonGreaterOrEqual(element2->endLocation, location2));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      // Deal with possible overlap of element1.
      if (-1 != element1->edge)
        {
          // element1 is associated.
          if (epsilonEqual(location1, element1->endLocation))
            {
              // The new edge starts at the end of element1.  Nothing to do.
            }
          else
            {
              // The new edge overlaps element1.  Chop off element1.
              element1Length = element1->endLocation - beginLocation(element1);
              length         = location2             - location1;
              
              // Calculate the split point that shortens both elements by the same ratio.  For example, each 1/3 shorter.
              newLocation = (location1 * element1Length + element1->endLocation * length) / (element1Length + length);
              
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              assert(location1 < newLocation && newLocation < element1->endLocation);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              fprintf(stderr, "WARNING in addStreamMeshEdge: Overlap on channel link %d.  Mesh edge %d moved from (%lf to %lf) to (%lf to %lf).  Mesh edge %d "
                              "moved from (%lf to %lf) to (%lf to %lf).\n", linkNo, element1->edge, beginLocation(element1), element1->endLocation,
                      beginLocation(element1), newLocation, edge, location1, location2, newLocation, location2);

              element1->endLocation = newLocation;
              location1             = newLocation;
            }
        }
      else // if (-1 == element1->edge)
        {
          // element1 is not associated
          if (epsilonEqual(location1, element1->endLocation))
            {
              // The new edge starts at the end of element1.  Nothing to do.
            }
          else if (epsilonEqual(location1, beginLocation(element1)))
            {
              // The new edge starts at the beginning of element1.  Use element1 as the struct.
              elementToUse = element1;
            }
          else
            {
              // The new edge overlaps element1.  Chop off element1.
              element1->endLocation = location1;
            }
        }
      
      // At this point location1 points to the final begin location of the LinkElementStruct for the new edge.  Also, the endLocation value of the struct
      // that will be previous to the LinkElementStruct for the new edge is set to location1.  This might be element1->endLocation, or it might be
      // element1->prev->endLocation if element1 is going to be used for the struct, or it might be implicit if location1 is zero and element1->prev is NULL.
      
      // Deal with possible overlap of element2.
      if (-1 != element2->edge)
        {
          // element2 is associated.
          if (epsilonEqual(location2, beginLocation(element2)))
            {
              // The new edge ends at the beginning of element2.  Nothing to do.
            }
          else
            {
              // The new edge overlaps element2.  Chop off element2.
              element2Length = element2->endLocation - beginLocation(element2);
              length         = location2             - location1;
              
              // Calculate the split point that shortens both elements by the same ratio.  For example, each 1/3 shorter.
              newLocation = (location2 * element2Length + beginLocation(element2) * length) / (element2Length + length);
              
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              assert(beginLocation(element2) < newLocation && newLocation < location2);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              fprintf(stderr, "WARNING in addStreamMeshEdge: Overlap on channel link %d.  Mesh edge %d moved from (%lf to %lf) to (%lf to %lf).  Mesh edge %d "
                              "moved from (%lf to %lf) to (%lf to %lf).\n", linkNo, element2->edge, beginLocation(element2), element2->endLocation,
                      newLocation, element2->endLocation, edge, location1, location2, location1, newLocation);

              location2 = newLocation;
            }
        }
      else // if (-1 == element2->edge)
        {
          // element2 is not associated
          if (epsilonEqual(location2, beginLocation(element2)))
            {
              // The new edge ends at the beginning of element2.  Nothing to do.
            }
          else if (epsilonEqual(location2, element2->endLocation))
            {
              // The new edge ends at the end of element2.
              // Use element2 as the struct unless element1 is already being used as the struct in which case we need to kill element2.
              if (NULL == elementToUse)
                {
                  elementToUse = element2;
                }
              else
                {
                  killElement2 = true; // Need to leave element2 in place for now to mark the end of the elements between 1 and 2.  Will kill later.
                }
            }
          else
            {
              // The new edge overlaps element2.  The beginning of element2 will get chopped off at location2.
            }
        }
      
      // At this point location2 points to the final end location of the LinkElementStruct for the new edge.  The endLocation value of the LinkElementStruct
      // for the new edge will be set to location2 below after we are sure we have a struct to use.  The begin location of the struct that will follow that
      // struct will be set at that time.
      
      // Remove any structs between element1 and element2, except use one of them if we don't have a struct to use yet.
      tempElement = element1->next;

      while (!error && tempElement != element2)
        {
          nextElement = tempElement->next;
          
          if (NULL == elementToUse)
            {
              elementToUse = tempElement;
            }
          else
            {
              error = killLinkElement(channels, size, linkNo, tempElement);
            }
          
          tempElement = nextElement;
        }
      
      // Get rid of element2 if necessary.
      if (!error && killElement2)
        {
          error = killLinkElement(channels, size, linkNo, element2);
        }

      if (!error)
        {
          if (NULL == elementToUse)
            {
              // Create an element if we haven't already found one to use.
              error = createLinkElementAfter(channels, size, linkNo, element1, location2, edge, -1);
            }
          else
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              assert(-1 == elementToUse->edge && -1 == elementToUse->movedTo);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              // Otherwise use the one we have.
              elementToUse->endLocation = location2;
              elementToUse->edge        = edge;
            }
        }
    } // End if (element1 != element2).

  return error;
}

// Assign all of the stream edges in a mesh to link elements in the channel
// network.  This function assumes that no links have been split and moved yet
// so movedTo should be -1 for all elements.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels     - The channel network as a 1D array of ChannelLinkStruct.
// size         - The number of elements in channels.
// nodeFilename - The name of the .node file to read.
// edgeFilename - The name of the .edge file to read.
bool addAllStreamMeshEdges(ChannelLinkStruct* channels, int size, const char* nodeFilename, const char* edgeFilename)
{
  bool    error = false;      // Error flag.
  int     ii;                 // Loop counter.
  FILE*   nodeFile;           // For opening the .node file.
  FILE*   edgeFile;           // For opening the .edge file.
  int     numScanned;         // Used to check that fscanf scanned all of the requested values.
  int     numberOfNodes;      // The number of nodes in the .node file.
  int     numberOfEdges;      // The number of edges in the .edge file.
  int     numberOfAttributes; // The number of attributes in the .node file.
  int     dimension;          // Used to check the dimensions in the files.
  int     boundary;           // Used to check the number of boundary markers in the files.
  int     index;              // Used to read node numbers and edge numbers.
  int     firstIndex;         // Used to store whether the file is zero based or one based.
  double  xCoordinate;        // Used to read X coordinates.
  double  yCoordinate;        // Used to read Y coordinates.
  double* nodesX;             // X coordinates of nodes.
  double* nodesY;             // Y coordinates of nodes.
  int     vertex1;            // The node number of one of the vertices of an edge.
  int     vertex2;            // The node number of one of the vertices of an edge.
  double  location1;          // The 1D location of vertex1 in meters along the link.
  double  location2;          // The 1D location of vertex2 in meters along the link.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && NULL != nodeFilename && NULL != edgeFilename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Open the files.
  nodeFile = fopen(nodeFilename, "r");
  edgeFile = fopen(edgeFilename, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NULL != nodeFile))
    {
      fprintf(stderr, "ERROR in addAllStreamMeshEdges: Could not open node file %s.\n", nodeFilename);
      error = true;
    }
  
  if (!(NULL != edgeFile))
    {
      fprintf(stderr, "ERROR in addAllStreamMeshEdges: Could not open edge file %s.\n", edgeFilename);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

  // Read the number of nodes from the file.
  if (!error)
    {
      numScanned = fscanf(nodeFile, "%d %d %d %d", &numberOfNodes, &dimension, &numberOfAttributes, &boundary);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(4 == numScanned))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Unable to read header from node file %s.\n", nodeFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfNodes && 2 == dimension && 0 == numberOfAttributes && 1 == boundary))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Invalid header in node file %s.\n", nodeFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Allocate array.
  if (!error)
    {
      nodesX = new double[numberOfNodes];
      nodesY = new double[numberOfNodes];
    }
  else
    {
      nodesX = NULL;
      nodesY = NULL;
    }

  for (ii = 0; !error && ii < numberOfNodes; ii++)
    {
      // Read node file.
      numScanned = fscanf(nodeFile, "%d %lf %lf %*d", &index, &xCoordinate, &yCoordinate);
      
      // Set zero based or one based indices.
      if (0 == ii)
        {
          if (0 == index)
            {
              firstIndex = 0;
            }
          else
            {
              firstIndex = 1;
            }
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(3 == numScanned))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Unable to read entry %d from node file.\n", ii + firstIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii + firstIndex == index))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Invalid node number in node file.  %d should be %d.\n", index, ii + firstIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      if (!error)
        {
          // The array is zero based, but the node file might be one based.  Use mod rather than minus so that only one node gets renumbered, the last one.
          nodesX[index % numberOfNodes] = xCoordinate;
          nodesY[index % numberOfNodes] = yCoordinate;
        }
    }

  // Read the number of edges from the file.
  if (!error)
    {
      numScanned = fscanf(edgeFile, "%d %d", &numberOfEdges, &boundary);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Unable to read header from edge file %s.\n", edgeFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfEdges && 1 == boundary))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Invalid header in edge file %s.\n", edgeFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  for (ii = 0; !error && ii < numberOfEdges; ii++)
    {
      // Read edge file.
      numScanned = fscanf(edgeFile, "%d %d %d %d", &index, &vertex1, &vertex2, &boundary);
      
      // Set zero based or one based indices.
      if (0 == ii)
        {
          if (0 == index)
            {
              firstIndex = 0;
            }
          else
            {
              firstIndex = 1;
            }
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(4 == numScanned))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Unable to read entry %d from edge file.\n", ii + firstIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii + firstIndex == index))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Invalid edge number in edge file.  %d should be %d.\n", index, ii + firstIndex);
          error = true;
        }
      
      if (!(0 <= vertex1 && vertex1 <= numberOfNodes))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Invalid vertex number %d in edge file index %d.\n", vertex1, index);
          error = true;
        }
      
      if (!(0 <= vertex2 && vertex2 <= numberOfNodes))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Invalid vertex number %d in edge file index %d.\n", vertex2, index);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      // A neagtive boundary code indicates a channel edge.
      if (!error && 0 > boundary)
        {
          // Convert to positive link number.
          boundary *= -1;
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(boundary <= size))
            {
              fprintf(stderr, "ERROR in addAllStreamMeshEdges: Mesh edge linked to invalid channel link number %d.\n", boundary);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          
          // The array is zero based, but the link numbers might be one based.  Use mod rather than minus so that only one link gets renumbered, the last one.
          boundary %= size;
          
          // The array is zero based, but the node file might be one based.  Use mod rather than minus so that only one node gets renumbered, the last one.
          vertex1 %= numberOfNodes;
          vertex2 %= numberOfNodes;
          
          if (!error && STREAM == channels[boundary].type)
            {
              location1 = getLocation(channels[boundary].shapes[0], channels[boundary].length, nodesX[vertex1], nodesY[vertex1]);
              location2 = getLocation(channels[boundary].shapes[0], channels[boundary].length, nodesX[vertex2], nodesY[vertex2]);
              
              error = addStreamMeshEdge(channels, size, boundary, location1, location2, index);
            }
        }
    }

  // Deallocate node arrays.
  if (NULL != nodesX)
    {
      delete[] nodesX;
    }
  
  if (NULL != nodesY)
    {
      delete[] nodesY;
    }
  
  // Close the files.
  if (NULL != nodeFile)
    {
      fclose(nodeFile);
    }

  if (NULL != edgeFile)
    {
      fclose(edgeFile);
    }

  return error;
}

// FIXME remove
int main(void)
{
  bool               error = false;
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
  
  if (!error)
    {
      error = addAllStreamMeshEdges(channels, size, "/share/CI-WATER Simulation Data/small_green_mesh/mesh.1.node",
                                    "/share/CI-WATER Simulation Data/small_green_mesh/mesh.1.edge");
    }
  
  // FIXME clean up channels.
  
  return 0;
}
