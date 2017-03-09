#include "io_spec_reader.h"
#include "lexer.h"
#include "parser.h"
#include "interperter.h"
#include <fstream>
int main(int argc, char** argv)
{
  //IOSpecReader reader;
  //reader.readSpec();
    ifstream inputFile ("output_spec");
    if(!inputFile.is_open())
    {
        cout << "Error opening output_spec\n";
        return -1;
    }
  Lexer lexer(inputFile);
  Parser parser(lexer);
  AbstractSyntaxTree& tree = parser.parse();
  Interperter interperter(tree);
  interperter.interpert();
}
