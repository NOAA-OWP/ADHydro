Instructions for Converting WRF Products to CI-Water ADHydro Format

Script name: WRFtoADHydro_ncdf4.R

Creator: Hernan A. Moreno

Date: September 1^st^ 2014

Last Modification: January 26^th^ 2015

# Description

This R-script reads, clips and exports WRF forcing data to ADHydro -
netcdf format for a prescribed basin domain. The final output is an
array of **TIN** **mesh elements x WRF variables x Instances of time**
elements.

# Execution

This script runs in R (Programming language and software environment for
statistical computing and graphics) that can be downloaded for free from
<http://www.r-project.org/>. The user can run this script by opening a
terminal and writing **source("/pathtoRscript/WRFtoADHydro_ncdf4.R")**

# Libraries

Before running this R-script for the first time, the following libraries
have to be installed in R platform, by using the command
install.packages("name_of_the_package"): (1) ncdf4, (2) abind, and (3)
timeDate.

# Inputs

The user provides the following input parameters within the \### INPUT
MODULE \### section at the beginning of the script:

| Input type    |    Description
| ------------- | -------------- |
| **WRF_Folder** |   Absolute path to the WRF files. |
| **WRF_Files**    | A complete, ordered list of files to be read by the script. |
| **TINF**         | Geometry file (in netcdf format) with the ADHydro mesh elements for which the WRF data will be extracted. |
| **RE**         |   Earth\'s radius at the equator in meters (6378137 meters using GRS80 ellipsoid). |
| **Lambda00**  |    Reference meridian in degrees for ADHydro mesh. Sign indicates if it\'s west or east of Greenwich. |
| **FalseEast**  |   False east of origin of desired coordinate system. |
| **FalseNorth**  |  False north of origin of desired coordinate system |
| **cell_Buffer**   | Number of cells to be added to the selected rectangular WRF clip from the minimum and maximum TIN points to completely read the WRF data. It\'s suggested to use a number greater than 2 (usually set to 4). |
| **soillayers**  |  Number of total staggered soil layers. It can be extracted from the Rncdump.txt file when executing line 73 in this script. |
| **Outnames**   |   The names of the variables to extract from WRF starting with WRFHOUR and JULTIME. The list can be extended to all WRF variables. Caveat: all times in WRFHOUR and JULTIME are in GMT system. An example list is: c(\"WRFHOUR\", \"JULTIME\", \"T2\", \"QVAPOR\", \"QCLOUD\", \"PSFC\", \"U\", \"V\", \"VEGFRA\", \"MAXVEGFRA\", \"TPREC\", \"SWDOWN\", \"GLW\", \"PBLH\", \"TSLB\"). |
| **Outfolder**   |  Path and folder name for final netcdf output file. |
| **Outncfile**   |  Base name for final netcdf output file . |

# Outputs

This script generates a netcdf4 file with two dimensions corresponding
to:

1\. Mesh elements directly extracted from the geometry.nc file.

2\. Time instances directly extracted from the WRF files in GMT system.

And Outnames variables (14 in this case), described in the next table:

| WRF Variable |  Description |
|--------------|--------------|
| JULTIME     |    Absolute Julian date as defined by http://en.wikipedia.org/wiki/Julian_day |
| T2          |    Air temperature at 2 m elevation (ºC). |
| QVAPOR      |   Water vapor mixing ratio (kg/kg). |
| QCLOUD      |  Cloud water mixing ratio (kg/kg). |
| PSFC        |   Atmospheric surface pressure (Pa). |
| U           |   X-wind component at mid elevation of lowest element and staggered locations. |
| V           |   V-wind component at mid elevation of lowest element and staggered locations. |
| VEGFRA      |   Vegetation fraction \[0,1\]. |
| MAXVEGFRA   |   Maximum annual vegetation fraction \[0,1\]. |
| TPREC       |   Total precipitation rate including snow, convective, hail and graupel type \[m/s\]. |
| SWDOWN      |   Downward short wave flux at ground surface \[w/m^2^\]. |
| GLW         |   Downward long wave flux at ground surface \[w/m^2^\]. |
| PBLH        |   Planetary boundary layer height \[m\].
| TSLB        |   Soil bottom temperature (ºC). |
