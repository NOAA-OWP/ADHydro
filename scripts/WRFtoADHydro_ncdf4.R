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
################ INPUT MODULE #####################################################################################################################################################

WRF_Folder<<-"/localstore/MtMoranLink/project/CI-WATER/data/WRF/WRF_output_new/historical/"  # path to the WRF files
WRF_Files<<-c("2000_new/wrfout_d03_2000-01-01_00:00:00", "2000_new/wrfout_d03_2000-01-31_12:00:00", "2000_new/wrfout_d03_2000-03-02_00:00:00", "2000_new/wrfout_d03_2000-04-01_12:00:00",
 	       "2000_new/wrfout_d03_2000-05-02_00:00:00", "2000_new/wrfout_d03_2000-06-01_12:00:00", "2000_new/wrfout_d03_2000-07-02_00:00:00", "2000_new/wrfout_d03_2000-08-01_12:00:00",
       	"2000_new/wrfout_d03_2000-09-01_00:00:00", "2000_new/wrfout_d03_2000-10-01_12:00:00", "2000_new/wrfout_d03_2000-11-01_00:00:00", "2000_new/wrfout_d03_2000-12-01_12:00:00")
        "2001_new/wrfout_d03_2001-01-01_00:00:00", "2001_new/wrfout_d03_2001-01-31_12:00:00", "2001_new/wrfout_d03_2001-03-03_00:00:00", "2001_new/wrfout_d03_2001-04-02_12:00:00",
        "2001_new/wrfout_d03_2001-05-03_00:00:00", "2001_new/wrfout_d03_2001-06-02_12:00:00", "2001_new/wrfout_d03_2001-07-03_00:00:00", "2001_new/wrfout_d03_2001-08-02_12:00:00",
	      "2001_new/wrfout_d03_2001-09-02_00:00:00", "2001_new/wrfout_d03_2001-10-02_12:00:00", "2001_new/wrfout_d03_2001-11-02_00:00:00", "2001_new/wrfout_d03_2001-12-02_12:00:00",
        "2002_new/wrfout_d03_2002-01-01_00:00:00", "2002_new/wrfout_d03_2002-01-31_12:00:00", "2002_new/wrfout_d03_2002-03-03_00:00:00", "2002_new/wrfout_d03_2002-04-02_12:00:00",
        "2002_new/wrfout_d03_2002-05-03_00:00:00", "2002_new/wrfout_d03_2002-06-02_12:00:00", "2002_new/wrfout_d03_2002-07-03_00:00:00", "2002_new/wrfout_d03_2002-08-02_12:00:00"
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
       
TINF<<-"/share/CI-WATER Simulation Data/small_green_mesh/geometry.nc"  # netcdf file with the AD-Hydro mesh elements for which the WRF data will be extracted
RE=6378137 		# in meters; Earth's radius at the equator GRS80 ellipsoid
Lambda00<<--109 	# reference meridian in degrees. Sign indicates if it's west or east of Greenwich
FalseEast<<-20000000 	# False East of desired coordinate system
FalseNorth<<-10000000	# False North of  desired coordinate system
cell_Buffer<<-4 	# number of cells to be added to the selected rectangular from the minimum and maximum TIN points to efficiently read the WRF data. Use a number >2
soillayers<<-4  	# number of total staggered soil layers. It can be extracted from the Rncdump.txt file from line 39
JulOrigin<<-"2000-01-01 00:00:00" # origin for the Julian date column
Outnames<<-c("WRFHOUR","JULTIME","T2","QVAPOR","QCLOUD","PSFC","U","V","VEGFRA","MAXVEGFRA","TPREC","SWDOWN","GLW","PBLH","TSLB")  # Variables process + WRFHOUR and JULTIME 
Outfolder<<-"/share/CI-WATER Simulation Data/WRF_to_ADHydro/"	# Folder name for final netcdf output
Outncfile<<-"WRF_ADHydro_Small_Green_River" # Base name for output file


############### MODULE METADATA AND HEADERS #######################################################################################################################################

cat("Time started processing ",date(),fill=TRUE)
ex.nc = nc_open(paste(WRF_Folder, WRF_Files[1], sep="")) # it opens the first file
Rdump=capture.output(print(ex.nc), file = NULL, append = FALSE)
write.table(Rdump,paste(Outfolder,"Rncdump.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   ## dumps netcdf headers to a file in outfolder  
cat("A dump file has been created in ", Outfolder," called Rncdump",fill=TRUE)
############## MODULE COORDINATES CONVERSION AND BOUNDING BOX SELECTION ###########################################################################################################

mesh.nc = nc_open(TINF) # it opens the geometry file
#Rdump_mesh=capture.output(print(mesh.nc), file = NULL, append = FALSE)
#write.table(Rdump_mesh,paste(Outfolder,"Rncdump_mesh.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   ## dumps netcdf headers of geometry to a file in outfolder  
TINX = ncvar_get(mesh.nc, "meshElementX") 
TINY = ncvar_get(mesh.nc, "meshElementY")
TIN= cbind(seq(0,(length(TINX)-1)),TINX,TINY)

x11()
par(mfcol=c(1,2))
plot(TIN[,2],TIN[,3],pch=19,cex=0.2,main="Basin domain")

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
minxTIN=min(TIN[,2])
maxxTIN=max(TIN[,2])
minyTIN=min(TIN[,3])
maxyTIN=max(TIN[,3])

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

# Extraction of the cells of WRF that contain each of the TIN nodes for ADHydro
sel_WRF=matrix(NA,dim(TIN)[1],3) # Node, row, col
sel_WRF[,1]=TIN[,1]
mindi=rep(NA,dim(TIN)[1])
cat("** Extracting distances from ADHydro nodes to WRF cells.. may take a few minutes...",fill=TRUE)
for (ni in 1:dim(TIN)[1]){
dist=(((TIN[ni,2]-xss)^2)+((TIN[ni,3]-yss)^2))^0.5
mind=min(dist)
mindi[ni]=mind
wmind=which(dist==mind,arr.ind=TRUE)
sel_WRF[ni,2]=wmind[1] # row
sel_WRF[ni,3]=wmind[2] # col
}

cat("** Maximum distance between an ADHydro node and WRF cell center is", max(mindi)," m",fill=TRUE)

plot(WRFL[1]+sel_WRF[,2],WRFL[2]+sel_WRF[,3],xlim=c(1,colss),ylim=c(1,rowss),cex=0.2,pch=20,main="Basin domain comparison")

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


# Extraction of the cells of WRF that contain each of the TIN nodes for AD-Hydro
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
    }else {
     tini=tf+1
     tf=tini+length(zt)-1
    }
  cat("mi=",mi,", tini =",tini,", tf=",tf,", length(zt)=",length(zt),fill=TRUE)
  
  # According to the TIMES dimension of the first matrix it creates a new array with this dimension
  finalold<<-array(NA, c(dim(TIN)[1],1+length(Outnames),length(zt)))  # initial column is node number for ADHydro
	ordencols=c("node",Outnames)	
	colnames(finalold)=ordencols
	finalold[,1,]=TIN[,1] # first column are the nodes
	cat("variable WRFHOUR",fill=TRUE)
		for (ti in 1:length(zt)){	
		finalold[,u+1,ti]=rep(zt[ti],dim(finalold)[1])
		}
	}

	if (Outnames[u]=="JULTIME"){
	  cat("variable JULTIME",fill=TRUE)
    for (zi in 1:length(zt)){
      pa= substring(zt[zi], 1:nchar(zt[zi]), 1:nchar(zt[zi]))
      prejul=timeDate(paste(pa[1],pa[2],pa[3],pa[4],pa[5],pa[6],pa[7],pa[8],pa[9],pa[10]," ",pa[12],pa[13],pa[14],pa[15], pa[16], pa[17],pa[18],pa[19],sep=""))    
      finalold[,u+1,zi]=rep(as.numeric(julian(prejul,origin=timeDate(JulOrigin),units="secs")),dim(finalold)[1])
      }
  }
	
    
	if (Outnames[u]=="QVAPOR" | Outnames[u]=="QCLOUD"){
        varia=Outnames[u]
        z = ncvar_get( ex.nc, varia, start=c(WRFL[1],WRFL[2],1,1), count=c((WRFU[1]-WRFL[1]+1), (WRFU[2]-WRFL[2]+1),1,length(zt)))   # variable
	cat("variable ",varia,fill=TRUE)	
		for (v in 1:dim(TIN)[1]){
	        finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]
		}
	}

  
	if (Outnames[u]=="TSLB"){
	  varia="TSLB" #Outnames[u]
	  z = ncvar_get( ex.nc, varia, start=c(WRFL[1],WRFL[2],soillayers,1), count=c((WRFU[1]-WRFL[1]+1), (WRFU[2]-WRFL[2]+1),1,length(zt)))   # variable
	  cat("variable ",varia,fill=TRUE)	
	  for (v in 1:dim(TIN)[1]){
	    finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]-272.15  
	  }
	}
	  

	if (Outnames[u]=="T2" | Outnames[u]=="Q2" |  Outnames[u]=="PSFC" | Outnames[u]=="VEGFRA" | Outnames[u]=="HGT" | Outnames[u]=="SWDOWN" | Outnames[u]=="GLW" | 
        Outnames[u]=="POTEVP" | Outnames[u]=="PBLH" | Outnames[u]=="U10" | Outnames[u]=="V10" | Outnames[u]=="SOILTB"){
        varia=Outnames[u]
        z = ncvar_get( ex.nc, varia,start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(zt)))   # variable
	cat("variable ",varia,fill=TRUE)	
		for (v in 1:dim(TIN)[1]){
		if (varia=="T2" | varia=="SOILTB") {
		finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]-272.15	
		} else {
    finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]
		}
    if (varia=="VEGFRA"){
    finalold[v,u+1,]=(z[sel_WRF[v,2],sel_WRF[v,3],])/100  
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
		for (v in 1:dim(TIN)[1]){
                finalold[v,u+1,]=z[sel_WRFS[v,2],sel_WRFS[v,3],]
		}
       }


	if (Outnames[u]=="TPREC"){
	Paccum=matrix(NA,dim(TIN)[1],length(zt)) # rows are the nodes, columns are the times
  zrainc = ncvar_get( ex.nc, "RAINC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(zt)))   # 
	zrainsh = ncvar_get( ex.nc, "RAINSH",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(zt)))   # 
	zrainnc = ncvar_get( ex.nc, "RAINNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(zt)))   # 
	cat("variable  TPREC",fill=TRUE)
	for (v in 1:dim(TIN)[1]){
		if (mi==1){  # the first element does not have to be subtracted from the last hour's cumulative rainfall value at that node
    finalold[v,u+1,1]=(zrainc[sel_WRF[v,2],sel_WRF[v,3],1]+ zrainsh[sel_WRF[v,2],sel_WRF[v,3],1] + zrainnc[sel_WRF[v,2],sel_WRF[v,3],1])*(1/3600000) # mm/h to m/s
		} else {
		finalold[v,u+1,1]=((zrainc[sel_WRF[v,2],sel_WRF[v,3],1]+ zrainsh[sel_WRF[v,2],sel_WRF[v,3],1] + zrainnc[sel_WRF[v,2],sel_WRF[v,3],1])-(Paccumold[v,dim(Paccumold)[2]]))*(1/3600000) # mm/h to m/s  
		}
    Paccum[v,]=zrainc[sel_WRF[v,2],sel_WRF[v,3],]+ zrainsh[sel_WRF[v,2],sel_WRF[v,3],] + zrainnc[sel_WRF[v,2],sel_WRF[v,3],]
		P1=Paccum[v,(2:length(zt))]
    P0=Paccum[v,(1:(length(zt)-1))]
		finalold[v,u+1,(2:length(zt))]=(P1-P0)*(1/3600000) # mm/h to m/s
    }
  Paccumold=Paccum
	}

	if (Outnames[u]=="FRAC_FROZ_PREC"){
	Saccum=matrix(NA,dim(TIN)[1],length(zt)) # rows are the nodes, columns are the times
  zsnownc = ncvar_get( ex.nc, "SNOWNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(zt)))   #\
	zgraupelnc = ncvar_get( ex.nc, "GRAUPELNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(zt)))   #\
 	zhailnc = ncvar_get( ex.nc, "HAILNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(zt)))   #\
	cat("variable  FRAC_FROZ_PREC",fill=TRUE)
	for (v in 1:dim(TIN)[1]){
	  if (mi==1){
    finalold[v,u+1,1]=(zsnownc[sel_WRF[v,2],sel_WRF[v,3],1]+ zgraupelnc[sel_WRF[v,2],sel_WRF[v,3],1] + zhailnc[sel_WRF[v,2],sel_WRF[v,3],1])*(1/3600000) # mm/h to m/s
	  } else {
	  finalold[v,u+1,1]=((zsnownc[sel_WRF[v,2],sel_WRF[v,3],1]+ zgraupelnc[sel_WRF[v,2],sel_WRF[v,3],1] + zhailnc[sel_WRF[v,2],sel_WRF[v,3],1])-(Saccumold[v,dim(Saccumold)[2]]))*(1/3600000) # mm/h to m/s    
    }
    Saccum[v,]=zsnownc[sel_WRF[v,2],sel_WRF[v,3],]+ zgraupelnc[sel_WRF[v,2],sel_WRF[v,3],] + zhailnc[sel_WRF[v,2],sel_WRF[v,3],]
	  S1=Saccum[v,2:length(zt)]
	  S0=Saccum[v,1:(length(zt)-1)]
	  finalold[v,u+1,(2:length(zt))]=(S1-S0)*(1/3600000) # mm/h to m/s
    }
	}

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
dim1 = ncdim_def( "Nodes","", as.integer(finalold[,1,1]))
dim2= ncdim_def( "Time",paste("Hours since ",finalold[1,2,1]),seq(1,(dim(finalold)[3])),unlim=TRUE) #


# define the EMPTY netcdf variables
varNODES=ncvar_def("NODES","", list(dim1), -99999, longname="NODES")
#varWRFHOUR=ncvar_def("WRFHOUR","hours", list(dim2),longname="WRFHOUR",prec='char')
varJULTIME=ncvar_def("JULTIME","secs", list(dim2), -99999, longname="JULTIME")
varT2 = ncvar_def("T2","Celsius", list(dim1,dim2), -99999, longname="T2")
varQVAPOR = ncvar_def("QVAPOR","kg kg-1", list(dim1,dim2), -99999, longname="QVAPOR")
varQCLOUD = ncvar_def("QCLOUD","kg kg-1", list(dim1,dim2), -99999, longname="QCLOUD")
varPSFC = ncvar_def("PSFC","Pa", list(dim1,dim2), -99999, longname="PSFC")
varU = ncvar_def("U","ms-1", list(dim1,dim2), -99999, longname="U")
varV = ncvar_def("V","ms-1", list(dim1,dim2), -99999, longname="V")
varVEGFRA = ncvar_def("VEGFRA","", list(dim1,dim2), -99999, longname="VEGFRA")
varMAXVEGFRA = ncvar_def("MAXVEGFRA","", list(dim1,dim2), -99999, longname="MAXVEGFRA")
varTPREC = ncvar_def("TPREC","ms-1", list(dim1,dim2), -99999, longname="TPREC")
varSWDOWN = ncvar_def("SWDOWN","W m-2", list(dim1,dim2), -99999, longname="SWDOWN")
varGLW = ncvar_def("GLW","W m-2", list(dim1,dim2), -99999, longname="GLW")
varPBLH = ncvar_def("PBLH","m", list(dim1,dim2), -99999, longname="PBLH")
varTSLB = ncvar_def("TSLB","Celsius", list(dim1,dim2), -99999, longname="TSLB")
# associate the netcdf variable with a netcdf file   
# put the variable into the file, and
# close

nc.ex = nc_create(paste(Outfolder,Outncfile,"_",exportname,".nc",sep=""),list(varNODES, varJULTIME, varT2, varQVAPOR, varQCLOUD,
        varPSFC, varU, varV, varVEGFRA, varMAXVEGFRA, varTPREC, varSWDOWN, varGLW, varPBLH, varTSLB),force_v4=TRUE)
ncvar_put(nc.ex, varNODES, finalold[,which(ordencols=="node"),1])
}

ncvar_put(nc.ex, varJULTIME, finalold[1,which(ordencols=="JULTIME"),],start=tini,count=length(zt))
#ncvar_put(nc.ex, varWRFHOUR, finalold[1,which(ordencols=="WRFHOUR"),],start=tini,count=length(zt))
ncvar_put(nc.ex, varT2, finalold[,which(ordencols=="T2"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varQVAPOR, finalold[,which(ordencols=="QVAPOR"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varQCLOUD, as.double(finalold[,which(ordencols=="QCLOUD"),]),start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varPSFC, finalold[,which(ordencols=="PSFC"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varU, finalold[,which(ordencols=="U"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varV, finalold[,which(ordencols=="V"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varVEGFRA, finalold[,which(ordencols=="VEGFRA"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varTPREC, finalold[,which(ordencols=="TPREC"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varSWDOWN, finalold[,which(ordencols=="SWDOWN"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varGLW, finalold[,which(ordencols=="GLW"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varPBLH, finalold[,which(ordencols=="PBLH"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))
ncvar_put(nc.ex, varTSLB, finalold[,which(ordencols=="TSLB"),],start=c(1,tini),count=c(dim(TIN)[1],length(zt)))

}

### Before closing the file, compute MAXVEGFRAC if requested
if (length(which(Outnames=="MAXVEGFRA"))>0){
    cat("Adding MAXVEGFRA to finalold netcdf file",fill=TRUE)
    z = ncvar_get( nc.ex, "VEGFRA")   # variable      # Read the netcdf file recently created  
    nuevaMAXVEG=matrix(NA,dim(z)[1],dim(z)[2])   # matrix of nodes x times
    anos=rep(NA,dim(z)[2])
    ## identify how many different years involved in the simulation
      for (yi in 1:length(WRFchar)){
      pa= substring(WRFchar[yi], 1:nchar(WRFchar[yi]), 1:nchar(WRFchar[yi]))
      anos[yi]=paste(pa[1],pa[2],pa[3],pa[4],sep="")    
      }
    difano=unique(anos) # how many unique anos

      for (da in 1:length(difano)){
      dondeigual=grep(difano[da],WRFchar)
        for (nodis in 1:dim(z)[1]){
        nuevaMAXVEG[nodis,dondeigual]=max(z[nodis,dondeigual])  
        }
      }
     ncvar_put(nc.ex, varMAXVEGFRA, nuevaMAXVEG)
}    
    
nc_close(nc.ex) 

#### Verification of generated nc file:
#### Basin average for verification. Will have to read values from written matrices

ex.nco = nc_open(paste(Outfolder,Outncfile,"_",exportname,".nc",sep="")) # it opens the recently created nc file
Rdumpo=capture.output(print(ex.nco), file = NULL, append = FALSE)
write.table(Rdumpo,paste(Outfolder,"Rncdumpo.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   ## dumps netcdf headers to a file in outfolder  

cat("**********Averaging basin values for verification",fill=TRUE)
Avefinal=matrix(NA,rowsfinal,(length(Outnames)-2)) ## matrix containing the variables in the columns (not includign  node, WRFHOUR or JULTIME) and times in the rows
colnames(Avefinal)=Outnames[3:length(Outnames)]

for (vi in 1:(length(Outnames)-2)){  # by columns
  cat("Averaging column ",vi, " of ",length(Outnames)-2, fill=TRUE)
  z = ncvar_get( ex.nco, Outnames[vi+2])   # variable
  for (ti in 1:rowsfinal){	
	Avefinal[ti,vi]=mean(as.numeric(z[,ti]))  # FIX ME and CHECK ME 	
  }
}

### PLotting basin average values# 

if (dim(finalold)[2]<=8){
postscript(paste(Outfolder,"Basin_Average_TS.ps",sep=""),width=8000,height=5500)
mat=matrix(1:5,5,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 1:(length(Outnames)-2)){
plot(Avefinal[,pli],type="l",main=Outnames[pli+2])
}
dev.off()
}

if (dim(finalold)[2]<=13 & dim(finalold)[2]>8){
postscript(paste(Outfolder,"Basin_Average_TS1.ps",sep=""),width=8000,height=5500)
mat=matrix(1:5,5,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 1:5){
plot(Avefinal[,pli],type="l",main=Outnames[pli+2])
}
postscript(paste(Outfolder,"Basin_Average_TS2.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 6:dim(Avefinal)[2]){
plot(Avefinal[,pli],type="l",main=Outnames[pli+2])
}
}

if (dim(finalold)[2]<=18 & dim(finalold)[2]>13){
postscript(paste(Outfolder,"Basin_Average_TS1.ps",sep=""),width=8000,height=5500)
mat=matrix(1:5,5,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 1:5){
plot(Avefinal[,pli],type="l",main=Outnames[pli+2])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS2.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 6:10){
plot(Avefinal[,pli],type="l",main=Outnames[pli+2])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS3.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 11:dim(Avefinal)[2]){
plot(Avefinal[,pli],type="l",main=Outnames[pli+2])
}
dev.off()
}

	

cat("End of processes at ",date(),fill=TRUE)






