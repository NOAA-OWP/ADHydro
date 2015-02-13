###################################################################################################################################################################################
# Code to read, clip and export WRF forcing data to ADHydro (UW) in netcdf format 
# Hernan Moreno
# Aug 19 2014
###################################################################################################################################################################################
# The final output file will be an array of two dimensions (node and time) and length(Outnames) variables. The list of potential variables that can go in the columns are, up to now:
# (1) WRFHOUR (2) JULTIME (3) T2 [Celsius], (4) Q2 [kg/kg], (5) QVAPOR[kg/kg], (6)QCLOUD[kg/kg], (7)PSFC [Pa], (8) U10 [m/s], (9) V10 [m/s], (10) U, (11) V, (12) VEGFRA [-], 
# (13) MAXVEGFRA, (14) TPREC (RAINC+RAINSH+RAINCC) [m/s],(15) FRAC_FROZ_PREC (SNOWNC+GRAUPELNC+HAILNC) [m/s], (16) SWDOWN [w/m2], (17) GLW [W/M2], (18) PBLH [m], 
# (19) SOILTB[Celsius], (20) TSLB [Celsius]
# Caveat: all times are in GMT time
################# LOAD LIBRARIES ##################################################################################################################################################

rm(list=ls())  # remove all objects before starting
library(ncdf4)
library(abind)
library(timeDate)
options(digits=16)
################ INPUT MODULE #####################################################################################################################################################

WRF_Folder<<-"/localstore/MtMoranLink/project/CI-WATER/data/WRF/WRF_output_new/historical/"  # path to the WRF files
WRF_Files<<-c("2000_new/wrfout_d03_2000-01-01_00:00:00", "2000_new/wrfout_d03_2000-01-31_12:00:00", "2000_new/wrfout_d03_2000-03-02_00:00:00", "2000_new/wrfout_d03_2000-04-01_12:00:00",
       "2000_new/wrfout_d03_2000-05-02_00:00:00", "2000_new/wrfout_d03_2000-06-01_12:00:00", "2000_new/wrfout_d03_2000-07-02_00:00:00", "2000_new/wrfout_d03_2000-08-01_12:00:00",
       "2000_new/wrfout_d03_2000-09-01_00:00:00", "2000_new/wrfout_d03_2000-10-01_12:00:00", "2000_new/wrfout_d03_2000-11-01_00:00:00", "2000_new/wrfout_d03_2000-12-01_12:00:00",
       "2001_new/wrfout_d03_2001-01-01_00:00:00", "2001_new/wrfout_d03_2001-01-31_12:00:00", "2001_new/wrfout_d03_2001-03-03_00:00:00", "2001_new/wrfout_d03_2001-04-02_12:00:00",
       "2001_new/wrfout_d03_2001-05-03_00:00:00", "2001_new/wrfout_d03_2001-06-02_12:00:00", "2001_new/wrfout_d03_2001-07-03_00:00:00", "2001_new/wrfout_d03_2001-08-02_12:00:00",
       "2001_new/wrfout_d03_2001-09-02_00:00:00", "2001_new/wrfout_d03_2001-10-02_12:00:00", "2001_new/wrfout_d03_2001-11-02_00:00:00", "2001_new/wrfout_d03_2001-12-02_12:00:00",
       "2002_new/wrfout_d03_2002-01-01_00:00:00", "2002_new/wrfout_d03_2002-01-31_12:00:00", "2002_new/wrfout_d03_2002-03-03_00:00:00", "2002_new/wrfout_d03_2002-04-02_12:00:00",
       "2002_new/wrfout_d03_2002-05-03_00:00:00", "2002_new/wrfout_d03_2002-06-02_12:00:00", "2002_new/wrfout_d03_2002-07-03_00:00:00", "2002_new/wrfout_d03_2002-08-02_12:00:00",
       "2002_new/wrfout_d03_2002-09-02_00:00:00", "2002_new/wrfout_d03_2002-10-02_12:00:00", "2002_new/wrfout_d03_2002-11-02_00:00:00", "2002_new/wrfout_d03_2002-12-02_12:00:00",
       "2003_new/wrfout_d03_2003-01-01_00:00:00", "2003_new/wrfout_d03_2003-01-31_12:00:00", "2003_new/wrfout_d03_2003-03-03_00:00:00", "2003_new/wrfout_d03_2003-04-02_12:00:00",
       "2003_new/wrfout_d03_2003-05-03_00:00:00", "2003_new/wrfout_d03_2003-06-02_12:00:00", "2003_new/wrfout_d03_2003-07-03_00:00:00", "2003_new/wrfout_d03_2003-08-02_12:00:00",
       "2003_new/wrfout_d03_2003-09-02_00:00:00", "2003_new/wrfout_d03_2003-10-02_12:00:00", "2003_new/wrfout_d03_2003-11-02_00:00:00", "2003_new/wrfout_d03_2003-12-02_12:00:00",
       "2004_new/wrfout_d03_2004-01-01_00:00:00", "2004_new/wrfout_d03_2004-01-31_12:00:00", "2004_new/wrfout_d03_2004-03-02_00:00:00", "2004_new/wrfout_d03_2004-04-01_12:00:00",
       "2004_new/wrfout_d03_2004-05-02_00:00:00", "2004_new/wrfout_d03_2004-06-01_12:00:00", "2004_new/wrfout_d03_2004-07-02_00:00:00", "2004_new/wrfout_d03_2004-08-01_12:00:00",
       "2004_new/wrfout_d03_2004-09-01_00:00:00", "2004_new/wrfout_d03_2004-10-01_12:00:00", "2004_new/wrfout_d03_2004-11-01_00:00:00", "2004_new/wrfout_d03_2004-12-01_12:00:00",
      "2005_new/wrfout_d03_2005-01-01_00:00:00", "2005_new/wrfout_d03_2005-01-31_12:00:00", "2005_new/wrfout_d03_2005-03-03_00:00:00", "2005_new/wrfout_d03_2005-04-02_12:00:00",
      "2005_new/wrfout_d03_2005-05-03_00:00:00", "2005_new/wrfout_d03_2005-06-02_12:00:00", "2005_new/wrfout_d03_2005-07-03_00:00:00", "2005_new/wrfout_d03_2005-08-02_12:00:00",
      "2005_new/wrfout_d03_2005-09-02_00:00:00", "2005_new/wrfout_d03_2005-10-02_12:00:00", "2005_new/wrfout_d03_2005-11-02_00:00:00", "2005_new/wrfout_d03_2005-12-02_12:00:00",
      "2006_new/wrfout_d03_2006-01-01_00:00:00", "2006_new/wrfout_d03_2006-01-31_12:00:00", "2006_new/wrfout_d03_2006-03-03_00:00:00", "2006_new/wrfout_d03_2006-04-02_12:00:00",
      "2006_new/wrfout_d03_2006-05-03_00:00:00", "2006_new/wrfout_d03_2006-06-02_12:00:00", "2006_new/wrfout_d03_2006-07-03_00:00:00", "2006_new/wrfout_d03_2006-08-02_12:00:00",
      "2006_new/wrfout_d03_2006-09-02_00:00:00", "2006_new/wrfout_d03_2006-10-02_12:00:00", "2006_new/wrfout_d03_2006-11-02_00:00:00", "2006_new/wrfout_d03_2006-12-02_12:00:00",
      "2007_new/wrfout_d03_2007-01-01_00:00:00", "2007_new/wrfout_d03_2007-01-31_12:00:00", "2007_new/wrfout_d03_2007-03-03_00:00:00", "2007_new/wrfout_d03_2007-04-02_12:00:00",
      "2007_new/wrfout_d03_2007-05-03_00:00:00", "2007_new/wrfout_d03_2007-06-02_12:00:00", "2007_new/wrfout_d03_2007-07-03_00:00:00", "2007_new/wrfout_d03_2007-08-02_12:00:00",
      "2007_new/wrfout_d03_2007-09-02_00:00:00", "2007_new/wrfout_d03_2007-10-02_12:00:00", "2007_new/wrfout_d03_2007-11-02_00:00:00", "2007_new/wrfout_d03_2007-12-02_12:00:00",
      "2008_new/wrfout_d03_2008-01-01_00:00:00", "2008_new/wrfout_d03_2008-01-31_12:00:00", "2008_new/wrfout_d03_2008-03-02_00:00:00", "2008_new/wrfout_d03_2008-04-01_12:00:00", 
      "2008_new/wrfout_d03_2008-05-02_00:00:00", "2008_new/wrfout_d03_2008-06-01_12:00:00", "2008_new/wrfout_d03_2008-07-02_00:00:00", "2008_new/wrfout_d03_2008-08-01_12:00:00",
      "2008_new/wrfout_d03_2008-09-01_00:00:00", "2008_new/wrfout_d03_2008-10-01_12:00:00", "2008_new/wrfout_d03_2008-11-01_00:00:00", "2008_new/wrfout_d03_2008-12-01_12:00:00",
      "2009_new/wrfout_d03_2009-01-01_00:00:00", "2009_new/wrfout_d03_2009-01-31_12:00:00", "2009_new/wrfout_d03_2009-03-03_00:00:00", "2009_new/wrfout_d03_2009-04-02_12:00:00",
      "2009_new/wrfout_d03_2009-05-03_00:00:00", "2009_new/wrfout_d03_2009-06-02_12:00:00", "2009_new/wrfout_d03_2009-07-03_00:00:00", "2009_new/wrfout_d03_2009-08-02_12:00:00", 
      "2009_new/wrfout_d03_2009-09-02_00:00:00", "2009_new/wrfout_d03_2009-10-02_12:00:00", "2009_new/wrfout_d03_2009-11-02_00:00:00", "2009_new/wrfout_d03_2009-12-02_12:00:00",
      "2010_new/wrfout_d03_2010-01-01_00:00:00", "2010_new/wrfout_d03_2010-01-31_12:00:00", "2010_new/wrfout_d03_2010-03-03_00:00:00", "2010_new/wrfout_d03_2010-04-02_12:00:00",
      "2010_new/wrfout_d03_2010-05-03_00:00:00", "2010_new/wrfout_d03_2010-06-02_12:00:00", "2010_new/wrfout_d03_2010-07-03_00:00:00", "2010_new/wrfout_d03_2010-08-02_12:00:00",
      "2010_new/wrfout_d03_2010-09-02_00:00:00", "2010_new/wrfout_d03_2010-10-02_12:00:00", "2010_new/wrfout_d03_2010-11-02_00:00:00", "2010_new/wrfout_d03_2010-12-02_12:00:00") # Ordered list of WRF files
       
TINF<<-"/share/CI-WATER_Simulation_Data/small_green_mesh/geometry.nc"  # netcdf file with the AD-Hydro mesh elements for which the WRF data will be extracted
RE=6378137 		# in meters; Earth's radius at the equator GRS80 ellipsoid
Lambda00<<--109 	# reference meridian in degrees. Sign indicates if it's west or east of Greenwich
FalseEast<<-20000000 	# False East of desired coordinate system
FalseNorth<<-10000000	# False North of  desired coordinate system
cell_Buffer<<-4 	# number of cells to be added to the selected rectangular from the minimum and maximum TIN points to efficiently read the WRF data. Use a number >2
soillayers<<-4  	# number of total staggered soil layers. It can be extracted from the Rncdump.txt file from line 39
Outnames<<-c("WRFHOUR","JULTIME","T2","QVAPOR","QCLOUD","PSFC","U","V","VEGFRA","MAXVEGFRA","TPREC","SWDOWN","GLW","PBLH","TSLB")  # Variables process + WRFHOUR and JULTIME 
Outfolder<<-"/share/CI-WATER_Simulation_Data/WRF_to_ADHydro/"	# Folder name for final netcdf output
Outncfile<<-"WRF_ADHydro_Small_Green_River" # Base name for output file
firsthour=seq(1,120,12)  # this is applied for the precipitation and GLW matrices that contain zero values for the first hour of a new year, so this sequence may change according to WRF-files

############### MODULE METADATA AND HEADERS #######################################################################################################################################
nccolumns<<-c(Outnames,paste(Outnames[3:length(Outnames)],"_C",sep=""))
JulOrigin<<-"0001-01-01 12:00:00" # Refence for AC Julian date era. Please do not modify this reference as years BC will be added to this number
cat("************************************************* WRF to ADHydro ***********************************************************************",fill=TRUE)
cat("Time started processing ",date(),fill=TRUE)
ex.nc = nc_open(paste(WRF_Folder, WRF_Files[1], sep="")) # it opens the first file
Rdump=capture.output(print(ex.nc), file = NULL, append = FALSE)
write.table(Rdump,paste(Outfolder,"Rncdump.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   ## dumps netcdf headers to a file in outfolder  
cat("A dump file has been created in ", Outfolder," called Rncdump",fill=TRUE)
############## MODULE COORDINATES CONVERSION AND BOUNDING BOX SELECTION ###########################################################################################################
mesh.nc = nc_open(TINF) # it opens the geometry file
Rdump_mesh=capture.output(print(mesh.nc), file = NULL, append = FALSE)
write.table(Rdump_mesh,paste(Outfolder,"Rncdump_mesh.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   ## dumps netcdf headers of geometry to a file in outfolder  
TINX = ncvar_get(mesh.nc, "meshElementX")  # mesh_elements
TINY = ncvar_get(mesh.nc, "meshElementY")
TIN= cbind(seq(0,(length(TINX)-1)),TINX,TINY)

CEX= ncvar_get(mesh.nc, "channelElementX")  # channel_elements
CEY= ncvar_get(mesh.nc, "channelElementY")  # channel_elements
CE= cbind(seq(0,(length(CEX)-1)),CEX,CEY)

x11()
par(mfcol=c(1,2))
plot(TIN[,2],TIN[,3],pch=19,cex=0.2,main="Basin domain")
par(new=TRUE)
plot(CE[,2],CE[,3],pch=19,cex=0.2,col="cyan")


## Read only the dimension file to determine matrices size
LON_WRF = ncvar_get(ex.nc, "XLONG")   # Matrix of longitudes in this order: (west_east,south_north,times)
LAT_WRF = ncvar_get(ex.nc, "XLAT")   # Matrix of latitudes
rowss=dim(LON_WRF)[2]
colss=dim(LON_WRF)[1]
TIMES=dim(LON_WRF)[3]  # normally the third dimension are times

#### Cell-centered coordinate transformation ######################################################################################################################################
# Transform LON-LAT matrices to sinusoidal projection
Lambda0=Lambda00*(pi/180)  # degrees to radians
xs=RE*((LON_WRF[,,1]*pi/180)-Lambda0)*cos(LAT_WRF[,,1]*pi/180)+FalseEast
ys=RE*(LAT_WRF[,,1]*pi/180)+FalseNorth

# New module-- extraction of a smaller rectangular area to process data from here on
minxTIN=min(TIN[,2],CE[,2])
maxxTIN=max(TIN[,2],CE[,2])
minyTIN=min(TIN[,3],CE[,3])
maxyTIN=max(TIN[,3],CE[,3])

# Extraction of the points closer to minTIN and maxTIN
distLL=(((minxTIN-xs)^2)+((minyTIN-ys)^2))^0.5
distUR=(((maxxTIN-xs)^2)+((maxyTIN-ys)^2))^0.5

mindLL=min(distLL)
WRFLL=which(distLL==mindLL,arr.ind=TRUE)
mindUR=min(distUR)
WRFUR=which(distUR==mindUR,arr.ind=TRUE)
WRFL=WRFLL-cell_Buffer
WRFU=WRFUR+cell_Buffer

cat("Initial WRF domain had ",dim(xs)," cells",fill=TRUE)
cat("New WRF domain has ",WRFUR-WRFLL+1," cells",fill=TRUE)
cat("Applying buffer matrix has ",WRFU-WRFL+1," cells",fill=TRUE)

## Creating new xs(xss) and ys(yss).. These are smaller matrices that contain only the TIN points
xss=xs[WRFL[1]:WRFU[1],WRFL[2]:WRFU[2]]
yss=ys[WRFL[1]:WRFU[1],WRFL[2]:WRFU[2]]

# Extraction of the cells of WRF that contain each of the mesh elements for ADHydro
sel_WRF=matrix(NA,dim(TIN)[1],3) # Node, row, col
sel_WRF[,1]=TIN[,1]
mindi=rep(NA,dim(TIN)[1])
cat("** Extracting distances from ADHydro mesh elements to WRF cells.. may take a few minutes...",fill=TRUE)
for (ni in 1:dim(TIN)[1]){
dist=(((TIN[ni,2]-xss)^2)+((TIN[ni,3]-yss)^2))^0.5
mind=min(dist)
mindi[ni]=mind
wmind=which(dist==mind,arr.ind=TRUE)
sel_WRF[ni,2]=wmind[1] # row
sel_WRF[ni,3]=wmind[2] # col
}

cat("** Maximum distance between an ADHydro mesh element and WRF cell center is", max(mindi)," m",fill=TRUE)
plot(WRFL[1]+sel_WRF[,2],WRFL[2]+sel_WRF[,3],xlim=c(1,colss),ylim=c(1,rowss),cex=0.2,pch=20,main="Basin domain comparison")

# Extraction of the cells of WRF that contain each of the channel elements for ADHydro
sel_WRFC=matrix(NA,dim(CE)[1],3) # Node, row, col
sel_WRFC[,1]=CE[,1]
mindiC=rep(NA,dim(CE)[1])
cat("** Extracting distances from ADHydro channel elements to WRF cells.. may take a few minutes...",fill=TRUE)

for (ni in 1:dim(CE)[1]){
distC=(((CE[ni,2]-xss)^2)+((CE[ni,3]-yss)^2))^0.5
mindC=min(distC)
mindiC[ni]=mindC
wmindC=which(distC==mindC,arr.ind=TRUE)
sel_WRFC[ni,2]=wmindC[1] # row
sel_WRFC[ni,3]=wmindC[2] # col
}

cat("** Maximum distance between an ADHydro channel element and WRF cell center is", max(mindiC)," m",fill=TRUE)
par(new=TRUE)
plot(WRFL[1]+sel_WRFC[,2],WRFL[2]+sel_WRFC[,3],xlim=c(1,colss),ylim=c(1,rowss),cex=0.2,pch=20,col="cyan")

### Staggered coordinate transformation ###########################################################################################################################################
# Reads only the staggered-LON - LAT field and transform them to sinusoidal coordinates
LONU_WRF = ncvar_get(ex.nc, "XLONG_U",start=c(1,1,1),count=c(colss+1,rowss, 1))   # Matrix of longitudes is in inverse order (south_north, west_east)
LATU_WRF = ncvar_get(ex.nc, "XLAT_U",start=c(1,1,1),count=c(colss+1,rowss, 1))   # Matrix of latitudes

# Transform LONU-LATU, LONV, LATV matrices to sinusoidal projection
xsu=RE*((LONU_WRF*pi/180)-Lambda0)*cos(LATU_WRF*pi/180)+FalseEast   #  xsu and xsv are very similar.. only separated by few meters..so we assume xsu=xsv
ysu=RE*(LATU_WRF*pi/180)+FalseNorth

## Creating new xssu and yssu.. These are smaller matrices that contain only the TIN points, assuming the same corners found for centered cells apply for staggered data
xssu=xsu[WRFL[1]:WRFU[1],WRFL[2]:WRFU[2]] 
yssu=ysu[WRFL[1]:WRFU[1],WRFL[2]:WRFU[2]]

# Extraction of the cells of WRF that contain each of the TIN mesh elements for AD-Hydro at staggered locations
sel_WRFS=matrix(NA,dim(TIN)[1],3) # Node, row, col
sel_WRFS[,1]=TIN[,1]   
mindiS=rep(NA,dim(TIN)[1])
cat("** Extracting distances from AD-Hydro nodes to WRF staggered points.. may take few minutes...",fill=TRUE)
for (ni in 1:dim(TIN)[1]){
        distS=(((TIN[ni,2]-xssu)^2)+((TIN[ni,3]-yssu)^2))^0.5
        mindS=min(distS)
        mindiS[ni]=mindS
        wmindS=which(distS==mindS,arr.ind=TRUE)
        sel_WRFS[ni,2]=wmindS[1] # row
        sel_WRFS[ni,3]=wmindS[2] # col
}
	
cat("** Maximum distance between an ADHydro node and WRF staggered point is", max(mindiS)," m",fill=TRUE)

# Extraction of the cells of WRF that contain each of the channel elements for AD-Hydro at staggered locations
sel_WRFSC=matrix(NA,dim(CE)[1],3) # Node, row, col
sel_WRFSC[,1]=CE[,1]   
mindiSC=rep(NA,dim(CE)[1])
cat("** Extracting distances from AD-Hydro channel elements to WRF staggered points.. may take few minutes...",fill=TRUE)
for (ni in 1:dim(CE)[1]){
        distSC=(((CE[ni,2]-xssu)^2)+((CE[ni,3]-yssu)^2))^0.5
        mindSC=min(distSC)
        mindiSC[ni]=mindSC
        wmindSC=which(distSC==mindSC,arr.ind=TRUE)
        sel_WRFSC[ni,2]=wmindSC[1] # row
        sel_WRFSC[ni,3]=wmindSC[2] # col
}
	
cat("** Maximum distance between an ADHydro channel element and WRF staggered point is", max(mindiSC)," m",fill=TRUE)


## Creation of the final array that will be transformed into a netcdf file
## Starts reading the WRF matrices as named in WRF_Files
for (mi in 1:length(WRF_Files)){

cat("--Processing WRF netcdf file ",WRF_Files[mi],fill=TRUE)
cat("Time at this moment is ",date(),fill=TRUE)
ex.nc = nc_open(paste(WRF_Folder, WRF_Files[mi], sep="")) 
## Starts reading the WRF data by variables
for (u in 1:length(Outnames)){
        
if (Outnames[u]=="WRFHOUR"){
zt = ncvar_get( ex.nc, "Times")   # variable
	
    if (mi==1){
    tini=1
    tf=length(zt)
    } else {
    tini=tf+1
    tf=tini+length(zt)-1
    }
    cat("mi=",mi,", tini =",tini,", tf=",tf,", length(zt)=",length(zt),fill=TRUE)

# According to the TIN  and CE and TIMES dimension of the first matrix it creates a new array with this dimension
  finalold<<-array(NA, c(dim(TIN)[1],1+length(Outnames),length(zt)))  # initial column is node number for ADHydro
  ordencols=c("MESHELEMENT",Outnames)	
  colnames(finalold)=ordencols
  finalold[,1,]=TIN[,1] # first column are the nodes
  cat("variable WRFHOUR",fill=TRUE)
  for (ti in 1:length(zt)){	
  finalold[,u+1,ti]=rep(zt[ti],dim(finalold)[1])
  }

  finaloldC<<-array(NA, c(dim(CE)[1],1+length(Outnames)-2,length(zt)))  # initial column is node number for ADHydro
  ordencolsC=c("CHANNELELEMENT",Outnames[3:length(Outnames)])	
  colnames(finaloldC)=ordencolsC
  finaloldC[,1,]=CE[,1] # first column are the nodes
}

if (Outnames[u]=="JULTIME"){
cat("variable JULTIME",fill=TRUE)
  for (zi in 1:length(zt)){
  pa= substring(zt[zi], 1:nchar(zt[zi]), 1:nchar(zt[zi]))
  prejul=timeDate(paste(pa[1],pa[2],pa[3],pa[4],pa[5],pa[6],pa[7],pa[8],pa[9],pa[10]," ",pa[12],pa[13],pa[14],pa[15], pa[16], pa[17],pa[18],pa[19],sep=""))    
  finalold[,u+1,zi]=rep(as.numeric(julian(prejul,origin=timeDate("0001-01-01 12:00:00"),units="days")),dim(finalold)[1])+rep((1721424+2),dim(finalold)[1]) # 1721424+2 days from Julian Origin
  }
}

    
if (Outnames[u]=="QVAPOR" | Outnames[u]=="QCLOUD"){
varia=Outnames[u]
z = ncvar_get( ex.nc, varia, start=c(WRFL[1],WRFL[2],1,1), count=c((WRFU[1]-WRFL[1]+1), (WRFU[2]-WRFL[2]+1),1,length(zt)))   # variable
cat("variable ",varia,fill=TRUE)	
   for (v in 1:dim(TIN)[1]){ # for the mesh elements
   finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]
   }
   for (v in 1:dim(CE)[1]){ # for the channel elements
   finaloldC[v,u-1,]=z[sel_WRFC[v,2],sel_WRFC[v,3],]
   }
}

  
if (Outnames[u]=="TSLB"){
varia="TSLB" #Outnames[u]
z = ncvar_get( ex.nc, varia, start=c(WRFL[1],WRFL[2],soillayers,1), count=c((WRFU[1]-WRFL[1]+1), (WRFU[2]-WRFL[2]+1),1,length(zt)))   # variable
cat("variable ",varia,fill=TRUE)	
   for (v in 1:dim(TIN)[1]){ # For the mesh elements
   finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]-272.15  
   }
   for (v in 1:dim(CE)[1]){ # For the channel elements
   finaloldC[v,u-1,]=z[sel_WRFC[v,2],sel_WRFC[v,3],]-272.15  
   }
}
	  

if (Outnames[u]=="T2" | Outnames[u]=="Q2" |  Outnames[u]=="PSFC" | Outnames[u]=="VEGFRA" | Outnames[u]=="HGT" | Outnames[u]=="SWDOWN" | Outnames[u]=="GLW" | 
Outnames[u]=="POTEVP" | Outnames[u]=="PBLH" | Outnames[u]=="U10" | Outnames[u]=="V10" | Outnames[u]=="SOILTB"){
varia=Outnames[u]
z = ncvar_get( ex.nc, varia,start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(zt)))   # variable
cat("variable ",varia,fill=TRUE)	
   for (v in 1:dim(TIN)[1]){  # filling finalold
      if (varia=="T2" | varia=="SOILTB") {
      finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]-272.15	
      } else {
      finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]
      }
      if (varia=="VEGFRA"){
      finalold[v,u+1,]=(z[sel_WRF[v,2],sel_WRF[v,3],])/100  
      }
      if (varia=="GLW"){      
         if (any(firsthour=mi)){
         finalold[v,u+1,1]=z[sel_WRF[v,2],sel_WRF[v,3],2]  # first hour value is equal to second hour because this data is zero in the original WRF files
         } else {
         finalold[v,u+1,2:length(zt)]=z[sel_WRF[v,2],sel_WRF[v,3],2:length(zt)]
         }
      }
   }
	
    for (v in 1:dim(CE)[1]){  # filling finaloldC
       if (varia=="T2" | varia=="SOILTB") {
       finaloldC[v,u-1,]=z[sel_WRFC[v,2],sel_WRFC[v,3],]-272.15	
       } else {
       finaloldC[v,u-1,]=z[sel_WRFC[v,2],sel_WRFC[v,3],]
       }
       if (varia=="VEGFRA"){
       finaloldC[v,u-1,]=(z[sel_WRFC[v,2],sel_WRFC[v,3],])/100  
       }
       if (varia=="GLW"){      
         if (any(firsthour=mi)){
         finaloldC[v,u-1,1]=z[sel_WRFC[v,2],sel_WRFC[v,3],2]  # first hour value is equal to second hour because this data is zero in the original WRF files
         } else {
         finaloldC[v,u-1,2:length(zt)]=z[sel_WRFC[v,2],sel_WRFC[v,3],2:length(zt)]
         }
      }
   }
}
	

if (Outnames[u]=="MAXVEGFRA"){
aveg<<-u
varia=Outnames[u]
cat("variable ",varia," skipped for the end",fill=TRUE)
}

if (Outnames[u]=="U" | Outnames[u]=="V"){  # staggered wind speed values
varia=Outnames[u]
z = ncvar_get( ex.nc, varia, start=c(WRFL[1],WRFL[2],1,1),count=c((WRFU[1]-WRFL[1]+1), (WRFU[2]-WRFL[2]+1),1,length(zt)))   # v STAGGERED wind
cat("variable ",varia,"--Staggered",fill=TRUE)
   for (v in 1:dim(TIN)[1]){ # for the mesh elements
   finalold[v,u+1,]=z[sel_WRFS[v,2],sel_WRFS[v,3],]
   }
   for (v in 1:dim(CE)[1]){ # for the channel elements
   finaloldC[v,u-1,]=z[sel_WRFSC[v,2],sel_WRFSC[v,3],]
   }
}


if (Outnames[u]=="TPREC"){
Paccum=matrix(NA,dim(TIN)[1],length(zt)) # for the mesh elements, rows are the nodes, columns are the times
PaccumC=matrix(NA,dim(CE)[1],length(zt)) # for the mesh elements, rows are the nodes, columns are the times
zrainc = ncvar_get( ex.nc, "RAINC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(zt)))   # 
zrainsh = ncvar_get( ex.nc, "RAINSH",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(zt)))   # 
zrainnc = ncvar_get( ex.nc, "RAINNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(zt)))   # 
cat("variable  TPREC",fill=TRUE)
for (v in 1:dim(TIN)[1]){ # For the meshelements matrix
    if ((mi %in% firsthour) == TRUE){  # the first element does not have to be subtracted from the last hour's cumulative rainfall value at that node
       if (v==1) {
       cat("Computing first hour of first month for Precip", fill=TRUE)
       }
    finalold[v,u+1,1]=(zrainc[sel_WRF[v,2],sel_WRF[v,3],1]+ zrainsh[sel_WRF[v,2],sel_WRF[v,3],1] + zrainnc[sel_WRF[v,2],sel_WRF[v,3],1])*(1/3600000) # mm/h to m/s
    } else {
    finalold[v,u+1,1]=((zrainc[sel_WRF[v,2],sel_WRF[v,3],1]+ zrainsh[sel_WRF[v,2],sel_WRF[v,3],1] + zrainnc[sel_WRF[v,2],sel_WRF[v,3],1])-(Paccumold[v,dim(Paccumold)[2]]))*(1/3600000) # mm/h to m/s  
    }
    Paccum[v,]=zrainc[sel_WRF[v,2],sel_WRF[v,3],]+ zrainsh[sel_WRF[v,2],sel_WRF[v,3],] + zrainnc[sel_WRF[v,2],sel_WRF[v,3],]
    P1=Paccum[v,(2:length(zt))]
    P0=Paccum[v,(1:(length(zt)-1))]
    finalold[v,u+1,(2:length(zt))]=(P1-P0)*(1/3600000) # mm/h to m/s
}
      
if (min(finalold[,u+1,])<0) {
cat("Negative PRECIP found in finalold at mi=",mi,fill=TRUE)
#Sys.sleep(600000)
}
  
Paccumold=Paccum    

for (v in 1:dim(CE)[1]){ # For the channel elements matrix
    if ((mi %in% firsthour) == TRUE){  # the first element does not have to be subtracted from the last hour's cumulative rainfall value at that node
       if (v==1) {
       cat("Computing first hour of first month for Precip", fill=TRUE)
       }
    finaloldC[v,u-1,1]=(zrainc[sel_WRFC[v,2],sel_WRFC[v,3],1]+ zrainsh[sel_WRFC[v,2],sel_WRFC[v,3],1] + zrainnc[sel_WRFC[v,2],sel_WRFC[v,3],1])*(1/3600000) # mm/h to m/s
    } else {
    finaloldC[v,u-1,1]=((zrainc[sel_WRFC[v,2],sel_WRFC[v,3],1]+ zrainsh[sel_WRFC[v,2],sel_WRFC[v,3],1] + zrainnc[sel_WRFC[v,2],sel_WRFC[v,3],1])-(PaccumoldC[v,dim(PaccumoldC)[2]]))*(1/3600000) # mm/h to m/s  
    }
    PaccumC[v,]=zrainc[sel_WRFC[v,2],sel_WRFC[v,3],]+ zrainsh[sel_WRFC[v,2],sel_WRFC[v,3],] + zrainnc[sel_WRFC[v,2],sel_WRFC[v,3],]
    P1C=PaccumC[v,(2:length(zt))]
    P0C=PaccumC[v,(1:(length(zt)-1))]
    finaloldC[v,u-1,(2:length(zt))]=(P1C-P0C)*(1/3600000) # mm/h to m/s
    }
    	  
if (min(finaloldC[,u-1,])<0) {
cat("Negative PRECIP found in finaloldC at mi=",mi,fill=TRUE)
#Sys.sleep(600000)	
}		

PaccumoldC=PaccumC
}

#	if (Outnames[u]=="FRAC_FROZ_PREC"){
#	Saccum=matrix(NA,dim(TIN)[1],length(zt)) # rows are the nodes, columns are the times
#        zsnownc = ncvar_get( ex.nc, "SNOWNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(zt)))   #
#	zgraupelnc = ncvar_get( ex.nc, "GRAUPELNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(zt)))   #\
# 	zhailnc = ncvar_get( ex.nc, "HAILNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(zt)))   #\
#	cat("variable  FRAC_FROZ_PREC",fill=TRUE)
#	for (v in 1:dim(TIN)[1]){
#	  if (mi==1){
#         finalold[v,u+1,1]=(zsnownc[sel_WRF[v,2],sel_WRF[v,3],1]+ zgraupelnc[sel_WRF[v,2],sel_WRF[v,3],1] + zhailnc[sel_WRF[v,2],sel_WRF[v,3],1])*(1/3600000) # mm/h to m/s
#	  } else {
#	  finalold[v,u+1,1]=((zsnownc[sel_WRF[v,2],sel_WRF[v,3],1]+ zgraupelnc[sel_WRF[v,2],sel_WRF[v,3],1] + zhailnc[sel_WRF[v,2],sel_WRF[v,3],1])-(Saccumold[v,dim(Saccumold)[2]]))*(1/3600000) # mm/h to m/s    
#          }
#          Saccum[v,]=zsnownc[sel_WRF[v,2],sel_WRF[v,3],]+ zgraupelnc[sel_WRF[v,2],sel_WRF[v,3],] + zhailnc[sel_WRF[v,2],sel_WRF[v,3],]
#	  S1=Saccum[v,2:length(zt)]
#	  S0=Saccum[v,1:(length(zt)-1)]
#	  finalold[v,u+1,(2:length(zt))]=(S1-S0)*(1/3600000) # mm/h to m/s
#          }
#	}

}


# Creating temporal WRFchar variable
if (mi==1){
  WRFchar=finalold[1,2,]
} else {
  WRFchar=c(WRFchar,finalold[1,2,])
}


if (mi==1){
rowsfinal=dim(finalold)[3]
exportname=finalold[1,2,1]
} else {
rowsfinal=dim(finalold)[3]+rowsfinal
}


##### Exporting temporary NETCDF files ###################################################################################################################################################
# define the netcdf coordinate variables -- note that these have values!
cat("--Exporting final NETCDF file to Outputfolder",fill=TRUE)
if (mi==1){  # the first time we have to define the dimensions of the array
nfinal= finalold[1,2,1]
dim1 = ncdim_def( "Mesh_Elements","", as.integer(finalold[,1,1]))
dim2= ncdim_def( "Time",paste("Hours since ",finalold[1,2,1]),seq(0,(dim(finalold)[3])-1),unlim=TRUE) #
dim3 = ncdim_def( "Channel_Elements","", as.integer(finaloldC[,1,1]))

# define the EMPTY netcdf variables
#varNODES=ncvar_def("NODES","", list(dim1), -99999, longname="NODES")
#varWRFHOUR=ncvar_def("WRFHOUR","hours", list(dim2),longname="WRFHOUR",prec='char')
varJULTIME=ncvar_def("JULTIME","days", list(dim2), -99999, longname="JULTIME", prec="double") #,prec="double")
varT2 = ncvar_def("T2","Celsius", list(dim1,dim2), -99999, longname="T2")
varT2_C = ncvar_def("T2_C","Celsius", list(dim3,dim2), -99999, longname="T2_C")
varQVAPOR = ncvar_def("QVAPOR","kg kg-1", list(dim1,dim2), -99999, longname="QVAPOR")
varQVAPOR_C = ncvar_def("QVAPOR_C","kg kg-1", list(dim3,dim2), -99999, longname="QVAPOR_C")
varQCLOUD = ncvar_def("QCLOUD","kg kg-1", list(dim1,dim2), -99999, longname="QCLOUD")
varQCLOUD_C = ncvar_def("QCLOUD_C","kg kg-1", list(dim3,dim2), -99999, longname="QCLOUD_C")
varPSFC = ncvar_def("PSFC","Pa", list(dim1,dim2), -99999, longname="PSFC")
varPSFC_C = ncvar_def("PSFC_C","Pa", list(dim3,dim2), -99999, longname="PSFC_C")
varU = ncvar_def("U","ms-1", list(dim1,dim2), -99999, longname="U")
varU_C = ncvar_def("U_C","m s-1", list(dim3,dim2), -99999, longname="U_C")
varV = ncvar_def("V","m s-1", list(dim1,dim2), -99999, longname="V")
varV_C = ncvar_def("V_C","m s-1", list(dim3,dim2), -99999, longname="V_C")
varVEGFRA = ncvar_def("VEGFRA","m2 m-2", list(dim1,dim2), -99999, longname="VEGFRA")
varVEGFRA_C = ncvar_def("VEGFRA_C","m2 m-2", list(dim3,dim2), -99999, longname="VEGFRA_C")
varMAXVEGFRA = ncvar_def("MAXVEGFRA","m2 m-2", list(dim1,dim2), -99999, longname="MAXVEGFRA")
varMAXVEGFRA_C = ncvar_def("MAXVEGFRA_C","m2 m-2", list(dim3,dim2), -99999, longname="MAXVEGFRA_C")
varTPREC = ncvar_def("TPREC","m s-1", list(dim1,dim2), -99999, longname="TPREC")
varTPREC_C = ncvar_def("TPREC_C","m s-1", list(dim3,dim2), -99999, longname="TPREC_C")
varSWDOWN = ncvar_def("SWDOWN","W m-2", list(dim1,dim2), -99999, longname="SWDOWN")
varSWDOWN_C = ncvar_def("SWDOWN_C","W m-2", list(dim3,dim2), -99999, longname="SWDOWN_C")
varGLW = ncvar_def("GLW","W m-2", list(dim1,dim2), -99999, longname="GLW")
varGLW_C = ncvar_def("GLW_C","W m-2", list(dim3,dim2), -99999, longname="GLW_C")
varPBLH = ncvar_def("PBLH","m", list(dim1,dim2), -99999, longname="PBLH")
varPBLH_C = ncvar_def("PBLH_C","m", list(dim3,dim2), -99999, longname="PBLH_C")
varTSLB = ncvar_def("TSLB","Celsius", list(dim1,dim2), -99999, longname="TSLB")
varTSLB_C = ncvar_def("TSLB_C","Celsius", list(dim3,dim2), -99999, longname="TSLB_C")

# associate the netcdf variable with a netcdf file   
# put the variable into the file, and
# close

nc.ex = nc_create(paste(Outfolder,Outncfile,"_",exportname,".nc",sep=""),list(varJULTIME, varT2, varT2_C, varQVAPOR, varQVAPOR_C, varQCLOUD, varQCLOUD_C,
        varPSFC, varPSFC_C, varU, varU_C, varV, varV_C, varVEGFRA, varVEGFRA_C, varMAXVEGFRA, varMAXVEGFRA_C, varTPREC, varTPREC_C, varSWDOWN, varSWDOWN_C, 
        varGLW, varGLW_C, varPBLH, varPBLH_C, varTSLB, varTSLB_C),force_v4=TRUE)
#ncvar_put(nc.ex, varNODES, finalold[,which(ordencols=="node"),1])
}

ncvar_put(nc.ex, varJULTIME, as.numeric(finalold[1,which(ordencols=="JULTIME"),]),start=tini,count=length(zt))
#ncvar_put(nc.ex, varWRFHOUR, finalold[1,which(ordencols=="WRFHOUR"),],start=tini,count=length(zt))
ncvar_put(nc.ex, varT2, finalold[,which(ordencols=="T2"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varT2_C, finaloldC[,which(ordencolsC=="T2"),],start=c(1,tini),count=c(dim(CE)[1],length(zt)))
ncvar_put(nc.ex, varQVAPOR, finalold[,which(ordencols=="QVAPOR"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varQVAPOR_C, finaloldC[,which(ordencolsC=="QVAPOR"),],start=c(1,tini),count=c(dim(CE)[1],length(zt)))
ncvar_put(nc.ex, varQCLOUD, as.double(finalold[,which(ordencols=="QCLOUD"),]),start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varQCLOUD_C, as.double(finaloldC[,which(ordencolsC=="QCLOUD"),]),start=c(1,tini),count=c(dim(CE)[1],length(zt)))
ncvar_put(nc.ex, varPSFC, finalold[,which(ordencols=="PSFC"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varPSFC_C, finaloldC[,which(ordencolsC=="PSFC"),],start=c(1,tini),count=c(dim(CE)[1],length(zt)))
ncvar_put(nc.ex, varU, finalold[,which(ordencols=="U"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varU_C, finaloldC[,which(ordencolsC=="U"),],start=c(1,tini),count=c(dim(CE)[1],length(zt)))
ncvar_put(nc.ex, varV, finalold[,which(ordencols=="V"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varV_C, finaloldC[,which(ordencolsC=="V"),],start=c(1,tini),count=c(dim(CE)[1],length(zt)))
ncvar_put(nc.ex, varVEGFRA, finalold[,which(ordencols=="VEGFRA"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varVEGFRA_C, finaloldC[,which(ordencolsC=="VEGFRA"),],start=c(1,tini),count=c(dim(CE)[1],length(zt)))
ncvar_put(nc.ex, varTPREC, finalold[,which(ordencols=="TPREC"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varTPREC_C, finaloldC[,which(ordencolsC=="TPREC"),],start=c(1,tini),count=c(dim(CE)[1],length(zt)))
ncvar_put(nc.ex, varSWDOWN, finalold[,which(ordencols=="SWDOWN"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varSWDOWN_C, finaloldC[,which(ordencolsC=="SWDOWN"),],start=c(1,tini),count=c(dim(CE)[1],length(zt)))
ncvar_put(nc.ex, varGLW, finalold[,which(ordencols=="GLW"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varGLW_C, finaloldC[,which(ordencolsC=="GLW"),],start=c(1,tini),count=c(dim(CE)[1],length(zt)))
ncvar_put(nc.ex, varPBLH, finalold[,which(ordencols=="PBLH"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varPBLH_C, finaloldC[,which(ordencolsC=="PBLH"),],start=c(1,tini),count=c(dim(CE)[1],length(zt)))
ncvar_put(nc.ex, varTSLB, finalold[,which(ordencols=="TSLB"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varTSLB_C, finaloldC[,which(ordencolsC=="TSLB"),],start=c(1,tini),count=c(dim(CE)[1],length(zt)))
}

### Before closing the file, compute MAXVEGFRAC if requested
if (length(which(Outnames=="MAXVEGFRA"))>0){
    cat("Adding MAXVEGFRA to finalold netcdf file",fill=TRUE)
 
   cat("Deciding what is a best way (size) to read the final nc file", fill=TRUE)
   #memoryfinalold=object.size(finalold)
   projectedmemory=4*((length(TINX)+length(CEX))/36861)*(length(WRFchar)/96488)*(32) # 30 GB are achieved when 36861 nodes and 96488 hours are present
   cat("Projected Mememory in GB is", projectedmemory,fill=TRUE)
   
   cat("Identify how many different years involved in the simulation",fill=TRUE)
   anos=rep(NA,length(WRFchar))
   
   for (yi in 1:length(WRFchar)){
     pa= substring(WRFchar[yi], 1:nchar(WRFchar[yi]), 1:nchar(WRFchar[yi]))
     anos[yi]=paste(pa[1],pa[2],pa[3],pa[4],sep="")    
   }
   difano=unique(anos) # how many unique anos
         
   
  cat("The final nc matrix will have to be read in ",ceiling(projectedmemory/26),"  chunks",sep="",fill=TRUE) # 26 GB is a flexible number and depends on the type of computer and total memory available to read R objects
  chunks=ceiling(projectedmemory/26) 
  
  if (chunks > 1){ 
  startpedazos=floor(seq(from=1,to=dim(TINX)[1],length.out=chunks+1)) #by=floor(dim(TINX)[1]/chunks)
  startpedazosC=floor(seq(from=1,to=dim(CEX)[1],length.out=chunks+1)) #by=floor(dim(TINX)[1]/chunks)
  
  startpedazos[2:length(startpedazos)]=startpedazos[2:length(startpedazos)]+1
  startpedazosC[2:length(startpedazosC)]=startpedazosC[2:length(startpedazosC)]+1

  sp1=c(startpedazos[2:length(startpedazos)],startpedazos[length(startpedazos)])  
  countpedazos=sp1-startpedazos

  sp1C=c(startpedazosC[2:length(startpedazosC)],startpedazosC[length(startpedazosC)])  
  countpedazosC=sp1C-startpedazosC
  } else {
  startpedazos=1  
  countpedazos=dim(TINX)[1]  
  startpedazosC=1  
  countpedazosC=dim(CEX)[1]  
  }
  
       for (chu in 1:chunks){   # filling first varMAXVEGFRA
        cat("Reading chunk # ",chu,fill=TRUE)
        z = ncvar_get( nc.ex, "VEGFRA", start=c(startpedazos[chu],1),count=c(countpedazos[chu],length(WRFchar)))   # Reads the netcdf file recently created  
        cat("Creating nuevaMAXVEG of chunk # ",chu,fill=TRUE)
        nuevaMAXVEG=matrix(NA,countpedazos[chu],length(WRFchar))   # matrix of nodes x times
        
                cat("Filling in nuevaMAXVEG with elements of chunk # ",chu,fill=TRUE)
                for (da in 1:length(difano)){
                dondeigual=grep(difano[da],WRFchar)
                for (nodis in 1:dim(z)[1]){
                nuevaMAXVEG[nodis,dondeigual]=max(z[nodis,dondeigual])  
                }
      	        }
       cat("Writing values of chunk # ",chu," to nc file",fill=TRUE)
       ncvar_put(nc.ex, varMAXVEGFRA, nuevaMAXVEG,start=c(startpedazos[chu],1),count=c(countpedazos[chu],length(WRFchar)))
       }    

       for (chu in 1:chunks){   # filling secondly varMAXVEGFRA_C
        cat("Reading chunk # ",chu,fill=TRUE)
        z = ncvar_get( nc.ex, "VEGFRA_C", start=c(startpedazosC[chu],1),count=c(countpedazosC[chu],length(WRFchar)))   # Reads the netcdf file recently created  
        cat("Creating nuevaMAXVEG_C of chunk # ",chu,fill=TRUE)
        nuevaMAXVEG=matrix(NA,countpedazosC[chu],length(WRFchar))   # matrix of nodes x times
        
                cat("Filling in nuevaMAXVEG_C with elements of chunk # ",chu,fill=TRUE)
      	        for (da in 1:length(difano)){
      	        dondeigual=grep(difano[da],WRFchar)
                for (nodis in 1:dim(z)[1]){
                nuevaMAXVEG[nodis,dondeigual]=max(z[nodis,dondeigual])  
                }
      	        }
       cat("Writing values of chunk # ",chu," to nc file",fill=TRUE)
       ncvar_put(nc.ex, varMAXVEGFRA_C, nuevaMAXVEG,start=c(startpedazosC[chu],1),count=c(countpedazosC[chu],length(WRFchar)))
       }    

}    
nc_close(nc.ex) 

#### Verification of generated  nc file:
#### Basin average for verification. Will have to read values from written matrices
ex.nco = nc_open(paste(Outfolder,Outncfile,"_",exportname,".nc",sep="")) # it opens the recently created nc file
Rdumpo=capture.output(print(ex.nco), file = NULL, append = FALSE)
write.table(Rdumpo,paste(Outfolder,"Rncdumpo.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   ## dumps netcdf headers to a file in outfolder  

cat("**********Averaging basin values for verification",fill=TRUE)
Avefinal=matrix(NA,rowsfinal,(length(nccolumns)-2)) ## matrix containing the variables in the columns (not includign  node, WRFHOUR or JULTIME) and times in the rows
colnames(Avefinal)=nccolumns[3:length(nccolumns)]

for (vi in 1:(length(nccolumns)-2)){  # by columns
  cat("Averaging column ",vi, " of ",length(nccolumns)-2, fill=TRUE)
  z = ncvar_get( ex.nco, nccolumns[vi+2])   # variable
  for (ti in 1:rowsfinal){	
      if (nccolumns[vi+2]=="MAXVEGFRA" || nccolumns[vi+2]=="MAXVEGFRA_C" ){
      Avefinal[ti,vi]=max(as.numeric(z[,ti]))	
      } else {
      Avefinal[ti,vi]=mean(as.numeric(z[,ti]))  # FIX ME and CHECK ME 	
      }
  }
}
cat("Time at this moment is ",date(),fill=TRUE)
### PLotting basin average values# 

if ((length(nccolumns)-2)<=5){
postscript(paste(Outfolder,"Basin_Average_TS.ps",sep=""),width=8000,height=5500)
mat=matrix(1:5,5,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 1:(length(nccolumns)-2)){
plot(Avefinal[,pli],type="l",main=nccolumns[pli+2])
}
dev.off()
}

if ((length(nccolumns)-2)<=10 & (length(nccolumns)-2)>5){
postscript(paste(Outfolder,"Basin_Average_TS1.ps",sep=""),width=8000,height=5500)
mat=matrix(1:5,5,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 1:5){
plot(Avefinal[,pli],type="l",main=nccolumns[pli+2])
}
postscript(paste(Outfolder,"Basin_Average_TS2.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 6:dim(Avefinal)[2]){
plot(Avefinal[,pli],type="l",main=nccolumns[pli+2])
}
}

if ((length(nccolumns)-2)<=15 & (length(nccolumns)-2)>10){
postscript(paste(Outfolder,"Basin_Average_TS1.ps",sep=""),width=8000,height=5500)
mat=matrix(1:5,5,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 1:5){
plot(Avefinal[,pli],type="l",main=nccolumns[pli+2])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS2.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 6:10){
plot(Avefinal[,pli],type="l",main=nccolumns[pli+2])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS3.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 11:dim(Avefinal)[2]){
plot(Avefinal[,pli],type="l",main=nccolumns[pli+2])
}
dev.off()
}


if ((length(nccolumns)-2)<=30 & (length(nccolumns)-2)>25){
postscript(paste(Outfolder,"Basin_Average_TS1.ps",sep=""),width=8000,height=5500)
mat=matrix(1:5,5,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 1:5){
plot(Avefinal[,pli],type="l",main=nccolumns[pli+2])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS2.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 6:10){
plot(Avefinal[,pli],type="l",main=nccolumns[pli+2])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS3.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 11:15){
plot(Avefinal[,pli],type="l",main=nccolumns[pli+2])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS4.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 16:20){
plot(Avefinal[,pli],type="l",main=nccolumns[pli+2])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS5.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 21:25){
plot(Avefinal[,pli],type="l",main=nccolumns[pli+2])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS6.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 26:dim(Avefinal)[2]){
plot(Avefinal[,pli],type="l",main=nccolumns[pli+2])
}
dev.off()

}



