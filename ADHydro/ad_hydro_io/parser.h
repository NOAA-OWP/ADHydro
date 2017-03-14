#ifndef IO_PARSER_H
#define IO_PARSER_H

#include "lexer.h"
#include <vector>
#include <typeinfo> //For debugging only

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
    virtual ~AbstractSyntaxTree(){}
    virtual void accept(NodeVisitor* v){cout << "UNKNON ACCEPTANCE!!! "<<typeid(v).name()<<"\n";}

};

class IDList : public AbstractSyntaxTree
{
    /*
     * An id_list node consists of a list of ids
     */
public:
    IDList(std::vector<int> _list);
    void accept(NodeVisitor* v);

private:
    std::vector<int> list;
};

class MeshOutputVar : public AbstractSyntaxTree
{
    /*
     * mesh_output_var      :   surfaceWaterDepth | PrecipitationRate
     * A mesh_output_var node consists of the name of the ouput variable
     */
public:
    MeshOutputVar(string name);
    void accept(NodeVisitor* v);
    string value;
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
    ChannelOutputVar(string name);
    void accept(NodeVisitor* v);
    string value;
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

class Parser
{
    /*
    * 
    * This is a parser for the custom grammer for ADHydro IO specification.
    * The grammer is presented here as a context free grammer in BNF form.
    * 
    * spec                  :   REGION compound END
    * compound              :   statement | statement SEMI compound
    * statement             :   mesh_statement | channel_statement | compound
    * mesh_statement        :   MESH id_list mesh_output_var
    * channel_statement     :   CHANNEL id_list channel_output_var
    * id_list               :   (INT)+
    * mesh_output_var       :   surfaceWaterDepth | PrecipitationRate
    * channel_output_var    :   SurfacewaterDepth | PrecipitationRate
    * 
    * 
    * 
    * NOTE this is a first attempt at parsing an output spec.  This grammer will get more complicated,
    * and the possible output vars should include all mesh/channel state including neighbor flows!
    *
    * I don't think we can implement this as a pure binary tree...At least not at the top level.
    * Need to decide how to deal with a spec that includes both mesh and channel elements.  
    * To proceed, we probably need to establish the grammer in a form that should be, for now,
    * complete to the current desired results of the ADHydro output module.  Then we can translate
    * this appropriately to the nessicary AST node types and write the parser code.  Then the 
    * interperter visitor can be extended to create the appropriate structures/interfaces with the
    * ADHydro module to make the output work as desired.
    *
    */
public:
    /*
    * Constructor that takes a lexer instance to provide tokens to the parser.
    */
    Parser(Lexer _lexer):lexer(_lexer)
    {
        current_token = lexer.get_next_token();
    }
    
    /*
     * Destructor, must ensure current_token is properly cleaned up.
     */
    ~Parser(){delete current_token; delete root;}
    
    /*
    * Error function for reporting parsing errors
    */
    void error(string error){cout<<"Syntax error: "<<error<<"\n";}
    
    /*
    * Funtion for eating the current token, calls error if token type is 
    * not the expected type.
    */
    void eat(string token_type)
    {
        //FIXME remove this print, only useful for debugging
        cout << *current_token;
        if(current_token->getType() == token_type)
        {
            current_token = lexer.get_next_token();
        }
        else
        {
            error("Expected to see token type "+token_type+"but saw "+current_token->getType());
        }
    }
    
    
    /*
    * Function for parsing a spec based on grammer rule
    */
    Spec* spec()
    {
        //spec : REGION compound END
        eat(REGION);
        return new Spec(compound());
        eat(END);
        /*
        string outputVar;
        if(current_token->getType() == MESH)
        {
            //Consume the mesh token
            eat(MESH);
            IDList* ids = id_list();
            MeshOutputVar* output = mesh_output_var();
            return new Spec(ids, output);
        }
        else
        {
            eat(CHANNEL);
            IDList* ids = id_list();
            ChannelOutputVar* output = channel_output_var();
            return new Spec(ids, output);
        }
        */
    }
    //TODO/FIXME Circular dependencies in the recursion here require a declaration/definition
    //Decoupling.  Need to implement all these functions purely in the .cpp and let the linker
    //work its magic!!!
    /*
     * Function for parsing compound region statements based on grammer rule
     */
    Compound* compound()
    {
        //Should have at least one statement here, then possibly more
        Compound *result = new Compound();
        result->children.push_back(statement());
        while(current_token->getType() == SEMI)
        {
            result->children.push_back(statement());
        }
    }
    /*
    * Function for parsing an id_list based on grammer rule
    */
    IDList* id_list()
    {
        std::vector<int> ids;
        while(current_token->getType() == INT)
        {
            //Since we know we are dealing with an INT token, cast current_token
            //to the appropriate int template to get the value
            ids.push_back(static_cast<TokenTemplate<int>*>(current_token)->value);
            eat(INT);
        }
        return new IDList(ids);
    }
    
    /*
     * Function for parsing a mesh_output_var based on grammer rule
     */
    MeshOutputVar* mesh_output_var()
    {
        //Since we know we are dealing with an string type token, cast current_token
        //to the appropriate string template to get the value
        if(current_token->getType() == ID)
        {
            string tmp = static_cast<TokenTemplate<string>*>(current_token)->value;
            eat(ID);
            return new MeshOutputVar( tmp );
        }
        else
        {
            error("Expected ID token, but got "+current_token->getType());
        }
    }
    
    /*
     * Function for parsing a channel_output_var based on grammer rule
     */
    ChannelOutputVar* channel_output_var()
    {
        string tmp = static_cast<TokenTemplate<string>*>(current_token)->value;
        eat(ID);
        return new ChannelOutputVar( tmp );
    }
    
    /*
     * function for parsing individual statements based on grammer rule
     */
    
    AbstractSyntaxTree* statement()
    {
        if(current_token->getType() == REGION)
            return new Compound();
        if(current_token->getType() == MESH)
            return mesh_output_var();
        if(current_token->getType() == CHANNEL)
            return channel_output_var();
        //TODO/FIXME do we want an empty statement to return???
        else error("UNKNOWN STATEMENT for "+current_token->getType());
    }
    Spec& parse()
    {
        root = spec();
        if(current_token->getType() != EOI)
            error("Extra input at end of spec");
        else cout<<*current_token; //FIXME remove this print, only useful for debugging
        return *root;
    }
    
private:
    Lexer lexer;
    Token *current_token;
    Spec* root;
};

#endif
