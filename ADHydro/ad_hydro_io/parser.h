#ifndef IO_PARSER_H
#define IO_PARSER_H

#include "lexer.h"
#include <vector>

class AbstractSyntaxTree
{
    /*
     * This is the base class for the abstract syntax tree used by the parser/interperter
     * Each node in the tree will be a sublcass of this base.
     */
public:
    virtual void getNodeType(){}
};

class IDList : public AbstractSyntaxTree
{
    /*
     * An id_list node consists of a list of ids
     */
public:
    IDList(std::vector<int> _list):list(_list){};

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
    MeshOutputVar(string name):value(name){}
    
private:
    string value;
};

class ChannelOutputVar : public AbstractSyntaxTree
{
    /*
     * channel_output_var      :   surfaceWaterDepth | PrecipitationRate
     * A channel_output_var node consists of the name of the ouput variable
     */
public:
    ChannelOutputVar(string name):value(name){}
    
private:
    string value;
};

class Spec : public AbstractSyntaxTree
{
    /*
     * spec                 :   MESH id_list mesh_output_var | CHANNEL id_list channel_output_var
     * A spec node consists of an id_list node and mesh_outpt_var node
     */
public:
    Spec(IDList id_list, MeshOutputVar outputVar):left(id_list), right(outputVar){}
    Spec(IDList id_list, ChannelOutputVar outputVar):left(id_list), right(outputVar){}
    
    AbstractSyntaxTree left;
    AbstractSyntaxTree right;
private:

};

class NodeVisitor
{
    /*
     * Interface for implementing visitor pattern for each node type of the AST
     */
public:
    virtual ~NodeVisitor(){}
    virtual void visit(Spec& spec){}
    virtual void visit(IDList& idlist){}
    virtual void visit(MeshOutputVar& output){}
    virtual void visit(ChannelOutputVar& output){}
};

class Parser
{
    /*
    * This is a parser for the custom grammer for ADHydro IO specification.
    * The grammer is presented here as a context free grammer in BNF form.
    * 
    * spec                 :   MESH id_list mesh_output_var | CHANNEL id_list channel_output_var
    * id_list              :   (INT)+
    * mesh_output_var      :   surfaceWaterDepth | PrecipitationRate
    * channel_output_var   :   SurfacewaterDepth | PrecipitationRate
    * 
    * 
    * 
    * NOTE this is a first attempt at parsing an output spec.  This grammer will get more complicated,
    * and the possible output vars should include all mesh/channel state including neighbor flows!
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
    ~Parser(){delete current_token;}
    
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
    Spec spec()
    {
        //spec : MESH id_list mesh_output_var | CHANNEL id_list channel_output_var

        string outputVar;
        
        if(current_token->getType() == MESH)
        {
            //Consume the mesh token
            eat(MESH);
            IDList ids = id_list();
            MeshOutputVar output = mesh_output_var();
            return Spec(ids, output);
        }
        else
        {
            eat(CHANNEL);
            IDList ids = id_list();
            ChannelOutputVar output = channel_output_var();
            return Spec(ids, output);
        }
    }
    
    /*
    * Function for parsing an id_list based on grammer rule
    */
    IDList id_list()
    {
        std::vector<int> ids;
        while(current_token->getType() == INT)
        {
            //Since we know we are dealing with an INT token, cast current_token
            //to the appropriate int template to get the value
            ids.push_back(static_cast<TokenTemplate<int>*>(current_token)->value);
            eat(INT);
        }
        return IDList(ids);
    }
    
    /*
     * Function for parsing a mesh_output_var based on grammer rule
     */
    MeshOutputVar mesh_output_var()
    {
        //Since we know we are dealing with an string type token, cast current_token
        //to the appropriate string template to get the value
        if(current_token->getType() == ID)
        {
            string tmp = static_cast<TokenTemplate<string>*>(current_token)->value;
            eat(ID);
            return MeshOutputVar( tmp );
        }
        else
        {
            error("Expected ID token, but got "+current_token->getType());
        }
    }
    
    /*
     * Function for parsing a channel_output_var based on grammer rule
     */
    ChannelOutputVar channel_output_var()
    {
        string tmp = static_cast<TokenTemplate<string>*>(current_token)->value;
        eat(ID);
        return ChannelOutputVar( tmp );
    }
    
    Spec parse()
    {
        Spec root = spec();
        if(current_token->getType() != EOI)
            error("Extra input at end of spec");
        else cout<<*current_token; //FIXME remove this print, only useful for debugging
    }
    
private:
    Lexer lexer;
    Token *current_token;
};

#endif
