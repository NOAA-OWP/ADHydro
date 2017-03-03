#ifndef __READONLY_H__
#define __READONLY_H__

// This is a do-nothing class for holding global variables that are used as Charm++ readonly variables.
class Readonly
{
public:
    
    static int verbosityLevel; // Controls the amount of messages printed to console.
};

#endif // __READONLY_H__
