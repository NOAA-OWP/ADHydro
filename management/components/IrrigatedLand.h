#ifndef IRR_LAND_H
#define IRR_LAND_H

/*
    Base class for Irrigated Land components
*/
class IrrigatedLand
{
    public:
	/* Id identifying the Irrigated Land Parcel corresponding to this object*/
	long long id;
    /*
        IrrigatedLand constructor
        Most construction will be unique to the individual sublcasses, but global construction
        can occur here.
        Parameters:
        id_  the parcel's unique identifier
    */
	IrrigatedLand(long long id_);

    /*
        IrrigatedLand destrcutor
        Most destruction will be unique to the individual subclasses, but global destruction
        can occur here if needed.
    */
    virtual ~IrrigatedLand();
    /*
        This function will implement the managment rules of Irrigated Land.
        TODO: All state variables that may be needed from the Model need to be passed as parameters
              to this function
        This is a PURE VIRTUAL FUNCTION, so all sublcasses must implement it
    */
    virtual double call() = 0;
};
#endif
