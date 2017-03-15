#include "ast.h"

void AbstractSyntaxTree::accept(NodeVisitor* v)
{
    cout << "UNKNON ACCEPTANCE!!! "<<typeid(v).name()<<"\n";
}

/*
 * AST Nodes
 */
IDList::IDList(std::vector<int> _list):list(_list)
{
    
}
void IDList::accept(NodeVisitor* v)
{
    cout << "ACCEPTING IDLIST\n";
    return v->visit(*this);
}

MeshOutputVar::MeshOutputVar(string name, IDList* _id_list):value(name),id_list(_id_list)
{

}
MeshOutputVar::~MeshOutputVar()
{
    delete id_list;
}
void MeshOutputVar::accept(NodeVisitor* v)
{
    cout << "ACCEPTING MESHOUTPUTVAR\n";
    return v->visit(*this);
}

ChannelOutputVar::ChannelOutputVar(string name, IDList* _id_list):value(name),id_list(_id_list)
{
    
}
ChannelOutputVar::~ChannelOutputVar()
{
    delete id_list;
}
void ChannelOutputVar::accept(NodeVisitor* v)
{
    cout << "ACCEPTING CHANELOUTPUTVAR\n";
    return v->visit(*this);
}

//Empty constructor definition to make linker happy
Compound::Compound()
{
    
}
Compound::~Compound()
{
    for(int i = 0; i < children.size(); i++)
    {
        delete children[i];
    }
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
