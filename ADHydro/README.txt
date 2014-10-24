How to run ADHydro:

The latest version of ADHydro that works is checked in and pushed to the git repository.  To get a new copy of the repository:

git clone /share/ADHydro.git

To update a repository you have to the latest version:

git pull

To build ADHydro you first need to build NOAH-MP, then build ADHydro:

cd ADHydro/HRLDAS-v3.6
make
cd ../..
cd ADHydro/ADHydro
make

To run ADHydro on one processor:

./adhydro <input directory> <output directory>

To run ADHydro on more than one processor:

./charmrun +p<number of processors> ./adhydro <input directory> <output directory>

The input files for the small green river that are working are in:

/share/CI-WATER Simulation Data/small_green_mesh

The files you need are:

geometry.nc
parameter.nc
state.nc
forcing.nc
GENPARM.TBL
MPTABLE.TBL
SOILPARM.TBL
VEGPARM.TBL

The input files all need to be in the same directory.  The output files will all be put in the same directory.  The output directory must be different than the input directory because the output files have the same names as the input files.  I/O goes faster if you use a local hard drive.  To do this, copy the input files to /localstore and put your output files in /localstore as well.  Here is an example:

cd /localstore
mkdir input
mkdir output
cp /share/CI-WATER\ Simulation\ Data/small_green_mesh/*.nc input
cp /share/CI-WATER\ Simulation\ Data/small_green_mesh/*.TBL input
cd <wherever your ADHydro repository is>/ADHydro/ADHydro
./adhydro /localstore/input /localstore/output

This works for up to 8 processors on the local machine.  If you want to use more than 8 processors they will be spread across multiple machines.  All processors must have access to the input and output files so they must be in a network storage location, either /user2 or /share.  If you don't output too often the difference in I/O time should be small compared to compute time.  Here is an example:

cd ~/Desktop
mkdir output
cd <wherever your ADHydro repository is>/ADHydro/ADHydro
./charmrun +p16 ./adhydro /share/CI-WATER\ Simulation\ Data/small_green_mesh ~/Desktop/output

*** WARNING *** When I ran on multiple machines I could not get the file path with spaces in it to work.  You can copy the input files to somewhere else that doesn't have spaces, or we will probably want to rename the directory to CI-WATER_Simulation_Data.

The right versions of Charm++ and MPICH are installed on all machines in our lab.  The environment variables for MPICH are already set up on all machines to allow you to do runs across multiple machines in our lab.  We have 40 processors across the 5 machines in our lab.  However, my ArcGIS run on theis hasn't finished yet.  It's only using one processor so I would use a maximum of 39 processors and only have 7 on theis.  If more than one of you wants to do a run at the same time you will need to decide how to split up the machines.  You will need to look at the documentation of MPICH to see how to specify which machines to use and how many processors to put on each machine.

When you run on multiple machines it will ask for your ssh password to log in to the other machines and start processes on them.

When you run there is a pause at the beginning when it reads in the input files and creates the output files.  When I was reading from a network storage location the pause was half an hour.  Hopefully this will be better on Mt. Moran and Yellowstone with the infiniband network and high performance disks, but it is something we need to eventually look into optimizing.

If you are running on more than one processor, after iteration 4 there will be a pause while it does load balancing.  When I was running on 15 processors on ampt and theis the pause was 3 minutes.

FIXME talk about turning off assertions.
