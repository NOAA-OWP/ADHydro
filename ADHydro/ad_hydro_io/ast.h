#ifndef IO_AST_H
#define IO_AST_H
#include <vector>
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

class Spec : public AbstractSyntaxTree
{
    /*
     * spec                 :   MESH id_list mesh_output_var | CHANNEL id_list channel_output_var
     * A spec node consists of an id_list node and mesh_outpt_var node
     */
public:
    Spec(Compound* compound);
    Spec();
    ~Spec();
    void accept(NodeVisitor* v);
    AbstractSyntaxTree* child;
private:

};

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
};


#endif
