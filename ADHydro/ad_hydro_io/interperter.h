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
        cout <<"CHILD type "<<typeid(spec.child).name()<<"\n";
        spec.child->accept(this);
    }
    
    void visit(Compound& compound)
    {
        cout << "Visiting a compound node\n";
        //TODO maybe use iterator???
        for(int i = 0; i < compound.children.size(); i++)
        {
            compound.children[i]->accept(this);
        }
    }
    
    void visit(IDList& idlist)
    {
        cout << "Visiting an ID list with ids: ";
        for(int i = 0; i < idlist.list.size(); i++)
        {
            cout<<idlist.list[i]<<" ";
        }
        cout << "\n";
    }
    
    void visit(MeshOutputVar& output)
    {
        cout << "Visiting a MeshOutputVar: "<<output.value<<"\n";
        output.id_list->accept(this);
    }
    
    void visit(ChannelOutputVar& output)
    {
        cout << "Visiting a ChannelOutputVar: "<<output.value<<"\n";
        output.id_list->accept(this);
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
