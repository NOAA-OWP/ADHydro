#include "all.h"
#include <shapefil.h>
#include <assert.h>

// This code is a separate executable used to pre-process the channel network
// into a .chan.ele file and a .chan.node file that are read into the adhydro
// executable by the file managers.  In this code the channel network is
// represented as a tree of links including TauDEM stream links and NHD
// waterbodies.  The tree of links is stored as an array of ChannelLinkStruct
// with array indices instead of pointers for the tree connections.  The link
// number of each link is the array index where it is stored.  Link numbers are
// not contiguous in the TauDEM output so there may be some elements in the
// array that are unused.  The gaps in the TauDEM link numbers are used for
// waterbodies.

#define SHAPES_SIZE     (2)  // Size of array of shapes in ChannelLinkStruct.
#define UPSTREAM_SIZE   (16) // Size of array of upstream links in ChannelLinkStruct.
#define DOWNSTREAM_SIZE (3)  // Size of array of downstream links in ChannelLinkStruct.

// A linkElementStruct represents a section of a stream link.  The section
// goes from prev->endLocation to endLocation along the 1D length of the link
// or from zero to endLocation if prev is NULL.  For the last unmoved element
// in the list endLocation should always be the length of the link.  The
// element is associated with an edge number or is unassociated if edge is -1.
// The edge numbers are from the triangle .edge file and later they are further
// associated with mesh element edges.  movedTo indicates if this link was
// split and part of the link was moved to a new link number.  For a
// description of how splitting and moving links works see the comment of the
// function splitLink.
//
// These structures are also used in waterbody links to store lists of
// intersections with other links.  The link number of the other link is stored
// in edge.  If the other link is a stream the 1D location of the intersection
// along the stream is stored in endLocation, and whether the stream is
// upstream or downstream of the waterbody is stored in movedTo.
typedef struct LinkElementStruct LinkElementStruct;
struct LinkElementStruct
{
  LinkElementStruct* prev;        // The previous element of this link or NULL if this is the first element.
  LinkElementStruct* next;        // The next     element of this link or NULL if this is the last  element.
  double             endLocation; // The 1D location in meters along the link of the end of this element.
  int                edge;        // The mesh edge associated with this link element or -1 if unassociated.
  int                movedTo;     // The new link number that this section of this link has been moved to or -1 if unmoved.
};

// The channel network is stored as a 1D array of ChannelLinkStruct.  It is
// actually a tree stored in an array using array indices instead of pointers
// for the tree connections.  The reason it is done this way is because we know
// the size of the array and an index number for each link from the input
// files, and references to other links are stored as index numbers in the
// input files so it is just easier to do it this way.
typedef struct
{
  ChannelTypeEnum    type;                        // The type of the link.
  long long          reachCode;                   // For waterbodies, reach code.  For streams, original link number.  If a stream is split this will indicate
                                                  // the original link number of the unsplit stream.  For unused, -1.
  SHPObject*         shapes[SHAPES_SIZE];         // Shape object(s) of the link.  Streams will have only one.  Waterbodies may have more than one if they are
                                                  // multipart.  For moved streams and unused, NULL.
  double             length;                      // Meters.
  double             upstreamContributingArea;    // For unmoved streams, contributing area in square meters at   upstream end of shape.  For others, 0.0.
  double             downstreamContributingArea;  // For unmoved streams, contributing area in square meters at downstream end of shape.  For others, 0.0.
  int                upstream[UPSTREAM_SIZE];     // Array indices of links upstream from this link or boundary condition codes.
  int                downstream[DOWNSTREAM_SIZE]; // Array indices of links downstream from this link or boundary condition codes.
  LinkElementStruct* firstElement;                // Sections of the link associated with mesh edges or NULL if no elements.
  LinkElementStruct* lastElement;                 // Sections of the link associated with mesh edges or NULL if no elements.
  int                elementStart;                // The first channel element number on this link after assigning global element numbers.
  int                numberOfElements;            // The number of channel elements on this link after assigning global element numbers.
} ChannelLinkStruct;

// Returns: The begin location of a link element.
// 
// Parameters:
//
// element - The element to find the begin location of.
double beginLocation(LinkElementStruct* element)
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

// Create a new LinkElementStruct in the channel network in the given link
// number.  The LinkElementStruct is placed after prevLinkElement in the linked
// list or at the head of the linked list if prevLinkElement is NULL.
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
void createLinkElementAfter(ChannelLinkStruct* channels, int size, int linkNo, LinkElementStruct* prevLinkElement, double endLocation, int edge, int movedTo)
{
  LinkElementStruct* newLinkElement; // To point to the created LinkElementStruct.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= linkNo && linkNo < size);
  
  if (STREAM == channels[linkNo].type)
    {
      if (NULL == prevLinkElement)
        {
          assert(0.0 < endLocation || 0.0 == channels[linkNo].length);

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
}

// Remove a LinkElementStruct from its linked list and deallocate its memory.
//
// Parameters:
//
// channels          - The channel network as a 1D array of ChannelLinkStruct.
// size              - The number of elements in channels.
// linkNo            - The link to remove the LinkElementStruct from.
// linkElementToKill - The LinkElementStruct that will be removed from its
//                     linked list and deallocated.
void killLinkElement(ChannelLinkStruct* channels, int size, int linkNo, LinkElementStruct* linkElementToKill)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= linkNo && linkNo < size && NULL != linkElementToKill);
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
}

// Calculate the length of a stream link polyline and the 1D distance in meters
// along the polyline of the point on the polyline closest to a given point.
// 
// Parameters:
// 
// link     - The stream link polyline.
// x(y)     - The X and Y coordinates in meters of the point to find the
//            closest location to on the polyline.
// length   - Scalar passed by reference will be filled in with the length of
//            the polyline, or ignored if NULL.
// location - Scalar passed by reference will be filled in with the 1D distance
//            in meters along the polyline of the point on the polyline closest
//            to (x, y), or ignored if NULL.
void getLengthAndLocation(SHPObject* link, double x, double y, double* length, double* location)
{
  int    ii;              // Loop counter.
  double minDistance;     // The closest distance found so far from (x, y) to link.
  double tempLength;      // The length of line segments processed so far.
  double tempLocation;    // The 1D distance along link where minDistance occurs.
  double segmentLength;   // The length of a line segment in the polyline.
  double closestLocation; // The distance in meters along the current line segment of the closest location on the current line segment.
  double x_prime;         // The point on the line segment closest to (x, y).
  double y_prime;         // The point on the line segment closest to (x, y).
  double distance;        // The distance from (x', y') to (x, y).
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != link);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Start with the most upstream vertex.  In the TauDEM stream network shapefile the beginning
  // of the polyline is the downstream end so start with the last vertex in the polyline.
  ii           = link->nVertices - 1;
  minDistance  = sqrt((x - link->padfX[ii]) * (x - link->padfX[ii]) + (y - link->padfY[ii]) * (y - link->padfY[ii]));
  tempLength   = 0.0;
  tempLocation = 0.0;

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
              minDistance  = distance;
              tempLocation = tempLength + closestLocation;
            }
        }

      // Add the line segment to the length processed so far.
      tempLength += segmentLength;

      // Check the vertex ii.
      distance = sqrt((x - link->padfX[ii]) * (x - link->padfX[ii]) + (y - link->padfY[ii]) * (y - link->padfY[ii]));

      if (minDistance > distance)
        {
          minDistance  = distance;
          tempLocation = tempLength;
        }
    }
  
  if (NULL != length)
    {
      *length = tempLength;
    }
  
  if (NULL != location)
    {
      *location = tempLocation;
    }
}

// Add upstreamLinkNo to the upstream connection list of downstreamLinkNo.
//
// Returns: true if there is an error, false otherwise.  If there is an error
// no connections are modified.
//
// Parameters:
//
// channels         - The channel network as a 1D array of ChannelLinkStruct.
// size             - The number of elements in channels.
// upstreamLinkNo   - The link or boundary condition code to add to the
//                    upstream connection list of downstreamLinkNo.
// downstreamLinkNo - The link that upstreamLinkNo will be added to, or a
//                    boundary condition code in which case nothing is done.
bool addUpstreamConnection(ChannelLinkStruct* channels, int size, int upstreamLinkNo, int downstreamLinkNo)
{
  bool error = false; // Error flag.
  int  ii;            // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && (isBoundary(upstreamLinkNo) || (0 <= upstreamLinkNo && upstreamLinkNo < size)) &&
         (isBoundary(downstreamLinkNo) || (0 <= downstreamLinkNo && downstreamLinkNo < size)));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  if (!isBoundary(downstreamLinkNo))
    {
      // Find an empty slot in the upstream connection list of downstreamLinkNo.
      ii = 0;
      
      while (UPSTREAM_SIZE > ii && NOFLOW != channels[downstreamLinkNo].upstream[ii] && upstreamLinkNo != channels[downstreamLinkNo].upstream[ii])
        {
          ii++;
        }
      
      if (UPSTREAM_SIZE > ii)
        {
          channels[downstreamLinkNo].upstream[ii] = upstreamLinkNo;
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          fprintf(stderr, "ERROR in addUpstreamConnection: Channel link %d has more than the maximum allowable %d upstream connections.\n", downstreamLinkNo,
                  UPSTREAM_SIZE);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  return error;
}

// Remove upstreamLinkNo from the upstream connection list of downstreamLinkNo.
//
// Parameters:
//
// channels         - The channel network as a 1D array of ChannelLinkStruct.
// size             - The number of elements in channels.
// upstreamLinkNo   - The link or boundary condition code to remove from the
//                    upstream connection list of downstreamLinkNo.
// downstreamLinkNo - The link that upstreamLinkNo will be removed from, or a
//                    boundary condition code in which case nothing is done.
void removeUpstreamConnection(ChannelLinkStruct* channels, int size, int upstreamLinkNo, int downstreamLinkNo)
{
  int ii; // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && (isBoundary(upstreamLinkNo) || (0 <= upstreamLinkNo && upstreamLinkNo < size)) &&
         (isBoundary(downstreamLinkNo) || (0 <= downstreamLinkNo && downstreamLinkNo < size)));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  if (!isBoundary(downstreamLinkNo))
    {
      // Find upstreamLinkNo in the upstream connection list of downstreamLinkNo.
      ii = 0;

      while (UPSTREAM_SIZE > ii && NOFLOW != channels[downstreamLinkNo].upstream[ii] && upstreamLinkNo != channels[downstreamLinkNo].upstream[ii])
        {
          ii++;
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      // It is an error if the upstream connection is not found.
      assert(UPSTREAM_SIZE > ii && upstreamLinkNo == channels[downstreamLinkNo].upstream[ii]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)

      // Remove upstreamLinkNo from the list and move all other upstream connections forward to fill the gap.
      while (UPSTREAM_SIZE - 1 > ii && NOFLOW != channels[downstreamLinkNo].upstream[ii + 1])
        {
          channels[downstreamLinkNo].upstream[ii] = channels[downstreamLinkNo].upstream[ii + 1];
          ii++;
        }

      // Fill in the last connection with NOFLOW.
      channels[downstreamLinkNo].upstream[ii] = NOFLOW;
    }
}

// Add downstreamLinkNo to the downstream connection list of upstreamLinkNo.
//
// Returns: true if there is an error, false otherwise.  If there is an error
// no connections are modified.
//
// Parameters:
//
// channels         - The channel network as a 1D array of ChannelLinkStruct.
// size             - The number of elements in channels.
// upstreamLinkNo   - The link that downstreamLinkNo will be added to, or a
//                    boundary condition code in which case nothing is done.
// downstreamLinkNo - The link or boundary condition code to add to the
//                    downstream connection list of upstreamLinkNo.
bool addDownstreamConnection(ChannelLinkStruct* channels, int size, int upstreamLinkNo, int downstreamLinkNo)
{
  bool error = false; // Error flag.
  int  ii;            // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && (isBoundary(upstreamLinkNo) || (0 <= upstreamLinkNo && upstreamLinkNo < size)) &&
         (isBoundary(downstreamLinkNo) || (0 <= downstreamLinkNo && downstreamLinkNo < size)));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  if (!isBoundary(upstreamLinkNo))
    {
      // Find an empty slot in the downstream connection list of upstreamLinkNo.
      ii = 0;
      
      while (DOWNSTREAM_SIZE > ii && NOFLOW != channels[upstreamLinkNo].downstream[ii] && downstreamLinkNo != channels[upstreamLinkNo].downstream[ii])
        {
          ii++;
        }
      
      if (DOWNSTREAM_SIZE > ii)
        {
          channels[upstreamLinkNo].downstream[ii] = downstreamLinkNo;
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          fprintf(stderr, "ERROR in addDownstreamConnection: Channel link %d has more than the maximum allowable %d downstream connections.\n", upstreamLinkNo,
                  DOWNSTREAM_SIZE);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  return error;
}

// Remove downstreamLinkNo from the downstream connection list of upstreamLinkNo.
//
// Parameters:
//
// channels         - The channel network as a 1D array of ChannelLinkStruct.
// size             - The number of elements in channels.
// upstreamLinkNo   - The link that downstreamLinkNo will be removed from, or a
//                    boundary condition code in which case nothing is done.
// downstreamLinkNo - The link or boundary condition code to remove from the
//                    downstream connection list of upstreamLinkNo.
void removeDownstreamConnection(ChannelLinkStruct* channels, int size, int upstreamLinkNo, int downstreamLinkNo)
{
  int ii; // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && (isBoundary(upstreamLinkNo) || (0 <= upstreamLinkNo && upstreamLinkNo < size)) &&
         (isBoundary(downstreamLinkNo) || (0 <= downstreamLinkNo && downstreamLinkNo < size)));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  if (!isBoundary(upstreamLinkNo))
    {
      // Find downstreamLinkNo in the downstream connection list of upstreamLinkNo.
      ii = 0;

      while (DOWNSTREAM_SIZE > ii && NOFLOW != channels[upstreamLinkNo].downstream[ii] && downstreamLinkNo != channels[upstreamLinkNo].downstream[ii])
        {
          ii++;
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      // It is an error if the downstream connection is not found.
      assert(DOWNSTREAM_SIZE > ii && downstreamLinkNo == channels[upstreamLinkNo].downstream[ii]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)

      // Remove downstreamLinkNo from the list and move all other downstream connections forward to fill the gap.
      while (DOWNSTREAM_SIZE - 1 > ii && NOFLOW != channels[upstreamLinkNo].downstream[ii + 1])
        {
          channels[upstreamLinkNo].downstream[ii] = channels[upstreamLinkNo].downstream[ii + 1];
          ii++;
        }

      // Fill in the last connection with NOFLOW.
      channels[upstreamLinkNo].downstream[ii] = NOFLOW;
    }
}

// Add upstreamLinkNo to the upstream connection list of downstreamLinkNo and
// downstreamLinkNo to the downstream connection list of upstreamLinkNo.
//
// Returns: true if there is an error, false otherwise.  If there is an error
// no connections are modified.
//
// Parameters:
//
// channels         - The channel network as a 1D array of ChannelLinkStruct.
// size             - The number of elements in channels.
// upstreamLinkNo   - The link or boundary condition code that will be
//                    connected to downstreamLinkNo.
// downstreamLinkNo - The link or boundary condition code that will be
//                    connected to upstreamLinkNo.
bool addUpstreamDownstreamConnection(ChannelLinkStruct* channels, int size, int upstreamLinkNo, int downstreamLinkNo)
{
  bool error = false; // Error flag.
  
  error = addUpstreamConnection(channels, size, upstreamLinkNo, downstreamLinkNo);
  
  if (!error)
    {
      error = addDownstreamConnection(channels, size, upstreamLinkNo, downstreamLinkNo);
      
      if (error)
        {
          // Must remove upstream connection to enforce guarantee that on error no connections are modified.
          removeUpstreamConnection(channels, size, upstreamLinkNo, downstreamLinkNo);
        }
    }
  
  return error;
}

// Remove upstreamLinkNo from the upstream connection list of downstreamLinkNo
// and downstreamLinkNo from the downstream connection list of upstreamLinkNo.
//
// Parameters:
//
// channels         - The channel network as a 1D array of ChannelLinkStruct.
// size             - The number of elements in channels.
// upstreamLinkNo   - The link or boundary condition code that will be
//                    disconnected from downstreamLinkNo.
// downstreamLinkNo - The link or boundary condition code that will be
//                    disconnected from upstreamLinkNo.
void removeUpstreamDownstreamConnection(ChannelLinkStruct* channels, int size, int upstreamLinkNo, int downstreamLinkNo)
{
  removeUpstreamConnection(channels, size, upstreamLinkNo, downstreamLinkNo);
  removeDownstreamConnection(channels, size, upstreamLinkNo, downstreamLinkNo);
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
  int                ii;      // Loop counter.
  bool               prune;   // Whether to prune.
  LinkElementStruct* element; // For checking if elements are unassociated.
  int                linkNo2; // For storing a link that must be connected to.
  
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
          for (ii = 0; DOWNSTREAM_SIZE > ii && NOFLOW != channels[linkNo].downstream[ii]; ii++)
            {
              linkNo2 = channels[linkNo].downstream[ii];
              removeUpstreamDownstreamConnection(channels, size, linkNo, linkNo2);
              tryToPruneLink(channels, size, linkNo2);
            }
          
          channels[linkNo].type = PRUNED_STREAM;
        }
    }
}

// Used for the return value of upstreamDownstream.
typedef enum
{
  COINCIDENT, // The two intersections are at the same location.
  UPSTREAM,   // intersection2 is upstream   of intersection1.
  DOWNSTREAM, // intersection2 is downstream of intersection1.
  UNRELATED,  // Neither is upstream or downstream of the other.
} UpstreamDownstreamEnum;

// Return the upstream/downstream relationship between two intersections.
// This function assumes that each stream link has at most one downstream
// connection, which is true before linking waterbodies to streams, which is
// where this is run in the code.
//
// Parameters:
//
// channels      - The channel network as a 1D array of ChannelLinkStruct.
// size          - The number of elements in channels.
// intersection1 - The first intersection.
// intersection2 - The second intersection.
UpstreamDownstreamEnum getUpstreamDownstream(ChannelLinkStruct* channels, int size, LinkElementStruct* intersection1, LinkElementStruct* intersection2)
{
  UpstreamDownstreamEnum returnValue = UNRELATED; // Used to store the return value of the function.
  int                    linkNo;                  // Used to search upstream and downstream links.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && NULL != intersection1 && NULL != intersection2);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANT)
  assert(NULL != intersection1 && 0 <= intersection1->edge && intersection1->edge < size && STREAM == channels[intersection1->edge].type &&
         0.0 <= intersection1->endLocation && intersection1->endLocation <= channels[intersection1->edge].lastElement->endLocation &&
         (COINCIDENT == intersection1->movedTo || UPSTREAM == intersection1->movedTo || DOWNSTREAM == intersection1->movedTo ||
          UNRELATED == intersection1->movedTo) &&
         NULL != intersection2 && 0 <= intersection2->edge && intersection2->edge < size && STREAM == channels[intersection2->edge].type &&
         0.0 <= intersection2->endLocation && intersection2->endLocation <= channels[intersection2->edge].lastElement->endLocation &&
         (COINCIDENT == intersection2->movedTo || UPSTREAM == intersection2->movedTo || DOWNSTREAM == intersection2->movedTo ||
          UNRELATED == intersection2->movedTo));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANT)
  
  // If they are on the same link the 1D location determines the relationship.
  if (intersection1->edge == intersection2->edge)
    {
      if (intersection1->endLocation == intersection2->endLocation)
        {
          returnValue = COINCIDENT;
        }
      else if (intersection1->endLocation > intersection2->endLocation)
        {
          returnValue = UPSTREAM;
        }
      else // if (intersection1->endLocation < intersection2->endLocation)
        {
          returnValue = DOWNSTREAM;
        }
    }
  else
    {
      // Search downstream of intersection2 to find intersection1.
      linkNo = channels[intersection2->edge].downstream[0];
      
      while (UNRELATED == returnValue && !isBoundary(linkNo))
        {
          if (linkNo == intersection1->edge)
            {
              returnValue = UPSTREAM;
            }
          
          linkNo = channels[linkNo].downstream[0];
        }
      
      // Search downstream of intersection1 to find intersection2.
      linkNo = channels[intersection1->edge].downstream[0];
      
      while (UNRELATED == returnValue && !isBoundary(linkNo))
        {
          if (linkNo == intersection2->edge)
            {
              returnValue = DOWNSTREAM;
            }
          
          linkNo = channels[linkNo].downstream[0];
        }
    }
  
  return returnValue;
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
  bool      error = false; // Error flag.
  int       ii, jj;        // Loop counters.
  FILE*     linkFile;      // The link file to read from.
  int       numScanned;    // Used to check that fscanf scanned all of the requested values.
  int       linkNo;        // Used to read the link numbers in the file and check that they are sequential.
  int       dimension;     // Used to check that the dimension is 1.
  long long reachCode;     // Used to read the reach codes of the links.
  
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
      (*channels)[ii].upstreamContributingArea   = 0.0;
      (*channels)[ii].downstreamContributingArea = 0.0;
      
      for (jj = 0; jj < UPSTREAM_SIZE; jj++)
        {
          (*channels)[ii].upstream[jj] = NOFLOW;
        }
      
      for (jj = 0; jj < DOWNSTREAM_SIZE; jj++)
        {
          (*channels)[ii].downstream[jj] = NOFLOW;
        }
      
      (*channels)[ii].firstElement     = NULL;
      (*channels)[ii].lastElement      = NULL;
      (*channels)[ii].elementStart     = 0;
      (*channels)[ii].numberOfElements = 0;
      
      // Fill in the reach codes from the file.
      numScanned = fscanf(linkFile, "%d %lld", &linkNo, &reachCode);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          fprintf(stderr, "ERROR in readLink: Unable to read entry %d from link file %s.\n", ii, filename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == linkNo))
        {
          fprintf(stderr, "ERROR in readLink: Invalid link number in link file %s.  %d should be %d.\n", filename, linkNo, ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      // Save the reach code
      (*channels)[linkNo].reachCode = reachCode;
    }
  
  // Deallocate channels and set size to zero on error.
  if (error)
    {
      deleteArrayIfNonNull(channels);
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
  int             numScanned;     // Used to check that sscanf scanned all of the requested values.
  int             numberOfShapes; // Number of shapes in the shapefile.
  int             reachCodeIndex; // Index of metadata field.
  int             permanentIndex; // Index of metadata field.
  int             ftypeIndex;     // Index of metadata field.
  long long       reachCode;      // The reach code of the waterbody.
  long long       permanent;      // The permanent code of the waterbody.
  const char*     ftype;          // The type of the waterbody as a string.
  int             linkNo;         // The link number of the waterbody.
  ChannelTypeEnum linkType;       // The type of the waterbody as an enum.

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
      reachCodeIndex = DBFGetFieldIndex(dbfFile, "ReachCode");
      permanentIndex = DBFGetFieldIndex(dbfFile, "Permanent_");
      ftypeIndex     = DBFGetFieldIndex(dbfFile, "FTYPE");
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfShapes))
        {
          // Only warn because you could have zero waterbodies.
          fprintf(stderr, "WARNING in readWaterbodies: Zero shapes in dbf file %s.\n", fileBasename);
        }
      
      if (!(-1 != reachCodeIndex))
        {
          fprintf(stderr, "ERROR in readWaterbodies: Could not find field ReachCode in dbf file %s.\n", fileBasename);
          error = true;
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
      ftype      = DBFReadStringAttribute(dbfFile, ii, reachCodeIndex);
      numScanned = sscanf(ftype, "%lld", &reachCode);

      // Some shapes don't have a reach code, in which case we use the permanent code, so it is not an error if you can't scan a reach code.
      if (1 != numScanned)
        {
          reachCode = -1;
        }

      ftype      = DBFReadStringAttribute(dbfFile, ii, permanentIndex);
      numScanned = sscanf(ftype, "%lld", &permanent);

      // Some shapes don't have a permanent code, in which case we use the reach code, so it is not an error if you can't scan a permanent code.
      if (1 != numScanned)
        {
          permanent = -1;
        }

      ftype  = DBFReadStringAttribute(dbfFile, ii, ftypeIndex);
      linkNo = 0;

      // Get link number.
      while(linkNo < size && reachCode != channels[linkNo].reachCode && permanent != channels[linkNo].reachCode)
        {
          linkNo++;
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(linkNo < size))
        {
          fprintf(stderr, "ERROR in readWaterbodies: Could not find waterbody reach code %lld or pemanent code %lld in channel network.\n", reachCode,
                  permanent);
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
              fprintf(stderr, "ERROR in readWaterbodies: Waterbody reach code %lld has unknown type %s.\n", channels[linkNo].reachCode, ftype);
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
          // Because of the multipart to singlepart operation there can be duplicate reach codes in the shapefile.  Only tag the link if it has not already
          // been tagged.
          if (NOT_USED == channels[linkNo].type)
            {
              channels[linkNo].type = linkType;
            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          else if (!(linkType == channels[linkNo].type))
            {
              fprintf(stderr, "ERROR in readWaterbodies: Waterbody reach code %lld occurs multiple times with different link types.\n", channels[linkNo].reachCode);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }

      // Get polygon.
      if (!error)
        {
          jj = 0;

          while (jj < SHAPES_SIZE && NULL != channels[linkNo].shapes[jj])
            {
              jj++;
            }

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(jj < SHAPES_SIZE))
            {
              fprintf(stderr, "ERROR in readWaterbodies: Waterbody reach code %lld has more than the maximum allowable %d shapes.\n",
                      channels[linkNo].reachCode, SHAPES_SIZE);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }

      if (!error)
        {
          channels[linkNo].shapes[jj] = SHPReadObject(shpFile, ii);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NULL != channels[linkNo].shapes[jj]))
            {
              fprintf(stderr, "ERROR in readWaterbodies: Failed to read shape %d from shp file %s.\n", ii, fileBasename);
              error = true;
            }
          else if (!((5 == channels[linkNo].shapes[jj]->nSHPType || 15 == channels[linkNo].shapes[jj]->nSHPType ||
                      25 == channels[linkNo].shapes[jj]->nSHPType) && 1 < channels[linkNo].shapes[jj]->nVertices))
            {
              // Even after multipart to singlepart waterbody polygons may have multiple parts if they have holes so don't check nParts.
              fprintf(stderr, "ERROR in readWaterbodies: Invalid shape %d from shp file %s.\n", ii, fileBasename);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
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
// channel network.  Do not make any connections to the stream edges in the
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
  int       us_cont_arIndex; // Index of metadata field.
  int       ds_cont_arIndex; // Index of metadata field.
  int       linkNo;          // The link number of the stream.
  int       linkNo2;         // For storing a link that must be connected to.

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
      if (!(0 <= linkNo && linkNo < size))
        {
          fprintf(stderr, "ERROR in readTaudemStreamnet: Invalid link number %d in TauDEM stream network.\n", linkNo);
          error = true;
        }
      else
        {
          if (!(NOT_USED == channels[linkNo].type))
            {
              fprintf(stderr, "ERROR in readTaudemStreamnet: Link number %d used twice in TauDEM stream network.\n", linkNo);
              error = true;
            }

          if (!(linkNo == channels[linkNo].reachCode))
            {
              fprintf(stderr, "ERROR in readTaudemStreamnet: Link number %d does not match reach code in TauDEM stream network.\n", linkNo);
              error = true;
            }
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      if (!error)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // The following code requires at least two upstream links.
          assert(2 <= UPSTREAM_SIZE);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          // Fill in the ChannelLinkStruct.
          // FIXME error check these values, maybe do it in invariant?  Must check length, both contributing areas, upstream/downstream relationships
          // FIXME check that all non-NOFLOW upstream links are compacted to the front of the array.
          channels[linkNo].type                       = STREAM;
          channels[linkNo].shapes[0]                  = SHPReadObject(shpFile, ii);
          channels[linkNo].upstreamContributingArea   = DBFReadDoubleAttribute(dbfFile, ii, us_cont_arIndex);
          channels[linkNo].downstreamContributingArea = DBFReadDoubleAttribute(dbfFile, ii, ds_cont_arIndex);
          channels[linkNo].upstream[0]                = DBFReadIntegerAttribute(dbfFile, ii, uslinkno1Index);
          channels[linkNo].upstream[1]                = DBFReadIntegerAttribute(dbfFile, ii, uslinkno2Index);
          channels[linkNo].downstream[0]              = DBFReadIntegerAttribute(dbfFile, ii, dslinknoIndex);
          
          // Set the most downstream link to have an outflow boundary.
          if (NOFLOW == channels[linkNo].downstream[0])
            {
              channels[linkNo].downstream[0] = OUTFLOW;
            }
          
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
        }
      
      if (!error)
        {
          getLengthAndLocation(channels[linkNo].shapes[0], 0.0, 0.0, &channels[linkNo].length, NULL);
          createLinkElementAfter(channels, size, linkNo, NULL, channels[linkNo].length, -1, -1);
        }
    } // End fill in the links.
  
  // Eliminate links of zero length.
  // FIXME can't call tryToPrune or removeUpstreamDownstreamConnection until we know that the upstream/downstream links pass the invariant, but can't call the
  // invariant until we remove zero length links.
  for (ii = 0; !error && ii < size; ii++)
    {
      if (0.0 == channels[ii].length)
        {
          tryToPruneLink(channels, size, ii);
        }
      else
        {
          linkNo = channels[ii].downstream[0];
          
          while (!isBoundary(linkNo) && 0.0 == channels[linkNo].length)
            {
              linkNo = channels[linkNo].downstream[0];
            }
          
          if (linkNo != channels[ii].downstream[0])
            {
              linkNo2 = channels[ii].downstream[0];
              removeUpstreamDownstreamConnection(channels, size, ii, linkNo2);
              error = addUpstreamDownstreamConnection(channels, size, ii, linkNo);

              if (!error)
                {
                  tryToPruneLink(channels, size, linkNo2);
                }
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
void addStreamMeshEdge(ChannelLinkStruct* channels, int size, int linkNo, double location1, double location2, int edge)
{
  double             tempLocation;          // For swapping locations.
  LinkElementStruct* element1;              // The element at location1.
  LinkElementStruct* element2;              // The element at location2.
  LinkElementStruct* newElement1;           // The new element to use as element1 after the occluded element scan.
  LinkElementStruct* newElement2;           // The new element to use as element2 after the occluded element scan.
  LinkElementStruct* tempElement;           // For looping through elements between element1 and element2.
  LinkElementStruct* nextElement;           // For looping through elements between element1 and element2.
  LinkElementStruct* elementToUse;          // The LinkElementStruct that will be used for an added edge.
  LinkElementStruct* beginGapElement;       // The LinkElementStruct at the beginning of an unassociated gap in the occluded element scan.
  double             beginGapLocation;      // The 1D location in meters of the beginning of an unassociated gap in the occluded element scan.
  double             element1Length;        // Length of element1.
  double             element2Length;        // Length of element2.
  double             element2BeginLocation; // For saving the old value of the beginning of element2.
  bool               killElement2;          // Flag to indicate delayed removal of element2.
  double             length;                // Length of the element being added.
  double             afterGap;              // Used to calculate gaps when overlapping elements have to be shifted.
  double             beforeGap;             // Used to calculate gaps when overlapping elements have to be shifted.
  double             newLocation;           // Used for moving location1 or location2 when overlapping elements have to be shifted.
  double             oldEndLocation;        // For saving the old end location of an element that is being shifted.
  bool               secondOccludedScan;    // Flag for the second time through the occluded element scan.
  bool               foundUnassociated;     // Flag for whether the occluded element scan has found an unassociated gap.
  bool               inAssociated;          // Flag for whether the occluded element scan is in a stretch of associated elements.
  
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

              createLinkElementAfter(channels, size, linkNo, element1->prev, newLocation, edge, -1);
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
              
              createLinkElementAfter(channels, size, linkNo, element1, oldEndLocation, edge, -1);
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
              createLinkElementAfter(channels, size, linkNo, element1->prev, location2, edge, -1);
            }
          else if (epsilonEqual(location2, element1->endLocation))
            {
              // The new element is at the end of the unassociated space.
              location2             = element1->endLocation; // Could be only epsilon equal.  Set it to exactly equal.
              element1->endLocation = location1;
              
              createLinkElementAfter(channels, size, linkNo, element1, location2, edge, -1);
            }
          else
            {
              // The new element is in the middle of the unassociated space.
              createLinkElementAfter(channels, size, linkNo, element1->prev, location1, element1->edge, element1->movedTo);
              createLinkElementAfter(channels, size, linkNo, element1->prev, location2, edge, -1);
            }
        }
    }
  else // if (element1 != element2)
    {
      // The new element spans across multiple existing elements.
      
      // Deal with occluded elements.  If an element between element1 and element2 is associated it is occluded and we need to leave it in place and put the
      // new element either before or after it.
      tempElement        = element1->next;
      secondOccludedScan = false;

      while (tempElement != element2)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          assert(-1 == tempElement->movedTo);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          if (-1 != tempElement->edge)
            {
              if (epsilonEqual(location1, beginLocation(tempElement)))
                {
                  // location1 is at the beginning of tempElement, but element1 got set to tempElement->prev because location1 is also at the end of
                  // tempElement->prev.  This case is not really an occlusion.  It is just an overlap and we should just use tempElement as element1.
                  element1 = tempElement;
                  location1 = beginLocation(element1); // Could be only epsilon equal.  Set it to exactly equal.
                }
              else
                {
                  // tempElement is an associated occluded element.
                  fprintf(stderr, "WARNING in addStreamMeshEdge: Occlusion on channel link %d.  Mesh edge %d from %lf to %lf occluded by mesh edge %d from %lf"
                                  " to %lf.\n", linkNo, tempElement->edge, beginLocation(tempElement), tempElement->endLocation, edge, location1, location2);
                  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                  // A second occluded scan after fixing associated occluded elements should never find an associated occluded element.
                  assert(!secondOccludedScan);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                  
                  // Find the largest unassociated gap between element1 and element2 and put the new element in that gap.  If there is no unassociated gap put
                  // the new element in the junction between associated elements that is closest to the midpoint of location1 and location2.
                  if (-1 != element1->edge)
                    {
                      // We are starting in a stretch of associated elements.
                      length            = element2->endLocation -  // Until we have found an unassociated gap, length stores the distance from the closest
                                          beginLocation(element1); // junction found so far to the midpoint of location1 and location2.  There is guaranteed to
                                                                   // be such a junction closer than the distance from the beginning of element1 to the end of
                                                                   // element2.
                      foundUnassociated = false;
                      inAssociated      = true;
                    }
                  else
                    {
                      // We are starting in a stretch of unassociated elements.
                      length            = 0.0; // Once we have found an unassociated gap, length stores the length of the longest gap found so far.  There is
                                               // guaranteed to be such a gap longer than zero.
                      foundUnassociated = true;
                      inAssociated      = false;
                      beginGapElement   = element1;
                      beginGapLocation  = location1;
                    }
                  
                  newElement1 = element1; // this will always be overwritten, but initialize it to be safe.
                  newElement2 = element2; // this will always be overwritten, but initialize it to be safe.
                  tempElement = element1->next;
                  
                  while(tempElement != element2)
                    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                      // tempElement must be strictly beyond element1 so it must have a previous element.
                      assert(NULL != tempElement->prev);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

                      if (inAssociated)
                        {
                          if (-1 != tempElement->edge)
                            {
                              if (!foundUnassociated)
                                {
                                  // If you are in a stretch of associated elements and the next element is associated and you haven't found an unassociated
                                  // gap yet check if the junction is the closest to the midpoint of location1 and location2 found so far, and if it is use
                                  // this junction.
                                  if (length > fabs(tempElement->prev->endLocation - ((location1 + location2) * 0.5)))
                                    {
                                      length      = fabs(tempElement->prev->endLocation - ((location1 + location2) * 0.5));
                                      newElement1 = tempElement->prev;
                                      newElement2 = tempElement;
                                    }
                                }
                            }
                          else
                            {
                              // If you are in a stretch of associated elements and the next element is unassociated start recording an unassociated gap.
                              if (!foundUnassociated)
                                {
                                  length            = 0.0; // If this is the first unassociated gap set length to zero.
                                  foundUnassociated = true;
                                }
                              
                              inAssociated      = false;
                              beginGapElement   = tempElement->prev;
                              beginGapLocation  = tempElement->prev->endLocation;
                            }
                        }
                      else
                        {
                          if (-1 != tempElement->edge)
                            {
                              // If you are in a stretch of unassociated elements and the next element is associated check if this gap is the longest gap found
                              // so far, and if it is use this gap.
                              inAssociated = true;
                              
                              if (length < tempElement->prev->endLocation - beginGapLocation)
                                {
                                  length      = tempElement->prev->endLocation - beginGapLocation;
                                  newElement1 = beginGapElement;
                                  newElement2 = tempElement;
                                }
                            }
                          else
                            {
                              // If you are in a stretch of unassociated elements and the next element is unassociated do nothing.
                            }
                        }
                      
                      tempElement = tempElement->next; // Move on to the next element.
                    }
                  
                  // Close out the possibly last gap or last junction that ends at element2.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                  // element2 must be strictly beyond element1 so it must have a previous element.
                  assert(NULL != element2->prev);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

                  if (inAssociated)
                    {
                      if (-1 != element2->edge)
                        {
                          if (!foundUnassociated)
                            {
                              // If you are in a stretch of associated elements and element2 is associated and you haven't found an unassociated
                              // gap yet check if the junction is the closest to the midpoint of location1 and location2 found so far, and if it is use
                              // this junction.
                              if (length > fabs(element2->prev->endLocation - ((location1 + location2) * 0.5)))
                                {
                                  length      = fabs(element2->prev->endLocation - ((location1 + location2) * 0.5));
                                  newElement1 = element2->prev;
                                  newElement2 = element2;
                                }
                            }
                        }
                      else
                        {
                          // If you are in a stretch of associated elements and element2 is unassociated it is the beginning and end of an unassociated gap.
                          if (!foundUnassociated)
                            {
                              length            = 0.0; // If this is the first unassociated gap set length to zero.
                              foundUnassociated = true;
                            }
                          
                          inAssociated      = false;
                          beginGapElement   = element2->prev;
                          beginGapLocation  = element2->prev->endLocation;
                          
                          if (length < location2 - beginGapLocation)
                            {
                              length      = location2 - beginGapLocation;
                              newElement1 = beginGapElement;
                              newElement2 = element2;
                            }
                        }
                    }
                  else
                    {
                      if (-1 != element2->edge)
                        {
                          // If you are in a stretch of unassociated elements and element2 is associated check if this gap is the longest gap found
                          // so far, and if it is use this gap.
                          inAssociated = true;
                          
                          if (length < element2->prev->endLocation - beginGapLocation)
                            {
                              length      = element2->prev->endLocation - beginGapLocation;
                              newElement1 = beginGapElement;
                              newElement2 = element2;
                            }
                        }
                      else
                        {
                          // If you are in a stretch of unassociated elements and element2 is unassociated check if this gap is the longest gap found
                          // so far, and if it is use this gap.
                          if (length < location2 - beginGapLocation)
                            {
                              length      = location2 - beginGapLocation;
                              newElement1 = beginGapElement;
                              newElement2 = element2;
                            }
                        }
                    }
                  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                  // Make sure that we have changed one or both of element1 and element2 and that they do not point to the same element.
                  assert((newElement1 != element1 || newElement2 != element2) && newElement1 != newElement2);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                  
                  element1 = newElement1;
                  element2 = newElement2;
                  
                  if (location1 < beginLocation(element1))
                    {
                      location1 = beginLocation(element1);
                    }
                  
                  if (location2 > element2->endLocation)
                    {
                      location2 = element2->endLocation;
                    }
                  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                  // Scan for occluded elements again between the new element1 and element2.  This should never find an occluded element.
                  tempElement        = element1;
                  secondOccludedScan = true;
#else // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                  // Set tempElement to jump out of the loop.
                  tempElement = element2->prev;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                }
            }

          tempElement = tempElement->next; // Move on to the next element.
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // We should eventually reach element2 so we should be guaranteed that there is another element.
          assert(NULL != tempElement);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

      // At this point any elements between element1 and element2 are unassociated.  element1 and element2 themselves may be associated or unassociated.
      
      elementToUse          = NULL;                    // The LinkElementStruct that will be used for the added edge.  NULL until that struct is found.
      element2BeginLocation = beginLocation(element2); // The code after this might move the end location of element1, which might also be storing the begin
                                                       // location of element2 if element2 is immediately after element1.  Store the begin location of element2
                                                       // now for use later.
      killElement2          = false;                   // Flag to indicate delayed removal of element2.
      
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
              location1 = element1->endLocation; // Could be only epsilon equal.  Set it to exactly equal.
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
              location1 = element1->endLocation; // Could be only epsilon equal.  Set it to exactly equal.
            }
          else if (epsilonEqual(location1, beginLocation(element1)))
            {
              // The new edge starts at the beginning of element1.  Use element1 as the struct.
              location1    = beginLocation(element1); // Could be only epsilon equal.  Set it to exactly equal.
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
          if (epsilonEqual(location2, element2BeginLocation))
            {
              // The new edge ends at the beginning of element2.  Nothing to do.
              location2 = element2BeginLocation; // Could be only epsilon equal.  Set it to exactly equal.
            }
          else
            {
              // The new edge overlaps element2.  Chop off element2.
              element2Length = element2->endLocation - element2BeginLocation;
              length         = location2             - location1;
              
              // Calculate the split point that shortens both elements by the same ratio.  For example, each 1/3 shorter.
              newLocation = (location2 * element2Length + element2BeginLocation * length) / (element2Length + length);
              
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              assert(element2BeginLocation < newLocation && newLocation < location2);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              fprintf(stderr, "WARNING in addStreamMeshEdge: Overlap on channel link %d.  Mesh edge %d moved from (%lf to %lf) to (%lf to %lf).  Mesh edge %d "
                              "moved from (%lf to %lf) to (%lf to %lf).\n", linkNo, element2->edge, element2BeginLocation, element2->endLocation,
                      newLocation, element2->endLocation, edge, location1, location2, location1, newLocation);

              location2 = newLocation;
            }
        }
      else // if (-1 == element2->edge)
        {
          // element2 is not associated
          if (epsilonEqual(location2, element2BeginLocation))
            {
              // The new edge ends at the beginning of element2.  Nothing to do.
              location2 = element2BeginLocation; // Could be only epsilon equal.  Set it to exactly equal.
            }
          else if (epsilonEqual(location2, element2->endLocation))
            {
              // The new edge ends at the end of element2.
              location2 = element2->endLocation; // Could be only epsilon equal.  Set it to exactly equal.
              
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

      while (tempElement != element2)
        {
          nextElement = tempElement->next;
          
          if (NULL == elementToUse)
            {
              elementToUse = tempElement;
            }
          else
            {
              killLinkElement(channels, size, linkNo, tempElement);
            }
          
          tempElement = nextElement;
        }
      
      // Get rid of element2 if necessary.
      if (killElement2)
        {
          killLinkElement(channels, size, linkNo, element2);
        }

      if (NULL == elementToUse)
        {
          // Create an element if we haven't already found one to use.
          createLinkElementAfter(channels, size, linkNo, element1, location2, edge, -1);
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
    } // End if (element1 != element2).
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
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(3 == numScanned))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Unable to read entry %d from node file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == index))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Invalid node number in node file.  %d should be %d.\n", index, ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      if (!error)
        {
          nodesX[index] = xCoordinate;
          nodesY[index] = yCoordinate;
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
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(4 == numScanned))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Unable to read entry %d from edge file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == index))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Invalid edge number in edge file.  %d should be %d.\n", index, ii);
          error = true;
        }
      
      if (!(0 <= vertex1 && vertex1 < numberOfNodes))
        {
          fprintf(stderr, "ERROR in addAllStreamMeshEdges: Invalid vertex number %d in edge file index %d.\n", vertex1, index);
          error = true;
        }
      
      if (!(0 <= vertex2 && vertex2 < numberOfNodes))
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
          if (!(boundary < size))
            {
              fprintf(stderr, "ERROR in addAllStreamMeshEdges: Mesh edge linked to invalid channel link number %d.\n", boundary);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          
          if (!error && STREAM == channels[boundary].type)
            {
              getLengthAndLocation(channels[boundary].shapes[0], nodesX[vertex1], nodesY[vertex1], NULL, &location1);
              getLengthAndLocation(channels[boundary].shapes[0], nodesX[vertex2], nodesY[vertex2], NULL, &location2);
              
              addStreamMeshEdge(channels, size, boundary, location1, location2, index);
            }
        }
    }

  // Deallocate node arrays.
  deleteArrayIfNonNull(&nodesX);
  deleteArrayIfNonNull(&nodesY);
  
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

// Read all of the intersections between waterbodies and streams.  Each
// intersection is placed in the LinkElementStruct list of its waterbody.
// This function assumes that each stream link has at most one downstream
// connection, which is true before linking streams to waterbodies, which is
// where this is run in the code.
// 
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels     - The channel network as a 1D array of ChannelLinkStruct.
// size         - The number of elements in channels.
// fileBasename - The basename of the intersection shapefile to read.
//                readWaterbodyStreamIntersections will read the following
//                files: fileBasename.shp, and fileBasename.dbf.
bool readWaterbodyStreamIntersections(ChannelLinkStruct* channels, int size, const char* fileBasename)
{
  bool                   error = false;      // Error flag.
  int                    ii;                 // Loop counter.
  SHPHandle              shpFile;            // Geometry  part of the shapefile.
  DBFHandle              dbfFile;            // Attribute part of the shapefile.
  int                    numScanned;         // Used to check that fscanf scanned all of the requested values.
  int                    numberOfShapes;     // Number of shapes in the shapefile.
  int                    linknoIndex;        // Index of metadata field.
  int                    reachCodeIndex;     // Index of metadata field.
  int                    permanentIndex;     // Index of metadata field.
  int                    streamLinkNo;       // The link number of the intersecting stream.
  const char*            tempString;         // String representation of reachCode or permanent.
  long long              reachCode;          // The reach code of the intersecting waterbody.
  long long              permanent;          // The permanent code of the intersecting waterbody.
  int                    waterbodyLinkNo;    // The link number of the intersecting waterbody.
  SHPObject*             shape;              // The intersection point.
  double                 location;           // 1D location in meters along streamLinkNo of the intersection.
  LinkElementStruct*     intersection1;      // Used to loop over intersections.
  LinkElementStruct*     intersection2;      // Used to loop over intersections.
  UpstreamDownstreamEnum upstreamDownstream; // Whether intersection2 is upstream or downstream of intersection1.
  bool                   hasUpstream;        // Whether intersection1 has another intersection upstream of it.
  bool                   hasDownstream;      // Whether intersection1 has another intersection downstream of it.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && NULL != fileBasename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)

  // Open the geometry and attribute files.
  shpFile = SHPOpen(fileBasename, "rb");
  dbfFile = DBFOpen(fileBasename, "rb");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NULL != shpFile))
    {
      fprintf(stderr, "ERROR in readWaterbodyStreamIntersections: Could not open shp file %s.\n", fileBasename);
      error = true;
    }

  if (!(NULL != dbfFile))
    {
      fprintf(stderr, "ERROR in readWaterbodyStreamIntersections: Could not open dbf file %s.\n", fileBasename);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

  // Get the number of shapes and attribute indices
  if (!error)
    {
      numberOfShapes = DBFGetRecordCount(dbfFile);
      linknoIndex    = DBFGetFieldIndex(dbfFile, "LINKNO");
      reachCodeIndex = DBFGetFieldIndex(dbfFile, "ReachCode");
      permanentIndex = DBFGetFieldIndex(dbfFile, "Permanent_");
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfShapes))
        {
          // Only warn because you could have zero intersections.
          fprintf(stderr, "WARNING in readWaterbodyStreamIntersections: Zero shapes in dbf file %s.\n", fileBasename);
        }
      
      if (!(-1 != linknoIndex))
        {
          fprintf(stderr, "ERROR in readWaterbodyStreamIntersections: Could not find field LINKNO in dbf file %s.\n", fileBasename);
          error = true;
        }
      
      if (!(-1 != reachCodeIndex))
        {
          fprintf(stderr, "ERROR in readWaterbodies: Could not find field ReachCode in dbf file %s.\n", fileBasename);
          error = true;
        }
      
      if (!(-1 != permanentIndex))
        {
          fprintf(stderr, "ERROR in readWaterbodyStreamIntersections: Could not find field Permanent_ in dbf file %s.\n", fileBasename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Fill the intersections into the linked lists.
  for (ii = 0; !error && ii < numberOfShapes; ii++)
    {
      streamLinkNo = DBFReadIntegerAttribute(dbfFile, ii, linknoIndex);
      tempString   = DBFReadStringAttribute(dbfFile, ii, reachCodeIndex);
      numScanned   = sscanf(tempString, "%lld", &reachCode);

      // Some shapes don't have a reach code, in which case we use the permanent code, so it is not an error if you can't scan a reach code.
      if (1 != numScanned)
        {
          reachCode = -1;
        }

      tempString = DBFReadStringAttribute(dbfFile, ii, permanentIndex);
      numScanned = sscanf(tempString, "%lld", &permanent);

      // Some shapes don't have a permanent code, in which case we use the reach code, so it is not an error if you can't scan a permanent code.
      if (1 != numScanned)
        {
          permanent = -1;
        }
      
      waterbodyLinkNo = 0;
      shape           = NULL;
      location        = 0.0;
      
      while(waterbodyLinkNo < size && ((WATERBODY != channels[waterbodyLinkNo].type && ICEMASS != channels[waterbodyLinkNo].type) ||
                                       (reachCode != channels[waterbodyLinkNo].reachCode && permanent != channels[waterbodyLinkNo].reachCode)))
        {
          waterbodyLinkNo++;
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 <= streamLinkNo && streamLinkNo < size && STREAM == channels[streamLinkNo].type))
        {
          fprintf(stderr, "ERROR in readWaterbodyStreamIntersections: Stream link number %d out of range or not a stream link for intersection with waterbody "
                          "reach code %lld or permanent code %lld in dbf file %s.\n", streamLinkNo, reachCode, permanent, fileBasename);
          error = true;
        }
      
      if (!(0 <= waterbodyLinkNo && waterbodyLinkNo < size && (WATERBODY == channels[waterbodyLinkNo].type || ICEMASS == channels[waterbodyLinkNo].type)))
        {
          fprintf(stderr, "ERROR in readWaterbodyStreamIntersections: Could not find waterbody reach code %lld or permanent code %lld in channel network.\n",
                  reachCode, permanent);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      if (!error)
        {
          // Read the intersection point.
          shape = SHPReadObject(shpFile, ii);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NULL != shape))
            {
              fprintf(stderr, "ERROR in readWaterbodyStreamIntersections: Failed to read shape %d from shp file %s.\n", ii, fileBasename);
              error = true;
            }
          else if (!((8 == shape->nSHPType || 18 == shape->nSHPType || 28 == shape->nSHPType) && 0 == shape->nParts && 1 == shape->nVertices))
            {
              fprintf(stderr, "ERROR in readWaterbodyStreamIntersections: Invalid shape %d from shp file %s.\n", ii, fileBasename);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      if (!error)
        {
          // Get the location along streamLinkNo and insert into the waterbody's linked list.
          getLengthAndLocation(channels[streamLinkNo].shapes[0], shape->padfX[0], shape->padfY[0], NULL, &location);
          createLinkElementAfter(channels, size, waterbodyLinkNo, NULL, location, streamLinkNo, COINCIDENT);
        }

      if (NULL != shape)
        {
          SHPDestroyObject(shape);
        }
    }
  
  // Now mark each intersection with whether the stream should be connected upstream or downstream of the waterbody.  The marking is done by placing the values
  // from UpstreamDownstreamEnum into movedTo of the LinkElementStruct.  All of the marks start as COINCIDENT until the correct value is determined.  If the
  // intersection has no other intersections in the list downstream of it, then the stream is connected downstream of the waterbody and the intersection is
  // marked DOWNSTREAM.  Otherwise, if the intersection has no other intersections in the list upstream of it, then the stream is connected upstream of the
  // waterbody and the intersection is marked UPSTREAM.  Otherwise, the intersection is ignored, and the intersection is marked UNRELATED.  Duplicate
  // intersections are also ignored and marked UNRELATED.
  for (ii = 0; ii < size; ii++)
    {
      if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type)
        {
          intersection1 = channels[ii].firstElement;
          
          while (NULL != intersection1)
            {
              hasUpstream   = false;                     // Whether intersection1 has another intersection upstream of it.
              hasDownstream = false;                     // Whether intersection1 has another intersection downstream of it.
              intersection2 = channels[ii].firstElement; // For looping over all intersections to compare with intersection1.
              
              // For each intersection1, loop over all of the intersection2s.  If intersection1 will be ignored (hasUpstream && hasDownstream) then break out
              // of the loop.
              while (NULL != intersection2 && !(hasUpstream && hasDownstream))
                {
                  // Do not compare an intersection to itself.
                  if (intersection1 != intersection2)
                    {
                      // Get the upstream/downstream relationship between intersection1 and intersection2.
                      upstreamDownstream = getUpstreamDownstream(channels, size, intersection1, intersection2);

                      if (COINCIDENT == upstreamDownstream && COINCIDENT != intersection2->movedTo)
                        {
                          // If the two intersections are duplicates and intersection2 is already assigned a mark then set hasUpstream and hasDownstream to
                          // force intersection1 to be ignored.
                          hasUpstream   = true;
                          hasDownstream = true;
                        }
                      else if (UPSTREAM == upstreamDownstream)
                        {
                          hasUpstream = true;
                        }
                      else if (DOWNSTREAM == upstreamDownstream)
                        {
                          hasDownstream = true;
                        }
                    }

                  intersection2 = intersection2->next;
                }
              
              if (!hasDownstream)
                {
                  intersection1->movedTo = DOWNSTREAM;
                }
              else if (!hasUpstream)
                {
                  intersection1->movedTo = UPSTREAM;
                }
              else
                {
                  intersection1->movedTo = UNRELATED;
                }

              intersection1 = intersection1->next;
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

// When we make a stream flow downstream into a waterbody there are some
// complications that we must handle.  If the stream currently flows downstream
// into a stream we break that connection.  If the stream currently flows
// downstream into a boundary condition code we break that connection and add
// the boundary condition code to the downstream connections of the waterbody.
// If the stream currently flows downstream into a waterbody we leave that
// connection alone.  This function performs those tasks and then makes the new
// connection to the waterbody.
// 
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels        - The channel network as a 1D array of ChannelLinkStruct.
// size            - The number of elements in channels.
// streamLinkNo    - The stream to link to the waterbody.
// waterbodyLinkNo - The waterbody to link to the stream.
bool makeStreamFlowIntoWaterbody(ChannelLinkStruct* channels, int size, int streamLinkNo, int waterbodyLinkNo)
{
  bool error = false; // Error flag.
  int  ii;            // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= streamLinkNo && streamLinkNo < size && STREAM == channels[streamLinkNo].type &&
         0 <= waterbodyLinkNo && waterbodyLinkNo < size && (WATERBODY == channels[waterbodyLinkNo].type || ICEMASS == channels[waterbodyLinkNo].type));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // The following code requires at least two downstream links.
  assert(2 <= DOWNSTREAM_SIZE);
  
  // Because of the way the TauDEM stream network is created and the way this code works there are limited options for what can be downstream of a stream:
  // No connections, one boundary condition code, one stream, or one or more waterbodies.  Print a warning if one of these is not the case.
  if (isBoundary(channels[streamLinkNo].downstream[0]) || STREAM == channels[channels[streamLinkNo].downstream[0]].type)
    {
      if (!(NOFLOW == channels[streamLinkNo].downstream[1]))
        {
          fprintf(stderr, "WARNING in makeStreamFlowIntoWaterbody: Impossible downstream configuration of stream link %d.  Boundary condition code or stream "
                  "plus other connections.\n", streamLinkNo);
        }
    }
  else
    {
      for (ii = 0; ii < DOWNSTREAM_SIZE && NOFLOW != channels[streamLinkNo].downstream[ii]; ii++)
        {
          if (isBoundary(channels[streamLinkNo].downstream[ii]) || STREAM == channels[channels[streamLinkNo].downstream[ii]].type)
            {
              fprintf(stderr, "WARNING in makeStreamFlowIntoWaterbody: Impossible downstream configuration of stream link %d.  Waterbody plus boundary "
                      "condition code or stream .\n", streamLinkNo);
            }
        }
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  ii = 0;
  
  // handle the stream's existing downstream connections.
  while (!error && ii < DOWNSTREAM_SIZE && NOFLOW != channels[streamLinkNo].downstream[ii])
    {
      if (isBoundary(channels[streamLinkNo].downstream[ii]))
        {
          // Make the waterbody flow into the boundary condition code and remove it from the stream.
          error = addUpstreamDownstreamConnection(channels, size, waterbodyLinkNo, channels[streamLinkNo].downstream[ii]);

          if (!error)
            {
              removeUpstreamDownstreamConnection(channels, size, streamLinkNo, channels[streamLinkNo].downstream[ii]);
            }
        }
      else if (STREAM == channels[channels[streamLinkNo].downstream[ii]].type)
        {
          // Break the connection to the downstream stream.
          removeUpstreamDownstreamConnection(channels, size, streamLinkNo, channels[streamLinkNo].downstream[ii]);
        }
      else
        {
          // Leave a connection to a downstream waterbody alone.  Increment ii to go on to the next connection.
          ii++;
        }
    }
  
  // Add the connection to make the stream flow into the waterbody.
  if (!error)
    {
      error = addUpstreamDownstreamConnection(channels, size, streamLinkNo, waterbodyLinkNo);
    }
  
  return error;
}

// Split a link element and place the piece after the split in a new link
// number.
// 
// When a link is split the part before the split always stays where it is and
// the part after the split always moves.  So a link will have a contiguous
// region of unmoved elements at the beginning of the link possibly followed by
// a contiguous region of moved elements.
// 
// The length of channel represented by a link is only the unmoved elements.
// When water hits the end of the unmoved elements it flows to the downstream
// connections of the ChannelLinkStruct.  When a link is split, the length
// value in the ChannelLinkStruct is updated to reflect the new length of
// unmoved elements only.  The moved elements are just there for record keeping
// to find where those sections of channel went.
// 
// In the moved link, location values start over at zero.  Zero on the moved
// link is the same location as the begin location of the LinkElementStruct
// recording the move in the original link.
//
// Returns: true if there is an error, false otherwise.
// 
// Parameters:
//
// channels - The channel network as a 1D array of ChannelLinkStruct.
// size     - The number of elements in channels.
// linkNo   - The link to split.
// element  - The element to split the link near.  The split could be at the
//            beginning, middle, or end of this element.
// location - The 1D location in meters along the link to split at.
bool splitLink(ChannelLinkStruct* channels, int size, int linkNo, LinkElementStruct* element, double location)
{
  bool               error = false; // Error flag.
  int                newLinkNo;     // The moved part of the split link.
  int                linkNo2;       // For storing a link that must be connected to.
  LinkElementStruct* endElement;    // The last element that will be moved.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= linkNo && linkNo < size && NULL != element && beginLocation(element) <= location && location <= element->endLocation);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Get an unused link number.
  newLinkNo = 0;
  
  while (newLinkNo < size && NOT_USED != channels[newLinkNo].type)
    {
      newLinkNo++;
    }
  
  if (newLinkNo == size)
    {
      fprintf(stderr, "ERROR in splitLink: Need an unused link and none are left.  Add unused links to the .link file and run again.\n");
      error = true;
    }
  
  if (!error)
    {
      // If you are at the end of an element, point to the next one, the first one that will be moved.
      if (epsilonEqual(location, element->endLocation))
        {
          element = element->next;
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // There must be a next element or we would have been at the end of the link and we would not have split the link.
          assert(NULL != element);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }
      else if (!epsilonEqual(location, beginLocation(element)))
        {
          // If you are in the middle of an element split the element.

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)
          assert(-1 == element->edge); // Element must be unassociated or we would have moved to the end of it before here.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)

          createLinkElementAfter(channels, size, linkNo, element->prev, location, element->edge, element->movedTo);
        }
      
      // Element now points to the beginning of the list of elements to move.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)
      // There must be a previous element or we would have been at the beginning of the link and we would not have split the link.
      // We are also now splitting at the beginning of element.
      assert(NULL != element->prev && epsilon_equal(location, element->prev->end_location));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)

      // Find the end of the list of elements to move.
      endElement = element;
      
      while (NULL != endElement->next && -1 == endElement->next->movedTo)
        {
          // Update the locations relative to the beginning of the new link.
          endElement->endLocation -= location;
          endElement               = endElement->next;
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)
      // endElement now points to the last unmoved element in linkNo.
      assert(endElement->endLocation == channels[linkNo].length);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)

      endElement->endLocation -= location;

      // Now element is the first element that will be moved and end_element is the last element that will be moved, and their locations are the correct values
      // for the new moved link.

      // Create the movedTo element after endElement.
      createLinkElementAfter(channels, size, linkNo, endElement, channels[linkNo].length, -1, newLinkNo);
  
      // Update struct values.
      channels[newLinkNo].type      = STREAM;
      channels[newLinkNo].reachCode = channels[linkNo].reachCode;
      channels[newLinkNo].length    = channels[linkNo].length - location;
      channels[linkNo].length       = location;
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)
      // Can't move the first element in a link, and the movedTo element is after endElement so these must not be NULL.
      assert(NULL != element->prev && NULL != endElement->next);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)

      // Move everything between element and endElement to newLinkNo.
      element->prev->next              = endElement->next;
      endElement->next->prev           = element->prev;
      element->prev                    = NULL;
      endElement->next                 = NULL;
      channels[newLinkNo].firstElement = element;
      channels[newLinkNo].lastElement  = endElement;
    }

  // Move the connections with downstream links to the new stream link.
  while (!error && NOFLOW != channels[linkNo].downstream[0])
    {
      linkNo2 = channels[linkNo].downstream[0];
      removeUpstreamDownstreamConnection(channels, size, linkNo, linkNo2);
      error = addUpstreamDownstreamConnection(channels, size, newLinkNo, linkNo2);
    }

  // Create a connection between the two split parts of the link.
  if (!error)
    {
      error = addUpstreamDownstreamConnection(channels, size, linkNo, newLinkNo);
    }

  return error;
}

// Create a single upstream/downstream link between a waterbody and a stream.
// 
// This function is the first point in the code where stream links can get
// split and moved so from here on out we have to handle the case where
// movedTo is not -1.  It is also the first point in the code where links can
// get more than one downstream neighbor.
// 
// To link the waterbody and stream, first find the location on the stream.
// This may involve traversing moved links.  If the location of the
// intersection is at the beginning or end of a link, just create the
// connection between the existing links.  If the location of the intersection
// is in the middle of a link, split the stream.
//
// This function allows a stream to flow downstream into more than one
// waterbody or a waterbody to flow downstream into more than one stream.
// However, if we are making a connection where a stream is supposed to flow
// downstream into a waterbody and it currently flows downstream into a stream
// we break that existing downstream connection.
// 
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels                 - The channel network as a 1D array of
//                            ChannelLinkStruct.
// size                     - The number of elements in channels.
// streamLinkNo             - The stream to link to the waterbody.
// waterbodyLinkNo          - The waterbody to link to the stream.
// location                 - The 1D location in meters along streamLinkNo to
//                            make the link.
// streamUpstreamDownstream - Specifies whether to link the stream upstream or
//                            downstream of the waterbody.  If UPSTREAM the
//                            stream will flow into the waterbody.  If
//                            DOWNSTREAM the waterbody will flow into the
//                            stream.
bool linkWaterbodyStreamIntersection(ChannelLinkStruct* channels, int size, int streamLinkNo, int waterbodyLinkNo, double location,
                                     UpstreamDownstreamEnum streamUpstreamDownstream)
{
  bool               error   = false; // Error flag.
  int                ii;              // Loop counter.
  LinkElementStruct* element = NULL;  // The stream element that will be linked to the waterbody.
  double             oldLocation;     // For storing the old location if it is moved.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= streamLinkNo && streamLinkNo < size && STREAM == channels[streamLinkNo].type &&
         0 <= waterbodyLinkNo && waterbodyLinkNo < size && (WATERBODY == channels[waterbodyLinkNo].type || ICEMASS == channels[waterbodyLinkNo].type) &&
         0.0 <= location && location <= channels[streamLinkNo].lastElement->endLocation &&
         (UPSTREAM == streamUpstreamDownstream || DOWNSTREAM == streamUpstreamDownstream));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Find the element of the stream that will be linked to the waterbody.
  while (NULL == element)
    {
      element = channels[streamLinkNo].firstElement;

      while (epsilonLess(element->endLocation, location))
        {
          element = element->next; // Move on to the next element.
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // The last element's endLocation should never be less than the location of the intersection so we should be guaranteed that there is another
          // element.
          assert(NULL != element);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }
      
      if (-1 != element->movedTo)
        {
          // The stream element was moved.
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          assert(NULL != element->prev && 0 <= element->movedTo && element->movedTo < size && STREAM == channels[element->movedTo].type);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          location       -= element->prev->endLocation;
          streamLinkNo    = element->movedTo;
          element         = NULL;
        }
    }
  
  // At this point element points to the unmoved element that will be linked to this waterbody.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  assert(-1 == element->movedTo);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  // If element is associated and the intersection location is in the middle of element move the intersection to the beginning or end of element.
  if (-1 != element->edge && !epsilonEqual(location, element->endLocation) && !epsilonEqual(location, beginLocation(element)))
    {
      oldLocation = location;
      
      if (UPSTREAM == streamUpstreamDownstream)
        {
          // The stream will be upstream, move the intersection to the beginning of element.
          location = beginLocation(element);
        }
      else
        {
          // The stream will be downstream, move the intersection to the end of element.
          location = element->endLocation;
        }
      
      fprintf(stderr, "WARNING in linkWaterbodyStreamIntersection: Overlap between waterbody %d and mesh edge %d on stream %d.  Intersection moved from %lf "
                      "to %lf.\n", waterbodyLinkNo, element->edge, streamLinkNo, oldLocation, location);
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  assert(0 <= streamLinkNo && streamLinkNo < size && STREAM == channels[streamLinkNo].type && 0.0 <= location && location <= channels[streamLinkNo].length);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  if (epsilonEqual(location, 0.0))
    {
      if (UPSTREAM == streamUpstreamDownstream)
        {
          // The links upstream of the stream flow into the waterbody.
          for (ii = 0; !error && ii < UPSTREAM_SIZE && NOFLOW != channels[streamLinkNo].upstream[ii]; ii++)
            {
              if (isBoundary(channels[streamLinkNo].upstream[ii]))
                {
                  // Make the boundary condition code flow into the waterbody and remove it from the stream.
                  error = addUpstreamDownstreamConnection(channels, size, channels[streamLinkNo].upstream[ii], waterbodyLinkNo);

                  if (!error)
                    {
                      removeUpstreamDownstreamConnection(channels, size, channels[streamLinkNo].upstream[ii], streamLinkNo);
                      ii--; // Decrement ii because removing the upstream connection renumbers all connections after it.
                    }
                }
              else if (STREAM == channels[channels[streamLinkNo].upstream[ii]].type)
                {
                  // Make the upstream stream flow into the waterbody.  This will also remove it from the list of upstream connections.
                  error = makeStreamFlowIntoWaterbody(channels, size, channels[streamLinkNo].upstream[ii], waterbodyLinkNo);
                  ii--; // Decrement ii because removing the upstream connection renumbers all connections after it.
                }
              else
                {
                  // Make the upstream waterbody flow into the waterbody.  Do not remove it from the list of upstream connections.
                  error = addUpstreamDownstreamConnection(channels, size, channels[streamLinkNo].upstream[ii], waterbodyLinkNo);
                }
            }
        }
      else
        {
          // The waterbody flows into the beginning of the stream.
          error = addUpstreamDownstreamConnection(channels, size, waterbodyLinkNo, streamLinkNo);
        }
    }
  else
    {
      // If location is not at the end of the stream, split the stream and then location will be at the end of the stream.
      if (!epsilonEqual(location, channels[streamLinkNo].length))
        {
          error = splitLink(channels, size, streamLinkNo, element, location);
        }

      if (!error)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          assert(epsilonEqual(location, channels[streamLinkNo].length));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

          if (UPSTREAM == streamUpstreamDownstream)
            {
              // The end of the stream flows into the waterbody.
              error = makeStreamFlowIntoWaterbody(channels, size, streamLinkNo, waterbodyLinkNo);
            }
          else
            {
              // The waterbody flows into the links downstream of the stream.
              for (ii = 0; !error && ii < DOWNSTREAM_SIZE && NOFLOW != channels[streamLinkNo].downstream[ii]; ii++)
                {
                  error = addUpstreamDownstreamConnection(channels, size, waterbodyLinkNo, channels[streamLinkNo].downstream[ii]);
                }
            }
        }
    }
  
  return error;
}

// Create all of the upstream/downstream links between waterbodies and streams.
// The intersections have already been stored in the waterbodies'
// LinkElementStruct lists by readWaterbodyStreamIntersections.
// 
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels - The channel network as a 1D array of ChannelLinkStruct.
// size     - The number of elements in channels.
bool linkAllWaterbodyStreamIntersections(ChannelLinkStruct* channels, int size)
{
  int error = false; // Error flag.
  int ii;            // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Resolve all of the intersections.
  for (ii = 0; !error && ii < size; ii++)
    {
      if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type)
        {
          // FIXME remove
          printf("adding waterbody stream intersections for waterbody %d.\n", ii);
          
          while (!error && NULL != channels[ii].firstElement)
            {
              if (UPSTREAM == channels[ii].firstElement->movedTo || DOWNSTREAM == channels[ii].firstElement->movedTo)
                {
                  error = linkWaterbodyStreamIntersection(channels, size, channels[ii].firstElement->edge, ii, channels[ii].firstElement->endLocation,
                                                          (UpstreamDownstreamEnum)channels[ii].firstElement->movedTo);
                }
              
              if (!error)
                {
                  killLinkElement(channels, size, ii, channels[ii].firstElement);
                }
            }
        }
    }

  return error;
}

// FIXME make into a more user friendly main program.
int main(void)
{
  bool               error = false; // Error flag.
  //bool               tempError;     // Error flag.
  //int                ii;            // Loop counter.
  ChannelLinkStruct* channels;      // The channel network.
  int                size;          // The number of elements in channels.
  
  error = readLink(&channels, &size, "/share/CI-WATER_Simulation_Data/small_green_mesh/mesh.1.link");
  
  if (!error)
    {
      error = readWaterbodies(channels, size, "/share/CI-WATER_Simulation_Data/small_green_mesh/mesh_waterbodies");
    }
  
  if (!error)
    {
      error = readTaudemStreamnet(channels, size, "/share/CI-WATER_Simulation_Data/small_green_mesh/projectednet");
    }
  
  if (!error)
    {
      error = addAllStreamMeshEdges(channels, size, "/share/CI-WATER_Simulation_Data/small_green_mesh/mesh.1.node",
                                    "/share/CI-WATER_Simulation_Data/small_green_mesh/mesh.1.edge");
    }
  
  if (!error)
    {
      error = readWaterbodyStreamIntersections(channels, size, "/share/CI-WATER_Simulation_Data/small_green_mesh/mesh_waterbodies_streams_intersections");
    }
  
  if (!error)
    {
      error = linkAllWaterbodyStreamIntersections(channels, size);
    }
  
  /*
  if (!error)
    {
      error = readAndLinkWaterbodyWaterbodyIntersections(channels, size,
                                                         "/share/CI-WATER_Simulation_Data/small_green_mesh/mesh_waterbodies_waterbodies_intersections");
    }
  
  if (!error)
    {
      for (ii = 0; ii < size; ii++)
        {
          tryToPruneLink(channels, size, ii);
        }
    }
  
  if (!error)
    {
      error = writeChannelNetwork(channels, size);
    }
  
  if (NULL != channels)
    {
      tempError = channelNetworkDealloc(&channels, &size);
      error     = error || tempError;
    }
  */
  
  return 0;
}
