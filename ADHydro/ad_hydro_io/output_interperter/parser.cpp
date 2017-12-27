//Implementation details for AST parser

#include "parser.h"
#include <limits>

/*
 * Ctor initilizes the lexer and sets the current token to parse
 */
Parser::Parser(Lexer& _lexer):lexer(_lexer)
{
    //when a parser is created, load up the first token from the lexer
    current_token = lexer.get_next_token();
}

/*
 * Dtor cleans up allocated memory
 */
Parser::~Parser()
{
    delete current_token; 
    //delete root;
}

/*
 * Error handling routine that takes a string error message,
 * prints the error, frees memory for the current token, and exits the program.
 * 
 * TODO/FIXME make error handling more robust
 */
void Parser::error(string error)
{
    //Print the syntax errer reported in string error
    cout<<"Syntax error: "<<error<<"\n";
    delete current_token;
    exit(1);
}

/*
 * Eats the current token, verifying that it is the expected token and calling error if it is not
 */
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
        error("Expected to see token type "+token_type+" but saw "+current_token->getType());
    }
}

/*
 * Helper function to parse any number into an explicit double type
 */
double Parser::getNumberAsDouble(std::string errorPrefix)
{
  double num;
  if(current_token->getType() == INT_TOKEN)
  {
    num = static_cast<TokenTemplate<int>*>(current_token)->value;
    eat(INT_TOKEN);
  }
  else if(current_token->getType() == DOUBLE_TOKEN)
  {
    num = static_cast<TokenTemplate<double>*>(current_token)->value;
    eat(DOUBLE_TOKEN);
  }
  else
  {
    num = -std::numeric_limits<double>::infinity(); //Set to a value that is trappable if we decide not to error later
    error(errorPrefix+" expected numeric token INT or DOUBLE, but saw "+current_token->getType()+" instead\n");
  }
  return num;
}

/*
 * Parses a list of polygon points
 */
std::vector<std::pair<double, double> > Parser::polyPointList()
{
  std::vector<std::pair<double, double> > points; //TODO/FIXME use boost::geometry points??? Or maybe boost::geometry polygon
  double x;
  double y;
  std::string errorPrefix = "Polygon list";
  while(current_token->getType() == INT_TOKEN || current_token->getType() == DOUBLE_TOKEN)
  {
      
      x=getNumberAsDouble(errorPrefix);
      y=getNumberAsDouble(errorPrefix);
      points.push_back(std::make_pair(x, y));
      if(current_token->getType() != RIGHTPAREN) //This indicates the end of the list of polygon points
	eat(COMMA);
  }
  return points;
}

/*
 * Parses a polygon node
 */
Polygon* Parser::polygon()
{
  eat(POLYGON_TOKEN);
  eat(LEFTPAREN);
  Polygon* poly = new Polygon(polyPointList());
  eat(RIGHTPAREN);
  return poly;
}

/*
 * Parses a pointDistance node
 */
PointDistance* Parser::pointDistance()
{
  eat(POINTDIST);
  double x = getNumberAsDouble();
  double y = getNumberAsDouble();
  double dist = getNumberAsDouble();
  PointDistance* pointDist = new PointDistance(std::make_pair(x,y), dist);
  return pointDist;
}

/*
 * Parses a channelDistance node
 */
ChannelDistance* Parser::channelDistance()
{
  eat(CHANNELDIST);
  double dist = getNumberAsDouble();
  ChannelDistance* channelDist = new ChannelDistance(dist);
  return channelDist;
}

/*
 * Parses a channelConfluence node
 */
ChannelConfluence* Parser::channelConfluence()
{
  eat(CHANNELCONF);
  if(current_token->getType() == INT_TOKEN)
  {
    return new ChannelConfluence(static_cast<TokenTemplate<int>*>(current_token)->value);
  }
  else
  {
      error("CHANNELCONF order must be an integer.");
  }
}

/*
 * Parses a channelPoint node
 */
ChannelPoint* Parser::channelPoint()
{
  eat(CHANNELPOINT);
  double x = getNumberAsDouble();
  double y = getNumberAsDouble();
  ChannelPoint* channelPoint = new ChannelPoint(std::make_pair(x,y));
  return channelPoint;
}

/*
 * Parses a channelOrder node
 */
ChannelOrder* Parser::channelOrder()
{
  eat(CHANNELORDER);
  double x = getNumberAsDouble();
  double y = getNumberAsDouble();
  double dist = getNumberAsDouble();
  ChannelOrder* channelOrder = new ChannelOrder(std::make_pair(x,y), dist);
  return channelOrder;
}

/*
 * Parses the various types of where_statement
 */
AbstractSyntaxTree* Parser::where_statement()
{
    if(current_token->getType() == POLYGON_TOKEN)
    {
	return polygon();
    }
    if(current_token->getType() == POINTDIST)
    {
	return pointDistance();
    }
    if(current_token->getType() == CHANNELDIST)
    {
	return channelDistance();
    }
    if(current_token->getType() == CHANNELCONF)
    {
	return channelConfluence();
    }
    if(current_token->getType() == CHANNELPOINT)
    {
	return channelPoint();
    }
    if(current_token->getType() == CHANNELORDER)
    {
	return channelOrder();
    }
    
    error("Unknown WHERE clause: "+current_token->getType());
}

/*
 * Parses a where node
 */
Where* Parser::where()
{
    //Shoudl have at least one where statement here, then possibly more
  Where* result = new Where();
  AbstractSyntaxTree* s = where_statement();
  result->children.push_back(s);
  while(current_token->getType() == SEMI)
  {
      eat(SEMI);
      result->children.push_back(where_statement());
  }
  return result;
}

/*
 * Parses a time period as a start, stop, and interval double folowed by a resolution string
 */
TimePeriod* Parser::timePeriod()
{
  double start = getNumberAsDouble();
  double stop = getNumberAsDouble();
  double interval = getNumberAsDouble();
  string resolution;
  if(current_token->getType() == ID)
  {
      resolution = static_cast<TokenTemplate<string>*>(current_token)->value;
      eat(ID);
  }
  return new TimePeriod(start, stop, interval, resolution);
}

/*
 * Parses a when statement based on grammer rule
 */
AbstractSyntaxTree* Parser::when_statement()
{
  if(current_token->getType() == CONDITIONAL)
  {
      eat(CONDITIONAL);
      //TODO/FIXME finish implementing condional when
      return new Conditional();
  }
  else
  {
      //Must be time interval, parse start, end, interval resolution
      return timePeriod();
  }
}

/*
 * Parses a when node
 */
When* Parser::when()
{
  When* result = new When();
  AbstractSyntaxTree* s = when_statement();
  result->children.push_back(s);
  while(current_token->getType() == SEMI)
  {
      eat(SEMI);
      result->children.push_back(when_statement());
  }
  return result;
}

/*
 * Parses a what node
 */
What* Parser::what()
{
  return  new What();
}

/*
 * Parses the spec by parsing the three children nodes, Where, When, and What
 */
Spec* Parser::spec()
{
    //spec : REGION WHERE where WHEN when WHAT what END
    eat(REGION);
    eat(WHERE);
    //Could add some control structure after each clause if we want, such as eat(COLON);
    Where* output_where = where();
    eat(WHEN);
    When* output_when = when();
    eat(WHAT);
    What* output_what = what();
    Spec* spec = new Spec(output_where, output_when, output_what);
    eat(END);
    return spec;
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
    while(current_token->getType() == INT_TOKEN)
    {
        //Since we know we are dealing with an INT_TOKEN token, cast current_token
        //to the appropriate int template to get the value
        ids.push_back(static_cast<TokenTemplate<int>*>(current_token)->value);
        eat(INT_TOKEN);
    }
    return new IDList(ids);
}

MeshOutputVar* Parser::mesh_output_var(IDList* ids)
{
    //Since we know we are dealing with an string type token, cast current_token
    //to the appropriate string template to get the value
    if(current_token->getType() == ID)
    {
        //Should check that ID is an appropriate variable identifier at this point, and
        //raise a syntax error otherwise. TODO/FIXME
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
    if(current_token->getType() == ID)
    {
        //Should check that ID is an appropriate variable identifier at this point, and
        //raise a syntax error otherwise. TODO/FIXME
        string tmp = static_cast<TokenTemplate<string>*>(current_token)->value;
        eat(ID);
        return new ChannelOutputVar( tmp, ids );
    }
    else
    {
        error("Expected ID token, but got "+current_token->getType());
    }
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

Spec* Parser::parse()
{
    //Start by parsing the root of the tree, a Spec node
    Spec* root = spec();
    //When we are done, we should be at the End Of Input (End Of File)
    if(current_token->getType() != EOI)
        error("Extra input at end of spec");
    else cout<<*current_token; //FIXME remove this print, only useful for debugging
    return root;
}








/*
 *  New parsing function for version 2 of the grammer
 */

