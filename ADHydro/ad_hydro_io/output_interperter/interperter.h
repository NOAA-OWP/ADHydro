#ifndef IO_INTERPERTER_H
#define IO_INTERPERTER_H

#include "parser.h"
#include "lexer.h"
#include <fstream>
#include <string>
#include <algorithm>
#include <unordered_map>
/*
 * Include proj4 library for projections, since boost::geometry projects is still
 * an extension and not available with the release version of boost.
 */
#include <proj_api.h>
#include "adhydro_boost_types.h"

#define ADHYDRO_PROJ4 "+proj=sinu +lon_0=-109 +x_0=20000000 +y_0=10000000 +towgs84=0,0,0,0,0,0,0 +datum=WGS84 +units=m +no_defs"
#define LAT_LONG_PROJ4 "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"
#define DEG_TO_RADIANS (M_PI/180)

//TODO/FIXME MOVE IMPLEMENTATION TO .cpp FILE!!! OTHERWISE LINKING GETS FUBARED

class Interperter : public NodeVisitor
{
    /** @class Interperter
     * Class for interperting the abstract syntax tree built by the parser
     * Implements the visitor pattern
     */
public:
    //Containers for storing interperted results
    std::vector<box> boundingBoxes;
private:
    AbstractSyntaxTree* tree;
    Lexer* lexer;
    Parser* parser;
    //Interface references to ADHydro OutputManager
    rtree const& meshSpatialIndex;
    rtree const& channelSpatialIndex;
    std::unordered_map<int, bool>& meshOutputElements;
    std::unordered_map<int, bool>& channelOutputElements;
    //Coordinate systems for proj4 projections
    projPJ pj_latlong;
    projPJ pj_adhydro;
    
public:
    Interperter(std::string file, rtree const& _meshSpatialIndex, rtree const& _channelSpatialIndex, 
		std::unordered_map<int, bool>& _meshOutputElements, std::unordered_map<int, bool>& _channelOutputElements):
    meshSpatialIndex(_meshSpatialIndex),channelSpatialIndex(_channelSpatialIndex),
    meshOutputElements(_meshOutputElements), channelOutputElements(_channelOutputElements)
    {
	lexer = new Lexer(file);
	parser = new Parser(*lexer);
	tree = parser->parse();
	//Build the projection objects
	//TODO do we really want to build these here, or build them in the output manager and convert coordinates
	//there???
	pj_latlong = pj_init_plus(LAT_LONG_PROJ4);
	if(pj_latlong == NULL)
	{
	    cout << "Interperter::Interperter failed to initialize pj_latlong: "<< pj_strerrno(*pj_get_errno_ref())<<"\n";
	    exit(-1); //TODO/FIXME clean up lexer/parser/tree on error, use CkExit
	}
	pj_adhydro = pj_init_plus(ADHYDRO_PROJ4);
	if(pj_adhydro == NULL)
	{
	    cout << "Interperter::Interperter failed to initialize pj_adhydro: "<< pj_strerrno(*pj_get_errno_ref())<<"\n";
	    exit(-1); //TODO/FIXME clean up lexer/parser/tree on error, use CkExit
	}
	
    }
    //Interperter(){}    
    ~Interperter()
    {
      delete lexer;
      delete parser;
      delete tree;
      pj_free(pj_latlong);
      pj_free(pj_adhydro);
    }

    void visit(Spec& spec)
    {
        cout << "Interperting a spec\n";
        spec.where->accept(this);
	spec.when->accept(this);
	spec.what->accept(this);
    }
    
    void visit(Where& where)
    {
	cout << "Interperting a where clase\n";
	for(int i = 0; i < where.children.size(); i++)
	{
	    where.children[i]->accept(this);
	}
    }

    void visit(Polygon& polygon)
    {
	int error_code;
	::polygon poly;
	//TODO could make lexer/parser pick up some heavy lifting building a WKT string 
	//or polygon object in the AST itself...
	cout << "Interperting a Polygon; Assuming WSG84 Lat/Long coordinates.\n";
	for(int i = 0; i < polygon.polyPointList.size(); i++)
	{
	    //cout << polygon.polyPointList[i].first<<", "<<polygon.polyPointList[i].second<<"\t";
	    poly.outer().push_back(point(polygon.polyPointList[i].first, polygon.polyPointList[i].second));
	}
	//cout << "\n";

	box bounding_box = bg::return_envelope<box>(poly);
	double x1 = bounding_box.min_corner().get<0>() * DEG_TO_RADIANS;
	double y1 = bounding_box.min_corner().get<1>()  * DEG_TO_RADIANS;
	double x2 = bounding_box.max_corner().get<0>()  * DEG_TO_RADIANS;
	double y2 = bounding_box.max_corner().get<1>()  * DEG_TO_RADIANS;
	error_code = pj_transform(pj_latlong, pj_adhydro, 1, 1, &x1, &y1, NULL);
	if(error_code != 0)
	{
	    cout<<"Interperter::visitPolygon error transforming min coordinates: "<<pj_strerrno(error_code)<<"\n";
	    //FIXME exit???
	}
	error_code = pj_transform(pj_latlong, pj_adhydro, 1, 1, &x2, &y2, NULL);
	if(error_code != 0)
	{
	    cout<<"Interperter::visitPolygon error transforming max coordinates: "<<pj_strerrno(error_code)<<"\n";
	    //FIXME exit???
	}
	cout <<"BOUNDS_P: "<<x1<<","<<y1<<"\t"<<x2<<","<<y2<<"\n";
	bounding_box = box(point(x1, y1), point(x2, y2));
	boundingBoxes.push_back( bounding_box );
	cout<<"MADE BOUNDING BOX FOR POLYGON\n";
	std::vector<value> results_m;
        std::vector<value> results_c;
      
	meshSpatialIndex.query(bgi::intersects(bounding_box), std::back_inserter(results_m));
	channelSpatialIndex.query(bgi::intersects(bounding_box), std::back_inserter(results_c));
	cout <<"RESULTS MESH:\n";
	cout<<"(";
	BOOST_FOREACH(value const& v, results_m)
	{
	  cout<<"'"<<v.second.first<<"',";
	  meshOutputElements[v.second.first] = true;
	}
	cout<<")\n";
	
	cout<<"RESULTS CHANNEL:\n";
	cout<<"(";
	BOOST_FOREACH(value const& v, results_c)
	{
	  cout<<"'"<<v.second.first<<"',";
	  channelOutputElements[v.second.first] = true;
	}
	cout<<")\n";
    }
    
    void visit(PointDistance& pointDistance)
    {
	int error_code;
	cout<<"Interperting a PointDistance; Assuming WSG84 Lat/Long coordinates.\n";
	
	double x1 = pointDistance.point.first * DEG_TO_RADIANS;
	double y1 = pointDistance.point.second * DEG_TO_RADIANS;
	
	error_code = pj_transform(pj_latlong, pj_adhydro, 1, 1, &x1, &y1, NULL);
	if(error_code != 0)
	{
	    cout<<"Interperter::visitPointDistance error transforming point coordinates: "<<pj_strerrno(error_code)<<"\n";
	    //FIXME exit???
	}
	point min = point(x1-pointDistance.distance, y1-pointDistance.distance);
	point max = point(x1+pointDistance.distance, y1+pointDistance.distance);
	boundingBoxes.push_back( box(min, max) );
	
    }
    
    void visit(ChannelDistance& channelDistance)
    {
	cout<<"Channel Distenance unimplemented.\n";
	//Need to build a bounding box around all channel elements,
	//then simply subtrace dist from min_corner and add dist to max_corner
	//to define a new bounding box that covers all elements within dist
	//of the channel elements
	//TODO try using a polygon to define the rectange perpendicular to channel endpoints
	//dist away...

    }
    void visit(When& when)
    {
	cout << "Interperting a when clause\n";
    }
    
    void visit(What& what)
    {
	cout << "Interperting a what clase\n";
    }
    void visit(Compound& compound)
    {
        cout << "Visiting a compound node\n";
        //TODO maybe use iterator???
        for(int i = 0; i < compound.children.size(); i++)
        {
            compound.children[i]->accept(this);
        }
    }
    
    void visit(IDList& idlist)
    {
        cout << "Visiting an ID list with ids: ";
        for(int i = 0; i < idlist.list.size(); i++)
        {
            cout<<idlist.list[i]<<" ";
        }
        cout << "\n";
    }
    
    void visit(MeshOutputVar& output)
    {
        cout << "Visiting a MeshOutputVar: "<<output.value<<"\n";
        output.id_list->accept(this);
    }
    
    void visit(ChannelOutputVar& output)
    {
        cout << "Visiting a ChannelOutputVar: "<<output.value<<"\n";
        output.id_list->accept(this);
    }

    
    void interpert()
    {
        cout << "Interperting "<<typeid(tree).name()<<"\n";
        tree->accept(this);
    }

};

#endif
