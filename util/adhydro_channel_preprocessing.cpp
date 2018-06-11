/*
 * Filename: adhydro_channel_preprocessing.cpp
 * Author:   Robert Steinke
 * 
 * Purpose: This file contains an executable program that takes information from various sources about a channel network, waterbodies, and a 2D triangular land surface mesh.
 *          It then merges this data to assign neighbor relationships between elements and does some massaging to eliminate awkward situations that would slow down simulations run on the result.
 * 
 * Acknowledgements:
 * 
 * Financial support for this research includes NSF EPSCoR grant No. 1135483, and the National Oceanic and Atmospheric Administration, U.S. National Water Center, through the Universities Corporation for Atmospheric Research.
 * 
 * Copyright (C) 2017 University of Wyoming
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3, 2007 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * A copy of the full GNU General Public License is available at
 * http://www.gnu.org/copyleft/gpl.html
 * or from:
 * The Free Software Foundation, Inc., 59 Temple Place - Suite 330
 * Boston, MA  02111-1307, USA.
 * 
 */

#include "all.h"
#include <shapefil.h>
#include <cstdio>
#include <cstring>
#include <string>

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

// Locations of input files.  Edit these and recompile to change which files get read.
// FIXME make these command line parameters.

std::string mapDir;
std::string baseName;
std::string resolution;
std::string streamNetworkShapefile;
std::string waterbodiesShapefile;
std::string waterbodiesStreamsIntersectionsShapefile;
std::string waterbodiesWaterbodiesIntersectionsShapefile;
std::string meshLinkFilename;
std::string meshNodeFilename;
std::string meshElementFilename;
std::string meshEdgeFilename;
/*
const char* streamNetworkShapefile                       = "/share/CI-WATER_Simulation_Data/upper_colorado_mesh/projectednet";
const char* waterbodiesShapefile                         = "/share/CI-WATER_Simulation_Data/upper_colorado_mesh/mesh_waterbodies";
const char* waterbodiesStreamsIntersectionsShapefile     = "/share/CI-WATER_Simulation_Data/upper_colorado_mesh/mesh_waterbodies_streams_intersections";
const char* waterbodiesWaterbodiesIntersectionsShapefile = "/share/CI-WATER_Simulation_Data/upper_colorado_mesh/mesh_waterbodies_waterbodies_intersections";
const char* meshLinkFilename                             = "/share/CI-WATER_Simulation_Data/upper_colorado_mesh/mesh.1.link";
const char* meshNodeFilename                             = "/share/CI-WATER_Simulation_Data/upper_colorado_mesh/mesh.1.node";
const char* meshElementFilename                          = "/share/CI-WATER_Simulation_Data/upper_colorado_mesh/mesh.1.ele";
const char* meshEdgeFilename                             = "/share/CI-WATER_Simulation_Data/upper_colorado_mesh/mesh.1.edge";
*/
// Locations of output files.  Edit these and recompile to change which files get written.
// FIXME make these command line parameters.
std::string channelNodeFilename;
std::string channelElementFilename;
std::string channelPruneFilename;
/*
const char* channelNodeFilename    = "/share/CI-WATER_Simulation_Data/upper_colorado_mesh/mesh.1.chan.node";
const char* channelElementFilename = "/share/CI-WATER_Simulation_Data/upper_colorado_mesh/mesh.1.chan.ele";
const char* channelPruneFilename   = "/share/CI-WATER_Simulation_Data/upper_colorado_mesh/mesh.1.chan.prune";
*/

// These values should be the same as the values in the ChannelElement class, but we can't include channel_element.h here because this isn't a Charm++ program.
// If they are not the same it's actually not that bad.  When the output of this program gets read by the file managers if these numbers are less than the
// sizes in the ChannelElement class the extra entries will get filled in with defaults.  If they are more the file managers will report an error and not read
// the files.
// FIXME These numbers might not exist as fixed sizes in the rewritten code if everything is stored in vectors instead of arrays.
const int ChannelElement_channelVerticesSize  = 40;  // Maximum number of channel vertices.  Unlike the mesh, vertices are not necessarily equal to neighbors.
const int ChannelElement_channelNeighborsSize = 4;   // Maximum number of channel neighbors.
const int ChannelElement_meshNeighborsSize    = 8; // Maximum number of mesh neighbors.

#define SHAPES_SIZE     (20) // Size of array of shapes in ChannelLinkStruct.
#define UPSTREAM_SIZE   (95) // Size of array of upstream links in ChannelLinkStruct.
#define DOWNSTREAM_SIZE (4)  // Size of array of downstream links in ChannelLinkStruct.

// Used for the return value of upstreamDownstream.
typedef enum
{
  COINCIDENT, // The two intersections are at the same location.
  UPSTREAM,   // intersection2 is upstream   of intersection1.
  DOWNSTREAM, // intersection2 is downstream of intersection1.
  UNRELATED,  // Neither is upstream or downstream of the other.
} UpstreamDownstreamEnum;

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
//
// These structures are also used in waterbody link to store lists of mesh
// edges the waterbody is connected to.  In this case, the mesh edge number is
// stored in edge, endLocation is 0.0, and movedTo is -1.
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
  double             upstreamContributingArea;    // For streams, contributing area in square meters at   upstream end of shape.  For others, 0.0.
  double             downstreamContributingArea;  // For streams, contributing area in square meters at downstream end of shape.  For others, 0.0.
  int                streamOrder;                 // For unmoved streams, stream order.  For others, 0.
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

// Returns: The contributing area in square meters at a particular location along a stream.
//
// Parameters:
//
// upstreamContributingArea   - contributing area in square meters at   upstream end of shape.
// downstreamContributingArea - contributing area in square meters at downstream end of shape.
// lengthFraction             - Fraction of the length along the shape at the location to determine collecting area for.
//                            - 0 is the upstream end of the shape, 1 is the downstream end of the shape.  Must be between 0 and 1.
double contributingAreaAtLocation(double upstreamContributingArea, double downstreamContributingArea, double lengthFraction)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(0.0 < upstreamContributingArea && upstreamContributingArea < downstreamContributingArea && 0.0 <= lengthFraction && 1.0 >= lengthFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  return (1.0 - lengthFraction) * upstreamContributingArea + lengthFraction * downstreamContributingArea;
}

// As we construct the channel network it goes through some transitions when it doesn't quite pass the strictest form of the invariant.  These flags allow
// those conditions to pass the invariant temporarily.
bool allowNotUsedToHaveReachCode;
bool allowStreamToHaveZeroLength;
bool allowNonReciprocalUpstreamDownstreamLinks;
bool allowLastLinkElementLengthNotEqualToLinkLength;
bool allowLinkElementEndLocationsNotMonotonicallyIncreasing;

// Check the invariant on the channel network.
//
// Returns: true if the invariant is violated, false otherwise.
//
// Parameters:
//
// channels - The channel network as a 1D array of ChannelLinkStruct.
// size     - The number of elements in channels.
bool channelNetworkCheckInvariant(ChannelLinkStruct* channels, int size)
{
  bool               error = false;  // Error flag.
  int                ii, jj, kk;     // Loop counters.
  bool               foundCondition; // Used as a flag for various detected conditions.
  LinkElementStruct* tempElement;    // For looping through the link element linked list.
  LinkElementStruct* laggardElement; // Used to check for cycles.
  bool               advanceLaggard; // Toggled to advance laggardElement one for every two of tempElement.
  
  if (!(NULL != channels))
    {
      fprintf(stderr, "ERROR in channelNetworkCheckInvariant: channels must not be NULL.\n");
      error = true;
    }
  
  if (!(0 <= size))
    {
      fprintf(stderr, "ERROR in channelNetworkCheckInvariant: size must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!error)
    {
      // Check all links.
      for (ii = 0; ii < size; ii++)
        {
          if (NOT_USED == channels[ii].type)
            {
              if (!((-1 == channels[ii].reachCode || (allowNotUsedToHaveReachCode && 0 <= channels[ii].reachCode)) && 0.0 == channels[ii].length &&
                    0.0 == channels[ii].upstreamContributingArea && 0.0 == channels[ii].downstreamContributingArea && 0 == channels[ii].streamOrder &&
                    NULL == channels[ii].firstElement && NULL == channels[ii].lastElement && 0 <= channels[ii].elementStart &&
                    0 == channels[ii].numberOfElements))
                {
                  fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: unused link has invalid value(s).\n", ii);
                  error = true;
                }
              
              for (jj = 0; jj < SHAPES_SIZE; jj++)
                {
                  if (!(NULL == channels[ii].shapes[jj]))
                    {
                      fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: unused link shapes must be NULL.\n", ii);
                      error = true;
                    }
                }
              
              for (jj = 0; jj < UPSTREAM_SIZE; jj++)
                {
                  if (!(NOFLOW == channels[ii].upstream[jj]))
                    {
                      fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: unused link upstream must be NOFLOW.\n", ii);
                      error = true;
                    }
                }
              
              for (jj = 0; jj < DOWNSTREAM_SIZE; jj++)
                {
                  if (!(NOFLOW == channels[ii].downstream[jj]))
                    {
                      fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: unused link downstream must be NOFLOW.\n", ii);
                      error = true;
                    }
                }
            }
          else if (STREAM == channels[ii].type || PRUNED_STREAM == channels[ii].type)
            {
              if (!(0 <= channels[ii].reachCode && channels[ii].reachCode < size))
                {
                  fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link has invalid value(s).\n", ii);
                  error = true;
                }
              
              if (ii == channels[ii].reachCode)
                {
                  if (!(NULL != channels[ii].shapes[0] && 0.0 < channels[ii].upstreamContributingArea && 0.0 < channels[ii].downstreamContributingArea &&
                        0 < channels[ii].streamOrder))
                    {
                      fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: unmoved stream link has invalid value(s).\n", ii);
                      error = true;
                    }
                  
                  for (jj = 1; jj < SHAPES_SIZE; jj++)
                    {
                      if (!(NULL == channels[ii].shapes[jj]))
                        {
                          fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: unmoved stream link must have only one shape.\n", ii);
                          error = true;
                        }
                    }
                }
              else
                {
                  if (!((STREAM == channels[channels[ii].reachCode].type || PRUNED_STREAM == channels[channels[ii].reachCode].type) &&
                        0.0 < channels[ii].upstreamContributingArea && 0.0 < channels[ii].downstreamContributingArea && 0 == channels[ii].streamOrder))
                    {
                      fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: moved stream link has invalid value(s).\n", ii);
                      error = true;
                    }
                  
                  for (jj = 0; jj < SHAPES_SIZE; jj++)
                    {
                      if (!(NULL == channels[ii].shapes[jj]))
                        {
                          fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: moved stream link shapes must be NULL.\n", ii);
                          error = true;
                        }
                    }
                }
              
              if (STREAM == channels[ii].type)
                {
                  if (!((0.0 < channels[ii].length || (allowStreamToHaveZeroLength && 0.0 == channels[ii].length)) && 0 <= channels[ii].elementStart &&
                        0 <= channels[ii].numberOfElements))
                    {
                      fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link has invalid value(s).\n", ii);
                      error = true;
                    }
                  
                  for (jj = 0; jj < UPSTREAM_SIZE; jj++)
                    {
                      if (0 < jj && !(NOFLOW != channels[ii].upstream[jj - 1] || NOFLOW == channels[ii].upstream[jj]))
                        {
                          fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: upstream links must be compacted to the front of the array.\n",
                                  ii);
                          error = true;
                        }
                      
                      if (isBoundary(channels[ii].upstream[jj]))
                        {
                          if (!(OUTFLOW != channels[ii].upstream[jj]))
                            {
                              fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link upstream must not be OUTFLOW.\n", ii);
                              error = true;
                            }
                        }
                      else
                        {
                          if (!(0 <= channels[ii].upstream[jj] && channels[ii].upstream[jj] < size &&
                                (STREAM == channels[channels[ii].upstream[jj]].type || WATERBODY == channels[channels[ii].upstream[jj]].type ||
                                 ICEMASS == channels[channels[ii].upstream[jj]].type)))
                            {
                              fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link upstream must be valid linkNo.\n", ii);
                              error = true;
                            }
                          else if (!allowNonReciprocalUpstreamDownstreamLinks)
                            {
                              foundCondition = false;

                              for (kk = 0; !foundCondition && kk < DOWNSTREAM_SIZE; kk++)
                                {
                                  foundCondition = (ii == channels[channels[ii].upstream[jj]].downstream[kk]);
                                }
                              
                              if (!foundCondition)
                                {
                                  fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link upstream must have reciprocal downstream "
                                          "link.\n", ii);
                                  error = true;
                                }
                            }
                        }
                    }
                  
                  for (jj = 0; jj < DOWNSTREAM_SIZE; jj++)
                    {
                      if (0 < jj && !(NOFLOW != channels[ii].downstream[jj - 1] || NOFLOW == channels[ii].downstream[jj]))
                        {
                          fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: downstream links must be compacted to the front of the array.\n",
                                  ii);
                          error = true;
                        }
                      
                      if (isBoundary(channels[ii].downstream[jj]))
                        {
                          if (!(INFLOW != channels[ii].downstream[jj]))
                            {
                              fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link downstream must not be INFLOW.\n", ii);
                              error = true;
                            }
                        }
                      else
                        {
                          if (!(0 <= channels[ii].downstream[jj] && channels[ii].downstream[jj] < size &&
                                (STREAM == channels[channels[ii].downstream[jj]].type || WATERBODY == channels[channels[ii].downstream[jj]].type ||
                                 ICEMASS == channels[channels[ii].downstream[jj]].type)))
                            {
                              fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link downstream must be valid linkNo.\n", ii);
                              error = true;
                            }
                          else if (!allowNonReciprocalUpstreamDownstreamLinks)
                            {
                              foundCondition = false;

                              for (kk = 0; !foundCondition && kk < UPSTREAM_SIZE; kk++)
                                {
                                  foundCondition = (ii == channels[channels[ii].downstream[jj]].upstream[kk]);
                                }
                              
                              if (!foundCondition)
                                {
                                  fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link downstream must have reciprocal upstream "
                                          "link.\n", ii);
                                  error = true;
                                }
                            }
                        }
                    }
                } // End if (STREAM == channels[ii].type)
              else // if (PRUNED_STREAM == channels[ii].type)
                {
                  if (!(0.0 <= channels[ii].length && 0 <= channels[ii].elementStart && 0 == channels[ii].numberOfElements))
                    {
                      fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: pruned stream link has invalid value(s).\n", ii);
                      error = true;
                    }
                  
                  for (jj = 0; jj < UPSTREAM_SIZE; jj++)
                    {
                      if (!(NOFLOW == channels[ii].upstream[jj]))
                        {
                          fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: pruned stream link upstream must be NOFLOW.\n", ii);
                          error = true;
                        }
                    }
                  
                  for (jj = 0; jj < DOWNSTREAM_SIZE; jj++)
                    {
                      if (0 < jj && !(NOFLOW != channels[ii].downstream[jj - 1] || NOFLOW == channels[ii].downstream[jj]))
                        {
                          fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: downstream links must be compacted to the front of the array.\n",
                                  ii);
                          error = true;
                        }
                      
                      if (isBoundary(channels[ii].downstream[jj]))
                        {
                          if (!(INFLOW != channels[ii].downstream[jj]))
                            {
                              fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: pruned stream link downstream must not be INFLOW.\n", ii);
                              error = true;
                            }
                        }
                      else
                        {
                          if (!(0 <= channels[ii].downstream[jj] && channels[ii].downstream[jj] < size &&
                                (STREAM == channels[channels[ii].downstream[jj]].type || WATERBODY == channels[channels[ii].downstream[jj]].type ||
                                 ICEMASS == channels[channels[ii].downstream[jj]].type || PRUNED_STREAM == channels[channels[ii].downstream[jj]].type)))
                            {
                              fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: pruned stream link downstream must be valid linkNo.\n", ii);
                              error = true;
                            }
                          else
                            {
                              foundCondition = false;

                              for (kk = 0; !foundCondition && kk < UPSTREAM_SIZE; kk++)
                                {
                                  foundCondition = (ii == channels[channels[ii].downstream[jj]].upstream[kk]);
                                }
                              
                              if (foundCondition)
                                {
                                  fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: pruned stream link downstream must not have reciprocal "
                                          "upstream link.\n", ii);
                                  error = true;
                                }
                            }
                        }
                    }
                } // End if (PRUNED_STREAM == channels[ii].type)
              
              if (!(NULL != channels[ii].firstElement && NULL == channels[ii].firstElement->prev))
                {
                  fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link has invalid link element list.\n", ii);
                  error = true;
                }
              else
                {
                  tempElement    = channels[ii].firstElement; // For looping through the link element linked list.
                  laggardElement = channels[ii].firstElement; // Used to check for cycles.
                  advanceLaggard = false;                     // Toggled to advance laggardElement one for every two of tempElement.
                  foundCondition = false;                     // Turns true when we find our first moved link.

                  while (!error && NULL != tempElement)
                    {
                      if (!(beginLocation(tempElement) < tempElement->endLocation || allowLinkElementEndLocationsNotMonotonicallyIncreasing ||
                            (0.0 == channels[ii].length && tempElement == channels[ii].firstElement && 0.0 == tempElement->endLocation)))
                        {
                          fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link element list end locations must be monotonically "
                                  "increasing.\n", ii);
                          error = true;
                        }
                      
                      if (!foundCondition)
                        {
                          // We are not yet into the moved links.
                          if (-1 == tempElement->movedTo)
                            {
                              if (NULL == tempElement->next)
                                {
                                  // If this is the last element its end location must be the length of the stream.
                                  if (!(tempElement->endLocation == channels[ii].length || allowLastLinkElementLengthNotEqualToLinkLength))
                                    {
                                      fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link element list end location of last "
                                              "unmoved element must be the length of the stream.\n", ii);
                                      error = true;
                                    }
                                }
                              
                              if (STREAM == channels[ii].type)
                                {
                                  if (!(-1 <= tempElement->edge))
                                    {
                                      fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link element list edge must be a valid mesh "
                                              "edge.\n", ii);
                                      error = true;
                                    }
                                }
                              else // if (PRUNED_STREAM == channels[ii].type)
                                {
                                  if (!(-1 == tempElement->edge))
                                    {
                                      fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: pruned stream link element list edge must be "
                                              "unassociated.\n", ii);
                                      error = true;
                                    }
                                }
                            }
                          else
                            {
                              // This is the first moved link.
                              foundCondition = true;
                              
                              // Its begin location must be the length of the stream.
                              if (!(beginLocation(tempElement) == channels[ii].length || allowLastLinkElementLengthNotEqualToLinkLength))
                                {
                                  fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link element list end location of last unmoved "
                                          "element must be the length of the stream.\n", ii);
                                  error = true;
                                }
                              
                              if (!(-1 == tempElement->edge))
                                {
                                  fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link element list moved element edge must be "
                                          "unassociated.\n", ii);
                                  error = true;
                                }
                              
                              if (!(0 <= tempElement->movedTo && tempElement->movedTo < size && (STREAM == channels[tempElement->movedTo].type ||
                                                                                                 PRUNED_STREAM == channels[tempElement->movedTo].type)))
                                {
                                  fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link element list moved to element must be valid "
                                          "linkNo.\n", ii);
                                  error = true;
                                }
                            }
                        } // End if (!foundCondition)
                      else
                        {
                          // We are into the moved links.
                          if (-1 == tempElement->movedTo)
                            {
                              fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link element list moved links must come after all "
                                      "unmoved links.\n", ii);
                              error = true;
                            }
                          else
                            {
                              if (!(-1 == tempElement->edge))
                                {
                                  fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link element list moved element edge must be "
                                          "unassociated.\n", ii);
                                  error = true;
                                }
                              
                              if (!(0 <= tempElement->movedTo && tempElement->movedTo < size && (STREAM == channels[tempElement->movedTo].type ||
                                                                                                 PRUNED_STREAM == channels[tempElement->movedTo].type)))
                                {
                                  fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link element list moved to element must be valid "
                                          "linkNo.\n", ii);
                                  error = true;
                                }
                            }
                        }

                      if (NULL == tempElement->next)
                        {
                          if (!(tempElement == channels[ii].lastElement))
                            {
                              fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link has invalid link element list.\n", ii);
                              error = true;
                            }
                        }
                      else // if (NULL != tempElement->next)
                        {
                          if (!(tempElement == tempElement->next->prev))
                            {
                              fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link has invalid link element list.\n", ii);
                              error = true;
                            }
                        }

                      tempElement = tempElement->next;

                      if (advanceLaggard)
                        {
                          laggardElement = laggardElement->next;
                        }

                      advanceLaggard = !advanceLaggard;

                      if (tempElement == laggardElement)
                        {
                          fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link has cycle in link element list.\n", ii);
                          error = true;
                        }
                    } // End while (!error && NULL != tempElement)
                } // End if (!(NULL != channels[ii].firstElement && NULL == channels[ii].firstElement->prev))
            } // End else if (STREAM == channels[ii].type || PRUNED_STREAM == channels[ii].type)
          else if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type)
            {
              if (!(0 < channels[ii].reachCode && NULL != channels[ii].shapes[0] && 0.0 <= channels[ii].length &&
                    0.0 == channels[ii].upstreamContributingArea && 0.0 == channels[ii].downstreamContributingArea && 0 == channels[ii].streamOrder &&
                    0 <= channels[ii].elementStart && (0 == channels[ii].numberOfElements || 1 == channels[ii].numberOfElements)))
                {
                  fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: waterbody link has invalid value(s).\n", ii);
                  error = true;
                }
              
              for (jj = 1; jj < SHAPES_SIZE; jj++)
                {
                  if (!(NULL != channels[ii].shapes[jj - 1] || NULL == channels[ii].shapes[jj]))
                    {
                      fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: waterbody link shapes must all be at the front of the array.\n", ii);
                      error = true;
                    }
                }
              
              for (jj = 0; jj < UPSTREAM_SIZE; jj++)
                {
                  if (0 < jj && !(NOFLOW != channels[ii].upstream[jj - 1] || NOFLOW == channels[ii].upstream[jj]))
                    {
                      fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: upstream links must be compacted to the front of the array.\n",
                              ii);
                      error = true;
                    }
                  
                  if (isBoundary(channels[ii].upstream[jj]))
                    {
                      if (!(OUTFLOW != channels[ii].upstream[jj]))
                        {
                          fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: waterbody link upstream must not be OUTFLOW.\n", ii);
                          error = true;
                        }
                    }
                  else
                    {
                      if (!(0 <= channels[ii].upstream[jj] && channels[ii].upstream[jj] < size &&
                            (STREAM == channels[channels[ii].upstream[jj]].type || WATERBODY == channels[channels[ii].upstream[jj]].type ||
                             ICEMASS == channels[channels[ii].upstream[jj]].type)))
                        {
                          fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: waterbody link upstream must be valid linkNo.\n", ii);
                          error = true;
                        }
                      else if (!allowNonReciprocalUpstreamDownstreamLinks)
                        {
                          foundCondition = false;

                          for (kk = 0; !foundCondition && kk < DOWNSTREAM_SIZE; kk++)
                            {
                              foundCondition = (ii == channels[channels[ii].upstream[jj]].downstream[kk]);
                            }
                          
                          if (!foundCondition)
                            {
                              fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: waterbody link upstream must have reciprocal downstream "
                                      "link.\n", ii);
                              error = true;
                            }
                        }
                    }
                }
              
              for (jj = 0; jj < DOWNSTREAM_SIZE; jj++)
                {
                  if (0 < jj && !(NOFLOW != channels[ii].downstream[jj - 1] || NOFLOW == channels[ii].downstream[jj]))
                    {
                      fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: downstream links must be compacted to the front of the array.\n",
                              ii);
                      error = true;
                    }
                  
                  if (isBoundary(channels[ii].downstream[jj]))
                    {
                      if (!(INFLOW != channels[ii].downstream[jj]))
                        {
                          fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: waterbody link downstream must not be INFLOW.\n", ii);
                          error = true;
                        }
                    }
                  else
                    {
                      if (!(0 <= channels[ii].downstream[jj] && channels[ii].downstream[jj] < size &&
                            (STREAM == channels[channels[ii].downstream[jj]].type || WATERBODY == channels[channels[ii].downstream[jj]].type ||
                             ICEMASS == channels[channels[ii].downstream[jj]].type)))
                        {
                          fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: waterbody link downstream must be valid linkNo.\n", ii);
                          error = true;
                        }
                      else if (!allowNonReciprocalUpstreamDownstreamLinks)
                        {
                          foundCondition = false;

                          for (kk = 0; !foundCondition && kk < UPSTREAM_SIZE; kk++)
                            {
                              foundCondition = (ii == channels[channels[ii].downstream[jj]].upstream[kk]);
                            }
                          
                          if (!foundCondition)
                            {
                              fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: waterbody link downstream must have reciprocal upstream "
                                      "link.\n", ii);
                              error = true;
                            }
                        }
                    }
                }
              
              if (NULL == channels[ii].firstElement)
                {
                  if (!(NULL == channels[ii].lastElement))
                    {
                      fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: waterbody link has invalid link element list.\n", ii);
                      error = true;
                    }
                }
              else
                {
                  tempElement    = channels[ii].firstElement; // For looping through the link element linked list.
                  laggardElement = channels[ii].firstElement; // Used to check for cycles.
                  advanceLaggard = false;                     // Toggled to advance laggardElement one for every two of tempElement.

                  while (!error && NULL != tempElement)
                    {
                      if (!(0 <= tempElement->edge && 0.0 <= tempElement->endLocation &&
                            (COINCIDENT == tempElement->movedTo || UPSTREAM == tempElement->movedTo || DOWNSTREAM == tempElement->movedTo ||
                             UNRELATED == tempElement->movedTo)))
                        {
                          fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: waterbody link element has invalid value(s).\n", ii);
                          error = true;
                        }

                      if (NULL == tempElement->next)
                        {
                          if (!(tempElement == channels[ii].lastElement))
                            {
                              fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link has invalid link element list.\n", ii);
                              error = true;
                            }
                        }
                      else // if (NULL != tempElement->next)
                        {
                          if (!(tempElement == tempElement->next->prev))
                            {
                              fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link has invalid link element list.\n", ii);
                              error = true;
                            }
                        }

                      tempElement = tempElement->next;

                      if (advanceLaggard)
                        {
                          laggardElement = laggardElement->next;
                        }

                      advanceLaggard = !advanceLaggard;

                      if (tempElement == laggardElement)
                        {
                          fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: stream link has cycle in link element list.\n", ii);
                          error = true;
                        }
                    } // End while (!error && NULL != tempElement)
                } // End if (NULL == channels[ii].firstElement)
            } // End else if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type).
          else
            {
              fprintf(stderr, "ERROR in channelNetworkCheckInvariant, linkNo %d: type must be a valid enum value.\n", ii);
              error = true;
            }
        } // End check all links.
    } // End if (!error).
  
  return error;
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)

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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
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
  
  // Assertions on inputs not done because they will be done immediately in called functions.
  
  error = addUpstreamConnection(channels, size, upstreamLinkNo, downstreamLinkNo);
  
  if (!error)
    {
      // When adding an upstream/downstream connection we have to add one first and then the invariant will fail on the arguments to the function to add the other.
      allowNonReciprocalUpstreamDownstreamLinks = true;
      
      error = addDownstreamConnection(channels, size, upstreamLinkNo, downstreamLinkNo);
      
      if (error)
        {
          // Must remove upstream connection to enforce guarantee that on error no connections are modified.
          removeUpstreamConnection(channels, size, upstreamLinkNo, downstreamLinkNo);
        }
      
      // When adding an upstream/downstream connection we have to add one first and then the invariant will fail on the arguments to the function to add the other.
      allowNonReciprocalUpstreamDownstreamLinks = false;
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
  // Assertions on inputs not done because they will be done immediately in called functions.
  
  removeUpstreamConnection(channels, size, upstreamLinkNo, downstreamLinkNo);
  
  // When adding an upstream/downstream connection we have to add one first and then the invariant will fail on the arguments to the function to add the other.
  allowNonReciprocalUpstreamDownstreamLinks = true;
  
  removeDownstreamConnection(channels, size, upstreamLinkNo, downstreamLinkNo);
  
  // When adding an upstream/downstream connection we have to add one first and then the invariant will fail on the arguments to the function to add the other.
  allowNonReciprocalUpstreamDownstreamLinks = false;
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= linkNo && linkNo < size);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
  // Prune streams if there are no upstream links and all elements are unassociated.
  if (STREAM == channels[linkNo].type && NOFLOW == channels[linkNo].upstream[0])
    {
      // Check if the channel is associated with a mesh edge. Only prune if unassociated.
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
          for (ii = 0; ii < DOWNSTREAM_SIZE && NOFLOW != channels[linkNo].downstream[ii]; ii++)
            {
              // We have to detach all downstream links before we can set the type to PRUNED_STREAM, but that will temporarily leave a non-pruned stream with
              // non-reciprocal links.  Set this repeatedly inside the loop because the called tryToPruneLink might set it to false.
              allowNonReciprocalUpstreamDownstreamLinks = true;
              
              removeUpstreamConnection(channels, size, linkNo, channels[linkNo].downstream[ii]);
              
              if (!isBoundary(channels[linkNo].downstream[ii]))
                {
                  tryToPruneLink(channels, size, channels[linkNo].downstream[ii]);
                }
            }
          
          // We have to detach all downstream links before we can set the type to PRUNED_STREAM, but that will temporarily leave a non-pruned stream with
          // non-reciprocal links.
          allowNonReciprocalUpstreamDownstreamLinks = false;
          channels[linkNo].type                     = PRUNED_STREAM;
        }
    }
}

// This function actually implements isCoincidentOrDownstream.  See that
// function's comment for details.  This one does not check the invariant.
// Checking that invariant inside a recursive depth first search that doesn't
// modify channels is too insane even for the most extreme debug level.
bool isCoincidentOrDownstreamRecurse(ChannelLinkStruct* channels, int size, int linkNo1, int linkNo2, int hopCount, int numberOfLinks,
                                    bool* coincidentOrDownstream)
{
  bool error = false; // Error flag.
  int  ii;            // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= linkNo1 && linkNo1 < size && 0 <= linkNo2 && linkNo2 < size && 0 <= hopCount &&
         0 <= numberOfLinks && numberOfLinks <= size && NULL != coincidentOrDownstream);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (hopCount > numberOfLinks)
    {
      fprintf(stderr, "ERROR in isCoincidenOrDownstreamRecurse: Detected cycle in channel network in link %d.\n", linkNo1);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

  *coincidentOrDownstream = (linkNo1 == linkNo2); // Initialize coincidentOrDownstream to a value even if there is an error.
  
  // Search all downstream links of LinkNo1.
  if (!error)
    {
      for (ii = 0; !*coincidentOrDownstream && ii < DOWNSTREAM_SIZE && NOFLOW != channels[linkNo1].downstream[ii]; ii++)
        {
          if (!isBoundary(channels[linkNo1].downstream[ii]))
            {
              isCoincidentOrDownstreamRecurse(channels, size, channels[linkNo1].downstream[ii], linkNo2, hopCount + 1, numberOfLinks, coincidentOrDownstream);
            }
        }
    }
  
  return error;
}

// Calculate whether linkNo2 is the same link as or downstream of linkNo1.
// This function does a depth first search of the downstream connections of
// linkNo1 to find linkNo2.  It performs brute force cycle detection by keeping
// a hop count and if that exceeds the total number of used links a cycle is
// detected.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels               - The channel network as a 1D array of
//                          ChannelLinkStruct.
// size                   - The number of elements in channels.
// linkNo1                - The link to check if it is the same link as or
//                          upstream of linkNo2.
// linkNo2                - The link to check if it is the same link as or
//                          downstream of linkNo1.
// hopCount               - The number of links traversed so far.
// numberOfLinks          - The total number of used links.  If hopCount
//                          exceeds this there must be a cycle.
// coincidentOrDownstream - Scalar passed by reference will be filled in with
//                          true if linkNo2 is the same link as or downstream
//                          of linkNo1, false otherwise.
bool isCoincidentOrDownstream(ChannelLinkStruct* channels, int size, int linkNo1, int linkNo2, int hopCount, int numberOfLinks, bool* coincidentOrDownstream)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
  // This function is actually implemented in isCoincidenOrDownstreamRecurse, but that function does not check the invariant.  This way the invariant gets
  // checked once for the entire search, not once per step of the search.
  return isCoincidentOrDownstreamRecurse(channels, size, linkNo1, linkNo2, hopCount, numberOfLinks, coincidentOrDownstream);
}

// Determine the upstream/downstream relationship between two intersections.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels           - The channel network as a 1D array of ChannelLinkStruct.
// size               - The number of elements in channels.
// intersection1      - The first intersection.
// intersection2      - The second intersection.
// upstreamDownstream - Scalar passed by reference will be filled in with the
//                      upstream/downstream relationship between the two
//                      intersections.
bool getUpstreamDownstream(ChannelLinkStruct* channels, int size, LinkElementStruct* intersection1, LinkElementStruct* intersection2,
                           UpstreamDownstreamEnum* upstreamDownstream)
{
  bool error = false;          // Error flag.
  bool coincidentOrDownstream; // Whether one link is downstream of another.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && NULL != intersection1 && NULL != intersection2 && NULL != upstreamDownstream);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size) &&
         0 <= intersection1->edge && intersection1->edge < size && STREAM == channels[intersection1->edge].type &&
         0.0 <= intersection1->endLocation && intersection1->endLocation <= channels[intersection1->edge].lastElement->endLocation &&
         (COINCIDENT == intersection1->movedTo || UPSTREAM == intersection1->movedTo || DOWNSTREAM == intersection1->movedTo ||
          UNRELATED == intersection1->movedTo) &&
         0 <= intersection2->edge && intersection2->edge < size && STREAM == channels[intersection2->edge].type &&
         0.0 <= intersection2->endLocation && intersection2->endLocation <= channels[intersection2->edge].lastElement->endLocation &&
         (COINCIDENT == intersection2->movedTo || UPSTREAM == intersection2->movedTo || DOWNSTREAM == intersection2->movedTo ||
          UNRELATED == intersection2->movedTo));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
  // If they are on the same link the 1D location determines the relationship.
  if (intersection1->edge == intersection2->edge)
    {
      if (intersection1->endLocation == intersection2->endLocation)
        {
          *upstreamDownstream = COINCIDENT;
        }
      else if (intersection1->endLocation > intersection2->endLocation)
        {
          *upstreamDownstream = UPSTREAM;
        }
      else // if (intersection1->endLocation < intersection2->endLocation)
        {
          *upstreamDownstream = DOWNSTREAM;
        }
    }
  else
    {
      // Search downstream of intersection2 to find intersection1.
      error = isCoincidentOrDownstream(channels, size, intersection2->edge, intersection1->edge, 0, size, &coincidentOrDownstream);
      
      if (!error)
        {
          if (coincidentOrDownstream)
            {
              *upstreamDownstream = UPSTREAM;
            }
          else
            {
              // Search downstream of intersection1 to find intersection2.
              error = isCoincidentOrDownstream(channels, size, intersection1->edge, intersection2->edge, 0, size, &coincidentOrDownstream);

              if (!error && coincidentOrDownstream)
                {
                  *upstreamDownstream = DOWNSTREAM;
                }
              else
                {
                  *upstreamDownstream = UNRELATED;
                }
            }
        }
      else
        {
          *upstreamDownstream = UNRELATED;
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
      (*channels)[ii].streamOrder                = 0;
      
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
      if (!error)
        {
          (*channels)[linkNo].reachCode = reachCode;
        }
    }
  
  // Deallocate channels and set size to zero on error.
  if (error)
    {
      delete[] *channels;
      
      *channels = NULL;
      *size     = 0;
    }

  // Close the file.
  if (NULL != linkFile)
    {
      fclose(linkFile);
    }

  return error;
}

// Read the values of the attributes reachCode and permanent.
// There are some manipulations that must be done, and this code is used in multiple places so it is pulled into a function to eliminate duplicate code.
//
// Parameters:
//
// dbfFile - Attribute part of the shapefile.
// shapeNumber - Index of shape in dbfFile.
// reachCodeIndex - Index of metadata field.
// permanentIndex - Index of metadata field.
// reachCode      - Will be filled in with the reach code of the waterbody.
// permanent      - Will be filled in with the permanent code of the waterbody.
void readReachCodeAndPermanent(DBFHandle dbfFile, int shapeNumber, int reachCodeIndex, int permanentIndex, long long& reachCode, long long& permanent)
{
  int         numScanned; // Used to check that fscanf scanned all of the requested values.
  const char* tempString; // String representation of reachCode or permanent.
  
  tempString = DBFReadStringAttribute(dbfFile, shapeNumber, reachCodeIndex);
  numScanned = sscanf(tempString, "%lld", &reachCode);

  // Some shapes don't have a reach code, in which case we use the permanent code, so it is not an error if you can't scan a reach code.
  if (1 != numScanned)
    {
      reachCode = -1;
    }

  tempString = DBFReadStringAttribute(dbfFile, shapeNumber, permanentIndex);

  // Some permanent codes are formatted like GUIDs with hexadecimal numbers separated by '-'.  Read those as hexadecimal.
  if (NULL == strchr(tempString, '-'))
    {
      // There are no '-' separators.  Read as decimal.
      numScanned = sscanf(tempString, "%lld", &permanent);
    }
  else
    {
      // Some GUID permanent codes are enclosed with curly braces so skip over an opening curly brace.
      if ('{' == tempString[0])
        {
          tempString++;
        }

      // There are '-' separators.  Read as hexadecimal.
      numScanned = sscanf(tempString, "%llx", &permanent);
    }

  // Some shapes don't have a permanent code, in which case we use the reach code, so it is not an error if you can't scan a permanent code.
  if (1 != numScanned)
    {
      permanent = -1;
    }
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
  int             reachCodeIndex; // Index of metadata field.
  int             permanentIndex; // Index of metadata field.
  int             ftypeIndex;     // Index of metadata field.
  long long       reachCode;      // The reach code of the waterbody.
  long long       permanent;      // The permanent code of the waterbody.
  const char*     ftype;          // The type of the waterbody as a string.
  int             ftypeInt;       // The type of the waterbody as an int.
  int             linkNo;         // The link number of the waterbody.
  ChannelTypeEnum linkType;       // The type of the waterbody as an enum.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && NULL != fileBasename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
  // Open the geometry and attribute files.
  shpFile = SHPOpen(fileBasename, "rb");
  dbfFile = DBFOpen(fileBasename, "rb");
  
  // It's okay for the shapefile not to exist at all.  If that is the case treat it as if there are zero shapes in the file.
  if (NULL == shpFile && NULL == dbfFile)
    {
      numberOfShapes = 0;
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      fprintf(stderr, "WARNING in readWaterbodies: Waterbody shapefile %s does not exist.  Treating it as zero waterbodies.\n", fileBasename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  else
    {
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

      // Get the number of shapes.
      if (!error)
        {
          numberOfShapes = DBFGetRecordCount(dbfFile);

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(0 < numberOfShapes))
            {
              // Only warn because you could have zero waterbodies.
              fprintf(stderr, "WARNING in readWaterbodies: Zero waterbodies in dbf file %s.\n", fileBasename);
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
    }
  
  if (!error && 0 < numberOfShapes)
    {
      // Get the attribute indices.
      reachCodeIndex = DBFGetFieldIndex(dbfFile, "ReachCode");
      permanentIndex = DBFGetFieldIndex(dbfFile, "Permanent_");
      ftypeIndex     = DBFGetFieldIndex(dbfFile, "FTYPE");
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
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
      readReachCodeAndPermanent(dbfFile, ii, reachCodeIndex, permanentIndex, reachCode, permanent);

      ftype    = DBFReadStringAttribute( dbfFile, ii, ftypeIndex);
      ftypeInt = DBFReadIntegerAttribute(dbfFile, ii, ftypeIndex);
      linkNo   = 0;

      // Get link number.
      while(linkNo < size && (-1 == channels[linkNo].reachCode || (reachCode != channels[linkNo].reachCode && permanent != channels[linkNo].reachCode)))
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

      // Get link type.  We found that the values in the FTYPE field were not consistent across data sources.  In the files originally downloaded by Bob the
      // values were words: "Ice Mass", "LakePond", "SwampMarsh", and "Reservoir".  In the files later downloaded by Leticia the values were numbers:
      // "378" for Ice Mass, "390" for LakePond, "466" for SwampMarsh, and "436" for Reservoir.  It may be that Bob downloaded the NHDPlus dataset while
      // Leticia downloaded the NHD dataset, but we don't know for sure if that's what happened or if that's the reason for the difference.  You may need to
      // modify this code to account for different values in your data source.
      if (!error)
        {
          if (0 == strcmp("Ice Mass", ftype) || 378 == ftypeInt)
            {
              linkType = ICEMASS;
            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          else if (!(0 == strcmp("LakePond",  ftype) || 390 == ftypeInt || 0 == strcmp("SwampMarsh", ftype) || 466 == ftypeInt ||
                     0 == strcmp("Reservoir", ftype) || 436 == ftypeInt || 0 == strcmp("Playa",      ftype) || 361 == ftypeInt))
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
  int       orderIndex;      // Index of metadata field.
  int       linkNo;          // The link number of the stream.
  int       linkNo2;         // For storing a link that must be connected to.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && NULL != fileBasename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
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
      us_cont_arIndex = DBFGetFieldIndex(dbfFile, "USContArea"); //With version 5.3.8 of taudem, the field US_Cont_Ar doesn't exist, instead it is USContArea; NJF
      ds_cont_arIndex = DBFGetFieldIndex(dbfFile, "DSContArea"); //See above ^^
      orderIndex      = DBFGetFieldIndex(dbfFile, "strmOrder"); //Also in 5.3.8, Order has been changed to strmOrder
      
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
      
      if (!(-1 != orderIndex))
        {
          fprintf(stderr, "ERROR in readTaudemStreamnet: Could not find field Order in dbf file %s.\n", fileBasename);
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
          channels[linkNo].type                       = STREAM;
          channels[linkNo].shapes[0]                  = SHPReadObject(shpFile, ii);
          channels[linkNo].upstreamContributingArea   = DBFReadDoubleAttribute(dbfFile, ii, us_cont_arIndex);
          channels[linkNo].downstreamContributingArea = DBFReadDoubleAttribute(dbfFile, ii, ds_cont_arIndex);
          channels[linkNo].streamOrder                = DBFReadIntegerAttribute(dbfFile, ii, orderIndex);
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
          
          // Create an initial LinkElementStruct.  Do not use createLinkElementAfter because we don't need the full generality of that function and at this
          // moment the channel network might not pass the invariant, which might be checked in that function.
          channels[linkNo].firstElement              = new LinkElementStruct;
          channels[linkNo].lastElement               = channels[linkNo].firstElement;
          channels[linkNo].firstElement->prev        = NULL;
          channels[linkNo].firstElement->next        = NULL;
          channels[linkNo].firstElement->endLocation = channels[linkNo].length;
          channels[linkNo].firstElement->edge        = -1;
          channels[linkNo].firstElement->movedTo     = -1;
        }
    } // End fill in the links.
  
  // Use DEBUG_LEVEL_USER_INPUT_SIMPLE because here we are using the invariant to check simple conditions like link numbers within range.
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!error)
    {
      error = channelNetworkCheckInvariant(channels, size);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  // Eliminate links of zero length.
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)

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
              assert(element2BeginLocation <= newLocation && newLocation <= location2);
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
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
      
      // A boundary code of two or more indicates a channel edge.
      if (!error && 2 <= boundary)
        {
          // Convert to link number.
          boundary -= 2;
          
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
  
  // Close the files.
  if (NULL != nodeFile)
    {
      fclose(nodeFile);
    }

  if (NULL != edgeFile)
    {
      fclose(edgeFile);
    }

  // Deallocate node arrays.
  delete[] nodesX;
  delete[] nodesY;

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
  int                    numberOfShapes;     // Number of shapes in the shapefile.
  int                    linknoIndex;        // Index of metadata field.
  int                    reachCodeIndex;     // Index of metadata field.
  int                    permanentIndex;     // Index of metadata field.
  int                    streamLinkNo;       // The link number of the intersecting stream.
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)

  // Open the geometry and attribute files.
  shpFile = SHPOpen(fileBasename, "rb");
  dbfFile = DBFOpen(fileBasename, "rb");

  // It's okay for the shapefile not to exist at all.  If that is the case treat it as if there are zero shapes in the file.
  if (NULL == shpFile && NULL == dbfFile)
    {
      numberOfShapes = 0;

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      fprintf(stderr, "WARNING in readWaterbodyStreamIntersections: Intersection shapefile %s does not exist.  Treating it as zero intersections.\n", fileBasename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  else
    {
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

      // Get the number of shapes.
      if (!error)
        {
          numberOfShapes = DBFGetRecordCount(dbfFile);

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(0 < numberOfShapes))
            {
              // Only warn because you could have zero intersections.
              fprintf(stderr, "WARNING in readWaterbodyStreamIntersections: Zero intersections in dbf file %s.\n", fileBasename);
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
    }

  // Get the attribute indices.
  if (!error && 0 < numberOfShapes)
    {
      linknoIndex    = DBFGetFieldIndex(dbfFile, "LINKNO");
      reachCodeIndex = DBFGetFieldIndex(dbfFile, "ReachCode");
      permanentIndex = DBFGetFieldIndex(dbfFile, "Permanent_");
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
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
      readReachCodeAndPermanent(dbfFile, ii, reachCodeIndex, permanentIndex, reachCode, permanent);
      
      waterbodyLinkNo = 0;
      shape           = NULL;
      location        = 0.0;
      
      while(waterbodyLinkNo < size && (-1 == channels[waterbodyLinkNo].reachCode ||
                                       (WATERBODY != channels[waterbodyLinkNo].type && ICEMASS != channels[waterbodyLinkNo].type) ||
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
          else if (!((1 == shape->nSHPType || 8 == shape->nSHPType || 11 == shape->nSHPType || 18 == shape->nSHPType || 21 == shape->nSHPType ||
                      28 == shape->nSHPType) && 0 == shape->nParts && 1 == shape->nVertices))
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
  for (ii = 0; !error && ii < size; ii++)
    {
      if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type)
        {
          intersection1 = channels[ii].firstElement;
          
          while (!error && NULL != intersection1)
            {
              hasUpstream   = false;                     // Whether intersection1 has another intersection upstream of it.
              hasDownstream = false;                     // Whether intersection1 has another intersection downstream of it.
              intersection2 = channels[ii].firstElement; // For looping over all intersections to compare with intersection1.
              
              // For each intersection1, loop over all of the intersection2s.  If intersection1 will be ignored (hasUpstream && hasDownstream) then break out
              // of the loop.
              while (!error && NULL != intersection2 && !(hasUpstream && hasDownstream))
                {
                  // Do not compare an intersection to itself.
                  if (intersection1 != intersection2)
                    {
                      // Get the upstream/downstream relationship between intersection1 and intersection2.
                      error = getUpstreamDownstream(channels, size, intersection1, intersection2, &upstreamDownstream);

                      if (!error)
                        {
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
                    }

                  intersection2 = intersection2->next;
                }
              
              if (!error)
                {
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
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
  assert(NULL != channels && 0 <= linkNo && linkNo < size && STREAM == channels[linkNo].type && NULL != element && beginLocation(element) <= location &&
         location <= element->endLocation);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
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
      
      // When we split a link we update the end locations of link elements before moving them to the new link so they will temporarily fail the invariant.
      allowLastLinkElementLengthNotEqualToLinkLength         = true;
      allowLinkElementEndLocationsNotMonotonicallyIncreasing = true;

      // Create the movedTo element after endElement.
      createLinkElementAfter(channels, size, linkNo, endElement, channels[linkNo].length, -1, newLinkNo);
      
      // When we split a link we update the end locations of link elements before moving them to the new link so they will temporarily fail the invariant.
      allowLastLinkElementLengthNotEqualToLinkLength         = false;
      allowLinkElementEndLocationsNotMonotonicallyIncreasing = false;
  
      // Update struct values.
      channels[newLinkNo].type                       = STREAM;
      channels[newLinkNo].reachCode                  = channels[linkNo].reachCode;
      channels[newLinkNo].upstreamContributingArea   = contributingAreaAtLocation(channels[linkNo].upstreamContributingArea, channels[linkNo].downstreamContributingArea, location / channels[linkNo].length);
      channels[newLinkNo].downstreamContributingArea = channels[linkNo].downstreamContributingArea;
      channels[linkNo].downstreamContributingArea    = channels[newLinkNo].upstreamContributingArea;
      channels[newLinkNo].length                     = channels[linkNo].length - location;
      channels[linkNo].length                        = location;
      
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
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
  assert(0 <= streamLinkNo && streamLinkNo < size && STREAM == channels[streamLinkNo].type && 0.0 <= location && epsilonLessOrEqual(location, channels[streamLinkNo].length));
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
          // If location is epsilon equal to element->endLocation snap it to exactly equal.
          if (epsilonEqual(location, element->endLocation))
            {
              location = element->endLocation;
            }
          
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
  // Resolve all of the intersections.
  for (ii = 0; !error && ii < size; ii++)
    {
      if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type)
        {
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

// Returns: True if linkNo1 and LinkNo2 are already directly connected.  False
// otherwise.
//
// Parameters:
//
// channels - The channel network as a 1D array of ChannelLinkStruct.
// size     - The number of elements in channels.
// linkNo1  - The first link to check for connection.
// linkNo2  - The second link to check for connection.
bool alreadyConnected(ChannelLinkStruct* channels, int size, int linkNo1, int linkNo2)
{
  int  ii;                // Loop counter.
  bool connected = false; // return value.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= linkNo1 && linkNo1 < size && 0 <= linkNo2 && linkNo2 < size);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
  for (ii = 0; !connected && ii < UPSTREAM_SIZE && NOFLOW != channels[linkNo1].upstream[ii]; ii++)
    {
      connected = (linkNo2 == channels[linkNo1].upstream[ii]);
    }
  
  for (ii = 0; !connected && ii < DOWNSTREAM_SIZE && NOFLOW != channels[linkNo1].downstream[ii]; ii++)
    {
      connected = (linkNo2 == channels[linkNo1].downstream[ii]);
    }
  
  return connected;
}

// Read all of the intersections between waterbodies and other waterbodies and
// make those links.
// 
// This function opens the waterbodies waterbodies intersections shapefile and
// for each intersection links the two waterbodies together.  No links are
// broken.  If the waterbodies are already linked the intersection is ignored.
// If one waterbody is already transitively upstream of the other, but not
// directly linked it is linked upstream.  Otherwise, the upstream/downstream
// relationship is arbitrary.
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
  bool        error         = false;  // Error flag.
  int         ii;                     // Loop counter.
  int         numberOfLinks = 0;      // Total number of links in the channel network for detecting cycles.
  SHPHandle   shpFile;                // Geometry  part of the shapefile.
  DBFHandle   dbfFile;                // Attribute part of the shapefile.
  int         numberOfShapes;         // Number of shapes in the shapefile.
  int         reachCode1Index;        // Index of metadata field.
  int         permanent1Index;        // Index of metadata field.
  int         reachCode2Index;        // Index of metadata field.
  int         permanent2Index;        // Index of metadata field.
  long long   reachCode1;             // The reach code of the first intersecting waterbody.
  long long   permanent1;             // The permanent code of the first intersecting waterbody.
  long long   reachCode2;             // The reach code of the second intersecting waterbody.
  long long   permanent2;             // The permanent code of the second intersecting waterbody.
  int         linkNo1;                // The linkno of the first intersecting waterbody.
  int         linkNo2;                // The linkno of the second intersecting waterbody.
  bool        coincidentOrDownstream; // Whether linkNo2 is the same link as or downstream of linkNo1.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size && NULL != fileBasename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
  // We are using a brute force method to detect cycles in the channel network.  As we traverse downstream we keep a hop count and if the hop count exceeds
  // the total number of links we know we have visited at least one link more than once so there is a cycle.  Here we find the total number of links.
  for (ii = 0; ii < size; ii++)
    {
      if (NOT_USED != channels[ii].type)
        {
          numberOfLinks++;
        }
    }
  
  // Open the geometry and attribute files.
  shpFile = SHPOpen(fileBasename, "rb");
  dbfFile = DBFOpen(fileBasename, "rb");

  // It's okay for the shapefile not to exist at all.  If that is the case treat it as if there are zero shapes in the file.
  if (NULL == shpFile && NULL == dbfFile)
    {
      numberOfShapes = 0;

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      fprintf(stderr, "WARNING in readAndLinkWaterbodyWaterbodyIntersections: Intersection shapefile %s does not exist.  Treating it as zero intersections.\n", fileBasename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  else
    {
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

      // Get the number of shapes.
      if (!error)
        {
          numberOfShapes = DBFGetRecordCount(dbfFile);

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(0 < numberOfShapes))
            {
              // Only warn because you could have zero intersections.
              fprintf(stderr, "WARNING in readAndLinkWaterbodyWaterbodyIntersections: Zero intersections in dbf file %s.\n", fileBasename);
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
    }

  // Get the attribute indices.
  if (!error && 0 < numberOfShapes)
    {
      reachCode1Index = DBFGetFieldIndex(dbfFile, "ReachCode_");
      permanent1Index = DBFGetFieldIndex(dbfFile, "Permanent1");
      reachCode2Index = DBFGetFieldIndex(dbfFile, "ReachCode");
      permanent2Index = DBFGetFieldIndex(dbfFile, "Permanent_");
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(-1 != reachCode1Index))
        {
          fprintf(stderr, "ERROR in readAndLinkWaterbodyWaterbodyIntersections: Could not find field ReachCode_ in dbf file %s.\n", fileBasename);
          error = true;
        }
      
      if (!(-1 != permanent1Index))
        {
          fprintf(stderr, "ERROR in readAndLinkWaterbodyWaterbodyIntersections: Could not find field Permanent1 in dbf file %s.\n", fileBasename);
          error = true;
        }
      
      if (!(-1 != reachCode2Index))
        {
          fprintf(stderr, "ERROR in readAndLinkWaterbodyWaterbodyIntersections: Could not find field ReachCode in dbf file %s.\n", fileBasename);
          error = true;
        }
      
      if (!(-1 != permanent2Index))
        {
          fprintf(stderr, "ERROR in readAndLinkWaterbodyWaterbodyIntersections: Could not find field Permanent_ in dbf file %s.\n", fileBasename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Read and link the intersections.
  for (ii = 0; !error && ii < numberOfShapes; ii++)
    {
      readReachCodeAndPermanent(dbfFile, ii, reachCode1Index, permanent1Index, reachCode1, permanent1);
      readReachCodeAndPermanent(dbfFile, ii, reachCode2Index, permanent2Index, reachCode2, permanent2);
      
      linkNo1 = 0;
      
      while(linkNo1 < size && (-1 == channels[linkNo1].reachCode ||
                               (WATERBODY != channels[linkNo1].type && ICEMASS != channels[linkNo1].type) ||
                               (reachCode1 != channels[linkNo1].reachCode && permanent1 != channels[linkNo1].reachCode)))
        {
          linkNo1++;
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 <= linkNo1 && linkNo1 < size && (WATERBODY == channels[linkNo1].type || ICEMASS == channels[linkNo1].type)))
        {
          fprintf(stderr, "ERROR in readAndLinkWaterbodyWaterbodyIntersections: Could not find waterbody reach code %lld or permanent code %lld in channel "
                  "network.\n", reachCode1, permanent1);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      if (!error)
        {
          linkNo2 = 0;

          while(linkNo2 < size && (-1 == channels[linkNo2].reachCode ||
                                   (WATERBODY != channels[linkNo2].type && ICEMASS != channels[linkNo2].type) ||
                                   (reachCode2 != channels[linkNo2].reachCode && permanent2 != channels[linkNo2].reachCode)))
            {
              linkNo2++;
            }

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(0 <= linkNo2 && linkNo2 < size && (WATERBODY == channels[linkNo2].type || ICEMASS == channels[linkNo2].type)))
            {
              fprintf(stderr, "ERROR in readAndLinkWaterbodyWaterbodyIntersections: Could not find waterbody reach code %lld or permanent code %lld in "
                      "channel network.\n", reachCode2, permanent2);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
      
      if (!error && linkNo1 != linkNo2 && !alreadyConnected(channels, size, linkNo1, linkNo2))
        {
          error = isCoincidentOrDownstream(channels, size, linkNo1, linkNo2, 0, numberOfLinks, &coincidentOrDownstream);
          
          if (!error)
            {
              if (coincidentOrDownstream)
                {
                  error = addUpstreamDownstreamConnection(channels, size, linkNo1, linkNo2);
                }
              else
                {
                  error = addUpstreamDownstreamConnection(channels, size, linkNo2, linkNo1);
                }
            }
        }
    } // End read and link the intersections.

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
// The number of elements will be the link length divided by the desired
// element length rounded either up or down.  Try to get the actual element
// length as close as possible to the desired element length.  For a closeness
// metric we use the ratio of the longer of the desired or actual length to the
// shorter of the two.
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
// linkLength    - The actual length of the link in meters.
// desiredLength - The desired length of each element in meters.
int numberOfLinkElements(double linkLength, double desiredLength)
{
  double numLinkElementsFraction = linkLength / desiredLength;
  int    numLinkElements         = (int)numLinkElementsFraction;
  
  numLinkElementsFraction -= numLinkElements;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(0.0 < linkLength && 0.0 < desiredLength);
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

// Returns: The distance in meters along its original link of the beginning of
// a possibly moved link.  Will return zero if the link is unmoved.
//
// Parameters:
//
// channels       - The channel network as a 1D array of ChannelLinkStruct.
// size           - The number of elements in channels.
// linkNoToSearch - The link to search in for linkNoToFind.
// linkNoToFind   - The possibly moved link to find the location of in
//                  linkNoToSearch.
double locationOfMovedLink(ChannelLinkStruct* channels, int size, int linkNoToSearch, int linkNoToFind)
{
  double             location = NAN;
  LinkElementStruct* tempElement;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 <= linkNoToSearch && linkNoToSearch < size && 0 <= linkNoToFind && linkNoToFind < size);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
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

// Create a new node.
//
// Parameters:
//
// x             - The X coordinate of the node to create.
// y             - The Y coordinate of the node to create.
// nodesSize     - Scalar passed by reference containing the size of nodesX and
//                 nodesY arrays.  Will be updated if the arrays are realloced.
// numberOfNodes - Scalar passed by reference containing the number of channel
//                 nodes actually existing in nodesX and nodesY arrays.  Will
//                 be updated to the new number of nodes.
// nodesX        - 1D array passed by reference containing the X coordinates of
//                 the nodes.  Will be updated if the arrays are realloced.
// nodesY        - 1D array passed by reference containing the Y coordinates of
//                 the nodes.  Will be updated if the arrays are realloced.
// nodeNumber    - Scalar passed by reference will be filled in with the node
//                 number of the new node.
void createNode(double x, double y, int* nodesSize, int* numberOfNodes, double** nodesX, double** nodesY, int* nodeNumber)
{
  int     ii;            // Loop counter.
  int     newNodesSize;  // For reallocing.
  double* newNodesX;     // For reallocing.
  double* newNodesY;     // For reallocing.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != nodesSize && 0 < *nodesSize && NULL != numberOfNodes && 0 <= *numberOfNodes && *numberOfNodes <= *nodesSize && NULL != nodesX &&
         NULL != *nodesX && NULL != nodesY && NULL != *nodesY && NULL != nodeNumber);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // If the nodes arrays are full realloc.
  if (*numberOfNodes == *nodesSize)
    {
      newNodesSize = *nodesSize * 2;
      newNodesX    = new double[newNodesSize];
      newNodesY    = new double[newNodesSize];
      
      for (ii = 0; ii < *numberOfNodes; ii++)
        {
          newNodesX[ii] = (*nodesX)[ii];
          newNodesY[ii] = (*nodesY)[ii];
        }
      
      delete[] *nodesX;
      delete[] *nodesY;
      
      *nodesSize = newNodesSize;
      *nodesX    = newNodesX;
      *nodesY    = newNodesY;
    }

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  assert(*numberOfNodes < *nodesSize);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  (*nodesX)[*numberOfNodes] = x;
  (*nodesY)[*numberOfNodes] = y;
  *nodeNumber               = (*numberOfNodes)++;
}

#define NUMBER_OF_STREAM_ORDERS (10) // The number of stream orders to store data for.

double streamOrderElementLength[NUMBER_OF_STREAM_ORDERS + 1] = {NAN, 100.0, 140.0, 200.0, 280.0, 400.0, 550.0, 800.0, 1000.0, 1500.0, 2000.0};
                                     // Desired length in meters of channel elements for each stream order.  E.g. channel elements for first order streams
                                     // should be 100.0 meters long, second order should be 140.0, etc.

// If a stream is split into more than one channel element then the smallest
// that the actual channel element length can be is ~70% of the desired channel
// element length.  This occurs when the stream is just barely big enough to be
// split into two channel elements.  However, if an entire link is shorter than
// this it results in a single channel element that is too short.  Short
// channel elements can be problematic for running a simulation.  This function
// tries to fix links that are too short.
//
// The length of the short link is added on to the lengths of its upstream
// neighbors.  Then those neighbors are disconnected from the short link and
// connected upstream of all links downstream of the short link.  Then the
// short link is pruned.
//
// This function does not fix all links.  Any link that is not fixed here gets
// its length arbitrarily increased in writeChannelNetwork to 70% of the
// desired channel element length.
//
// This function only fixes links whose length is less than 50% of the desired
// channel element length.  It's better for a link that is almost long enough
// to be lengthened than pruned.
//
// This function does not fix links that are connected to mesh edges.
//
// This function does not fix waterbodies because their length is not
// calculated yet, and they are most likely connected to mesh edges.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels - The channel network as a 1D array of ChannelLinkStruct.
// size     - The number of elements in channels.
bool fixShortLinks(ChannelLinkStruct* channels, int size)
{
  bool               error = false;  // Error flag.
  int                ii, jj;         // Loop counters.
  int                streamOrder;    // Stream order of a stream.
  bool               prune;          // Whether to prune.
  LinkElementStruct* element;        // For checking if link elements are unassociated.
  int                upstreamLinkNo; // The upstream connection of a channel link.

  for (ii = 0; !error && ii < size; ii++)
    {
      if (STREAM == channels[ii].type)
        {
          // Get stream order from the original link number in case this link was moved.
          streamOrder = channels[channels[ii].reachCode].streamOrder;

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (1 > streamOrder)
            {
              fprintf(stderr, "WARNING in fixShortLinks: stream order less than one in link %d.  Using one for stream order.\n", ii);

              streamOrder = 1;
            }
          else if (NUMBER_OF_STREAM_ORDERS < streamOrder)
            {
              fprintf(stderr, "WARNING in fixShortLinks: stream order greater than %d in link %d.  Using %d for stream order.\n",
                      NUMBER_OF_STREAM_ORDERS, ii, NUMBER_OF_STREAM_ORDERS);

              streamOrder = NUMBER_OF_STREAM_ORDERS;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

          if (channels[ii].length  < 0.5 * streamOrderElementLength[streamOrder])
            {
              // Check if the channel is associated with a mesh edge. Only prune if unassociated.
              prune   = true;
              element = channels[ii].firstElement;

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
                  fprintf(stderr, "WARNING in fixShortLinks: pruning short link %d, stream order %d, length %lf.\n", ii, streamOrder, channels[ii].length);
                  
                  // For all upstream connections, add the length of link ii to the upstream connection, remove the connection with link ii, and create a
                  // connection with all of the links downstream of link ii.
                  while (!error && NOFLOW != channels[ii].upstream[0])
                    {
                      // When removing the upstream downstream connection, the connection at location 1 is moved to location 0.  So on every loop the next
                      // connection to be removed is at location 0. The loop stops when location 0 has the value NOFLOW.
                      upstreamLinkNo = channels[ii].upstream[0];

                      // Add the length of link ii to the upstream connection.
                      if(!isBoundary(upstreamLinkNo))
                        {
                          channels[upstreamLinkNo].length += channels[ii].length;
                          
                          // If the upstream connection is a stream update link element end locations.
                          if (STREAM == channels[upstreamLinkNo].type)
                            {
                              element = channels[upstreamLinkNo].lastElement;

                              // All moved link elements get pushed out by the added length.
                              while (-1 != element->movedTo)
                                {
                                  element->endLocation += channels[ii].length;
                                  element               = element->prev;
                                }

                              // The last unmoved link element has the length added to it.
                              element->endLocation += channels[ii].length;
                            }
                        }

                      removeUpstreamDownstreamConnection(channels, size, upstreamLinkNo, ii);

                      // Loop over all downstream connections
                      for (jj = 0; !error && DOWNSTREAM_SIZE > jj && NOFLOW != channels[ii].downstream[jj]; jj++)
                        {
                          error = addUpstreamDownstreamConnection(channels, size, upstreamLinkNo, channels[ii].downstream[jj]);
                        }
                    }

                  tryToPruneLink(channels, size, ii);
                }
            } // End if (channels[ii].length  < 0.7 * streamOrderElementLength[streamOrder]).
        } // End if (STREAM == channels[ii].type).
    } // End for (ii = 0; !error && ii < size; ii++).

  return error;
}

typedef int IntArrayMMN[3];                                 // Fixed size array of ints. Size is mesh mesh neighbors.
typedef int IntArrayMEE[2];                                 // Fixed size array of ints. Size is mesh edge elements.
typedef int IntArrayCV[ChannelElement_channelVerticesSize]; // Fixed size array of ints. Size is channel vertices.

// Used for the types of salient points.  Salient points are used when doing a parallel walk along channel elements and shape vertices.  A salient point is the
// next point on a channel element that we need to output whether or not it lies on a shape vertex.
typedef enum
{
  LINK_BEGIN,     // The salient point is the beginning of the link.
  ELEMENT_END,    // The salient point is the end of an element and the beginning of the next element in the same link.
  LINK_END,       // The salient point is the end of the link.
} SalientPointEnum;

// Write out the channel network output files.
//
// The format of the .chan.node file is the same as a triangle .node file.
//
// The format of the .chan.ele file is:
//
// First line: <# of elements>
// Then for each element:
// <element #> <element type> <reach code> <length> <top width> <bank full depth> <# of vertices> <# of channel neighbors> <# of mesh neighbors> <vertex> <vertex> ... <channel neighbor> <whether downstream> <channel neighbor> <whether downstream> ... <mesh neighbor> <mesh neighbor edge length in meters> <mesh neighbor> <mesh neighbor edge length in meters> ...
//
// The format of the .chan.prune file is:
//
// For each pruned stream:
// <reach code of pruned stream> <reach code of unpruned link that pruned stream flows downstream into>
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// channels               - The channel network as a 1D array of
//                          ChannelLinkStruct.
// size                   - The number of elements in channels.
// meshNodeFilename       - The name of the .node file to read.
// meshElementFilename    - The name of the .ele file to read.
// meshEdgeFilename       - The name of the .edge file to read.
// channelNodeFilename    - The name of the .chan.node file to write.
// channelElementFilename - The name of the .chan.ele file to write.
// channelPruneFilename   - The name of the .chan.prune file to write.
bool writeChannelNetwork(ChannelLinkStruct* channels, int size, const char* meshNodeFilename, const char* meshElementFilename, const char* meshEdgeFilename,
                         const char* channelNodeFilename, const char* channelElementFilename, const char* channelPruneFilename)
{
  bool               error                   = false; // Error flag.
  int                ii, jj, kk, ll, mm;              // Loop counters.  Generally, ii is for links, jj is for elements or shapes, kk is for element vertices,
                                                      // ll is for shape vertices, and mm is for neighbors or miscellaneous.
  int                numScanned;                      // Used to check that fscanf scanned all of the requested values.
  int                numberOfChannelElements = 0;     // Number of channel elements.
  int                numberOfNeighbors;               // Number of neighbors of a channel element.  Used for both channel and mesh neighbors.
  int                streamOrder;                     // Stream order of a stream.
  double             actualElementLength;             // Length in meters of actual channel elements.
  int                channelNodesSize;                // Size of channelNodesX and channelNodesY arrays.
  int                numberOfChannelNodes    = 0;     // Number of channel nodes actually existing in channelNodesX and channelNodesY arrays.
  double*            channelNodesX           = NULL;  // X coordinates of channel nodes.
  double*            channelNodesY           = NULL;  // Y coordinates of channel nodes.
  IntArrayCV*        channelVertices         = NULL;  // Channel node numbers of the channel element vertices.
  FILE*              nodeFile;                        // Input file for mesh nodes.
  int                meshNodeNumber;                  // For reading the node number of a mesh node.
  double             xCoordinate;                     // For reading X coordinates of mesh nodes.
  double             yCoordinate;                     // For reading Y coordinates of mesh nodes.
  FILE*              eleFile;                         // Input file for mesh elements.
  int                numberOfMeshElements;            // Number of Mesh elements.
  int                dimension;                       // Used to check that the dimension of the mesh element file is 3.
  int                numberOfAttributes;              // Used to check that the number of attributes in the mesh element file is 1.
  IntArrayMMN*       meshVertices            = NULL;  // Mesh node numbers of the mesh element vertices.
  int                meshElementNumber;               // For reading the element number of a mesh element.
  int                meshVertex0;                     // For reading a vertex of a mesh element.
  int                meshVertex1;                     // For reading a vertex of a mesh element.
  int                meshVertex2;                     // For reading a vertex of a mesh element.
  FILE*              edgeFile;                        // Input file for mesh edges.
  int                numberOfMeshEdges;               // Number of Mesh edges.
  int                boundary;                        // For reading the boundary code of a mesh edge.
  IntArrayMEE*       meshEdgeElements        = NULL;  // Mesh elements that are connected to each mesh edge.
  double*            meshEdgeLength          = NULL;  // Length of each mesh edge.
  int                meshEdgeNumber;                  // For reading the edge number of a mesh edge.
  SHPObject*         shape;                           // Shape object of a link.
  double             salientPointLocation;            // The 1D location in meters along a link of a salient point.
  SalientPointEnum   salientPointType;                // The type of a salient point.
  double             salientPointX;                   // X coordinate of salient point.
  double             salientPointY;                   // Y coordinate of salient point.
  double             salientPointFraction;            // Fraction of distance between two shape vertices of salient point.
  double             shapeVertexLocation;             // The 1D location in meters along a link of a shape vertex.
  bool               done;                            // Termination condition for complex loop.
  FILE*              outputFile              = NULL;  // Output file for channel nodes and elements.
  size_t             numPrinted;                      // Used to check that snprintf printed the correct number of characters.
  double             waterbodyArea;                   // For computing area of waterbody.
  double             xMin;                            // For computing bounding box of waterbody.
  double             xMax;                            // For computing bounding box of waterbody.
  double             yMin;                            // For computing bounding box of waterbody.
  double             yMax;                            // For computing bounding box of waterbody.
  double             contributingArea;                // Square kilometers.
  double             bankFullFlow;                    // Cubic meters per second.
  double             topWidth;                        // Meters.
  double             bankFullDepth;                   // Meters.
  int                numberOfMeshNodes       = 0;     // Size of and number of mesh nodes actually existing in meshNodesX and meshNodesY arrays.
  double*            meshNodesX              = NULL;  // X coordinates of mesh nodes.
  double*            meshNodesY              = NULL;  // Y coordinates of mesh nodes.
  LinkElementStruct* tempElement;                     // For simultaneous walk through channel elements and mesh edges when outputting mesh neighbors.
  double             elementBeginLocation;            // For calculating channel mesh neighbors edge length.
  double             elementEndLocation;              // For calculating channel mesh neighbors edge length.
  double             overlapBeginLocation;            // For calculating channel mesh neighbors edge length.
  double             overlapEndLocation;              // For calculating channel mesh neighbors edge length.
  double             overlapLength;                   // For calculating channel mesh neighbors edge length.
  int                meshNeighbors[ChannelElement_meshNeighborsSize];           // For storing channel mesh neighbors before outputting.
  double             meshNeighborsEdgeLength[ChannelElement_meshNeighborsSize]; // For storing channel mesh neighbors before outputting.
  int                waterbodyNumberOfVertices;       // Total number of vertices of a waterbody.
  double             waterbodyVerticesFraction;       // Fraction of those vertices that can be stored.
  int                waterbodyVerticesSoFar;          // Number of vertices seen so far through the loop.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && 0 < size);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  assert(!channelNetworkCheckInvariant(channels, size));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS)
  
  // Determine the number of elements of each link.
  for (ii = 0; ii < size; ii++)
    {
      channels[ii].elementStart = numberOfChannelElements;
      
      if (STREAM == channels[ii].type)
        {
          // Get stream order from the original link number in case this link was moved.
          streamOrder = channels[channels[ii].reachCode].streamOrder;
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (1 > streamOrder)
            {
              fprintf(stderr, "WARNING in writeChannelNetwork: stream order less than one in link %d.  Using one for stream order.\n", ii);
              
              streamOrder = 1;
            }
          else if (NUMBER_OF_STREAM_ORDERS < streamOrder)
            {
              fprintf(stderr, "WARNING in writeChannelNetwork: stream order greater than %d in link %d.  Using %d for stream order.\n",
                      NUMBER_OF_STREAM_ORDERS, ii, NUMBER_OF_STREAM_ORDERS);
              
              streamOrder = NUMBER_OF_STREAM_ORDERS;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          
          channels[ii].numberOfElements = numberOfLinkElements(channels[ii].length, streamOrderElementLength[streamOrder]);
        }
      else if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type)
        {
          channels[ii].numberOfElements = 1;
        }
      else
        {
          channels[ii].numberOfElements = 0;
        }
      
      numberOfChannelElements += channels[ii].numberOfElements;
    }
  
  // Read in mesh node, element, and edge files to determine the edge length and mesh element neighbors of each channel edge.
  
  // FIXME this could be done in addAllStreamMeshEdges and saved for later.
  // Open the files.
  nodeFile = fopen(meshNodeFilename, "r");
  eleFile  = fopen(meshElementFilename, "r");
  edgeFile = fopen(meshEdgeFilename, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NULL != nodeFile))
    {
      fprintf(stderr, "ERROR in writeChannelNetwork: Could not open mesh node file %s.\n", meshNodeFilename);
      error = true;
    }
  
  if (!(NULL != eleFile))
    {
      fprintf(stderr, "ERROR in writeChannelNetwork: Could not open mesh element file %s.\n", meshElementFilename);
      error = true;
    }
  
  if (!(NULL != edgeFile))
    {
      fprintf(stderr, "ERROR in writeChannelNetwork: Could not open mesh edge file %s.\n", meshEdgeFilename);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

  // Read the number of nodes from the file.
  if (!error)
    {
      numScanned = fscanf(nodeFile, "%d %d %d %d", &numberOfMeshNodes, &dimension, &numberOfAttributes, &boundary);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(4 == numScanned))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Unable to read header from mesh node file %s.\n", meshNodeFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfMeshNodes && 2 == dimension && 0 == numberOfAttributes && 1 == boundary))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Invalid header in mesh node file %s.\n", meshNodeFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Allocate arrays.
  if (!error)
    {
      meshNodesX = new double[numberOfMeshNodes];
      meshNodesY = new double[numberOfMeshNodes];
    }

  for (ii = 0; !error && ii < numberOfMeshNodes; ii++)
    {
      // Read node file.
      numScanned = fscanf(nodeFile, "%d %lf %lf %*d", &meshNodeNumber, &xCoordinate, &yCoordinate);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(3 == numScanned))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Unable to read entry %d from mesh node file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == meshNodeNumber))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Invalid node number in mesh node file.  %d should be %d.\n", meshNodeNumber, ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      if (!error)
        {
          meshNodesX[meshNodeNumber] = xCoordinate;
          meshNodesY[meshNodeNumber] = yCoordinate;
        }
    }
  
  // Read the number of mesh elements from the file.
  if (!error)
    {
      numScanned = fscanf(eleFile, "%d %d %d", &numberOfMeshElements, &dimension, &numberOfAttributes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(3 == numScanned))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Unable to read header from mesh element file %s.\n", meshElementFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfMeshElements && 3 == dimension && 1 == numberOfAttributes))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Invalid header in mesh element file %s.\n", meshElementFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Allocate the array.
  if (!error)
    {
      meshVertices = new int[numberOfMeshElements][3];
    }
  
  // Read the vertices.
  for (jj = 0; !error && jj < numberOfMeshElements; jj++)
    {
      numScanned = fscanf(eleFile, "%d %d %d %d %*d", &meshElementNumber, &meshVertex0, &meshVertex1, &meshVertex2);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(4 == numScanned))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Unable to read entry %d from mesh element file %s.\n", jj, meshElementFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(jj == meshElementNumber))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Invalid element number in mesh element file %s.  %d should be %d.\n", meshElementFilename,
                  meshElementNumber, jj);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      // Save the vertices.
      if (!error)
        {
          meshVertices[meshElementNumber][0] = meshVertex0;
          meshVertices[meshElementNumber][1] = meshVertex1;
          meshVertices[meshElementNumber][2] = meshVertex2;
        }
    }
  
  // Read the number of edges from the file.
  if (!error)
    {
      numScanned = fscanf(edgeFile, "%d %d", &numberOfMeshEdges, &boundary);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Unable to read header from edge file %s.\n", meshEdgeFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfMeshEdges && 1 == boundary))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Invalid header in edge file %s.\n", meshEdgeFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Allocate the array.
  if (!error)
    {
      meshEdgeElements = new int[numberOfMeshEdges][2];
      meshEdgeLength   = new double[numberOfMeshEdges];
    }

  // Read the edges.
  for (ii = 0; !error && ii < numberOfMeshEdges; ii++)
    {
      numScanned = fscanf(edgeFile, "%d %d %d %d", &meshEdgeNumber, &meshVertex1, &meshVertex2, &boundary);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(4 == numScanned))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Unable to read entry %d from edge file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == meshEdgeNumber))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Invalid edge number in edge file.  %d should be %d.\n", meshEdgeNumber, ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      // A boundary code of two or more indicates a channel edge.
      if (2 <= boundary)
        {
          if (!error)
            {
              // Convert to link number.
              boundary -= 2;

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
              if (!(boundary < size))
                {
                  fprintf(stderr, "ERROR in writeChannelNetwork: Mesh edge %d linked to invalid channel link number %d.\n", meshEdgeNumber, boundary);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
            }

          if (!error)
            {
              // Calculate the length of the mesh edge.
              meshEdgeLength[meshEdgeNumber] = sqrt((meshNodesX[meshVertex1] - meshNodesX[meshVertex2]) * (meshNodesX[meshVertex1] - meshNodesX[meshVertex2]) +
                                                    (meshNodesY[meshVertex1] - meshNodesY[meshVertex2]) * (meshNodesY[meshVertex1] - meshNodesY[meshVertex2]));
              
              // If the channel link is a waterbody we haven't yet recorded what mesh edges are linked to each waterbody so record that now in the element list
              // of the waterbody.
              if (WATERBODY == channels[boundary].type || ICEMASS == channels[boundary].type)
                {
                  createLinkElementAfter(channels, size, boundary, NULL, 0.0, meshEdgeNumber, COINCIDENT);
                }

              // Find what mesh elements the edge connects to.  A mesh edge can connect to at most two mesh elements.
              jj = 0;
              mm = 0;

              while (jj < numberOfMeshElements && mm < 2)
                {
                  if ((meshVertex1 == meshVertices[jj][0] || meshVertex1 == meshVertices[jj][1] || meshVertex1 == meshVertices[jj][2]) &&
                      (meshVertex2 == meshVertices[jj][0] || meshVertex2 == meshVertices[jj][1] || meshVertex2 == meshVertices[jj][2]))
                    {
                      meshEdgeElements[meshEdgeNumber][mm++] = jj;
                    }

                  jj++;
                }

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
              while (jj < numberOfMeshElements)
                {
                  if ((meshVertex1 == meshVertices[jj][0] || meshVertex1 == meshVertices[jj][1] || meshVertex1 == meshVertices[jj][2]) &&
                      (meshVertex2 == meshVertices[jj][0] || meshVertex2 == meshVertices[jj][1] || meshVertex2 == meshVertices[jj][2]))
                    {
                      fprintf(stderr, "WARNING in writeChannelNetwork: Mesh edge %d connected to more than two mesh elements.\n", meshEdgeNumber);
                    }

                  jj++;
                }
              
              if (!(0 < mm))
                {
                  fprintf(stderr, "WARNING in writeChannelNetwork: Mesh edge %d not connected to any mesh elements.\n", meshEdgeNumber);
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

              // Fill in any unused spaces with NOFLOW.
              while (mm < 2)
                {
                  meshEdgeElements[meshEdgeNumber][mm++] = NOFLOW;
                }

            }
        } // End if (2 <= boundary).
      else
        {
          // If the edge is not a channel edge we don't need to find out what mesh elements it connects to.  Fill in the unused spaces with NOFLOW.
          if (!error)
            {
              meshEdgeLength[meshEdgeNumber] = 0.0;
              
              for (mm = 0; mm < 2; mm++)
                {
                  meshEdgeElements[meshEdgeNumber][mm] = NOFLOW;
                }
            }
        }
    } // End read the edges.

  // Close the mesh node file.
  if (NULL != nodeFile)
    {
      fclose(nodeFile);
      nodeFile = NULL;
    }

  // Close the mesh element file.
  if (NULL != eleFile)
    {
      fclose(eleFile);
      eleFile = NULL;
    }

  // Close the mesh edge file.
  if (NULL != edgeFile)
    {
      fclose(edgeFile);
      edgeFile = NULL;
    }
  
  // deallocate arrays
  if (NULL != meshNodesX)
    {
      delete[] meshNodesX;
      meshNodesX = NULL;
    }
  
  if (NULL != meshNodesY)
    {
      delete[] meshNodesY;
      meshNodesY = NULL;
    }
  
  if (NULL != meshVertices)
    {
      delete[] meshVertices;
      meshVertices = NULL;
    }
  
  // Create arrays to hold the nodes and vertices. Start out assuming eight times as many nodes as elements.  Realloc if this proves too few.
  channelNodesSize = 8 * numberOfChannelElements;
  channelNodesX    = new double[channelNodesSize];
  channelNodesY    = new double[channelNodesSize];
  channelVertices  = new IntArrayCV[numberOfChannelElements];
  
  // channelVertices must be initialized because they may be written ahead.
  for (jj = 0; jj < numberOfChannelElements; jj++)
    {
      for (kk = 0; kk < ChannelElement_channelVerticesSize; kk++)
        {
          channelVertices[jj][kk] = -1;
        }
    }
  
  // Loop over all links.  For each used link loop over its shapes creating channel nodes and filling in channel vertices as you go.
  for (ii = 0; !error && ii < size; ii++)
    {
      if (STREAM == channels[ii].type)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          assert(0 <= channels[ii].reachCode && channels[ii].reachCode < size);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          // Get shape from the original link number in case this link was moved.
          shape = channels[channels[ii].reachCode].shapes[0];
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(NULL != shape))
            {
              fprintf(stderr, "ERROR in writeChannelNetwork: Channel link %d does not have a shape.\n", ii);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          
          // We are now going to do a simultaneous walk through the elements of the link and the vertices of the link's shape.  For each element fill in the
          // vertices that occur within that element.  Also fill in vertices at the beginning and end of the element, but do not add duplicate vertices if
          // shape already has vertices at the beginning or end of the element.
          if (!error)
            {
              // Start processing the first element of the link.
              jj = channels[ii].elementStart;
              
              // kk is the index into vertices[jj] of the next vertex to create.
              kk = 0;
              
              // The salient point is the next point along the link at the beginning or end of an element.  It is the next point that must be added as a vertex
              // even if it does not appear in shape.  The first salient point is the beginning of the link.  Get the location along shape of the beginning of
              // the link in case this link was moved.  If the link was not moved this will return zero.
              salientPointLocation = locationOfMovedLink(channels, size, channels[ii].reachCode, ii);
              salientPointType     = LINK_BEGIN;
              
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              assert(!isnan(salientPointLocation));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              // Move to the first vertex of shape that is at or beyond the beginning of the link.
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
              
              // Now we will add vertices and advance the salient point.  In this loop, ii is the link number, jj is the element number, kk is the vertex
              // number of element jj, and ll is the vertex number of shape.
              while (!error && !done)
                {
                  if (0 < ll && epsilonLess(shapeVertexLocation, salientPointLocation))
                    {
                      // The next shape vertex comes before the salient point so add it to the element.
                      if (kk < ChannelElement_channelVerticesSize)
                        {
                          // Save the shape vertex
                          createNode(shape->padfX[ll], shape->padfY[ll], &channelNodesSize, &numberOfChannelNodes, &channelNodesX, &channelNodesY,
                                     &channelVertices[jj][kk++]);

                          // Advance the shape vertex.
                          ll--;
                          shapeVertexLocation += sqrt((shape->padfX[ll] - shape->padfX[ll + 1]) * (shape->padfX[ll] - shape->padfX[ll + 1]) +
                                                      (shape->padfY[ll] - shape->padfY[ll + 1]) * (shape->padfY[ll] - shape->padfY[ll + 1]));
                        }
                      else
                        {
                          fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of vertices exceeds maximum number %d.\n",
                                  jj, ChannelElement_channelVerticesSize);
                          error = true;
                        }
                    }
                  else if (LINK_BEGIN == salientPointType)
                    {
                      // The next vertex to add to the element is the beginning of the link.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                      assert(0 == kk);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                              
                      // If the first vertex hasn't already been created by a neighbor, create it.
                      if (-1 == channelVertices[jj][kk])
                        {
                          // Get the X,Y coordinates of the vertex to add.
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
                          
                          createNode(salientPointX, salientPointY, &channelNodesSize, &numberOfChannelNodes, &channelNodesX, &channelNodesY,
                                     &channelVertices[jj][kk]);

                          // Set the vertex in all upstream STREAM neighbors.
                          for (mm = 0; mm < UPSTREAM_SIZE && NOFLOW != channels[ii].upstream[mm]; mm++)
                            {
                              if (!isBoundary(channels[ii].upstream[mm]) && STREAM == channels[channels[ii].upstream[mm]].type)
                                {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                                  assert(-1 == channelVertices[channels[channels[ii].upstream[mm]].elementStart +
                                                               channels[channels[ii].upstream[mm]].numberOfElements - 1]
                                                              [ChannelElement_channelVerticesSize - 1]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

                                  channelVertices[channels[channels[ii].upstream[mm]].elementStart +
                                                  channels[channels[ii].upstream[mm]].numberOfElements - 1]
                                                 [ChannelElement_channelVerticesSize - 1] = channelVertices[jj][kk];
                                }
                            }
                        } // End if the first vertex hasn't already been created by a neighbor, create it.
                      
                      // Whether it was just created or already there the first vertex is done.
                      kk++;

                      // If the shape vertex was already used as the salient point advance past it.
                      if (0 < ll && epsilonLessOrEqual(shapeVertexLocation, salientPointLocation))
                        {
                          ll--;
                          shapeVertexLocation += sqrt((shape->padfX[ll] - shape->padfX[ll + 1]) * (shape->padfX[ll] - shape->padfX[ll + 1]) +
                                                      (shape->padfY[ll] - shape->padfY[ll + 1]) * (shape->padfY[ll] - shape->padfY[ll + 1]));
                        }

                      // Advance an element length to the next salient point.
                      salientPointLocation += channels[ii].length / channels[ii].numberOfElements;

                      if (jj < channels[ii].elementStart + channels[ii].numberOfElements - 1)
                        {
                          salientPointType = ELEMENT_END;
                        }
                      else
                        {
                          salientPointType = LINK_END;
                        }
                    } // End else if (LINK_BEGIN == salientPointType).
                  else if (ELEMENT_END == salientPointType)
                    {
                      // The next vertex to add to the element is the end of the element and the beginning of the next element in the same link.
                      
                      // Create the vertex.
                      if (kk < ChannelElement_channelVerticesSize)
                        {
                          // Get the X,Y coordinates of the vertex to add.
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
                          
                          createNode(salientPointX, salientPointY, &channelNodesSize, &numberOfChannelNodes, &channelNodesX, &channelNodesY,
                                     &channelVertices[jj][kk]);

                          // Fill in the rest of the vertices with duplicates of the last vertex.
                          for (mm = kk + 1; mm < ChannelElement_channelVerticesSize; mm++)
                            {
                              channelVertices[jj][mm] = channelVertices[jj][kk];
                            }

                          // Set the first vertex in the next element.
                          channelVertices[jj + 1][0] = channelVertices[jj][kk];

                          // Advance to the next element.
                          jj++;
                          kk = 1;
                        } // End create the vertex.
                      else
                        {
                          fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of vertices exceeds maximum number %d.\n",
                                  jj, ChannelElement_channelVerticesSize);
                          error = true;
                        }
                      
                      if (!error)
                        {
                          // If the shape vertex was already used as the salient point advance past it.
                          if (0 < ll && epsilonLessOrEqual(shapeVertexLocation, salientPointLocation))
                            {
                              ll--;
                              shapeVertexLocation += sqrt((shape->padfX[ll] - shape->padfX[ll + 1]) * (shape->padfX[ll] - shape->padfX[ll + 1]) +
                                                          (shape->padfY[ll] - shape->padfY[ll + 1]) * (shape->padfY[ll] - shape->padfY[ll + 1]));
                            }

                          // Advance an element length to the next salient point.
                          salientPointLocation += channels[ii].length / channels[ii].numberOfElements;

                          if (jj < channels[ii].elementStart + channels[ii].numberOfElements - 1)
                            {
                              salientPointType = ELEMENT_END;
                            }
                          else
                            {
                              salientPointType = LINK_END;
                            }
                        }
                    } // End else if (ELEMENT_END == salientPointType).
                  else if (LINK_END == salientPointType)
                    {
                      // The next vertex to add to the element is the end of the last element in the link.
                      if (kk < ChannelElement_channelVerticesSize)
                        {
                          // If the last vertex hasn't already been created by a neighbor, create it.
                          if (-1 == channelVertices[jj][ChannelElement_channelVerticesSize - 1])
                            {
                              // Get the X,Y coordinates of the vertex to add.
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

                              createNode(salientPointX, salientPointY, &channelNodesSize, &numberOfChannelNodes, &channelNodesX, &channelNodesY,
                                         &channelVertices[jj][kk]);

                              // Set the vertex in the downstream STREAM neighbor and all upstream STREAM neighbors of that neighbor.
                              if (!isBoundary(channels[ii].downstream[0]) && STREAM == channels[channels[ii].downstream[0]].type)
                                {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                                  assert(-1 == channelVertices[channels[channels[ii].downstream[0]].elementStart][0]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

                                  channelVertices[channels[channels[ii].downstream[0]].elementStart][0] = channelVertices[jj][kk];

                                  for (mm = 0; mm < UPSTREAM_SIZE && NOFLOW != channels[channels[ii].downstream[0]].upstream[mm]; mm++)
                                    {
                                      if (ii != channels[channels[ii].downstream[0]].upstream[mm] &&
                                          !isBoundary(channels[channels[ii].downstream[0]].upstream[mm]) &&
                                          STREAM == channels[channels[channels[ii].downstream[0]].upstream[mm]].type)
                                        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                                          assert(-1 == channelVertices[channels[channels[channels[ii].downstream[0]].upstream[mm]].elementStart +
                                                                       channels[channels[channels[ii].downstream[0]].upstream[mm]].numberOfElements - 1]
                                                                      [ChannelElement_channelVerticesSize - 1]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

                                          channelVertices[channels[channels[channels[ii].downstream[0]].upstream[mm]].elementStart +
                                                          channels[channels[channels[ii].downstream[0]].upstream[mm]].numberOfElements - 1]
                                                         [ChannelElement_channelVerticesSize - 1] = channelVertices[jj][kk];
                                        }
                                    }
                                }
                            } // End if the last vertex hasn't already been created by a neighbor, create it.
                          else if (kk != ChannelElement_channelVerticesSize - 1)
                            {
                              // Move the vertex created by a neighbor into the right position.
                              channelVertices[jj][kk] = channelVertices[jj][ChannelElement_channelVerticesSize - 1];
                            }

                          if (!error)
                            {
                              // Fill in the rest of the vertices with duplicates of the last vertex.
                              for (mm = kk + 1; mm < ChannelElement_channelVerticesSize; mm++)
                                {
                                  channelVertices[jj][mm] = channelVertices[jj][kk];
                                }
                              
                              done = true;
                            }
                        } // End the next vertex to add to the element is the end of the last element in the link.
                      else
                        {
                          fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of vertices exceeds maximum number %d.\n",
                                  jj, ChannelElement_channelVerticesSize);
                          error = true;
                        }
                    } // End else if (LINK_END == salientPointType).
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                  else
                    {
                      assert(false);
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                } // End now we will add vertices and advance the salient point.  In this loop, ii is the link number, jj is the element number, kk is the
                  // vertex number of element jj, and ll is the vertex number of shape.
            } // End if (!error).
        } // End if (STREAM == channels[ii].type).
      else if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type)
        {
          // Create the vertices.  Waterbodies can have lots of vertices.  In order to keep the ChannelElement_channelVerticesSize array dimension from
          // getting too big we make it only a warning for a waterbody to have more than that many vertices, and we throw any extra vertices out.  In order
          // not to distort the waterbody shape too much we don't want to throw out a bunch of vertices at the end.  Instead, we throw out every Nth one.
          // To do this we first need to figure out how many total vertices the waterbody has and the fraction of them that can be stored.
          waterbodyNumberOfVertices = 0;
          
          for (jj = 0; !error && jj < SHAPES_SIZE && NULL != channels[ii].shapes[jj]; jj++)
            {
              waterbodyNumberOfVertices += channels[ii].shapes[jj]->nVertices;
            }
          
          // Get the fraction of the number of vertices that can be stored.  Can be greater than one in which case all vertices are stored.
          waterbodyVerticesFraction = ((double)ChannelElement_channelVerticesSize) / waterbodyNumberOfVertices;
          waterbodyVerticesSoFar    = 0;
          
          // Now store the vertices.  In this loop, ii is the link number, jj is the shape number, kk is the number of vertices that have been stored for the
          // one and only element of the waterbody, and ll is the vertex number of shape jj.
          kk = 0;
          
          for (jj = 0; !error && jj < SHAPES_SIZE && NULL != channels[ii].shapes[jj]; jj++)
            {
              shape = channels[ii].shapes[jj];

              for (ll = shape->nVertices - 1; !error && ll >= 0; ll--)
                {
                  // Now we ask oursleves, if we store this vertex will it exceed the allowable fraction of vertices seen so far?
                  if ((((double)(kk + 1)) / ++waterbodyVerticesSoFar) <= waterbodyVerticesFraction)
                    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                      assert(kk < ChannelElement_channelVerticesSize);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                      
                      // FIXME see if the point is a duplicate with any connected link.
                      createNode(shape->padfX[ll], shape->padfY[ll], &channelNodesSize, &numberOfChannelNodes, &channelNodesX, &channelNodesY,
                                 &channelVertices[channels[ii].elementStart][kk++]);
                    }
                }
            }
          
          // Fill in the rest of the vertices with duplicates of the last vertex.
          for (mm = kk; !error && mm < ChannelElement_channelVerticesSize; mm++)
            {
              channelVertices[channels[ii].elementStart][mm] = channelVertices[channels[ii].elementStart][kk - 1];
            }
          
          // Warn if vertices were thrown away.
          if (1.0 > waterbodyVerticesFraction)
            {
              fprintf(stderr, "WARNING in writeChannelNetwork: element %d: waterbody has %d vertices, which exceeds maximum number %d.  Extra vertices being "
                              "discarded.\n", channels[ii].elementStart, waterbodyNumberOfVertices, ChannelElement_channelVerticesSize);
              
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              assert(kk == ChannelElement_channelVerticesSize);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            }
        } // End else if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type).
    } // End loop over all links.  For each used link loop over its shapes creating channel nodes and filling in channel vertices as you go.
  
  // Open the channel node file.
  if (!error)
    {
      outputFile = fopen(channelNodeFilename, "w");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != outputFile))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Could not open channel node file %s.\n", channelNodeFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write the header into the channel node file.
  if (!error)
    {
      numPrinted = fprintf(outputFile, "%d 2 0 1\n", numberOfChannelNodes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(0 < numPrinted))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: incorrect return value %lu of snprintf when writing header in channel node file.\n", numPrinted);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write the channel nodes into the channel node file.
  for (ii = 0; !error && ii < numberOfChannelNodes; ii++)
    {
      numPrinted = fprintf(outputFile, "%d %lf %lf 0\n", ii, channelNodesX[ii], channelNodesY[ii]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(0 < numPrinted))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: incorrect return value %lu of snprintf when writing node %d in channel node file.\n", numPrinted, ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Close the channel node file.
  if (NULL != outputFile)
    {
      fclose(outputFile);
      outputFile = NULL;
    }
  
  // Open the channel element file.
  if (!error)
    {
      outputFile = fopen(channelElementFilename, "w");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != outputFile))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Could not open channel element file %s.\n", channelElementFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write the header into the channel element file.
  if (!error)
    {
      numPrinted = fprintf(outputFile, "%d\n", numberOfChannelElements);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(0 < numPrinted))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: incorrect return value %lu of snprintf when writing header in channel element file.\n", numPrinted);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write the channel elements into the channel element file.
  for (ii = 0; !error && ii < size; ii++)
    {
      if (STREAM == channels[ii].type || WATERBODY == channels[ii].type || ICEMASS == channels[ii].type)
        {
          // For waterbodies calculate length, top width, and bank full depth.
          // FIXME is there a better way to calculate these?
          if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type)
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
                  // Find the bounding box of all of the shapes of the waterbody.
                  SHPComputeExtents(shape);
                  
                  waterbodyArea = getAreaOfPolygon(shape->padfX, shape->padfY, shape->nVertices);
                  
                  xMin = shape->dfXMin;
                  xMax = shape->dfXMax;
                  yMin = shape->dfYMin;
                  yMax = shape->dfYMax;

                  for (jj = 1; jj < SHAPES_SIZE && NULL != channels[ii].shapes[jj]; jj++)
                    {
                      shape = channels[ii].shapes[jj];
                      
                      SHPComputeExtents(shape);

                      waterbodyArea += getAreaOfPolygon(shape->padfX, shape->padfY, shape->nVertices);
                      
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
                  
                  // Set the length of the waterbody as the long dimension of the bounding box.  Use += because we might have added length to the waterbody in
                  // fixShortLinks.
                  if (xMax - xMin > yMax - yMin)
                    {
                      channels[ii].length += xMax - xMin;
                    }
                  else
                    {
                      channels[ii].length += yMax - yMin;
                    }
                  
                  // Set the top width to preserve the area of the waterbody, and the bank full depth as ten percent of the top width.
                  topWidth      = waterbodyArea / channels[ii].length;
                  bankFullDepth = topWidth * 0.1;
                }
            } // End for waterbodies calculate length, top width, and bank full depth.
          
          // Calculate the length of the elements of this link.
          if (!error)
            {
              actualElementLength = channels[ii].length / channels[ii].numberOfElements;

              // If a stream is split into more than one channel element then the smallest that the actual channel element length can be is ~70% of the desired
              // channel element length.  This occurs when the stream is just barely big enough to be split into two channel elements.  However, if an entire
              // link is shorter than this it results in a single channel element that is too short.  Short channel elements can be problematic for running a
              // simulation.  Arbitrarily increase the length of these short links.
              if (STREAM == channels[ii].type)
                {
                  // Get stream order from the original link number in case this link was moved.
                  streamOrder = channels[channels[ii].reachCode].streamOrder;

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                  if (1 > streamOrder)
                    {
                      fprintf(stderr, "WARNING in writeChannelNetwork: stream order less than one in link %d.  Using one for stream order.\n", ii);

                      streamOrder = 1;
                    }
                  else if (NUMBER_OF_STREAM_ORDERS < streamOrder)
                    {
                      fprintf(stderr, "WARNING in writeChannelNetwork: stream order greater than %d in link %d.  Using %d for stream order.\n",
                          NUMBER_OF_STREAM_ORDERS, ii, NUMBER_OF_STREAM_ORDERS);

                      streamOrder = NUMBER_OF_STREAM_ORDERS;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                }
              else
                {
                  // For waterbodies use the minimum element length of first order streams.
                  streamOrder = 1;
                }

              if (actualElementLength < 0.7 * streamOrderElementLength[streamOrder])
                {
                  fprintf(stderr, "WARNING in writeChannelNetwork: element %d length increased from %lf to %lf.\n", channels[ii].elementStart,
                          actualElementLength, 0.7 * streamOrderElementLength[streamOrder]);
                  
                  actualElementLength = 0.7 * streamOrderElementLength[streamOrder];
                }
            } // End calculate the length of the elements of this link.
          
          if (STREAM == channels[ii].type)
            {
              // Initialize these values for the simultaneous walk through channel elements and mesh edges when outputting mesh neighbors.
              tempElement        = channels[ii].firstElement;
              elementEndLocation = 0.0;
            }
          
          // Write out the elements of the link.
          for (jj = channels[ii].elementStart; !error && jj < channels[ii].elementStart + channels[ii].numberOfElements; jj++)
            {
              // Calculate topWidth and bankFullDepth for streams.
              if (STREAM == channels[ii].type)
                {
                  // Calculate contributing area as the weighted average of upstream and downstream contributing area weighted by the distance of this element
                  // along the link.  Divide by 1.0e6 to convert from square meters to square kilometers.
                  contributingArea = contributingAreaAtLocation(channels[ii].upstreamContributingArea, channels[ii].downstreamContributingArea, (jj - channels[ii].elementStart + 0.5) / channels[ii].numberOfElements) / 1.0e6;
                  bankFullFlow     = pow(10.0, log10(contributingArea) * 0.7851283403 - 0.371348345); // Cubic meters per second.  Ref. FIXME Nels Frasier.
                  topWidth         = 7.70 * pow(bankFullFlow, 0.29); // Meters. Ref. Wohl and Merritt (2008) for plane-bed streams.
                  bankFullDepth    = 0.24 * pow(bankFullFlow, 0.38); // Meters. Ref. Wohl and Merritt (2008) for plane-bed streams.
                }
              
              // Clip topWidth and BankFullDepth if they are too small.
              if (0.1 > topWidth)
                {
                  topWidth = 0.1;
                }
              
              if (0.1 > bankFullDepth)
                {
                  bankFullDepth = 0.1;
                }
              
              // Output fixed length per-element information.
              numPrinted = fprintf(outputFile, "%d %d %lld %lf %lf %lf %d %d %d", jj, channels[ii].type, channels[ii].reachCode, actualElementLength,
                                   topWidth, bankFullDepth, ChannelElement_channelVerticesSize, ChannelElement_channelNeighborsSize,
                                   ChannelElement_meshNeighborsSize);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(0 < numPrinted))
                {
                  fprintf(stderr, "ERROR in writeChannelNetwork: incorrect return value %lu of snprintf when writing element in element file.\n", numPrinted);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              
              // Output vertices.
              for (kk = 0; !error && kk < ChannelElement_channelVerticesSize; kk++)
                {
                  numPrinted = fprintf(outputFile, " %d", channelVertices[jj][kk]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(0 < numPrinted))
                    {
                      fprintf(stderr, "ERROR in writeChannelNetwork: incorrect return value %lu of snprintf when writing vertex in element file.\n",
                              numPrinted);
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
              
              // Output upstream channel neighbors.
              if (!error)
                {
                  numberOfNeighbors = 0;

                  if (jj == channels[ii].elementStart)
                    {
                      // The first element of a link is connected to the last element of all upstream neighbors of that link.
                      for (mm = 0; !error && mm < UPSTREAM_SIZE && NOFLOW != channels[ii].upstream[mm]; mm++)
                        {
                          if (numberOfNeighbors < ChannelElement_channelNeighborsSize)
                            {
                              numberOfNeighbors++;
                              
                              if (isBoundary(channels[ii].upstream[mm]))
                                {
                                  numPrinted = fprintf(outputFile, " %d 0", channels[ii].upstream[mm]);
                                }
                              else
                                {
                                  numPrinted = fprintf(outputFile, " %d 0", (channels[channels[ii].upstream[mm]].elementStart +
                                                                             channels[channels[ii].upstream[mm]].numberOfElements - 1));
                                }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                              if (!(0 < numPrinted))
                                {
                                  fprintf(stderr, "ERROR in writeChannelNetwork: incorrect return value %lu of snprintf when writing channel neighbor in "
                                          "element file.\n", numPrinted);
                                  error = true;
                                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                          else
                            {
                              fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of channel neighbors exceeds maximum number %d.\n",
                                      jj, ChannelElement_channelNeighborsSize);
                              error = true;
                            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                        }
                    }
                  else
                    {
                      // Elements other than the first element in a link are only connected to the previous element in the link.
                      if (numberOfNeighbors < ChannelElement_channelNeighborsSize)
                        {
                          numberOfNeighbors++;
                          numPrinted = fprintf(outputFile, " %d 0", jj - 1);
                          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                          if (!(0 < numPrinted))
                            {
                              fprintf(stderr, "ERROR in writeChannelNetwork: incorrect return value %lu of snprintf when writing channel neighbor in "
                                      "element file.\n", numPrinted);
                              error = true;
                            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                      else
                        {
                          fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of channel neighbors exceeds maximum number %d.\n",
                                  jj, ChannelElement_channelNeighborsSize);
                          error = true;
                        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                    }
                } // End output upstream channel neighbors.
              
              // Output downstream channel neighbors.
              if (!error)
                {
                  if (jj == channels[ii].elementStart + channels[ii].numberOfElements - 1)
                    {
                      // The last element of a link is connected to the first element of all downstream neighbors of that link.
                      for (mm = 0; !error && mm < DOWNSTREAM_SIZE && NOFLOW != channels[ii].downstream[mm]; mm++)
                        {
                          if (numberOfNeighbors < ChannelElement_channelNeighborsSize)
                            {
                              numberOfNeighbors++;
                              
                              if (isBoundary(channels[ii].downstream[mm]))
                                {
                                  numPrinted = fprintf(outputFile, " %d 1", channels[ii].downstream[mm]);
                                }
                              else
                                {
                                  numPrinted = fprintf(outputFile, " %d 1", channels[channels[ii].downstream[mm]].elementStart);
                                }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                              if (!(0 < numPrinted))
                                {
                                  fprintf(stderr, "ERROR in writeChannelNetwork: incorrect return value %lu of snprintf when writing channel neighbor in "
                                          "element file.\n", numPrinted);
                                  error = true;
                                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                          else
                            {
                              fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of channel neighbors exceeds maximum number %d.\n",
                                      jj, ChannelElement_channelNeighborsSize);
                              error = true;
                            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                        }
                    }
                  else
                    {
                      // Elements other than the last element in a link are only connected to the next element in the link.
                      if (numberOfNeighbors < ChannelElement_channelNeighborsSize)
                        {
                          numberOfNeighbors++;
                          numPrinted = fprintf(outputFile, " %d 1", jj + 1);
                          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                          if (!(0 < numPrinted))
                            {
                              fprintf(stderr, "ERROR in writeChannelNetwork: incorrect return value %lu of snprintf when writing channel neighbor in "
                                      "element file.\n", numPrinted);
                              error = true;
                            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                      else
                        {
                          fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of channel neighbors exceeds maximum number %d.\n",
                                  jj, ChannelElement_channelNeighborsSize);
                          error = true;
                        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                    }
                } // End output downstream channel neighbors.
              
              // Fill in unused channel neighbors with NOFLOW.
              while (!error && numberOfNeighbors < ChannelElement_channelNeighborsSize)
                {
                  numberOfNeighbors++;
                  numPrinted = fprintf(outputFile, " %d 0", NOFLOW);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(0 < numPrinted))
                    {
                      fprintf(stderr, "ERROR in writeChannelNetwork: incorrect return value %lu of snprintf when writing channel neighbor in element file.\n",
                              numPrinted);
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
              
              // Output mesh neighbors.
              if (!error)
                {
                  numberOfNeighbors = 0;

                  if (STREAM == channels[ii].type)
                    {
                      // Store new channel element location.
                      elementBeginLocation = elementEndLocation;
                      elementEndLocation  += channels[ii].length / channels[ii].numberOfElements;

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                      // We should not get past the last mesh neighbor element until we are done processing the last channel element for this link, and we have already
                      // walked forward to the first mesh neighbor element that overlaps this channel element.
                      assert(NULL != tempElement && epsilonGreater(tempElement->endLocation, elementBeginLocation));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

                      // Output mesh neighbor elements that overlap this channel element and not the next channel element.
                      while (!error && NULL != tempElement && epsilonLessOrEqual(tempElement->endLocation, elementEndLocation))
                        {
                          if (-1 != tempElement->edge)
                            {
                              // Calculate the fraction of the edge's length that overlaps this channel element.  NOTE: It is the fraction of the mesh edge's
                              // length, not the fraction of the channel element's length.  Then multiply by the mesh edge's length to get the length in meters
                              // of the overlap.
                              if (beginLocation(tempElement) > elementBeginLocation)
                                {
                                  overlapBeginLocation = beginLocation(tempElement);
                                }
                              else
                                {
                                  overlapBeginLocation = elementBeginLocation;
                                }

                              if (tempElement->endLocation < elementEndLocation)
                                {
                                  overlapEndLocation = tempElement->endLocation;
                                }
                              else
                                {
                                  overlapEndLocation = elementEndLocation;
                                }
                              
                              overlapLength = ((overlapEndLocation - overlapBeginLocation) / (tempElement->endLocation - beginLocation(tempElement))) *
                                              meshEdgeLength[tempElement->edge];

                              // Add the mesh neighbors that are connected to this mesh edge.
                              for (mm = 0; !error && mm < 2 && NOFLOW != meshEdgeElements[tempElement->edge][mm]; mm++)
                                {
                                  // Scan to see if this channel element is already connected to this mesh element along another edge.
                                  ll = 0;
                                  
                                  while (ll < numberOfNeighbors && meshNeighbors[ll] != meshEdgeElements[tempElement->edge][mm])
                                    {
                                      ll++;
                                    }
                                  
                                  if (ll < numberOfNeighbors)
                                    {
                                      // If already connected, just add the length to that neighbor.
                                      meshNeighborsEdgeLength[ll] += overlapLength;
                                    }
                                  else
                                    {
                                      // Otherwise, add a new neighbor.
                                      if (numberOfNeighbors < ChannelElement_meshNeighborsSize)
                                        {
                                          meshNeighbors[numberOfNeighbors]           = meshEdgeElements[tempElement->edge][mm];
                                          meshNeighborsEdgeLength[numberOfNeighbors] = overlapLength;
                                          numberOfNeighbors++;
                                        }
    #if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                                      else
                                        {
                                          fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of mesh neighbors exceeds maximum number %d.\n",
                                                  jj, ChannelElement_meshNeighborsSize);
                                          error = true;
                                        }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                                    }
                                }
                            } // End if (-1 != tempElement->edge).

                          tempElement = tempElement->next;
                        } // End output mesh neighbor elements that overlap this channel element and not the next channel element.

                      // Output a mesh neighbor element that overlaps this channel element and the next channel element.
                      if (!error && NULL != tempElement && epsilonLess(beginLocation(tempElement), elementEndLocation))
                        {
                          if (-1 != tempElement->edge)
                            {
                              // Calculate the fraction of the edge's length that overlaps this channel element.  NOTE: It is the fraction of the mesh edge's
                              // length, not the fraction of the channel element's length.  Then multiply by the mesh edge's length to get the length in meters
                              // of the overlap.
                              if (beginLocation(tempElement) > elementBeginLocation)
                                {
                                  overlapBeginLocation = beginLocation(tempElement);
                                }
                              else
                                {
                                  overlapBeginLocation = elementBeginLocation;
                                }

                              if (tempElement->endLocation < elementEndLocation)
                                {
                                  overlapEndLocation = tempElement->endLocation;
                                }
                              else
                                {
                                  overlapEndLocation = elementEndLocation;
                                }
                              
                              overlapLength = ((overlapEndLocation - overlapBeginLocation) / (tempElement->endLocation - beginLocation(tempElement))) *
                                              meshEdgeLength[tempElement->edge];

                              // Add the mesh neighbors that are connected to this mesh edge.
                              for (mm = 0; !error && mm < 2 && NOFLOW != meshEdgeElements[tempElement->edge][mm]; mm++)
                                {
                                  // Scan to see if this channel element is already connected to this mesh element along another edge.
                                  ll = 0;
                                  
                                  while (ll < numberOfNeighbors && meshNeighbors[ll] != meshEdgeElements[tempElement->edge][mm])
                                    {
                                      ll++;
                                    }
                                  
                                  if (ll < numberOfNeighbors)
                                    {
                                      // If already connected, just add the length to that neighbor.
                                      meshNeighborsEdgeLength[ll] += overlapLength;
                                    }
                                  else
                                    {
                                      // Otherwise, add a new neighbor.
                                      if (numberOfNeighbors < ChannelElement_meshNeighborsSize)
                                        {
                                          meshNeighbors[numberOfNeighbors]           = meshEdgeElements[tempElement->edge][mm];
                                          meshNeighborsEdgeLength[numberOfNeighbors] = overlapLength;
                                          numberOfNeighbors++;
                                        }
    #if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                                      else
                                        {
                                          fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of mesh neighbors exceeds maximum number %d.\n",
                                                  jj, ChannelElement_meshNeighborsSize);
                                          error = true;
                                        }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                                    }
                                }
                            } // End if (-1 != tempElement->edge).
                        } // End output a mesh neighbor element that overlaps this channel element and the next channel element.
                    } // End if (STREAM == channels[ii].type).
                  else // if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type).
                    {
                      for (tempElement = channels[ii].firstElement; !error && NULL != tempElement; tempElement = tempElement->next)
                        {
                          if (-1 != tempElement->edge)
                            {
                              // Add the mesh neighbors that are connected to this mesh edge.
                              for (mm = 0; !error && mm < 2 && NOFLOW != meshEdgeElements[tempElement->edge][mm]; mm++)
                                {
                                  // Scan to see if this channel element is already connected to this mesh element along another edge.
                                  ll = 0;

                                  while (ll < numberOfNeighbors && meshNeighbors[ll] != meshEdgeElements[tempElement->edge][mm])
                                    {
                                      ll++;
                                    }

                                  if (ll < numberOfNeighbors)
                                    {
                                      // If already connected, just add the length to that neighbor.
                                      meshNeighborsEdgeLength[ll] += meshEdgeLength[tempElement->edge];
                                    }
                                  else
                                    {
                                      // Otherwise, add a new neighbor.
                                      if (numberOfNeighbors < ChannelElement_meshNeighborsSize)
                                        {
                                          meshNeighbors[numberOfNeighbors]           = meshEdgeElements[tempElement->edge][mm];
                                          meshNeighborsEdgeLength[numberOfNeighbors] = meshEdgeLength[tempElement->edge];
                                          numberOfNeighbors++;
                                        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                                      else
                                        {
                                          fprintf(stderr, "ERROR in writeChannelNetwork: element %d: number of mesh neighbors exceeds maximum number %d.\n",
                                              jj, ChannelElement_meshNeighborsSize);
                                          error = true;
                                        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                                    }
                                }
                            }
                        }
                    } // End if (WATERBODY == channels[ii].type || ICEMASS == channels[ii].type).
                  
                  // It's possible for a channel element to connect with a mesh neighbor over a very small edge length.  Remove those neighbor connections.
                  if (!error)
                    {
                      ll = 0;
                      
                      while (ll < numberOfNeighbors)
                        {
                          if (1.0 > meshNeighborsEdgeLength[ll]) // FIXME How small is too small? 
                            {
                              numberOfNeighbors--;
                              
                              for (mm = ll; mm < numberOfNeighbors; mm++)
                                {
                                  meshNeighbors[mm]           = meshNeighbors[mm + 1];
                                  meshNeighborsEdgeLength[mm] = meshNeighborsEdgeLength[mm + 1];
                                }
                              
                              meshNeighbors[numberOfNeighbors]           = NOFLOW;
                              meshNeighborsEdgeLength[numberOfNeighbors] = 0.0;
                            }
                          else
                            {
                              ll++;
                            }
                        }
                    }
                  
                  // Print mesh neighbors to file
                  for (ll = 0; !error && ll < numberOfNeighbors; ll++)
                    {
                      numPrinted = fprintf(outputFile, " %d %.2lf", meshNeighbors[ll], meshNeighborsEdgeLength[ll]);

    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                      if (!(0 < numPrinted))
                        {
                          fprintf(stderr, "ERROR in writeChannelNetwork: incorrect return value %lu of snprintf when writing mesh neighbor in element file.\n",
                                  numPrinted);
                          error = true;
                        }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                    }
                } // End output mesh neighbors.
              
              // Fill in unused mesh neighbors with NOFLOW.
              while (!error && numberOfNeighbors < ChannelElement_meshNeighborsSize)
                {
                  numberOfNeighbors++;
                  numPrinted = fprintf(outputFile, " %d %.0lf", NOFLOW, 0.0);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(0 < numPrinted))
                    {
                      fprintf(stderr, "ERROR in writeChannelNetwork: incorrect return value %lu of snprintf when writing mesh neighbor in element file.\n",
                              numPrinted);
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              // End the line.
              if (!error)
                {
                  numPrinted = fprintf(outputFile, "\n");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(0 < numPrinted))
                    {
                      fprintf(stderr, "ERROR in writeChannelNetwork: incorrect return value %lu of snprintf when writing end of line in element file.\n",
                              numPrinted);
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            } // End write out the elements of the link.
        } // End if (STREAM == channels[ii].type || WATERBODY == channels[ii].type || ICEMASS == channels[ii].type).
    } // End write the channel elements into the channel element file.

  // Close the channel element file.
  if (NULL != outputFile)
    {
      fclose(outputFile);
      outputFile = NULL;
    }
  
  // Open the channel prune file.
  if (!error)
    {
      outputFile = fopen(channelPruneFilename, "w");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != outputFile))
        {
          fprintf(stderr, "ERROR in writeChannelNetwork: Could not open channel prune file %s.\n", channelPruneFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write the pruned links into the channel prune file.
  for (ii = 0; !error && ii < size; ii++)
    {
      if (PRUNED_STREAM == channels[ii].type)
        {
          jj = channels[ii].downstream[0];

          while (!isBoundary(jj) && (channels[ii].reachCode == channels[jj].reachCode || PRUNED_STREAM == channels[jj].type))
            {
              jj = channels[jj].downstream[0];
            }

          if (!isBoundary(jj))
            {
              numPrinted = fprintf(outputFile, "%lld %lld\n", channels[ii].reachCode, channels[jj].reachCode);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(0 < numPrinted))
                {
                  fprintf(stderr, "ERROR in writeChannelNetwork: incorrect return value %lu of snprintf when writing link %d in channel prune file.\n",
                          numPrinted, ii);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
          else
            {
              fprintf(stderr, "WARNING in writeChannelNetwork: pruned stream %d does not flow downstream into any unpruned link.\n", ii);
            }
        }
    }

  // Close the channel prune file.
  if (NULL != outputFile)
    {
      fclose(outputFile);
      outputFile = NULL;
    }

  // Deallocate arrays.
  if (NULL != channelNodesX)
    {
      delete[] channelNodesX;
    }
  
  if (NULL != channelNodesY)
    {
      delete[] channelNodesY;
    }
  
  if (NULL != channelVertices)
    {
      delete[] channelVertices;
    }
  
  if (NULL != meshEdgeElements)
    {
      delete[] meshEdgeElements;
    }
  
  if (NULL != meshEdgeLength)
    {
      delete[] meshEdgeLength;
    }
  
  return error;
}

// Free memory allocated for the channel network.
//
// Parameters:
//
// channels - A pointer to the channel network passed by reference.
//            Will be set to NULL after the memory is deallocated.
// size     - A scalar passed by reference containing the number of elements in
//            channels.  Will be set to zero after the memory is deallocated.
void channelNetworkDealloc(ChannelLinkStruct** channels, int* size)
{
  int ii, jj; // Loop counters.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != channels && NULL != *channels && NULL != size && 0 < *size);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Don't check invariant because dealloc can be called after an error when the invariant will fail.
  
  for (ii = 0; ii < *size; ii++)
    {
      for (jj = 0; jj < SHAPES_SIZE; jj++)
        {
          if (NULL != (*channels)[ii].shapes[jj])
            {
              SHPDestroyObject((*channels)[ii].shapes[jj]);
            }
        }
      
      // Delete all LinkElementStructs.  Do not use killLinkElement because we don't need the full generality of that function and at this moment the channel
      // network might not pass the invariant, which might be checked in that function.
      while (NULL != (*channels)[ii].firstElement)
        {
          if (NULL != (*channels)[ii].firstElement->next)
            {
              (*channels)[ii].firstElement = (*channels)[ii].firstElement->next;
              
              delete (*channels)[ii].firstElement->prev;
            }
          else
            {
              delete (*channels)[ii].firstElement;
              
              (*channels)[ii].firstElement = NULL;
            }
        }
    }
  
  delete[] *channels;
  
  *channels = NULL;
  *size     = 0;
}

int main(int argc, char** argv)
{
  
  bool               error = false; // Error flag.
  int                ii;            // Loop counter.
  ChannelLinkStruct* channels = NULL;      // The channel network.
  int                size;          // The number of elements in channels.
  
  // Reach codes get set when reading the link file, but the types of those links get set in readWaterbodies and readTaudemStreamnet so temporarily there are
  // reach codes in not used links.
  allowNotUsedToHaveReachCode = true;
  
  // There can be zero length streams in the TauDEM input.  We prune them out in readTaudemStreamnet, but it checks the invariant before the pruning process so
  // temporarily there are unpruned streams with zero length.
  allowStreamToHaveZeroLength = true;
  
  // When adding an upstream/downstream connection we have to add one first and then the invariant will fail on the arguments to the function to add the other.
  allowNonReciprocalUpstreamDownstreamLinks = false;
  
  // When adding a link element at the end of a link the length of the last unmoved element must temporarily be different than the link length.
  allowLastLinkElementLengthNotEqualToLinkLength = false;
  
  // When we split a link we update the end locations of link elements before moving them to the new link so they will temporarily fail the invariant.
  allowLinkElementEndLocationsNotMonotonicallyIncreasing = false;
 
  if(argc == 2)
  {
    //Only passed mapDir
    mapDir = std::string(argv[1]);
    baseName = std::string("");
    resolution = std::string("");
  }
  else if(argc == 3)
  {
    //Only passed mapDir and baseName
    mapDir = std::string(argv[1]);
    baseName = std::string(argv[2]);
    resolution = std::string("");
  }
  else if(argc == 4)
  {
        //Passed mapDir, baseName, and a resolution    
        mapDir = std::string(argv[1]);
        baseName = std::string(argv[2]);
        resolution = std::string(argv[3]);
  }
  else
  {
    error = true;
    printf("Usage: adhydroMapDir [baseName] [resolution]\n");
  }
  
  if(!error)
  {
    std::string asciiDir = mapDir+"/ASCII/";
    if(argc==4)
        asciiDir = asciiDir+resolution+"_meter/";
    std::string taudemDir = mapDir+"/TauDEM/";
    std::string arcgisDir = mapDir+"/ArcGIS/";

    streamNetworkShapefile = arcgisDir+baseName+"projectednet";
    waterbodiesShapefile = arcgisDir+baseName+"waterbodies_final";
    waterbodiesStreamsIntersectionsShapefile = arcgisDir+baseName+"wtrbodies_streams_intersect_final";
    waterbodiesWaterbodiesIntersectionsShapefile = arcgisDir+baseName+"intersect_wtrbodies_final";
    meshLinkFilename = asciiDir+"mesh.1.link";
    meshNodeFilename = asciiDir+"mesh.1.node";
    meshElementFilename = asciiDir+"mesh.1.ele";
    meshEdgeFilename = asciiDir+"mesh.1.edge";

    channelNodeFilename = asciiDir+"mesh.1.chan.node";
    channelElementFilename = asciiDir+"mesh.1.chan.ele";
    channelPruneFilename = asciiDir+"mesh.1.chan.prune";

  }

  if(!error)
  {  
    error = readLink(&channels, &size, meshLinkFilename.c_str());
  }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  if (!error)
    {
      error = channelNetworkCheckInvariant(channels, size);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  
  if (!error)
    {
      error = readWaterbodies(channels, size, waterbodiesShapefile.c_str());
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  if (!error)
    {
      error = channelNetworkCheckInvariant(channels, size);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  
  if (!error)
    {
      error = readTaudemStreamnet(channels, size, streamNetworkShapefile.c_str());
    }
  
  // Reach codes get set when reading the link file, but the types of those links get set in readWaterbodies and readTaudemStreamnet so temporarily there are
  // reach codes in not used links.
  allowNotUsedToHaveReachCode = false;
  
  // There can be zero length streams in the TauDEM input.  We prune them out in readTaudemStreamnet, but it checks the invariant before the pruning process so
  // temporarily there are unpruned streams with zero length.
  allowStreamToHaveZeroLength = false;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  if (!error)
    {
      error = channelNetworkCheckInvariant(channels, size);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  
  // When adding a link element at the end of a link the length of the last unmoved element must temporarily be different than the link length.
  allowLastLinkElementLengthNotEqualToLinkLength = true;
  
  if (!error)
    {
      error = addAllStreamMeshEdges(channels, size, meshNodeFilename.c_str(), meshEdgeFilename.c_str());
    }
  
  // When adding a link element at the end of a link the length of the last unmoved element must temporarily be different than the link length.
  allowLastLinkElementLengthNotEqualToLinkLength = false;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  if (!error)
    {
      error = channelNetworkCheckInvariant(channels, size);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  
  if (!error)
    {
      error = readWaterbodyStreamIntersections(channels, size, waterbodiesStreamsIntersectionsShapefile.c_str());
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  if (!error)
    {
      error = channelNetworkCheckInvariant(channels, size);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  
  if (!error)
    {
      error = linkAllWaterbodyStreamIntersections(channels, size);
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  if (!error)
    {
      error = channelNetworkCheckInvariant(channels, size);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  
  if (!error)
    {
      error = readAndLinkWaterbodyWaterbodyIntersections(channels, size, waterbodiesWaterbodiesIntersectionsShapefile.c_str());
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  if (!error)
    {
      error = channelNetworkCheckInvariant(channels, size);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)

  if (!error)
    {
      for (ii = 0; ii < size; ii++)
        {
          tryToPruneLink(channels, size, ii);
        }
    }

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  if (!error)
    {
      error = channelNetworkCheckInvariant(channels, size);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)

  if (!error)
    {
      error = fixShortLinks(channels, size);
    }

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  if (!error)
    {
      error = channelNetworkCheckInvariant(channels, size);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)

  if (!error)
    {
      error = writeChannelNetwork(channels, size, meshNodeFilename.c_str(), meshElementFilename.c_str(), meshEdgeFilename.c_str(), channelNodeFilename.c_str(), channelElementFilename.c_str(),
                                  channelPruneFilename.c_str());
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  assert(!allowNotUsedToHaveReachCode && !allowStreamToHaveZeroLength && !allowNonReciprocalUpstreamDownstreamLinks &&
         !allowLastLinkElementLengthNotEqualToLinkLength && !allowLinkElementEndLocationsNotMonotonicallyIncreasing);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  if (!error)
    {
      error = channelNetworkCheckInvariant(channels, size);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)

  if (NULL != channels)
    {
      channelNetworkDealloc(&channels, &size);
    }
  
  return error;
}
