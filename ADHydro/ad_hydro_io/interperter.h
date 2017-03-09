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
    Interperter(AbstractSyntaxTree& _tree):tree(_tree){}
    ~Interperter(){}

    void visit(Spec& spec)
    {
        cout << "Interperting a spec\n";
        cout <<"LEFT type "<<typeid(spec.left).name()<<"\n";
        cout <<"RIGHT type "<<typeid(spec.right).name()<<"\n";
        spec.left->accept(this);
        spec.right->accept(this);
    }
    void visit(IDList idlist)
    {
        cout << "Visiting an ID list\n";
    }
    void visit(MeshOutputVar output)
    {
        cout << "Visiting a MeshOutputVar: "<<output.value<<"\n";
    }
    void visit(ChannelOutputVar output)
    {
        
    }

    
    void interpert()
    {
        cout << "Interperting "<<typeid(tree).name()<<"\n";
        tree.accept(this);
    }
    
private:
    AbstractSyntaxTree& tree;
};

#endif
