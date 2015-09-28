//#include "Reservoir.h"
#include "factory/ReservoirFactory.h"
#include <iostream>


int main(int argc, char** argv)
{
	//Define some reachCodes for testing
	//TODO What happens when a region contains two waterbodies with the same reach code????
	int numRes = 3;
	long long codes[numRes];
	codes[0] = 14050001002411;
	codes[1] = 14050001003012;
	codes[2] = 14050001004242;
	//Create an array of Reservoir Object Pointers
	Reservoir* res[numRes];
	//Create the ReservoirFactory used to create the indvidual reservoirs
    ReservoirFactory* resFactory = new ReservoirFactory();
    //For each reservoir, use the factory to create the approriate reservoir object
    for(int i = 0; i < numRes; i++)
    {
    	res[i] = resFactory->create(codes[i]);
    }
    //We don't need the factory anymore since the reservoirs are all created
    delete resFactory;

    //Call each reservoirs release
    for(int i = 0; i < numRes; i++)
    {
    	if(res[i] != NULL)
    	{
    		std::cout<<"Test Driver: "<<res[i]->release()<<std::endl;
    	}
    	else
    	{
    		std::cout<<"Test Driver: NULL reservoir"<<std::endl;
    	}
    }
    std::cout<<std::endl;
    //Search through the reservoir array for a particular ID an call it's release
    //We are assuming that each REGION has a very small set of reservoirs to search,
    //so just use a linear search...if larger number of reservoirs exist, we can
    //sort the reachCodes and use binary search
    long long id = 14050001003012;
    for(int i = 0; i< numRes; i++)
    {
    	if(res[i] != NULL && res[i]->reachCode == id)
    	{
    		std::cout<<"Found "<<id<<". Release: "<<res[i]->release()<<std::endl;
    	}
    }
    /*
     * IMPORTANT:Need to manually clean up the reservoirs
     *
     * To reduce memory footprint, the Factory is deleted before the
     * reservoir objects, since they are used throught the simulation
     * so the model needs to clean up these reservoir objects
     *
     * TODO might be able to write some static methods to help with this
     */
    for(int i = 0; i < numRes; i++)
    {
    	delete res[i];
    }
}
