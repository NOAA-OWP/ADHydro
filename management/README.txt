Every new component (i.e. reservoir, irrigated land, ect.) needs to be added to its appropritate factory class constructor.

For exame, creating a reservoir called Res1 would require the following line added to the ReservoirFactory() constructor
in the ReservoirFactory.cpp file:

creators.push_back(new ReservoirCreator<Res1>(Res1::reachCode, this));
