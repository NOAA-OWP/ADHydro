#include "lexer.h"

Lexer::Lexer(ifstream& input):input(input)
{
    //Upon initialization, advance the lexer to the first character
    advance();
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
        if(result == ";")
            return new TokenTemplate<string>(SEMI, result);
        
        return new TokenTemplate<string>(ID, result);
}

Token* Lexer::number()
{
    string result;
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
        return new TokenTemplate<double>(DOUBLE, atof(result.c_str()));
    }
    else
    {
        return new TokenTemplate<int>(INT, atoi(result.c_str()));   
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
            if(isdigit(current_char))
            {
                return number();   
            }
            result = current_char;
            if(result == ";")
            {
                advance();
                return new TokenTemplate<string>(SEMI, result);
            }
    }
    
    return new TokenTemplate<string>(EOI, "EOF");
}
