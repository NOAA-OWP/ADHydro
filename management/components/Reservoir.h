#ifndef RES_H
#define RES_H

/*
    Base class for reservoir managment components
*/
class Reservoir
{
    public:
	/* ReachCode identifying the NHD waterbody corresponding to this reservoir */
	long long reachCode;
    /*
        Reservoir constructor
        Most construction will be unique to the individual sublcasses, but global constrcution
        can occur here.
        Parameters:
        reachCode_  the NHD reachCode identifier for the water body
    */
    Reservoir(long long reachCode_);

    /*
        Reservoir destrcutor
        Most destrcution will be unique to the individual subclasses, but global destrcution
        can occur here if needed.
    */
    virtual ~Reservoir();
    /*
        This function will implement the managment rules of Reservoir instances.
        TODO: All state variables that may be needed from the Model need to be passed as parameters
              to this function
        This is a PURE VIRTUAL FUNCTION, so all sublcasses must implement it
    */
    virtual double release() = 0;
};
#endif
