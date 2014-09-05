##########################################################################################################################################################
# Code to read, clip and export WRF forcing data to ADHydro (UW) in netcdf format 
# Hernan Moreno
# Aug 19 2014
##########################################################################################################################################################
# The final output file will be an array of (Number of nodes x Number of variables+1 x Time) dimensions. The variables that go in the columns are:
# (1) Node (2) T2 [Celsius], (3) Q2 [kg/kg], (4) PSFC [Pa], (5) W10 [m/s], (6) VEGFRA [m/s], (7) HGT [m], (8) TPREC (RAINC+RAINSH+RAINCC) [mm], 
# (9) FRAC_FROZ_PREC (SNOWNC+GRAUPELNC+HAILNC) [mm], (10) SWDOWN [w/m2], (11) GLW [W/M2], (12) PET [w/m2]
# Caveat: all times are in GMT time
################# LOAD LIBRARIES #########################################################################################################################
rm(list=ls())  # remove all objects before starting
library(ncdf)
library(abind)
################ INPUT MODULE ############################################################################################################################

# New version
WRF_Folder<<-"/share/CI-WATER Simulation Data/WRF/Utah_WRF_output/2005/"  # path to the WRF files
WRF_Files<<-c("wrfout_d03_2005-01-01_00:00:00","wrfout_d03_2005-01-31_12:00:00","wrfout_d03_2005-03-03_00:00:00") # ordered names of the WRF files to read 
TINF<<-"/share/CI-WATER Simulation Data/small_green_mesh/mesh.1.node"  # txt file with the AD-Hydro nodes for which the WRF data will be extracted
RE=6378137  # in meters; Earth's radius at the equator GRS80 ellipsoid
Lambda00<<--109  # reference meridian in degrees. sign indicates if it's west or east of Greenwich
FalseEast<<-20000000 
FalseNorth<<-10000000
cell_Buffer<<-4 # number of cells to be added to the selected rectangular from the minimum and maximum TIN points to efficiently read the WRF data. Use a number >2
Outnames<<-c("Times","T2","Q2","QVAPOR","QCLOUD","PSFC","W10","W","VEGFRA","TPREC","FRAC_FROZ_PREC","SWDOWN","GLW","PBLH")
Outfolder<<-"/share/CI-WATER Simulation Data/WRF_to_ADHydro/"
Outncfile<<-"WRF_ADHydro_Green_River_mod" # Base name for output file


############### MODULE METADATA AND HEADERS #####################################################################################################
cat("Time started processing ",date(),fill=TRUE)
## This module is optional.. It takes an example file and prints the file and variable information
ex.nc = open.ncdf(paste(WRF_Folder, WRF_Files[1], sep="")) # it opens the first file
#print(ex.nc)  # uncomment if want to dump metadata info

############### MODULE COORDINATES CONVERSION AND BOUNDING BOX SELECTION #################################################################################
TIN<<-read.table(TINF,skip=1) 
x11()
par(mfcol=c(1,2))
plot(TIN[,2],TIN[,3],pch=19,cex=0.2,main="Basin domain")
## Read only the dimension file to determine matrices size
rowss=length(get.var.ncdf( ex.nc, "south_north"))          # rows
colss=length(get.var.ncdf( ex.nc, "west_east"))          # columns
TIMES=get.var.ncdf( ex.nc, "Times")         # time id

#### Cell-centered coordinate transformation ##########################################################################################################
# Reads only the cell-centered LON - LAT field and transform them to sinusoidal coordinates
LON_WRF = get.var.ncdf(ex.nc, "XLONG",start=c(1,1,1),count=c(colss,rowss, 1))   # Matrix of longitudes is in inverse order (south_north, west_east)
LAT_WRF = get.var.ncdf(ex.nc, "XLAT",start=c(1,1,1),count=c(colss,rowss, 1))   # Matrix of latitudes

# Transform LON-LAT matrices to sinusoidal projection
Lambda0=Lambda00*(pi/180)  # degrees to radians
xs=RE*((LON_WRF*pi/180)-Lambda0)*cos(LAT_WRF*pi/180)+FalseEast
ys=RE*(LAT_WRF*pi/180)+FalseNorth

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

cat("Initial WRF matrix had ",dim(xs)," cells",fill=TRUE)
cat("New WRF matrix has ",WRFUR-WRFLL+1," cells",fill=TRUE)
cat("Applying buffer matrix has ",WRFU-WRFL+1," cells",fill=TRUE)

## Creating new xs(xss) and ys(yss).. These are smaller matrices that contain only the TIN points
xss=xs[WRFL[1]:WRFU[1],WRFL[2]:WRFU[2]]
yss=ys[WRFL[1]:WRFU[1],WRFL[2]:WRFU[2]]

# Extraction of the cells of WRF that contain each of the TIN nodes for ADHydro
sel_WRF=matrix(NA,dim(TIN)[1],3) # Node, row, col
sel_WRF[,1]=TIN[,1]
mindi=rep(NA,dim(TIN)[1])
cat("***** RUNNING WRF TO ADHYDRO PREPROCESSING SCRIPT *********", fill=TRUE)
cat("** Extracting distances from ADHydro nodes to WRF cells.. may take few minutes...",fill=TRUE)
for (ni in 1:dim(TIN)[1]){
dist=(((TIN[ni,2]-xss)^2)+((TIN[ni,3]-yss)^2))^0.5
mind=min(dist)
mindi[ni]=mind
wmind=which(dist==mind,arr.ind=TRUE)
sel_WRF[ni,2]=wmind[1] # row
sel_WRF[ni,3]=wmind[2] # col
}

cat("** Maximum distance between an AD-Hydro node and WRF cell center is", max(mindi)," m",fill=TRUE)

plot(WRFL[1]+sel_WRF[,2],WRFL[2]+sel_WRF[,3],xlim=c(1,colss),ylim=c(1,rowss),cex=0.2,pch=20,main="Basin domain comparison")

### Staggered coordinate transformation ###############################################################################################################
# Reads only the staggered-LON - LAT field and transform them to sinusoidal coordinates
LONU_WRF = get.var.ncdf(ex.nc, "XLONG_U",start=c(1,1,1),count=c(colss+1,rowss, 1))   # Matrix of longitudes is in inverse order (south_north, west_east)
LATU_WRF = get.var.ncdf(ex.nc, "XLAT_U",start=c(1,1,1),count=c(colss+1,rowss, 1))   # Matrix of latitudes

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
ex.nc = open.ncdf(paste(WRF_Folder, WRF_Files[mi], sep="")) 
## Starts reading the WRF data by variables
for (u in 1:length(Outnames)){
        
	if (Outnames[u]=="Times"){
	varia=Outnames[u]
	z = get.var.ncdf( ex.nc, varia)   # variable
	TIMES=z
# According to the times read in the xz variable it creates finalold matrix
	finalold<<-array(NA, c(dim(TIN)[1],1+length(Outnames),length(TIMES)))  # initial column is node number for ADHydro
	colnames(finalold)=c("node",Outnames)
	finalold[,1,]=TIN[,1]
	cat("variable ",varia,fill=TRUE)
		for (ti in 1:length(z)){	
		finalold[,u+1,ti]=rep(z[ti],dim(finalold)[1])
		}
	}


	if (Outnames[u]=="QVAPOR" | Outnames[u]=="QCLOUD"){
        varia=Outnames[u]
        z = get.var.ncdf( ex.nc, varia, start=c(WRFL[1],WRFL[2],1,1), count=c((WRFU[1]-WRFL[1]+1), (WRFU[2]-WRFL[2]+1),1,length(TIMES)))   # variable
	cat("variable ",varia,fill=TRUE)	
		for (v in 1:dim(TIN)[1]){
	        finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]
		}
	}


	if (Outnames[u]=="T2" | Outnames[u]=="Q2" |  Outnames[u]=="PSFC" | Outnames[u]=="VEGFRA" | Outnames[u]=="HGT" | Outnames[u]=="SWDOWN" | Outnames[u]=="GLW" | Outnames[u]=="POTEVP" | Outnames[u]=="PBLH"){
        varia=Outnames[u]
        z = get.var.ncdf( ex.nc, varia,start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(TIMES)))   # variable
	cat("variable ",varia,fill=TRUE)	
		for (v in 1:dim(TIN)[1]){
		if (varia=="T2") {
		finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]-272.15	
		} else {
                finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]
		}
		}
	}
	
	if (Outnames[u]=="W10"){
	zu = get.var.ncdf( ex.nc, "U10",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(TIMES)))   # u10 wind
	zv = get.var.ncdf( ex.nc, "V10",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(TIMES)))   # v10 wind
	cat("variable  W10",fill=TRUE)
		for (v in 1:dim(TIN)[1]){
                finalold[v,u+1,]=(((zu[sel_WRF[v,2],sel_WRF[v,3],])^2)+((zv[sel_WRF[v,2],sel_WRF[v,3],])^2))^0.5
		}
       }

	if (Outnames[u]=="W"){
	zu = get.var.ncdf( ex.nc, "U",start=c(WRFL[1],WRFL[2],1,1),count=c((WRFU[1]-WRFL[1]+1), (WRFU[2]-WRFL[2]+1),1,length(TIMES)))   # u STAGGERED wind
	zv = get.var.ncdf( ex.nc, "V",start=c(WRFL[1],WRFL[2],1,1),count=c((WRFU[1]-WRFL[1]+1), (WRFU[2]-WRFL[2]+1),1,length(TIMES)))   # v STAGGERED wind
	cat("variable  W--Staggered",fill=TRUE)
		for (v in 1:dim(TIN)[1]){
                finalold[v,u+1,]=(((zu[sel_WRFS[v,2],sel_WRFS[v,3],])^2)+((zv[sel_WRFS[v,2],sel_WRFS[v,3],])^2))^0.5
		}
       }


	if (Outnames[u]=="TPREC"){
	zrainc = get.var.ncdf( ex.nc, "RAINC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(TIMES)))   # 
	zrainsh = get.var.ncdf( ex.nc, "RAINSH",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(TIMES)))   # 
	zrainnc = get.var.ncdf( ex.nc, "RAINNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(TIMES)))   # 
	cat("variable  TPREC",fill=TRUE)
	for (v in 1:dim(TIN)[1]){
		finalold[v,u+1,1]=zrainc[sel_WRF[v,2],sel_WRF[v,3],1]+ zrainsh[sel_WRF[v,2],sel_WRF[v,3],1] + zrainnc[sel_WRF[v,2],sel_WRF[v,3],1]        
        	Paccum=zrainc[sel_WRF[v,2],sel_WRF[v,3],]+ zrainsh[sel_WRF[v,2],sel_WRF[v,3],] + zrainnc[sel_WRF[v,2],sel_WRF[v,3],]
			for (vv in 2:length(TIMES)){
			finalold[v,u+1,vv]=Paccum[vv]-Paccum[vv-1]
			}
		}
	}

	if (Outnames[u]=="FRAC_FROZ_PREC"){
	zsnownc = get.var.ncdf( ex.nc, "SNOWNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(TIMES)))   #\
	zgraupelnc = get.var.ncdf( ex.nc, "GRAUPELNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(TIMES)))   #\
 	zhailnc = get.var.ncdf( ex.nc, "HAILNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,length(TIMES)))   #\
	cat("variable  FRAC_FROZ_PREC",fill=TRUE)
	for (v in 1:dim(TIN)[1]){
	        finalold[v,u+1,1]=zsnownc[sel_WRF[v,2],sel_WRF[v,3],1]+ zgraupelnc[sel_WRF[v,2],sel_WRF[v,3],1] + zhailnc[sel_WRF[v,2],sel_WRF[v,3],1]
		Saccum=zsnownc[sel_WRF[v,2],sel_WRF[v,3],]+ zgraupelnc[sel_WRF[v,2],sel_WRF[v,3],] + zhailnc[sel_WRF[v,2],sel_WRF[v,3],]
			for (vv in 2:length(TIMES)){
			finalold[v,u+1,vv]=Saccum[vv]-Saccum[vv-1]
			}
		}
	}
}

if (mi==1){
final=finalold
} else {
final=abind(final, finalold,along=3)
}

}

##### Exporting to NETCDF files #################################################################
# define the netcdf coordinate variables -- note these have values!
cat("--Exporting final NETCDF file to Outputfolder",fill=TRUE)
dim1 = dim.def.ncdf( "Nodes","", as.integer(final[,1,1]))
dim2= dim.def.ncdf( "Time",paste("Hours since ",final[1,2,1]),seq(0,(dim(final)[3]-1))) #final[1,2,]))
#dim2= dim.def.ncdf( "Time","",as.data.frame(final[1,2,])) 


# define the EMPTY (elevation) netcdf variable
varT2 = var.def.ncdf("T2","celsius", list(dim1,dim2), -99999, longname="T2")
varQ2 = var.def.ncdf("Q2","kg kg-1", list(dim1,dim2), -99999, longname="Q2")
varQVAPOR = var.def.ncdf("QVAPOR","kg kg-1", list(dim1,dim2), -99999, longname="QVAPOR")
varQCLOUD = var.def.ncdf("QCLOUD","kg kg-1", list(dim1,dim2), -99999, longname="QCLOUD")
varPSFC = var.def.ncdf("PSFC","Pa", list(dim1,dim2), -99999, longname="PSFC")
varW10 = var.def.ncdf("W10","ms-1", list(dim1,dim2), -99999, longname="W10")
varW = var.def.ncdf("W","ms-1", list(dim1,dim2), -99999, longname="W")
varVEGFRA = var.def.ncdf("VEGFRA","", list(dim1,dim2), -99999, longname="VEGFRA")
varTPREC = var.def.ncdf("TPREC","mm", list(dim1,dim2), -99999, longname="TPREC")
varFRAC_FROZ_PREC = var.def.ncdf("FRAC_FROZ_PREC","mm", list(dim1,dim2), -99999, longname="FRAC_FROZ_PREC")
varSWDOWN = var.def.ncdf("SWDOWN","W m-2", list(dim1,dim2), -99999, longname="SWDOWN")
varGLW = var.def.ncdf("GLW","W m-2", list(dim1,dim2), -99999, longname="GLW")
varPBLH = var.def.ncdf("PBLH","m", list(dim1,dim2), -99999, longname="PBLH")


# associate the netcdf variable with a netcdf file   
# put the variable into the file, and
# close

nc.ex = create.ncdf(paste(Outfolder,Outncfile,"_",final[1,2,1],".nc",sep=""),list(varT2,varQ2,varQVAPOR,varQCLOUD, varPSFC, varW10, varW, varVEGFRA, varTPREC,varFRAC_FROZ_PREC, varSWDOWN, varGLW, varPBLH))

put.var.ncdf(nc.ex, varT2, final[,3,])
put.var.ncdf(nc.ex, varQ2, final[,4,])
put.var.ncdf(nc.ex, varQVAPOR, final[,5,])
put.var.ncdf(nc.ex, varQCLOUD, as.double(final[,6,]))
put.var.ncdf(nc.ex, varPSFC, final[,7,])
put.var.ncdf(nc.ex, varW10, final[,8,])
put.var.ncdf(nc.ex, varW, final[,9,])
put.var.ncdf(nc.ex, varVEGFRA, final[,10,])
put.var.ncdf(nc.ex, varTPREC, final[,11,])
put.var.ncdf(nc.ex, varFRAC_FROZ_PREC, final[,12,])
put.var.ncdf(nc.ex, varSWDOWN, final[,13,])
put.var.ncdf(nc.ex, varGLW, final[,14,])
put.var.ncdf(nc.ex, varPBLH, final[,15,])

close.ncdf(nc.ex)

### basin average for verification ##
cat("**********Averaging basin values for verification",fill=TRUE)
Avefinal=matrix(NA,dim(final)[3],(dim(final)[2]-2)) ## matrix containing the variables in thes column (not includign  node or time) and times in the rows
colnames(Avefinal)=Outnames[2:length(Outnames)]
for (vi in 1:(dim(final)[2]-2)){  # by columns
	for (ti in 1:dim(final)[3]){	
	Avefinal[ti,vi]=mean(as.numeric(final[,(vi+2),ti])) 	
        }
}

### PLotting basin average values# 

if (dim(final)[2]<=8){
x11()
par(mar = rep(2, 4),mfrow=c(dim(final)[2],1))
for (pli in 3:length(Outnames)){
plot(Avefinal[,pli],type="l",main=Outnames[pli+1])
}
}

if (dim(final)[2]<=14 & dim(final)[2]>8){
x11()
par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 1:6){
plot(Avefinal[,pli],type="l",main=Outnames[pli+1])
}
x11()
par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 7:dim(Avefinal)[2]){
plot(Avefinal[,pli],type="l",main=Outnames[pli+1])
}
}

if (dim(final)[2]<=20 & dim(final)[2]>14){
x11()
par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 1:6){
plot(Avefinal[,pli],type="l",main=Outnames[pli+1])
}
x11()
par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 7:12){
plot(Avefinal[,pli],type="l",main=Outnames[pli+1])
}
x11()
par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 13:dim(Avefinal)[2]){
plot(Avefinal[,pli],type="l",main=Outnames[pli+1])
}
}	
	

cat("End of processes at ",date(),fill=TRUE)







