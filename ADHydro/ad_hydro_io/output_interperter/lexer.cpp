#include "lexer.h"

Lexer::Lexer(std::string fileName)
{
    input.open(fileName.c_str());
    if(!input.is_open())
    {
      cout << "Error opening output_spec\n";
      //TODO handle this error appropriately
      //return -1;
    }
    //Upon initialization, advance the lexer to the first character
    advance();
}

Lexer::~Lexer()
{
  input.close();
}
void Lexer::advance()
{
    input.get(current_char);
}

void Lexer::skip_whitespace()
{
    //Have to check for EOF here, or we will loop infinitely on isspace, since appearently EOF is a space
    while(isspace(current_char) && !input.eof())
    {
        advance();
    }
}

Token* Lexer::_id()
{
        string result;
        while(!input.eof() && isalpha(current_char))
        {
            result += current_char;
            advance();
        }
        //Check for keywords
        //TODO/FIXME make keyword map???
        //TODO/FIXME make case insensitive
        if(result == "mesh")
            return new TokenTemplate<string>(MESH, result);
        if(result == "channel")
            return new TokenTemplate<string>(CHANNEL, result);
        if(result == "region")
            return new TokenTemplate<string>(REGION, result);
        if(result == "end")
            return new TokenTemplate<string>(END, result);
	if(result == "where")
	    return new TokenTemplate<string>(WHERE, result);
	if(result == "when")
	    return new TokenTemplate<string>(WHEN, result);
	if(result == "what")
	    return new TokenTemplate<string>(WHAT, result);
	if(result == "POLYGON")
	    return new TokenTemplate<string>(POLYGON_TOKEN, result);
	if(result == "POINTDIST")
	    return new TokenTemplate<string>(POINTDIST, result);
	if(result == "CHANNELDIST")
	    return new TokenTemplate<string>(CHANNELDIST, result);
	if(result == "CHANNELCONF")
	    return new TokenTemplate<string>(CHANNELCONF, result);
	if(result == "CHANNELPOINT")
	    return new TokenTemplate<string>(CHANNELPOINT, result);
	if(result == "CHANNELORDER")
	    return new TokenTemplate<string>(CHANNELORDER, result);
        if(result == "if")
	    return new TokenTemplate<string>(CONDITIONAL, result);
        return new TokenTemplate<string>(ID, result);
}

Token* Lexer::number()
{
    string result;
    if(current_char == '-')
    {
	result += current_char;
	advance();
    }
    while(!input.eof() && isdigit(current_char))
    {
        result += current_char;
        advance();
    }
    if(current_char == '.')
    {
        result += current_char;
        advance();
        
        while(!input.eof() && isdigit(current_char))
        {
            result += current_char;
            advance();
        }
        return new TokenTemplate<double>(DOUBLE_TOKEN, atof(result.c_str()));
    }
    else
    {
        return new TokenTemplate<int>(INT_TOKEN, atoi(result.c_str()));   
    }
}

Token* Lexer::get_next_token()
{
    string result;
    while(!input.eof())
    {
            skip_whitespace();
            if(isalpha(current_char))
            {
                return _id();
            }
            if(isdigit(current_char) || current_char=='-') //FIXME QUICK HACK FOR NEGATIVES
            {
                return number();   
            }
            result = current_char;
	    advance();
            if(result == ";")
		return new TokenTemplate<string>(SEMI, result);
	    if(result == "(")
		return new TokenTemplate<string>(LEFTPAREN, result);
	    if(result == ")")
		return new TokenTemplate<string>(RIGHTPAREN, result);
	    if(result == ",")
		return new TokenTemplate<string>(COMMA, result);
    }
    
    return new TokenTemplate<string>(EOI, "EOF");
}
