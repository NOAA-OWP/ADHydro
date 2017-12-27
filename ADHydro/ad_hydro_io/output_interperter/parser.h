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
    * Version 1 of the grammer
    * spec                  :   REGION compound END
    * compound              :   statement | statement SEMI compound
    * statement             :   mesh_statement | channel_statement | compound
    * mesh_statement        :   MESH id_list mesh_output_var
    * channel_statement     :   CHANNEL id_list channel_output_var
    * id_list               :   (INT_TOKEN)+
    * mesh_output_var       :   surfaceWaterDepth | PrecipitationRate
    * channel_output_var    :   SurfacewaterDepth | PrecipitationRate | channelSurfacewaterChannelNeighborsFlowRate
    * 
    * Version 2 of the grammer:
    * spec			:	REGION WHERE where WHEN when WHAT what END
    * 
    * ***The following grammer elements specify the where of a region***
    * 
    * where			:	where_statement | where_statment SEMI where
    * where_statement		:	polygon | pointDistance | channelDistance | channelConfluence | channelPoint | chanelOrder | where
    * ***Polygon is a geographical region inside a polygon, USING WKT style polygon, with single ring defining exterior, in COUNTER CLOCKWISE direction (right hand rule)***
    * ***All elements within the polygon are subject to output***
    * polygon			:	POLYGON_TOKEN LEFTPAREN polyPointList RIGHTPAREN
    * ***Polygon must consist of at least four points, must be topologically closed so last point must be first point***
    * polyPointList		:	point COMMA point COMMA (point COMMA)+ point
    * ***Point in space as X Y pair
    * point			:	number	number
    * 
    * ***All elements within distance of point are subject to output***
    * pointDistance		:	POINTDIST point distance
    * distance			:	number
    * 
    * ***All elements withing distance of a chanel element are subject to output***
    * channelDistance		:	CHANNELDIST distance
    * 
    * ***All channel elements of order or higher which join are subject to output***
    * channelConfluence		:	CHANNELCONF order
    * order			:	INT_TOKEN
    * 
    * ***Channel element nearest to point is subject to output***
    * channelPoint		:	CHANNELPOINT point
    * ***Channel element with highest stream order within distance of point is subject to output***
    * channelOrder		:	CHANNNELORDER point distance
    * 
    * number			:	INT_TOKEN | DOUBLE_TOKEN
    * ***The following grammer elements specify the when of a region***
    * 
    * when			:	when_statement | when_statement SEMI when 
    * when_statement		:	timePeriod | conditional
    * timePeriod		:	startDate endDate interval
    * startDate			:	julianDate
    * endDate			:	julianDate
    * interval			:	INT_TOKEN temporalResolution
    * julianDate		:	FLOAT
    * temporalResolution	:	M | H | D
    * conditional		:	variable conditional_op variable TODO finish implementing condtional
    * 
    * ***The following grammer elements specify the what of a region***
    * TODO grammer for aggregation and binops on output
    * what			:	what_statement | what_statement SEMI what
    * what_statement		:	output_var
    * 
    * NOTE this is a first attempt at parsing an output spec.  This grammer will get more complicated,
    * and the possible output vars should include all mesh/channel state including neighbor flows!
    *
    *
    */
public:
    /*
    * Constructor that takes a lexer instance to provide tokens to the parser.
    */
    Parser(Lexer& _lexer);
    
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
        
    /*
     * Function for parsing X based on grammer rule
     */
    
    //Helper function for parsing number into a double without enforcing . in the input
    double getNumberAsDouble(std::string errorPrefix = "");
    /*
     * Functions for parsing the where clause of a region
     */
    Where* where();
    Polygon* polygon();
    std::vector<std::pair<double, double> > polyPointList();
    PointDistance* pointDistance(); 
    ChannelDistance* channelDistance();
    ChannelConfluence* channelConfluence(); 
    ChannelPoint* channelPoint();
    ChannelOrder* channelOrder();
    AbstractSyntaxTree* where_statement();
    
    /*
     * Functions for parsing the when clase of a region
     */
    When* when();
    AbstractSyntaxTree* when_statement();
    TimePeriod* timePeriod();
    
    /*
     * Functions for parsing the what clase of a region
     */
    What* what();
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
    Spec* parse();
    
private:
    Lexer& lexer;
    Token* current_token;
    //Spec* root;
};

#endif
