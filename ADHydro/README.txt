How to run ADHydro:

The latest version of ADHydro is checked in and pushed to the github repository.  To get a new copy of the repository:

git clone https://<your Github username>@github.com/ADHydro-devel/alpha1.git <name that you want your local copy to have>

For example,

git clone https://rsteinke@github.com/ADHydro-devel/alpha1.git ADHydro

To build ADHydro you first need to build inih and NOAH-MP, then build ADHydro:

cd ADHydro/inih
make
cd ../..
cd ADHydro/HRLDAS-v3.6
make
cd ../..
cd ADHydro/ADHydro
make

ADHydro is set up to compile in debug mode with assertions turned on.  To compile in production mode edit all.h to change the following lines to whatever debug level you want to use.  NOTE: change the string DEBUG_LEVEL_DEVELOPMENT.  Do not change the string DEBUG_LEVEL.

// Set this macro to the debug level to use.
#define DEBUG_LEVEL (DEBUG_LEVEL_DEVELOPMENT)

To run ADHydro on one processor:

./adhydro <superfile>

To run ADHydro on more than one processor:

./charmrun +p<number of processors> ./adhydro <superfile>

An example superfile can be found in ADHydro/input

The input files you need are:

geometry.nc
parameter.nc
state.nc
forcing.nc
GENPARM.TBL
MPTABLE.TBL
SOILPARM.TBL
VEGPARM.TBL

The input files all need to be in the same directory.  The output files will all be put in the same directory.  The output directory must be different than the input directory because the output files have the same names as the input files.  The exception to this is if you are appending your output to your input files.  In that case, ADHydro will use the input directory as the output directory regardless of what is specified for output directory in the superfile.
