#!/bin/bash
set -x
#Get the shapefile name
shpfile=$1
base=`basename $shpfile .shp`
#Get the extents of the shapefile, shift the y's around to be in the correct order for gdalwarp
extent=`ogrinfo -so $shpfile $base | grep Extent | sed 's/Extent: //g' | sed 's/(//g' | sed 's/)//g' | sed 's/ - /, /g'`
extent=`echo $extent | awk -F ',' '{print $1 " " $4 " " $3 " " $2}'`

#Now clip the raster file to the extent of the shapefile (for optimization)
raster=$2
echo "Clipping $raster to $extent"

rbase=`basename $raster .vrt`
rbase=`basename $(dirname $shpfile)`
rtmp=/project/CI-WATER/data/tmp/$rbase\_bbclip.tiff
otmp=/project/CI-WATER/data/tmp/$rbase\_shpclip.tiff
rm $otmp
#gdal_translate -projwin $extent -of GTiff $raster $rtmp
echo "Clipping $rtmp to vector in $shpfile, saving to $otmp"
#gdalwarp -co COMPRESS=DEFLATE -co TILED=YES -of GTiff -r lanczos -cutline $shpfile $rtmp $otmp
#run gdalwarp, using cubicspline interpolation, 1024 MB of cache, using multiple threads.  Set the destination
#no data to -1, and cut to the provided shapefile.  Also creates an alpha band
gdalwarp -overwrite -r cubicspline -wm 1024 -multi -dstnodata -1 -cutline $shpfile -dstalpha -of GTiff $rtmp $otmp
