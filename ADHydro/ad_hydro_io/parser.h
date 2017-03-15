#ifndef IO_PARSER_H
#define IO_PARSER_H

#include "lexer.h"
#include "ast.h"
#include <typeinfo> //For debugging only


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
    Parser(Lexer _lexer);
    
    /*
     * Destructor, must ensure current_token is properly cleaned up.
     */
    ~Parser();
    
    /*
    * Error function for reporting parsing errors
    */
    void error(string error);
    
    /*
    * Funtion for eating the current token, calls error if token type is 
    * not the expected type.
    */
    void eat(string token_type);
    
    
    /*
    * Function for parsing a spec based on grammer rule
    */
    Spec* spec();
    
    //TODO/FIXME Circular dependencies in the recursion here require a declaration/definition
    //Decoupling.  Need to implement all these functions purely in the .cpp and let the linker
    //work its magic!!!
    /*
     * Function for parsing compound region statements based on grammer rule
     */
    Compound* compound();
    
    /*
    * Function for parsing an id_list based on grammer rule
    */
    IDList* id_list();
    
    /*
     * Function for parsing a mesh_output_var based on grammer rule
     */
    MeshOutputVar* mesh_output_var(IDList*);
    
    /*
     * Function for parsing a channel_output_var based on grammer rule
     */
    ChannelOutputVar* channel_output_var(IDList*);
    
    /*
     * function for parsing individual statements based on grammer rule
     */
    AbstractSyntaxTree* statement();
    
    /*
     * Function for parsing a mesh_statement based on grammer rule
     */
    MeshOutputVar* mesh_statement();
    
    /*
     * Function for parsing a channel_statement based on grammer rule
     */
    ChannelOutputVar* channel_statement();
    
    /*
     * Function for parsing input from constructed parser
     */
    Spec& parse();
    
private:
    Lexer lexer;
    Token* current_token;
    Spec* root;
};

#endif
