#ifndef IO_INTERPERTER_H
#define IO_INTERPERTER_H

#include "parser.h"

class Interperter : public NodeVisitor
{
    /*
     * Class for interperting the abstract syntax tree built by the parser
     * Implements the visitor pattern
     */
public:
    Interperter(Spec _tree):tree(_tree){}
    ~Interperter(){}
    void visit(Spec& spec)
    {
        cout << "Interperting a spec\n";
        visit(spec.left);
        visit(spec.right);
    }
    void visit(IDList& idlist)
    {
        cout << "Visiting an ID list\n";
    }
    void visit(MeshOutputVar& output)
    {
        cout << "Visiting a MeshOutputVar: "<<output.value<<"\n";
    }
    void visit(ChannelOutputVar& output)
    {
        
    }

    
    void interpert()
    {
        visit(tree);
    }
    
private:
    Spec tree;
};

#endif
