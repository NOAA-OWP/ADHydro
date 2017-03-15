//Implementation details for AST parser

#include "parser.h"

Parser::Parser(Lexer _lexer):lexer(_lexer)
{
    //when a parser is created, load up the first token from the lexer
    current_token = lexer.get_next_token();
}

Parser::~Parser()
{
    delete current_token; 
    delete root;
}

void Parser::error(string error)
{
    //Print the syntax errer reported in string error
    cout<<"Syntax error: "<<error<<"\n";
}

void Parser::eat(string token_type)
{
    //FIXME remove this print, only useful for debugging
    cout << *current_token;
    if(current_token->getType() == token_type)
    {
        //Manage memory appropriately, delete previously allocated token
        delete current_token;
        current_token = lexer.get_next_token();
    }
    else
    {
        error("Expected to see token type "+token_type+"but saw "+current_token->getType());
    }
}

Spec* Parser::spec()
{
    //spec : REGION compound END
    eat(REGION);
    Spec* spec = new Spec(compound());
    eat(END);
    return spec;
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

Compound* Parser::compound()
{
    //Should have at least one statement here, then possibly more
    Compound *result = new Compound();
    AbstractSyntaxTree* s = statement();
    result->children.push_back(s);
    while(current_token->getType() == SEMI)
    {
        eat(SEMI);
        result->children.push_back(statement());
    }
    return result;
}

IDList* Parser::id_list()
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

MeshOutputVar* Parser::mesh_output_var(IDList* ids)
{
    //Since we know we are dealing with an string type token, cast current_token
    //to the appropriate string template to get the value
    if(current_token->getType() == ID)
    {
        string tmp = static_cast<TokenTemplate<string>*>(current_token)->value;
        eat(ID);
        return new MeshOutputVar( tmp, ids );
    }
    else
    {
        error("Expected ID token, but got "+current_token->getType());
    }
}

ChannelOutputVar* Parser::channel_output_var(IDList* ids)
{
    string tmp = static_cast<TokenTemplate<string>*>(current_token)->value;
    eat(ID);
    return new ChannelOutputVar( tmp, ids );
}

AbstractSyntaxTree* Parser::statement()
{
    //TODO This isn't quite right...need to deal with compound statements differently
    if(current_token->getType() == REGION)
    {
        return spec();
    }
    if(current_token->getType() == MESH)
    {
        return mesh_statement();
    }
    if(current_token->getType() == CHANNEL)
    {
        return channel_statement();
    }
    //TODO/FIXME do we want an empty statement to return???
    else error("UNKNOWN STATEMENT for "+current_token->getType());
}

MeshOutputVar* Parser::mesh_statement()
{
    eat(MESH);
    //Now we get the id_list
    IDList* ids = id_list();
    //Then we get the output var/s
    return mesh_output_var(ids);
}

ChannelOutputVar* Parser::channel_statement()
{
    eat(CHANNEL);
    //Now we get the id_list
    IDList* ids = id_list();
    //Then we get the output var/s
    return channel_output_var(ids);
}

Spec& Parser::parse()
{
    root = spec();
    if(current_token->getType() != EOI)
        error("Extra input at end of spec");
    else cout<<*current_token; //FIXME remove this print, only useful for debugging
    return *root;
}
