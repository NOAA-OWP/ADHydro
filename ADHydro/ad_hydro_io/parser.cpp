//Implementation details for AST parser

#include "parser.h"

/*
 * AST Nodes
 */
IDList::IDList(std::vector<int> _list):list(_list){}
void IDList::accept(NodeVisitor* v)
{
    cout << "ACCEPTING IDLIST\n";
    return v->visit(*this);
}

MeshOutputVar::MeshOutputVar(string name):value(name){}
void MeshOutputVar::accept(NodeVisitor* v)
{
    cout << "ACCEPTING MESHOUTPUTVAR\n";
    return v->visit(*this);
}

ChannelOutputVar::ChannelOutputVar(string name):value(name){}
void ChannelOutputVar::accept(NodeVisitor* v)
{
    cout << "ACCEPTING CHANELOUTPUTVAR\n";
    return v->visit(*this);
}

void Compound::accept(NodeVisitor* v)
{
    cout << "ACCEPTING COMPOUND\n";
    return v->visit(*this);
}

Spec::Spec(Compound* compound){child = compound;}
//Empty constructor
Spec::Spec(){};
Spec::~Spec()
{
    delete child;
}
void Spec::accept(NodeVisitor* v)
{
    cout << "ACCEPTING SPEC\n";
    return v->visit(*this);
}
