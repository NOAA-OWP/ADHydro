#ifndef ADHYDRO_BOOST_TYPES_H
#define ADHYDRO_BOOST_TYPES_H

/*
 * Using Boost for geometry operations and spatial indexing
 */
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp> //For building more complex geometries
#include <boost/geometry/geometries/box.hpp> //For building bounding boxes
#include <boost/geometry/geometries/polygon.hpp> //For defining mesh elements
#include <boost/geometry/geometries/linestring.hpp> //For defining channel elements
#include <boost/geometry/algorithms/envelope.hpp> //For envelope function
#include <boost/geometry/index/rtree.hpp>
#include <boost/variant.hpp>
#include <boost/foreach.hpp>

//Simplify the boost namespaces
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
//Simplify geometry types
typedef bg::model::point<double, 2, bg::cs::cartesian> point;
typedef bg::model::box<point> box;
//A polygon is made up of point type points, is counter clockwise, and is closed
typedef bg::model::polygon<point, false, true> polygon; // ccw, closed polygon
typedef bg::model::linestring<point> linestring;
typedef boost::variant<polygon, linestring> geometry;

 //These are the enteries stored in the R-tree, the envelope geometry, and the ID and original geometry
typedef std::pair< box, std::pair<unsigned, geometry> > value;
typedef bgi::rtree<value, bgi::quadratic<16, 4> > rtree;

#endif
