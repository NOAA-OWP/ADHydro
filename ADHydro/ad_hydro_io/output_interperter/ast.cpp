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

/*
 * Methods of the Polygon AST class
 */
Polygon::Polygon(std::vector<std::pair<double, double> > _polyPointList):polyPointList(_polyPointList)
{

}
Polygon::~Polygon()
{

}
void Polygon::accept(NodeVisitor* v)
{
  return v->visit(*this);
}

/*
 * Methods of the PointDistance AST class
 */
PointDistance::PointDistance(std::pair<double, double> _point, double _distance):point(_point),distance(_distance)
{

}
PointDistance::~PointDistance()
{

}
void PointDistance::accept(NodeVisitor* v)
{
    return v->visit(*this);
}

/*
 * Methods of the ChannelDistance AST class
 */
ChannelDistance::ChannelDistance(double _distance):distance(_distance)
{

}
ChannelDistance::~ChannelDistance()
{

}
void ChannelDistance::accept(NodeVisitor* v)
{
    return v->visit(*this);
}

/*
 * Methods of the ChannelConfluence AST class
 */
ChannelConfluence::ChannelConfluence(int _order):order(_order)
{

}
ChannelConfluence::~ChannelConfluence()
{

}
void ChannelConfluence::accept(NodeVisitor* v)
{
    return v->visit(*this);
}

/*
 * Methods of the ChannelPoint AST class
 */
ChannelPoint::ChannelPoint(std::pair< double, double > _point):point(_point)
{

}
ChannelPoint::~ChannelPoint()
{

}
void ChannelPoint::accept(NodeVisitor* v)
{
    return v->visit(*this);
}

/*
 * Methods of the ChannelOrder AST class
 */
ChannelOrder::ChannelOrder(std::pair< double, double > _point, double _distance):point(_point), distance(_distance)
{

}
ChannelOrder::~ChannelOrder()
{

}
void ChannelOrder::accept(NodeVisitor* v)
{
    return v->visit(*this);
}


/*
 * Methods of the Where AST class
 */
Where::Where()
{
  
}
Where::~Where()
{
    for(int i = 0; i < children.size(); i++)
    {
	delete children[i];
    }
}
void Where::accept(NodeVisitor* v)
{
    return v->visit(*this);
}

/*
 * Methods of the TimePeriod AST class
 */
TimePeriod::TimePeriod(double _start, double _stop, double _interval, string _resolution):start(_start),stop(_stop),interval(_interval),resolution(_resolution)
{

}
TimePeriod::~TimePeriod()
{

}
void TimePeriod::accept(NodeVisitor* v)
{
    return v->visit(*this);
}



/*
 * Methods of the When AST class
 */
When::When()
{
  
}
When::~When()
{
    for(int i = 0; i < children.size(); i++)
    {
	delete children[i];
    }
}
void When::accept(NodeVisitor* v)
{
    return v->visit(*this);
}

/*
 * Methods of the What AST class
 */
What::What()
{
  
}
What::~What()
{
    for(int i = 0; i < children.size(); i++)
    {
	delete children[i];
    }
}
void What::accept(NodeVisitor* v)
{
    return v->visit(*this);
}

Spec::Spec(Where* _where, When* _when, What* _what)
{
    where = _where;
    when = _when;
    what = _what;
}

Spec::~Spec()
{
    delete where;
    delete when;
    delete what;
}
void Spec::accept(NodeVisitor* v)
{
    cout << "ACCEPTING SPEC\n";
    return v->visit(*this);
}
