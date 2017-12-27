#ifndef IO_LEXER_H
#define IO_LEXER_H
#include <iostream>
#include <string>
#include <ctype.h>
#include <stdlib.h>
#include <fstream>

using namespace::std;

//Define the token labels we wish to create
#define REGION "REGION"
#define END "END"
#define SEMI "SEMI"
#define LEFTPAREN "("
#define RIGHTPAREN ")"
#define COMMA ","
#define COLON ":"
#define ID "ID"
#define DOUBLE_TOKEN "DOUBLE"
#define INT_TOKEN "INT"
#define EOI "EOF"
//#define POINT "POINT"
#define CONDITIONAL "CONDITIONAL"
//KEYWORDS
#define MESH "MESH"
#define CHANNEL "CHANNEL"
#define WHERE "WHERE"
#define WHEN "WHEN"
#define WHAT "WHAT"
#define POLYGON_TOKEN "POLYGON"
#define POINTDIST "POINTDIST"
#define CHANNELDIST "CHANNELDIST"
#define CHANNELCONF "CHANNELCONF"
#define CHANNELPOINT "CHANNELPOINT"
#define CHANNELORDER "CHANNELORDER"

/*
 * Virtual base class for a Token to provide a unified interface
 */
class Token
{
  public:
    friend ostream& operator<<(ostream& os, const Token& token)
    {
        token.print(os);
        return os;
    }
    virtual ~Token(){};
    virtual string getType(){};
  protected:
    virtual void print(ostream& os) const {};
    
};
/*
 * Token class for representing a lexical token
 */
template <typename T>
class TokenTemplate : public Token
{
  public:
    /*
     * Construct a Token with a type and a string value
     */
    TokenTemplate(string type, T value)
    {
        this->type = type;
        this->value = value;
    }
    ~TokenTemplate(){};
    string getType(){return type;}

    /*
     * Type of the token, must be #defined at the beginning of this file
     * These are the internal structures
     */
    string type;
    /*
     * The value read from the input for the given type
     */
    T value;
  
  protected:
    void print(ostream& os) const
    {
        os << "Token(" << this->type << ", " << this->value << ")\n";
    }
};


class Lexer
{
  public:
    Lexer(std::string fileName);
    ~Lexer();
    void error();
    /*
     * Advance the pointer read_position in the input text and set current_char to be whatever character is at text[pos]
     */
    void advance();
    /*
     * Peek function for looking ahead at the next character without moving the current position pointer read_position;
     */
    char peek();
    /*
     * Function for consuming white space between tokens in the input
     */
    void skip_whitespace();
    /*
     * Function for consuming comments in the input
     */
    void skip_comment();
    //Helper functions for identifying complex sequences in input
    Token* number();
    Token* _id();
    //Function to scan input for next valid token
    
    Token* get_next_token();
  
  private:
    ifstream input;
    int read_position;
    char current_char;
    
};
#endif
