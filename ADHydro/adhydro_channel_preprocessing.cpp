#include "channel_element.h"
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

// FIXME refactor to have multiple downstream connections for each link.

#define SHAPES_SIZE   (2)  // Size of array of shapes in ChannelLinkStruct.
#define UPSTREAM_SIZE (64) // Size of array of upstream links in ChannelLinkStruct.

typedef int intarraycvn[ChannelElement::channelVerticesSize]; // Fixed size array of ints.  Size is channel vertices.

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
//
// These structures are also used in waterbody links to store lists of
// intersections with other links.  The link number of the other link is stored
// in edge.  If the other link is a stream the 1D location of the intersection
// along the stream is stored in endLocation.
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
  int                elementStart;               // The first channel element number on this link after assigning global element numbers.
  int                numberOfElements;           // The number of channel elements on this link after assigning global element numbers.
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
  assert(NULL != channels && 0 <= linkNo && linkNo < size);
  
  if (STREAM == channels[linkNo].type)
    {
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
  
  return error;
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
// upstreamLinkNo   - The link to make flow into downstreamLinkNo or a boundary
//                    condition code.
// downstreamLinkNo - The link that upstreamLinkNo will flow into.  Cannot be a
//                    boundary condition code.
bool addUpstreamConnection(ChannelLinkStruct* channels, int size, int upstreamLinkNo, int downstreamLinkNo)
{
  bool error = false; // Error flag.
  int  ii;            // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && (isBoundary(upstreamLinkNo) || (0 <= upstreamLinkNo && upstreamLinkNo < size)) && 0 <= downstreamLinkNo &&
         downstreamLinkNo < size);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
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
      fprintf(stderr, "ERROR in addUpstreamConnection: Channel link %d has more than the maximum allowable %d upstream neighbors.\n",
              downstreamLinkNo, UPSTREAM_SIZE);
      error = true;
    }
  
  return error;
}

// Remove upstreamLinkNo from the upstream connection list of downstreamLinkNo.
//
// Returns: true if there is an error, false otherwise.  If there is an error
// no connections are modified.
// 
// Parameters:
//
// channels         - The channel network as a 1D array of ChannelLinkStruct.
// size             - The number of elements in channels.
// upstreamLinkNo   - The link to remove from downstreamLinkNo or a boundary
//                    condition code.
// downstreamLinkNo - The link that upstreamLinkNo will be removed from.
//                    Cannot be a boundary condition code.
bool removeUpstreamConnection(ChannelLinkStruct* channels, int size, int upstreamLinkNo, int downstreamLinkNo)
{
  bool error = false; // Error flag.
  int  ii;            // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && (isBoundary(upstreamLinkNo) || (0 <= upstreamLinkNo && upstreamLinkNo < size)) && 0 <= downstreamLinkNo &&
         downstreamLinkNo < size);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Find UpstreamLinkNo in the upstream connection list of its downstream neighbor.
  ii = 0;
  
  while (UPSTREAM_SIZE > ii && upstreamLinkNo != channels[downstreamLinkNo].upstream[ii])
    {
      ii++;
    }
  
  // Remove UpstreamLinkNo from the list and move all other upstream connections forward to fill the gap.
  while (UPSTREAM_SIZE - 1 > ii && NOFLOW != channels[downstreamLinkNo].upstream[ii])
    {
      channels[downstreamLinkNo].upstream[ii] = channels[downstreamLinkNo].upstream[ii + 1];
      ii++;
    }
  
  // Fill in the last connection with NOFLOW.
  if (UPSTREAM_SIZE - 1 == ii)
    {
      channels[downstreamLinkNo].upstream[ii] = NOFLOW;
    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  else if (UPSTREAM_SIZE == ii)
    {
      fprintf(stderr, "ERROR in removeUpstreamConnection: Trying to break upstream connection to channel link %d from channel link %d and no connection "
                      "exists.\n", upstreamLinkNo, downstreamLinkNo);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && (isBoundary(upstreamLinkNo) || (0 <= upstreamLinkNo && upstreamLinkNo < size)) &&
         (isBoundary(downstreamLinkNo) || (0 <= downstreamLinkNo && downstreamLinkNo < size)));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Add upstreamLinkNo to downstreamLinkNo.
  if (!isBoundary(downstreamLinkNo) && NOFLOW != upstreamLinkNo)
    {
      error = addUpstreamConnection(channels, size, upstreamLinkNo, downstreamLinkNo);
    }
  
  // Add downstreamLinkNo to UpstreamLinkNo.
  if (!error && !isBoundary(upstreamLinkNo))
    {
      // Break downstream connection of upstreamLinkNo.
      if (!isBoundary(channels[upstreamLinkNo].downstream))
        {
          error = removeUpstreamConnection(channels, size, upstreamLinkNo, channels[upstreamLinkNo].downstream);
        }
      
      // Make new downstream connection.
      if (!error)
        {
          channels[upstreamLinkNo].downstream = downstreamLinkNo;
        }
      else if (!isBoundary(downstreamLinkNo))
        {
          // Remove the connection we added to make no change to connections on error.
          removeUpstreamConnection(channels, size, upstreamLinkNo, downstreamLinkNo);
        }
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

// Used for the return value of upstreamDownstream.
typedef enum
{
  UPSTREAM,   // intersection is upstream   of streamLinkNo, location.
  DOWNSTREAM, // intersection is downstream of streamLinkNo, location.
  UNRELATED,  // Neither is upstream or downstream of the other.
} UpstreamDownstreamEnum;

// Return the upstream/downstream relationship between intersection and
// streamLinkNo, location.
//
// Parameters:
//
// channels     - The channel network as a 1D array of ChannelLinkStruct.
// size         - The number of elements in channels.
// intersection - The first intersection point.
// streamLinkNo - The stream of the second intersection point.
// location     - The 1D location in meters along streamLinkNo of the second
//                intersection point.
UpstreamDownstreamEnum upstreamDownstream(ChannelLinkStruct* channels, int size, LinkElementStruct* intersection, int streamLinkNo, double location)
{
  UpstreamDownstreamEnum returnValue = UNRELATED; // Used to store the return value of the function.
  int                    tempLinkNo;              // Used to search upstream and downstream links.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && NULL != intersection && 0 <= intersection->edge && intersection->edge < size && STREAM == channels[intersection->edge].type &&
         0.0 <= intersection->endLocation && intersection->endLocation <= channels[intersection->edge].lastElement->endLocation && 0 <= streamLinkNo &&
         streamLinkNo < size && 0.0 <= location && location <= channels[streamLinkNo].lastElement->endLocation);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // If they are on the same link the 1D location determines the relationship.
  if (intersection->edge == streamLinkNo)
    {
      if (intersection->endLocation < location)
        {
          returnValue = UPSTREAM;
        }
      else if (intersection->endLocation > location)
        {
          returnValue = DOWNSTREAM;
        }
    }
  else
    {
      // Search downstream of intersection to find streamLinkNo.
      tempLinkNo = channels[intersection->edge].downstream;
      
      while (UNRELATED == returnValue && !isBoundary(tempLinkNo))
        {
          if (tempLinkNo == streamLinkNo)
            {
              returnValue = UPSTREAM;
            }
          
          tempLinkNo = channels[tempLinkNo].downstream;
        }
      
      // Search downstream of streamLinkNo to find intersection.
      tempLinkNo = channels[streamLinkNo].downstream;
      
      while (UNRELATED == returnValue && !isBoundary(tempLinkNo))
        {
          if (tempLinkNo == intersection->edge)
            {
              returnValue = DOWNSTREAM;
            }
          
          tempLinkNo = channels[tempLinkNo].downstream;
        }
    }
  
  return returnValue;
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
// link of the ChannelLinkStruct.  When a link is split, the length value in
// the ChannelLinkStruct is updated to reflect the new length of unmoved
// elements only.  The moved elements are just there for record keeping to find
// where those sections of channel went.
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
  int                ii;            // Loop counter.
  int                newLinkNo;     // The moved part of the split link.
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
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          assert(NULL != element->next); // There must be a next element or we would have been at the end of the link and we would not have split the link.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

          element = element->next;
        }
      else if (!epsilonEqual(location, beginLocation(element)))
        {
          // If you are in the middle of an element split the element.

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)
          assert(-1 == element->edge); // Element must be unassociated or we would have moved to the end of it before here.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)

          error = createLinkElementAfter(channels, size, linkNo, element->prev, location, element->edge, element->movedTo);
        }
    }
  
  if (!error)
    {
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
    }
  
  // Now element is the first element that will be moved and end_element is the last element that will be moved, and their locations are the correct values for
  // the new moved link.
  
  // Create the movedTo element after endElement.
  if (!error)
    {
      error = createLinkElementAfter(channels, size, linkNo, endElement, channels[linkNo].length, -1, newLinkNo);
    }
  
  // Split the link.
  if (!error)
    {
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
      
      // Update connection from the downstream link to the new stream link.
      ii = 0;
      
      while (ii < UPSTREAM_SIZE && linkNo != channels[channels[linkNo].downstream].upstream[ii])
        {
          ii++;
        }
      
      if (ii < UPSTREAM_SIZE)
        {
          channels[channels[linkNo].downstream].upstream[ii] = newLinkNo;
        }
      else
        {
          fprintf(stderr, "ERROR in splitLink: Channel link %d not found in downstream neighbor's upstream connection list.\n", linkNo);
          error = true;
        }
      
      if (!error)
        {
          // Configure the new stream link.
          channels[newLinkNo].type                         = STREAM;
          channels[newLinkNo].permanent                    = channels[linkNo].permanent;
          channels[newLinkNo].length                       = channels[linkNo].length - location;
          channels[newLinkNo].upstream[0]                  = linkNo;
          channels[newLinkNo].downstream                   = channels[linkNo].downstream;

          // Update old stream link.
          channels[linkNo].downstream = newLinkNo;
          channels[linkNo].length     = location;
        }
    }
  
  return error;
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
      
      (*channels)[ii].downstream       = NOFLOW;
      (*channels)[ii].firstElement     = NULL;
      (*channels)[ii].lastElement      = NULL;
      (*channels)[ii].elementStart     = 0;
      (*channels)[ii].numberOfElements = 0;
      
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
          // FIXME error check these values, maybe do it in invariant?  Must check length, both contributing areas, upstream/downstream relationships
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
  bool               error = false;         // Error flag.
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
              location2             = element1->endLocation; // Could be only epsilon equal.  Set it to exactly equal.
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

// Read all of the intersections between waterbodies and streams.  Each
// intersection is placed in a sorted linked list in its waterbody.  The linked
// lists are sorted with the downstream intersections toward the head of the
// list.  This function uses LinkElementStructs to store the linked list.  This
// function assumes that no links have been split and moved yet so movedTo
// should be -1 for all elements.
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
  bool               error = false;    // Error flag.
  int                ii;               // Loop counter.
  SHPHandle          shpFile;          // Geometry  part of the shapefile.
  DBFHandle          dbfFile;          // Attribute part of the shapefile.
  int                numberOfShapes;   // Number of shapes in the shapefile.
  int                linknoIndex;      // Index of metadata field.
  int                permanentIndex;   // Index of metadata field.
  int                streamLinkNo;     // The link number of the intersecting stream.
  int                permanent;        // The permanent code of the intersecting waterbody.
  int                waterbodyLinkNo;  // The link number of the intersecting waterbody.
  SHPObject*         shape;            // The intersection point.
  double             location;         // 1D location in meters along streamLinkNo of the intersection.
  LinkElementStruct* tempIntersection; // For sorting the linked list of intersections.
  bool               foundIt;          // For sorting the linked list of intersections.
  bool               ignoreIt;         // For sorting the linked list of intersections.

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
      
      if (!(-1 != permanentIndex))
        {
          fprintf(stderr, "ERROR in readWaterbodyStreamIntersections: Could not find field Permanent_ in dbf file %s.\n", fileBasename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Fill the intersections into the sorted linked lists.
  for (ii = 0; !error && ii < numberOfShapes; ii++)
    {
      streamLinkNo    = DBFReadIntegerAttribute(dbfFile, ii, linknoIndex);
      permanent       = DBFReadIntegerAttribute(dbfFile, ii, permanentIndex);
      waterbodyLinkNo = 0;
      shape           = NULL;
      location        = 0.0;
      
      while(waterbodyLinkNo < size && ((WATERBODY != channels[waterbodyLinkNo].type && ICEMASS != channels[waterbodyLinkNo].type) || permanent != channels[waterbodyLinkNo].permanent))
        {
          waterbodyLinkNo++;
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 <= streamLinkNo && streamLinkNo <= size && STREAM == channels[streamLinkNo % size].type))
        {
          fprintf(stderr, "ERROR in readWaterbodyStreamIntersections: Stream link number %d out of range or not a stream link for intersection with waterbody "
                          "permanent code %d in dbf file %s.\n", streamLinkNo, permanent, fileBasename);
          error = true;
        }
      
      if (!(0 <= waterbodyLinkNo && waterbodyLinkNo < size && (WATERBODY == channels[waterbodyLinkNo].type || ICEMASS == channels[waterbodyLinkNo].type)))
        {
          fprintf(stderr, "ERROR in readWaterbodyStreamIntersections: Could not find waterbody permanent code %d in channel network.\n", permanent);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      if (!error)
        {
          // The array is zero based, but the shapefile might be one based.  Use mod rather than minus so that only one link gets renumbered, the last one.
          streamLinkNo %= size;
          
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
          location = getLocation(channels[streamLinkNo].shapes[0], channels[streamLinkNo].length, shape->padfX[0], shape->padfY[0]);
        }

      if (NULL != shape)
        {
          SHPDestroyObject(shape);
        }
      
      // At this point we have an intersection between waterbodyLinkNo and streamLinkNo at location along streamLinkNo.
      
      // Insert the intersection into the list for the waterbody.  Do an insertion sort to order downstream intersections toward the head of the list.
      if (!error)
        {
          // Start at the tail of the list and walk towards the head until you find the first intersection downstream of this one.
          tempIntersection = channels[waterbodyLinkNo].lastElement;
          foundIt          = false;
          ignoreIt         = false;
          
          while (NULL != tempIntersection && !foundIt)
            {
              if (tempIntersection->edge == streamLinkNo && epsilonEqual(tempIntersection->endLocation, location))
                {
                  // This is a duplicate.  Ignore it.
                  foundIt  = true;
                  ignoreIt = true;
                }
              else if (DOWNSTREAM == upstreamDownstream(channels, size, tempIntersection, streamLinkNo, location))
                {
                  // Insert the new intersection after here.
                  foundIt = true;
                }
              else
                {
                  tempIntersection = tempIntersection->prev;
                }
            }
          
          if (!ignoreIt)
            {
              error = createLinkElementAfter(channels, size, waterbodyLinkNo, tempIntersection, location, streamLinkNo, -1);
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

// Create a single upstream/downstream link between a waterbody and a stream.
// 
// This function is the first point in the code where stream links can get
// split and moved so from here on out we have to handle the case where
// movedTo is not -1.
// 
// To link the waterbody and stream, first find the location on the stream.
// This may involve traversing moved links.  If the location of the
// intersection is at the beginning or end of a link, just create the
// connection between the existing links.  If the location of the intersection
// is in the middle of a link, split the stream.
//
// This function allows a stream to flow downstream into more than one
// waterbody or a waterbody to flow downstream into more than one stream.
// Additional downstream connections after the first are placed in the upstream
// connection list.  This is okay because we eventually ignore the distinction
// between upstream and downstream links in the simulation.  Flow direction is
// determined by water level only.  However, if we are making a connection
// where a stream is supposed to flow downstream into a waterbody and it
// currently flows downstream into a stream we break that existing downstream
// connection.
// 
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels                    - The channel network as a 1D array of
//                               ChannelLinkStruct.
// size                        - The number of elements in channels.
// waterbodyLinkNo             - The waterbody to link to the stream.
// waterbodyUpstreamDownstream - Specifies whether to link the waterbody
//                               upstream or downstream of the stream.  If
//                               UPSTREAM the waterbody will flow into the
//                               stream.  IF DOWNSTREAM the stream will flow
//                               into the waterbody.
// streamLinkNo                - The stream to link to the waterbody.
// location                    - The 1D location in meters along streamLinkNo
//                               to make the link.
bool linkWaterbodyStreamIntersection(ChannelLinkStruct* channels, int size, int waterbodyLinkNo, UpstreamDownstreamEnum waterbodyUpstreamDownstream,
                                     int streamLinkNo, double location)
{
  bool               error   = false; // Error flag.
  int                ii;              // Loop counter.
  LinkElementStruct* element = NULL;  // The stream element that will be linked to the waterbody.
  double             oldLocation;     // For storing the old location if it is moved.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= waterbodyLinkNo && waterbodyLinkNo < size &&
         (WATERBODY == channels[waterbodyLinkNo].type || ICEMASS == channels[waterbodyLinkNo].type) &&
         (UPSTREAM == waterbodyUpstreamDownstream || DOWNSTREAM == waterbodyUpstreamDownstream) &&
         0 <= streamLinkNo && streamLinkNo < size && STREAM == channels[streamLinkNo].type &&
         0.0 <= location && location <= channels[streamLinkNo].lastElement->endLocation);
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
      
      if (UPSTREAM == waterbodyUpstreamDownstream)
        {
          // The waterbody will be upstream, move the intersection to the end of element.
          location = element->endLocation;
        }
      else
        {
          // The waterbody will be downstream, move the intersection to the beginning of element.
          location = beginLocation(element);
        }
      
      fprintf(stderr, "WARNING in linkWaterbodyStreamIntersection: Overlap between waterbody %d and mesh edge %d on stream %d.  Intersection moved from %lf "
                      "to %lf.\n", waterbodyLinkNo, element->edge, streamLinkNo, oldLocation, location);
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  assert(0 <= streamLinkNo && streamLinkNo < size && STREAM == channels[streamLinkNo].type && 0.0 <= location && location <= channels[streamLinkNo].length);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  if (epsilonEqual(location, 0.0))
    {
      if (UPSTREAM == waterbodyUpstreamDownstream)
        {
          // The waterbody flows into the beginning of the stream.
          error = addUpstreamConnection(channels, size, waterbodyLinkNo, streamLinkNo);
          
          // Make the waterbody flow into the stream, but don't break any existing downstream connection.  If the waterbody already has a downstream connection
          // just add the new connection to the upstream connection list.
          if (!error)
            {
              if (NOFLOW == channels[waterbodyLinkNo].downstream)
                {
                  channels[waterbodyLinkNo].downstream = streamLinkNo;
                }
              else
                {
                  error = addUpstreamConnection(channels, size, streamLinkNo, waterbodyLinkNo);
                }
            }
        }
      else
        {
          // The links upstream of the stream flow into the waterbody.
          ii = 0;
          
          while (!error && ii < UPSTREAM_SIZE && NOFLOW != channels[streamLinkNo].upstream[ii])
            {
              if (isBoundary(channels[streamLinkNo].upstream[ii]))
                {
                  // If it's a boundary condition code add it to the waterbody and remove it from the stream.
                  error = addUpstreamConnection(channels, size, channels[streamLinkNo].upstream[ii], waterbodyLinkNo);
                  
                  if (!error)
                    {
                      error = removeUpstreamConnection(channels, size, channels[streamLinkNo].upstream[ii], streamLinkNo);
                    }
                }
              else if (STREAM == channels[channels[streamLinkNo].upstream[ii]].type)
                {
                  // If it's a stream break the existing downstream link.
                  error = makeChannelConnection(channels, size, channels[streamLinkNo].upstream[ii], waterbodyLinkNo);
                }
              else
                {
                  // If it's a waterbody don't break the existing downstream link.
                  error = addUpstreamConnection(channels, size, channels[streamLinkNo].upstream[ii], waterbodyLinkNo);
                  
                  if (!error)
                    {
                      if (NOFLOW == channels[channels[streamLinkNo].upstream[ii]].downstream)
                        {
                          channels[channels[streamLinkNo].upstream[ii]].downstream = waterbodyLinkNo;
                        }
                      else
                        {
                          error = addUpstreamConnection(channels, size, waterbodyLinkNo, channels[streamLinkNo].upstream[ii]);
                        }
                    }
                  
                  // Since the existing connection is left go on to the next connection.
                  ii++;
                }
            }
        }
    }
  else if (epsilonEqual(location, channels[streamLinkNo].length))
    {
      if (UPSTREAM == waterbodyUpstreamDownstream)
        {
          // The waterbody flows into the link downstream of the stream.
          error = addUpstreamConnection(channels, size, waterbodyLinkNo, channels[streamLinkNo].downstream);
          
          // Make the waterbody flow into the downstream link, but don't break any existing downstream connection.  If the waterbody already has a downstream
          // connection just add the new connection to the upstream connection list.
          if (!error)
            {
              if (NOFLOW == channels[waterbodyLinkNo].downstream)
                {
                  channels[waterbodyLinkNo].downstream = channels[streamLinkNo].downstream;
                }
              else
                {
                  error = addUpstreamConnection(channels, size, channels[streamLinkNo].downstream, waterbodyLinkNo);
                }
            }
        }
      else
        {
          // The end of the stream flows into the waterbody.
          if (!isBoundary(channels[streamLinkNo].downstream) && WATERBODY == channels[channels[streamLinkNo].downstream].type)
            {
              // Don't break the stream's exsting downstream connection to a waterbody.
              error = addUpstreamConnection(channels, size, streamLinkNo, waterbodyLinkNo);
              
              if (!error)
                {
                  error = addUpstreamConnection(channels, size, waterbodyLinkNo, streamLinkNo);
                }
            }
          else
            {
              // Do break the stream's existing downstream connection to a stream or boundary condition.
              error = makeChannelConnection(channels, size, streamLinkNo, waterbodyLinkNo);
            }
        }
    }
  else
    {
      // If you flow into the middle of a stream, split the stream at location.
      error = splitLink(channels, size, streamLinkNo, element, location);
      
      if (!error)
        {
          if (UPSTREAM == waterbodyUpstreamDownstream)
            {
              // The waterbody will now flow into the beginning of the split link, which is immediately downstream of streamLinkNo.
              error = addUpstreamConnection(channels, size, waterbodyLinkNo, channels[streamLinkNo].downstream);
              
              // Make the waterbody flow into the stream, but don't break any existing downstream connection.  If the waterbody already has a downstream connection
              // just add the new connection to the upstream connection list.
              if (!error)
                {
                  if (NOFLOW == channels[waterbodyLinkNo].downstream)
                    {
                      channels[waterbodyLinkNo].downstream = channels[streamLinkNo].downstream;
                    }
                  else
                    {
                      error = addUpstreamConnection(channels, size, channels[streamLinkNo].downstream, waterbodyLinkNo);
                    }
                }
            }
          else
            {
              // The end of the unmoved part of the stream will now flow into the waterbody.
              error = makeChannelConnection(channels, size, streamLinkNo, waterbodyLinkNo);
            }
        }
    }
  
  return error;
}

// Create all of the upstream/downstream links between waterbodies and streams.
// 
// For any intersection that does not have another intersection downstream of
// it the waterbody will flow into the stream.  For all other intersections the
// stream will flow into the waterbody.
// 
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels - The channel network as a 1D array of ChannelLinkStruct.
// size     - The number of elements in channels.
bool linkAllWaterbodyStreamIntersections(ChannelLinkStruct* channels, int size)
{
  int                    error = false;               // Error flag.
  int                    ii;                          // Loop counter.
  LinkElementStruct*     tempIntersection1;           // For looping over intersections.
  LinkElementStruct*     tempIntersection2;           // For looping over intersections.
  UpstreamDownstreamEnum waterbodyUpstreamDownstream; // Used to flag whether the waterbody is upstream or downstream of the intersection.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Resolve all of the intersections.
  for (ii = 0; !error && ii < size; ii++)
    {
      if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type)
        {
          tempIntersection1 = channels[ii].firstElement;
          
          while (!error && NULL != tempIntersection1)
            {
              tempIntersection2           = tempIntersection1->prev;
              waterbodyUpstreamDownstream = UPSTREAM;
              
              // If there is an intersection downstream of intersection1 then the waterbody will be downstream of tempIntersection1.
              while (NULL != tempIntersection2 && DOWNSTREAM != waterbodyUpstreamDownstream)
                {
                  if (DOWNSTREAM == upstreamDownstream(channels, size, tempIntersection2, tempIntersection1->edge, tempIntersection1->endLocation))
                    {
                      waterbodyUpstreamDownstream = DOWNSTREAM;
                    }
                  
                  tempIntersection2 = tempIntersection2->prev;
                }
              
              error = linkWaterbodyStreamIntersection(channels, size, ii, waterbodyUpstreamDownstream, tempIntersection1->edge, tempIntersection1->endLocation);
              
              tempIntersection1 = tempIntersection1->next;
            }
        }
    }

  return error;
}

// Read all of the intersections between waterbodies and other waterbodies and
// make those links.
// 
// This function opens the waterbodies waterbodies intersections shapefile and
// for each intersection links the two waterbodies together.  No links are
// broken.  Connections are made by adding upstream links if the waterbodies
// already have downstream links.
// 
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels     - The channel network as a 1D array of ChannelLinkStruct.
// size         - The number of elements in channels.
// fileBasename - The basename of the waterbody shapefile to read.
//                readAndLinkWaterbodyWaterbodyIntersections will read the
//                following files: fileBasename.shp, and fileBasename.dbf.
bool readAndLinkWaterbodyWaterbodyIntersections(ChannelLinkStruct* channels, int size, const char* fileBasename)
{
  bool               error = false;        // Error flag.
  int                ii;                   // Loop counter.
  SHPHandle          shpFile;              // Geometry  part of the shapefile.
  DBFHandle          dbfFile;              // Attribute part of the shapefile.
  int                numberOfShapes;       // Number of shapes in the shapefile.
  int                permanent1Index;      // Index of metadata field.
  int                permanent2Index;      // Index of metadata field.
  int                permanent1;           // The permanent code of the first  intersecting waterbody.
  int                permanent2;           // The permanent code of the second intersecting waterbody.
  int                linkNo1;              // The linkno of the first  intersecting waterbody.
  int                linkNo2;              // The linkno of the second intersecting waterbody.
  LinkElementStruct* intersections = NULL; // Singly linked list of intersections for eliminating duplicates.  The two waterbody link numbers are stored in
                                           // edge and movedTo.
  LinkElementStruct* tempIntersection;     // Used for searching intersections.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && NULL != fileBasename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Open the geometry and attribute files.
  shpFile = SHPOpen(fileBasename, "rb");
  dbfFile = DBFOpen(fileBasename, "rb");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NULL != shpFile))
    {
      fprintf(stderr, "ERROR in readAndLinkWaterbodyWaterbodyIntersections: Could not open shp file %s.\n", fileBasename);
      error = true;
    }

  if (NULL == dbfFile)
    {
      fprintf(stderr, "ERROR in readAndLinkWaterbodyWaterbodyIntersections: Could not open dbf file %s.\n", fileBasename);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

  // Get the number of shapes and attribute indices
  if (!error)
    {
      numberOfShapes  = DBFGetRecordCount(dbfFile);
      permanent1Index = DBFGetFieldIndex(dbfFile, "Permanent1");
      permanent2Index = DBFGetFieldIndex(dbfFile, "Permanent_");
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfShapes))
        {
          // Only warn because you could have zero intersections.
          fprintf(stderr, "WARNING in readAndLinkWaterbodyWaterbodyIntersections: Zero shapes in dbf file %s.\n", fileBasename);
        }
      
      if (!(-1 != permanent1Index))
        {
          fprintf(stderr, "ERROR in readAndLinkWaterbodyWaterbodyIntersections: Could not find field Permanent1 in dbf file %s.\n", fileBasename);
          error = true;
        }
      
      if (!(-1 != permanent2Index))
        {
          fprintf(stderr, "ERROR in readAndLinkWaterbodyWaterbodyIntersections: Could not find field Permanent_ in dbf file %s.\n", fileBasename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Read the intersections.
  for (ii = 0; !error && ii < numberOfShapes; ii++)
    {
      permanent1 = DBFReadIntegerAttribute(dbfFile, ii, permanent1Index);
      permanent2 = DBFReadIntegerAttribute(dbfFile, ii, permanent2Index);
      linkNo1    = 0;
      linkNo2    = 0;
      
      if (permanent1 != permanent2)
        {
          while(linkNo1 < size && ((WATERBODY != channels[linkNo1].type && ICEMASS != channels[linkNo1].type) || permanent1 != channels[linkNo1].permanent))
            {
              linkNo1++;
            }

          while(linkNo2 < size && ((WATERBODY != channels[linkNo2].type && ICEMASS != channels[linkNo2].type) || permanent2 != channels[linkNo2].permanent))
            {
              linkNo2++;
            }

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(linkNo1 < size && (WATERBODY == channels[linkNo1].type || ICEMASS == channels[linkNo1].type)))
            {
              fprintf(stderr, "ERROR in readAndLinkWaterbodyWaterbodyIntersections: Could not find waterbody permanent code %d in channel network.\n",
                      permanent1);
              error = true;
            }
      
          if (!(linkNo2 < size && (WATERBODY == channels[linkNo2].type || ICEMASS == channels[linkNo2].type)))
            {
              fprintf(stderr, "ERROR in readAndLinkWaterbodyWaterbodyIntersections: Could not find waterbody permanent code %d in channel network.\n",
                      permanent2);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

          if (!error)
            {
              tempIntersection = intersections;
              
              while(NULL != tempIntersection && !((linkNo1 == tempIntersection->edge    && linkNo2 == tempIntersection->movedTo) ||
                                                  (linkNo1 == tempIntersection->movedTo && linkNo2 == tempIntersection->edge)))
                {
                  tempIntersection = tempIntersection->next;
                }
              
              if (NULL == tempIntersection)
                {
                  // This intersection is not a duplicate.
                  tempIntersection          = new LinkElementStruct;
                  tempIntersection->next    = intersections;
                  tempIntersection->edge    = linkNo1;
                  tempIntersection->movedTo = linkNo2;
                  intersections             = tempIntersection;
                }
            }
        } // End  if (permanent1 != permanent2).
    } // End read the intersections.
  
  // Process the intersections.
  while(!error && NULL != intersections)
    {
      if (NOFLOW == channels[intersections->edge].downstream)
        {
          channels[intersections->edge].downstream = intersections->movedTo;
        }
      else
        {
          error = addUpstreamConnection(channels, size, intersections->movedTo, intersections->edge);
        }
      
      if (!error)
        {
          if (NOFLOW == channels[intersections->movedTo].downstream)
            {
              channels[intersections->movedTo].downstream = intersections->edge;
            }
          else
            {
              error = addUpstreamConnection(channels, size, intersections->edge, intersections->movedTo);
            }
        }
      
      if (!error)
        {
          tempIntersection = intersections->next;

          delete intersections;
          
          intersections = tempIntersection;
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

// Determine the integer number of chanel elements in a given channel link.
// The number of elements will be the link length divided by desiredLength
// rounded either up or down.  Try to get the actual element length as close as
// possible to desiredLength.  For a closeness metric we use the ratio of the
// longer of the desired or actual length to the shorter of the two.
//
// The total length of the link is:
//   (numLinkElements + numLinkElementsFraction) * desiredLength
// If we round down the actual link element length will be:
//   (numLinkElements + numLinkElementsFraction) * desiredLength /
//   numLinkElements
// The actual will be longer and the ratio of actual to desired will be:
//   ((numLinkElements + numLinkElementsFraction) * desiredLength /
//    numLinkElements) / desiredLength
// or:
//   (numLinkElements + numLinkElementsFraction) / numLinkElements
// If we round up the actual link element length will be:
//   (numLinkElements + numLinkElementsFraction) * desiredLength /
//   (numLinkElements + 1)
// The desired will be longer and the ratio of desired to actual will be:
//   desiredLength / ((numLinkElements + numLinkElementsFraction) *
//                    desiredLength / (numLinkElements + 1))
// or:
//   (numLinkElements + 1) / (numLinkElements + numLinkElementsFraction)
// To find the regions where one or the other is less, find the condition where
// they are equal.
//   (numLinkElements + numLinkElementsFraction) / numLinkElements ==
//   (numLinkElements + 1) / (numLinkElements + numLinkElementsFraction)
// Simplifying the equation results in the following:
//   0 == numLinkElements - 2 * numLinkElements * numLinkElementsFraction -
//        numLinkElementsFraction^2
// If this expression is positive use numLinkElements.  If it is negative use
// numLinkElements + 1.  If it is zero it doesn't matter.  If the total link
// length is less than desiredLength then numLinkElements will be zero and you
// must use numLinkElements + 1.
//
// Returns: The number of channel elements in a given channel link.
//
// Parameters:
//
// channels      - The channel network as a 1D array of ChannelLinkStruct.
// size          - The number of elements in channels.
// linkNo        - The link to determine the number of elements for.
// desiredLength - The desired length of each element in meters.
int numberOfLinkElements(ChannelLinkStruct* channels, int size, int linkNo, double desiredLength)
{
  double numLinkElementsFraction = channels[linkNo].length / desiredLength;
  int    numLinkElements         = (int)numLinkElementsFraction;
  
  numLinkElementsFraction -= numLinkElements;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= linkNo && linkNo < size && 0.0 < desiredLength);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  assert(0 <= numLinkElements && 0.0 <= numLinkElementsFraction && 1.0 > numLinkElementsFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  if (0 == numLinkElements || 0 > numLinkElements - 2.0 * numLinkElements * numLinkElementsFraction - numLinkElementsFraction * numLinkElementsFraction)
    {
      numLinkElements++;
    }
  
  return numLinkElements;
}

// FIXME comment
double locationOfMovedLink(ChannelLinkStruct* channels, int size, int linkNoToSearch, int linkNoToFind)
{
  double             location = NAN;
  LinkElementStruct* tempElement;
  
  // FIXME assertions
  
  if (linkNoToSearch == linkNoToFind)
    {
      location = 0.0;
    }
  else
    {
      tempElement = channels[linkNoToSearch].firstElement;
      
      while (NULL != tempElement && isnan(location))
        {
          if (-1 != tempElement->movedTo)
            {
              // If linkNoToFind is not found location will be set to NAN.
              location = beginLocation(tempElement) + locationOfMovedLink(channels, size, tempElement->movedTo, linkNoToFind);
            }
          
          tempElement = tempElement->next;
        }
    }
  
  return location;
}

// FIXME comment
bool createNode(double x, double y, int* nodesSize, int* numberOfNodes, double* nodeX, double* nodeY, int* nodeNumber)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != nodesSize && 0 < *nodesSize && NULL != numberOfNodes && 0 <= *numberOfNodes && NULL != nodeX && NULL != nodeY && NULL != nodeNumber);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  if (*numberOfNodes >= *nodesSize)
    {
      // FIXLATER realloc to make more space.
      fprintf(stderr, "ERROR in createNode: number of nodes exceeds maximum number %d.\n", *nodesSize);
      error = true;
    }
  
  if (!error)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      assert(*numberOfNodes < *nodesSize);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      nodeX[*numberOfNodes] = x;
      nodeY[*numberOfNodes] = y;
      *nodeNumber           = (*numberOfNodes)++;
    }
  
  return error;
}

// Used for the types of salient points.  Salient points are used when doing a parallel walk along channel elements and shape vertices.  A salient point is the
// next point on a channel element that we need to output whether or not it lies on a shape vertex.
typedef enum
{
  LINK_BEGIN,     // The salient point is the beginning of the link.
  ELEMENT_MIDDLE, // The salient point is the middle of an element.
  ELEMENT_END,    // The salient point is the end of an element.
  LINK_END,       // The salient point is the end of the link.
} SalientPointEnum;

// FIXME comment
bool writeChannelNetwork(ChannelLinkStruct* channels, int size)
{
  bool             error            = false; // Error flag.
  int              ii, jj, kk, ll, mm;       // Loop counters.  Generally, ii is for links, jj is for elements or shapes, kk is for element vertices, ll is for
                                             // shape vertices, and mm is for neighbors.
  int              numberOfElements = 0;     // Number of channel elements.
  int*             elementCenters   = NULL;  // Node numbers of the element centers.
  intarraycvn*     vertices         = NULL;  // Node numbers of the element vertices.
  int              nodesSize;                // Size of nodesX and nodesY arrays.
  int              numberOfNodes    = 0;     // Number of channel nodes.
  double*          nodesX           = NULL;  // X coordinates of nodes.
  double*          nodesY           = NULL;  // Y coordinates of nodes.
  SHPObject*       shape;                    // Shape object of a link.
  double           salientPointLocation;     // The 1D location in meters along a link of a salient point.
  SalientPointEnum salientPointType;         // The type of a salient point.
  double           salientPointX;            // X coordinate of salient point.
  double           salientPointY;            // Y coordinate of salient point.
  double           salientPointFraction;     // Fraction of distance between two shape vertices of salient point.
  double           shapeVertexLocation;      // The 1D location in meters along a link of a shape vertex.
  bool             done;                     // Termination condition for complex loop.
  double           xMin;                     // For computing bounding box of waterbody.
  double           xMax;                     // For computing bounding box of waterbody.
  double           yMin;                     // For computing bounding box of waterbody.
  double           yMax;                     // For computing bounding box of waterbody.
  FILE*            elementFile;              // Output file for channel elements.
  FILE*            nodeFile;                 // Output file for channel nodes.
  
  // FIXME error checking
  
  for (ii = 0; ii < size; ii++)
    {
      channels[ii].elementStart = numberOfElements;
      
      if (STREAM == channels[ii].type)
        {
          // FIXME calculate desired length based on channel size.  For now use fixed 100 meters.
          channels[ii].numberOfElements = numberOfLinkElements(channels, size, ii, 100.0);
        }
      else if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type)
        {
          channels[ii].numberOfElements = 1;
        }
      else
        {
          channels[ii].numberOfElements = 0;
        }
      
      numberOfElements += channels[ii].numberOfElements;
    }
  
  elementCenters = new int[numberOfElements];
  vertices       = new intarraycvn[numberOfElements];
  
  // vertices must be initialized because they may be written ahead.
  for (jj = 0; jj < numberOfElements; jj++)
    {
      for (kk = 0; kk < ChannelElement::channelVerticesSize; kk++)
        {
          vertices[jj][kk] = -1;
        }
    }
  
  // Start out assuming sixteen times as many nodes as vertices.  Realloc if this proves too few.
  nodesSize = 16 * numberOfElements;
  nodesX    = new double[nodesSize];
  nodesY    = new double[nodesSize];
  
  // Loop over all used links.  For each link loop over shapes filling in elementCenters and vertices and creating nodes as you go.
  for (ii = 0; !error && ii < size; ii++)
    {
      if (STREAM == channels[ii].type)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          assert(0 <= channels[ii].permanent && channels[ii].permanent <= size);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          // Get shape from the original link number in case this link was moved.
          shape = channels[channels[ii].permanent % size].shapes[0];
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(NULL != shape))
            {
              fprintf(stderr, "ERROR in writeChannelNetwork: Channel link %d does not have a shape.\n", ii);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          
          if (!error)
            {
              // Start processing the first element of the link.
              jj = channels[ii].elementStart;
              kk = 0; // Index into vertices[jj].
              
              // Get the location along shape of the beginning of the link in case this link was moved.  If the link was not moved this will return zero.
              salientPointLocation = locationOfMovedLink(channels, size, channels[ii].permanent % size, ii);
              salientPointType     = LINK_BEGIN;
              
              assert(!isnan(salientPointLocation));
              
              // Move to the first shape vertex that is at or beyond the beginning of the link.
              ll                  = shape->nVertices - 1;
              shapeVertexLocation = 0.0;
              
              while (0 < ll && epsilonLess(shapeVertexLocation, salientPointLocation))
                {
                  // Advance the shape vertex.
                  ll--;
                  shapeVertexLocation += sqrt((shape->padfX[ll] - shape->padfX[ll + 1]) * (shape->padfX[ll] - shape->padfX[ll + 1]) +
                                              (shape->padfY[ll] - shape->padfY[ll + 1]) * (shape->padfY[ll] - shape->padfY[ll + 1]));
                }
              
              done = false;
              
              while (!error && !done)
                {
                  if (0 < ll && epsilonLess(shapeVertexLocation, salientPointLocation))
                    {
                      if (kk < ChannelElement::channelVerticesSize)
                        {
                          // Save the shape vertex
                          error = createNode(shape->padfX[ll], shape->padfY[ll], &nodesSize, &numberOfNodes, nodesX, nodesY, &vertices[jj][kk++]);

                          if (!error)
                            {
                              // Advance the shape vertex.
                              ll--;
                              shapeVertexLocation += sqrt((shape->padfX[ll] - shape->padfX[ll + 1]) * (shape->padfX[ll] - shape->padfX[ll + 1]) +
                                                          (shape->padfY[ll] - shape->padfY[ll + 1]) * (shape->padfY[ll] - shape->padfY[ll + 1]));
                            }
                        }
                      else
                        {
                          fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of vertices exceeds maximum number %d.\n",
                                  jj, ChannelElement::channelVerticesSize);
                          error = true;
                        }
                    }
                  else if (LINK_BEGIN == salientPointType)
                    {
                      // If the first vertex hasn't already been created by a neighbor, create it.
                      if (-1 == vertices[jj][kk])
                        {
                          if (epsilonLessOrEqual(shapeVertexLocation, salientPointLocation))
                            {
                              // Use the shape vertex as the salient point.
                              salientPointX = shape->padfX[ll];
                              salientPointY = shape->padfY[ll];
                            }
                          else
                            {
                              // Find the salient point between the shape vertex and the previous shape vertex.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                              assert(ll < shape->nVertices - 1);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                              
                              salientPointFraction = (shapeVertexLocation - salientPointLocation) /
                                                     sqrt((shape->padfX[ll] - shape->padfX[ll + 1]) * (shape->padfX[ll] - shape->padfX[ll + 1]) +
                                                          (shape->padfY[ll] - shape->padfY[ll + 1]) * (shape->padfY[ll] - shape->padfY[ll + 1]));
                              
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                              assert(0.0 < salientPointFraction && 1.0 > salientPointFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                              
                              salientPointX        = (1 - salientPointFraction) * shape->padfX[ll] + salientPointFraction * shape->padfX[ll + 1];
                              salientPointY        = (1 - salientPointFraction) * shape->padfY[ll] + salientPointFraction * shape->padfY[ll + 1];
                            }
                          
                          error = createNode(salientPointX, salientPointY, &nodesSize, &numberOfNodes, nodesX, nodesY, &vertices[jj][kk]);
                          
                          if (!error)
                            {
                              // Set the vertex in all upstream STREAM neighbors.
                              for (mm = 0; mm < UPSTREAM_SIZE && NOFLOW != channels[ii].upstream[mm]; mm++)
                                {
                                  if (!isBoundary(channels[ii].upstream[mm]) && STREAM == channels[channels[ii].upstream[mm]].type)
                                    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                                      assert(-1 == vertices[channels[channels[ii].upstream[mm]].elementStart +
                                                            channels[channels[ii].upstream[mm]].numberOfElements - 1]
                                                           [ChannelElement::channelVerticesSize - 1]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

                                      vertices[channels[channels[ii].upstream[mm]].elementStart +
                                               channels[channels[ii].upstream[mm]].numberOfElements - 1]
                                              [ChannelElement::channelVerticesSize - 1] = vertices[jj][kk];
                                    }
                                }
                            }
                        }
                      
                      if (!error)
                        {
                          // Whether it was just created or already there the first vertex is done.
                          kk++;

                          // If the shape vertex was already used as the salient point advance past it.
                          if (0 < ll && epsilonLessOrEqual(shapeVertexLocation, salientPointLocation))
                            {
                              ll--;
                              shapeVertexLocation += sqrt((shape->padfX[ll] - shape->padfX[ll + 1]) * (shape->padfX[ll] - shape->padfX[ll + 1]) +
                                  (shape->padfY[ll] - shape->padfY[ll + 1]) * (shape->padfY[ll] - shape->padfY[ll + 1]));
                            }

                          // Advance half an element length to the next salient point.
                          salientPointLocation += channels[ii].length / (channels[ii].numberOfElements * 2.0);
                          salientPointType      = ELEMENT_MIDDLE;
                        }
                    } // End else if (LINK_BEGIN == salientPointType).
                  else if (ELEMENT_MIDDLE == salientPointType)
                    {
                      if (epsilonLessOrEqual(shapeVertexLocation, salientPointLocation))
                        {
                          // Use the shape vertex as the salient point.
                          salientPointX = shape->padfX[ll];
                          salientPointY = shape->padfY[ll];
                        }
                      else
                        {
                          // Find the salient point between the shape vertex and the previous shape vertex.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                          assert(ll < shape->nVertices - 1);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                          
                          salientPointFraction = (shapeVertexLocation - salientPointLocation) /
                                                 sqrt((shape->padfX[ll] - shape->padfX[ll + 1]) * (shape->padfX[ll] - shape->padfX[ll + 1]) +
                                                      (shape->padfY[ll] - shape->padfY[ll + 1]) * (shape->padfY[ll] - shape->padfY[ll + 1]));
                          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                              assert(0.0 < salientPointFraction && 1.0 > salientPointFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                              
                          salientPointX        = (1 - salientPointFraction) * shape->padfX[ll] + salientPointFraction * shape->padfX[ll + 1];
                          salientPointY        = (1 - salientPointFraction) * shape->padfY[ll] + salientPointFraction * shape->padfY[ll + 1];
                        }
                      
                      error = createNode(salientPointX, salientPointY, &nodesSize, &numberOfNodes, nodesX, nodesY, &elementCenters[jj]);
                      
                      // If the shape vertex is at the same location use the just created node number.
                      if (!error && 0 < ll && epsilonLessOrEqual(shapeVertexLocation, salientPointLocation))
                        {
                          if (kk < ChannelElement::channelVerticesSize)
                            {
                              // Save the shape vertex
                              vertices[jj][kk++] = elementCenters[jj];

                              // Advance the shape vertex.
                              ll--;
                              shapeVertexLocation += sqrt((shape->padfX[ll] - shape->padfX[ll + 1]) * (shape->padfX[ll] - shape->padfX[ll + 1]) +
                                                          (shape->padfY[ll] - shape->padfY[ll + 1]) * (shape->padfY[ll] - shape->padfY[ll + 1]));
                            }
                          else
                            {
                              fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of vertices exceeds maximum number %d.\n",
                                      jj, ChannelElement::channelVerticesSize);
                              error = true;
                            }
                        }
                      
                      if (!error)
                        {
                          // Advance half an element length to the next salient point.
                          salientPointLocation += channels[ii].length / (channels[ii].numberOfElements * 2.0);

                          if (jj == channels[ii].elementStart + channels[ii].numberOfElements - 1)
                            {
                              salientPointType = LINK_END;
                            }
                          else
                            {
                              salientPointType = ELEMENT_END;
                            }
                        }
                    } // End else if (ELEMENT_MIDDLE == salientPointType).
                  else if (ELEMENT_END == salientPointType)
                    {
                      if (kk < ChannelElement::channelVerticesSize)
                        {
                          // Create the vertex.
                          if (epsilonLessOrEqual(shapeVertexLocation, salientPointLocation))
                            {
                              // Use the shape vertex as the salient point.
                              salientPointX = shape->padfX[ll];
                              salientPointY = shape->padfY[ll];
                            }
                          else
                            {
                              // Find the salient point between the shape vertex and the previous shape vertex.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                              assert(ll < shape->nVertices - 1);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                              
                              salientPointFraction = (shapeVertexLocation - salientPointLocation) /
                                                     sqrt((shape->padfX[ll] - shape->padfX[ll + 1]) * (shape->padfX[ll] - shape->padfX[ll + 1]) +
                                                          (shape->padfY[ll] - shape->padfY[ll + 1]) * (shape->padfY[ll] - shape->padfY[ll + 1]));
                              
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                              assert(0.0 < salientPointFraction && 1.0 > salientPointFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                              
                              salientPointX        = (1 - salientPointFraction) * shape->padfX[ll] + salientPointFraction * shape->padfX[ll + 1];
                              salientPointY        = (1 - salientPointFraction) * shape->padfY[ll] + salientPointFraction * shape->padfY[ll + 1];
                            }
                          
                          error = createNode(salientPointX, salientPointY, &nodesSize, &numberOfNodes, nodesX, nodesY, &vertices[jj][kk]);
                          
                          if (!error)
                            {
                              // Fill in the rest of the vertices with duplicates of the last vertex.
                              for (mm = kk + 1; mm < ChannelElement::channelVerticesSize; mm++)
                                {
                                  vertices[jj][mm] = vertices[jj][kk];
                                }
                              
                              // Set the first vertex in the next element.
                              vertices[jj + 1][0] = vertices[jj][kk];
                              
                              // Advance to the next element.
                              jj++;
                              kk = 1;
                            }
                        }
                      else
                        {
                          fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of vertices exceeds maximum number %d.\n",
                                  jj, ChannelElement::channelVerticesSize);
                          error = true;
                        }
                      
                      // If the shape vertex was already used as the salient point advance past it.
                      if (0 < ll && epsilonLessOrEqual(shapeVertexLocation, salientPointLocation))
                        {
                          ll--;
                          shapeVertexLocation += sqrt((shape->padfX[ll] - shape->padfX[ll + 1]) * (shape->padfX[ll] - shape->padfX[ll + 1]) +
                                                      (shape->padfY[ll] - shape->padfY[ll + 1]) * (shape->padfY[ll] - shape->padfY[ll + 1]));
                        }
                      
                      // Advance half an element length to the next salient point.
                      salientPointLocation += channels[ii].length / (channels[ii].numberOfElements * 2.0);
                      salientPointType      = ELEMENT_MIDDLE;
                    } // End else if (ELEMENT_END == salientPointType).
                  else if (LINK_END == salientPointType)
                    {
                      if (kk < ChannelElement::channelVerticesSize)
                        {
                          // If the last vertex hasn't already been created by a neighbor, create it.
                          if (-1 == vertices[jj][ChannelElement::channelVerticesSize - 1])
                            {
                              if (epsilonLessOrEqual(shapeVertexLocation, salientPointLocation))
                                {
                                  // Use the shape vertex as the salient point.
                                  salientPointX = shape->padfX[ll];
                                  salientPointY = shape->padfY[ll];
                                }
                              else
                                {
                                  // Find the salient point between the shape vertex and the previous shape vertex.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                                  assert(ll < shape->nVertices - 1);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

                                  salientPointFraction = (shapeVertexLocation - salientPointLocation) /
                                                         sqrt((shape->padfX[ll] - shape->padfX[ll + 1]) * (shape->padfX[ll] - shape->padfX[ll + 1]) +
                                                              (shape->padfY[ll] - shape->padfY[ll + 1]) * (shape->padfY[ll] - shape->padfY[ll + 1]));

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                                  assert(0.0 < salientPointFraction && 1.0 > salientPointFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

                                  salientPointX        = (1 - salientPointFraction) * shape->padfX[ll] + salientPointFraction * shape->padfX[ll + 1];
                                  salientPointY        = (1 - salientPointFraction) * shape->padfY[ll] + salientPointFraction * shape->padfY[ll + 1];
                                }

                              error = createNode(salientPointX, salientPointY, &nodesSize, &numberOfNodes, nodesX, nodesY, &vertices[jj][kk]);

                              if (!error)
                                {
                                  // Set the vertex in the downstream STREAM neighbor and all upstream STREAM neighbors of that neighbor.
                                  if (!isBoundary(channels[ii].downstream) && STREAM == channels[channels[ii].downstream].type)
                                    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                                      assert(-1 == vertices[channels[channels[ii].downstream].elementStart][0]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

                                      vertices[channels[channels[ii].downstream].elementStart][0] = vertices[jj][kk];
                                      
                                      for (mm = 0; mm < UPSTREAM_SIZE && NOFLOW != channels[channels[ii].downstream].upstream[mm]; mm++)
                                        {
                                          if (ii != channels[channels[ii].downstream].upstream[mm] &&
                                              !isBoundary(channels[channels[ii].downstream].upstream[mm]) &&
                                              STREAM == channels[channels[channels[ii].downstream].upstream[mm]].type)
                                            {
        #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                                              assert(-1 == vertices[channels[channels[channels[ii].downstream].upstream[mm]].elementStart +
                                                                    channels[channels[channels[ii].downstream].upstream[mm]].numberOfElements - 1]
                                                                   [ChannelElement::channelVerticesSize - 1]);
        #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

                                              vertices[channels[channels[channels[ii].downstream].upstream[mm]].elementStart +
                                                       channels[channels[channels[ii].downstream].upstream[mm]].numberOfElements - 1]
                                                      [ChannelElement::channelVerticesSize - 1] = vertices[jj][kk];
                                            }
                                        }
                                    }
                                }
                            } // End if the last vertex hasn't already been created by a neighbor, create it.
                          else if (kk != ChannelElement::channelVerticesSize - 1)
                            {
                              // Move the vertex created by a neighbor into the right position.
                              vertices[jj][kk] = vertices[jj][ChannelElement::channelVerticesSize - 1];
                            }

                          if (!error)
                            {
                              // Fill in the rest of the vertices with duplicates of the last vertex.
                              for (mm = kk + 1; mm < ChannelElement::channelVerticesSize; mm++)
                                {
                                  vertices[jj][mm] = vertices[jj][kk];
                                }
                              
                              done = true;
                            }
                        }
                      else
                        {
                          fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of vertices exceeds maximum number %d.\n",
                                  jj, ChannelElement::channelVerticesSize);
                          error = true;
                        }
                    } // End else if (LINK_END == salientPointType).
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                  else
                    {
                      assert(false);
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                } // End while (!error && !done).
            } // End if (!error).
        } // End if (STREAM == channels[ii].type).
      else if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type)
        {
          shape = channels[ii].shapes[0];
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(NULL != shape))
            {
              fprintf(stderr, "ERROR in writeChannelNetwork: Channel link %d does not have a shape.\n", ii);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          
          if (!error)
            {
              // Set the center of the waterbody as the center of the bounding box of all of its shapes.
              xMin = shape->dfXMin;
              xMax = shape->dfXMax;
              yMin = shape->dfYMin;
              yMax = shape->dfYMax;

              for (jj = 1; jj < SHAPES_SIZE && NULL != channels[ii].shapes[jj]; jj++)
                {
                  shape = channels[ii].shapes[jj];

                  if (xMin > shape->dfXMin)
                    {
                      xMin = shape->dfXMin;
                    }

                  if (xMax < shape->dfXMax)
                    {
                      xMax = shape->dfXMax;
                    }

                  if (yMin > shape->dfYMin)
                    {
                      yMin = shape->dfYMin;
                    }

                  if (yMax < shape->dfYMax)
                    {
                      yMax = shape->dfYMax;
                    }
                }

              error = createNode(0.5 * (xMin + xMax), 0.5 * (yMin + yMax), &nodesSize, &numberOfNodes, nodesX, nodesY,
                                 &elementCenters[channels[ii].elementStart]);
            }

          // Create the vertices.
          kk = 0;
          
          for (jj = 0; !error && jj < SHAPES_SIZE && NULL != channels[ii].shapes[jj]; jj++)
            {
              shape = channels[ii].shapes[jj];

              for (ll = shape->nVertices - 1; !error && ll >= 0; ll--)
                {
                  if (kk < ChannelElement::channelVerticesSize)
                    {
                      // FIXME see if the point is a duplicate with any connected link.
                      
                      error = createNode(shape->padfX[ll], shape->padfY[ll], &nodesSize, &numberOfNodes, nodesX, nodesY,
                                         &vertices[channels[ii].elementStart][kk++]);
                    }
                  else
                    {
                      fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of vertices exceeds maximum number %d.\n",
                              channels[ii].elementStart, ChannelElement::channelVerticesSize);
                      error = true;
                    }
                }
            }
          
          // Fill in the rest of the vertices with duplicates of the last vertex.
          kk--;
          
          for (mm = kk + 1; mm < ChannelElement::channelVerticesSize; mm++)
            {
              vertices[channels[ii].elementStart][mm] = vertices[channels[ii].elementStart][kk];
            }
        } // End else if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type).
    } // End loop over all used links.  For each link loop over shapes filling in elementCenters and vertices and creating nodes as you go.

  // FIXME error check these
  nodeFile     = fopen("/share/CI-WATER Simulation Data/small_green_mesh/mesh.1.chan.node", "w");
  elementFile  = fopen("/share/CI-WATER Simulation Data/small_green_mesh/mesh.1.chan.ele",  "w");
  
  fprintf(elementFile, "%d\n", numberOfElements);
  
  for (ii = 0; ii < size; ii++)
    {
      for (jj = channels[ii].elementStart; jj < channels[ii].elementStart + channels[ii].numberOfElements; jj++)
        {
          fprintf(elementFile, "%d %d %d %lf %d %d %d %d", jj, channels[ii].type, channels[ii].permanent, channels[ii].length / channels[ii].numberOfElements,
                  elementCenters[jj], ChannelElement::channelVerticesSize, 0 /* FIXME ChannelElement::channelNeighborsSize */, 0 /* FIXME ChannelElement::meshNeighborsSize */);
          
          for (kk = 0; kk < ChannelElement::channelVerticesSize; kk++)
            {
              fprintf(elementFile, " %d", vertices[jj][kk]);
            }
          
          // FIXME output channel neighbors
          
          // FIXME output mesh neighbors

          fprintf(elementFile, "\n");
        }
    }
  
  fprintf(nodeFile, "%d 2 0 1\n", numberOfNodes);
  
  for (ii = 0; ii < numberOfNodes; ii++)
    {
      fprintf(nodeFile, "%d %lf %lf 0\n", ii, nodesX[ii], nodesY[ii]);
    }

  // Deallocate node arrays.
  if (NULL != elementCenters)
    {
      delete[] elementCenters;
    }
  
  if (NULL != vertices)
    {
      delete[] vertices;
    }
  
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

  if (NULL != elementFile)
    {
      fclose(elementFile);
    }
  
  return error;
}

// Free memory allocated for the channel network.
//
// Returns: true if there is an error, false otherwise.  Even if there is an
// error every effort is made to free as much as possible.
//
// Parameters:
//
// channels - A pointer to the channel network passed by reference.
//            Will be set to NULL after the memory is deallocated.
// size     - A scalar passed by reference containing the number of elements in
//            channels.  Will be set to zero after the memory is deallocated.
bool channelNetworkDealloc(ChannelLinkStruct** channels, int* size)
{
  bool error = false; // Error flag.
  bool tempError;     // Error flag.
  int  ii, jj;        // Loop counters.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && NULL != *channels && NULL != size && 0 < *size);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  for (ii = 0; ii < *size; ii++)
    {
      for (jj = 0; jj < SHAPES_SIZE; jj++)
        {
          if (NULL != (*channels)[ii].shapes[jj])
            {
              SHPDestroyObject((*channels)[ii].shapes[jj]);
            }
        }
      
      while (NULL != (*channels)[ii].firstElement)
        {
          tempError = killLinkElement(*channels, *size, ii, (*channels)[ii].firstElement);
          error     = error || tempError;
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      assert(NULL == (*channels)[ii].lastElement);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  delete[] *channels;
  
  *channels = NULL;
  *size     = 0;

  return error;
}

// FIXME make into a more user friendly main program.
int main(void)
{
  bool               error = false; // Error flag.
  bool               tempError;     // Error flag.
  int                ii;            // Loop counter.
  ChannelLinkStruct* channels;      // The channel network.
  int                size;          // The number of elements in channels.
  
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
  
  if (!error)
    {
      error = readWaterbodyStreamIntersections(channels, size, "/share/CI-WATER Simulation Data/small_green_mesh/mesh_waterbodies_streams_intersections");
    }
  
  if (!error)
    {
      error = linkAllWaterbodyStreamIntersections(channels, size);
    }
  
  if (!error)
    {
      error = readAndLinkWaterbodyWaterbodyIntersections(channels, size,
                                                         "/share/CI-WATER Simulation Data/small_green_mesh/mesh_waterbodies_waterbodies_intersections");
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
  
  return 0;
}
