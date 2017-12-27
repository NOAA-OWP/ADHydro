#ifndef IO_AST_H
#define IO_AST_H
#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include <typeinfo> //For debugging only
using std::string;
using std::cout;
//forward declaration of NodeVisitor
class NodeVisitor;
//Forward declaration of AST nodes
class Compound;

class AbstractSyntaxTree
{
    /*
     * This is the base class for the abstract syntax tree used by the parser/interperter
     * Each node in the tree will be a sublcass of this base.
     */
public:
    virtual ~AbstractSyntaxTree(){};
    virtual void accept(NodeVisitor* v);

};

class IDList : public AbstractSyntaxTree
{
    /*
     * An id_list node consists of a list of ids and a child outputvar node
     */
public:
    IDList(std::vector<int> _list);
    void accept(NodeVisitor* v);
    std::vector<int> list;

private:
    
};

class MeshOutputVar : public AbstractSyntaxTree
{
    /*
     * mesh_output_var      :   surfaceWaterDepth | PrecipitationRate
     * A mesh_output_var node consists of the name of the ouput variable
     */
public:
    MeshOutputVar(string name, IDList* _id_list);
    ~MeshOutputVar();
    void accept(NodeVisitor* v);
    string value;
    IDList* id_list;
private:
    //string value;
};

class ChannelOutputVar : public AbstractSyntaxTree
{
    /*
     * channel_output_var      :   surfaceWaterDepth | PrecipitationRate
     * A channel_output_var node consists of the name of the ouput variable
     */
public:
    ChannelOutputVar(string name, IDList* _id_list);
    ~ChannelOutputVar();
    void accept(NodeVisitor* v);
    string value;
    IDList* id_list;
private:

};

class Compound: public AbstractSyntaxTree
{
    /*
     * compound : (statement)+
     * A compound node consists of a list of children statements
     */
public:
    Compound();
    ~Compound();
    void accept(NodeVisitor* v);
    //TODO clean up children pointers in dtor
    std::vector<AbstractSyntaxTree*> children;
};


class Polygon: public AbstractSyntaxTree
{
    /*
    * polygon			:	POLYGON_TOKEN LEFTPAREN polyPointList RIGHTPAREN
    * polyPointList		:	point COMMA point COMMA (point COMMA)+ point
    * point			:	NUMBER	NUMBER
    * A polygon node contains a polyPointList of all points defining the outer ring of the polygon
    * 
    * There is no need to implement the entire list in the AST, so this class combines the terminal nodes of
    * a Polygon using a simple vector<pair>.
    * 
    * Somewhere a simple validity check of number of points >= 4 and firstPoint == lastPoint needs to be performed TODO/FIXME
    */
  public:
    Polygon(std::vector<std::pair<double, double> > _polyPointList);
    ~Polygon();
    void accept(NodeVisitor* v);
    std::vector<std::pair<double, double> > polyPointList;
};

class PointDistance: public AbstractSyntaxTree
{
    /*
     * pointDistance		:	POINTDIST point distance
     * point			:	NUMBER NUMBER
     * distance			:	NUMBER
     * 
     * This node contains a point and a distance.  All elements within distance of point
     * should be included in the output.  The visitor of the interperter is likely the appropriate place to define
     * that list of elements, but maybe not.  Will continue without them for now and revisit this idea.
     * TODO/FIXME!
     */
  public:
    PointDistance(std::pair<double, double> _point, double _distance);
    ~PointDistance();
    void accept(NodeVisitor* v);
    std::pair<double, double> point;
    double distance;
};

class ChannelDistance: public AbstractSyntaxTree
{
    /*
     * channelDistance		:	CHANNELDIST distance
     * This node simply contains the distance form channelel elements that output is valid for
     */
  public:
    ChannelDistance(double _distance);
    ~ChannelDistance();
    void accept(NodeVisitor* v);
    double distance;
};

class ChannelConfluence: public AbstractSyntaxTree
{
    /*
     * channelConfluence		:	CHANNELCONF order
     * order			:	INT_TOKEN
     */
  public:
    ChannelConfluence(int _order);
    ~ChannelConfluence();
    void accept(NodeVisitor* v);
    int order;
};

class ChannelPoint: public AbstractSyntaxTree
{
    /*
     * channelPoint		:	CHANNELPOINT point
     * A single channel element at point
     */
  public:
    ChannelPoint(std::pair<double, double> _point);
    ~ChannelPoint();
    void accept(NodeVisitor* v);
    std::pair<double, double> point;
};

class ChannelOrder: public AbstractSyntaxTree
{
    /*
     * channelOrder		:	CHANNNELORDER point distance
     * element with highest order within distance of point needs to be found
     */
  public:
    ChannelOrder(std::pair<double, double> _point, double _distance);
    ~ChannelOrder();
    void accept(NodeVisitor* v);
    std::pair<double, double> point;
    double distance;
};

class Where: public AbstractSyntaxTree
{
    /*
     * where			:	where_statement | where_statment SEMI where
     * where_statement		:	polygon | pointDistance | channelDistance | channelConfluence | channelPoint | chanelOrder | where
     * A where node is a list of one or more where_statements
     */
  public:
    Where();
    ~Where();
    void accept(NodeVisitor* v);
    std::vector<AbstractSyntaxTree*> children;
};

class TimePeriod: public AbstractSyntaxTree
{
    /*
     * timePeriod		:	startDate endDate interval
     * startDate		:	julianDate
     * endDate			:	julianDate
     * interval			:	INT_TOKEN temporalResolution
     * julianDate		:	FLOAT
     * temporalResolution	:	M | H | D
     */
  public:
    TimePeriod(double _start, double _stop, double _interval, string _resolution);
    ~TimePeriod();
    void accept(NodeVisitor* v);
    double start;
    double stop;
    double interval;
    string resolution;
};

class Conditional: public AbstractSyntaxTree
{
    //TODO implement me!!!
};

class When: public AbstractSyntaxTree
{
    /*
     * when			:	when_statement | when_statment SEMI when
     * when_statement		:	timePeriod | conditional
     * A where node is a list of one or more when statements
     */
  public:
    When();
    ~When();
    void accept(NodeVisitor* v);
    std::vector<AbstractSyntaxTree*> children;
};

class What: public AbstractSyntaxTree
{
    /*
     * what			:	what_statement | what_statment SEMI what
     * A where node is a list of one or more when statements
     */
  public:
    What();
    ~What();
    void accept(NodeVisitor* v);
    std::vector<AbstractSyntaxTree*> children;
};

class Spec : public AbstractSyntaxTree
{
    /*
     * spec			:	REGION where when what END
     * A spec node consists of three children nodes, where, when and what
     */
public:
    Spec(Where* _where, When* _when, What* what);
    ~Spec();
    void accept(NodeVisitor* v);
    AbstractSyntaxTree* where;
    AbstractSyntaxTree* when;
    AbstractSyntaxTree* what;
private:

};

/*
 * Following are nodes for version 2 of the grammer laid out in parser.h
 */

class NodeVisitor
{
    /*
     * Interface for implementing visitor pattern for each node type of the AST
     */
public:
    virtual ~NodeVisitor(){}
    
    virtual void visit(Spec& spec){};
    virtual void visit(Compound& compound){};
    virtual void visit(IDList& idlist){};
    virtual void visit(MeshOutputVar& output){};
    virtual void visit(ChannelOutputVar& output){};
    virtual void visit(Polygon& polygon){};
    virtual void visit(PointDistance& pointDistance){};
    virtual void visit(ChannelDistance& channelDistance){};
    virtual void visit(ChannelConfluence& channelConfluence){};
    virtual void visit(ChannelPoint& channelPoint){};
    virtual void visit(ChannelOrder& channelOrder){};
    
    virtual void visit(Where& where){};
    virtual void visit(When& when){};
    virtual void visit(What& what){};
    
    virtual void visit(TimePeriod& timePeriod){};
};


#endif
