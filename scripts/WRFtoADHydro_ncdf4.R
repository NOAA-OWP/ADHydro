##########################################################################################################################################################
# Code to read, clip and export WRF forcing data to ADHydro (UW) in netcdf format 
# Hernan Moreno
# Aug 19 2014
##########################################################################################################################################################
# The final output file will be an array of two dimensions (node and time) and length(Outnames) variables. The variables that go in the columns are up to now:
# (1) WRFHOUR (2) JULTIME (3) T2 [Celsius], (4) Q2 [kg/kg], (5) QVAPOR[kg/kg], (6)QCLOUD[kg/kg], (7)PSFC [Pa], (8) U10 [m/s], (9) V10 [m/s], (10) U, (11) V, (12) VEGFRA [-], 
# (13) MAXVEGFRA, (14) TPREC (RAINC+RAINSH+RAINCC) [m/s],(15) FRAC_FROZ_PREC (SNOWNC+GRAUPELNC+HAILNC) [m/s], (16) SWDOWN [w/m2], (17) GLW [W/M2], (18) PBLH [m], 
# (19) SOILTB[Celsius], (20) TSLB [Celsius]
# Caveat: all times are in GMT time
################# LOAD LIBRARIES #########################################################################################################################

rm(list=ls())  # remove all objects before starting
library(ncdf4)
library(abind)
library(timeDate)
################ INPUT MODULE ############################################################################################################################

WRF_Folder<<-"/share/CI-WATER Simulation Data/WRF/Utah_WRF_output/2005/"  # path to the WRF files
WRF_Files<<-c("wrfout_d03_2005-01-01_00:00:00","wrfout_d03_2005-01-31_12:00:00","wrfout_d03_2005-03-03_00:00:00") # ordered names of the WRF files to read 
TINF<<-"/share/CI-WATER Simulation Data/small_green_mesh/mesh.1.node"  # txt file with the AD-Hydro nodes for which the WRF data will be extracted
RE=6378137  # in meters; Earth's radius at the equator GRS80 ellipsoid
Lambda00<<--109  # reference meridian in degrees. Sign indicates if it's west or east of Greenwich
FalseEast<<-20000000 
FalseNorth<<-10000000
cell_Buffer<<-4 # number of cells to be added to the selected rectangular from the minimum and maximum TIN points to efficiently read the WRF data. Use a number >2
soillayers<<-4  # number of total staggered soil layers. It can be extracted from the Rncdump.txt file from line 39
JulOrigin<<-"2005-01-01 00:00:00" # origin for the Julian date column
Outnames<<-c("WRFHOUR","JULTIME","T2","Q2","QVAPOR","QCLOUD","PSFC","U10","V10","U","V","VEGFRA","MAXVEGFRA","TPREC","FRAC_FROZ_PREC","SWDOWN","GLW","PBLH","SOILTB","TSLB")
Outfolder<<-"/share/CI-WATER Simulation Data/WRF_to_ADHydro/"
Outncfile<<-"WRF_ADHydro_Green_River" # Base name for output file


############### MODULE METADATA AND HEADERS #############################################################################################################

cat("Time started processing ",date(),fill=TRUE)
## This module is optional.. It takes an example file and prints the file and variable information
ex.nc = nc_open(paste(WRF_Folder, WRF_Files[1], sep="")) # it opens the first file
Rdump=capture.output(print(ex.nc), file = NULL, append = FALSE)
write.table(Rdump,paste(Outfolder,"Rncdump.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   ## dumps netcdf headers to a file in outfolder  
cat("A dump file has been created in Outfolder called Rncdump",fill=TRUE)
############## MODULE COORDINATES CONVERSION AND BOUNDING BOX SELECTION #################################################################################

TIN<<-read.table(TINF,skip=1) 
x11()
par(mfcol=c(1,2))
plot(TIN[,2],TIN[,3],pch=19,cex=0.2,main="Basin domain")

## Read only the dimension file to determine matrices size
LON_WRF = ncvar_get(ex.nc, "XLONG")   # Matrix of longitudes in this order: (west_east,south_north,times)
LAT_WRF = ncvar_get(ex.nc, "XLAT")   # Matrix of latitudes
rowss=dim(LON_WRF)[2]
colss=dim(LON_WRF)[1]
TIMES=dim(LON_WRF)[3]  # normally the third dimension are times

#### Cell-centered coordinate transformation ##########################################################################################################
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
	# According to the TIMES dimension of the first matrix it creates a new array with this dimension
  finalold<<-array(NA, c(dim(TIN)[1],1+length(Outnames),TIMES))  # initial column is node number for ADHydro
	colnames(finalold)=c("node",Outnames)
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
        z = ncvar_get( ex.nc, varia, start=c(WRFL[1],WRFL[2],1,1), count=c((WRFU[1]-WRFL[1]+1), (WRFU[2]-WRFL[2]+1),1,TIMES))   # variable
	cat("variable ",varia,fill=TRUE)	
		for (v in 1:dim(TIN)[1]){
	        finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]
		}
	}

  
	if (Outnames[u]=="TSLB"){
	  varia="TSLB" #Outnames[u]
	  z = ncvar_get( ex.nc, varia, start=c(WRFL[1],WRFL[2],soillayers,1), count=c((WRFU[1]-WRFL[1]+1), (WRFU[2]-WRFL[2]+1),1,TIMES))   # variable
	  cat("variable ",varia,fill=TRUE)	
	  for (v in 1:dim(TIN)[1]){
	    finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]-272.15  
	  }
	}
	  

	if (Outnames[u]=="T2" | Outnames[u]=="Q2" |  Outnames[u]=="PSFC" | Outnames[u]=="VEGFRA" | Outnames[u]=="HGT" | Outnames[u]=="SWDOWN" | Outnames[u]=="GLW" | 
        Outnames[u]=="POTEVP" | Outnames[u]=="PBLH" | Outnames[u]=="U10" | Outnames[u]=="V10" | Outnames[u]=="SOILTB"){
        varia=Outnames[u]
        z = ncvar_get( ex.nc, varia,start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,TIMES))   # variable
	cat("variable ",varia,fill=TRUE)	
		for (v in 1:dim(TIN)[1]){
		if (varia=="T2" | varia=="SOILTB") {
		finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]-272.15	
		} else {
                finalold[v,u+1,]=z[sel_WRF[v,2],sel_WRF[v,3],]
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
  z = ncvar_get( ex.nc, varia, start=c(WRFL[1],WRFL[2],1,1),count=c((WRFU[1]-WRFL[1]+1), (WRFU[2]-WRFL[2]+1),1,TIMES))   # v STAGGERED wind
	cat("variable ",varia,"--Staggered",fill=TRUE)
		for (v in 1:dim(TIN)[1]){
                finalold[v,u+1,]=z[sel_WRFS[v,2],sel_WRFS[v,3],]
		}
       }


	if (Outnames[u]=="TPREC"){
	zrainc = ncvar_get( ex.nc, "RAINC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,TIMES))   # 
	zrainsh = ncvar_get( ex.nc, "RAINSH",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,TIMES))   # 
	zrainnc = ncvar_get( ex.nc, "RAINNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,TIMES))   # 
	cat("variable  TPREC",fill=TRUE)
	for (v in 1:dim(TIN)[1]){
		if (mi==1){  # the first element does not have to be subtracted from the last hour's cumulative rainfall value at that node
    finalold[v,u+1,1]=(zrainc[sel_WRF[v,2],sel_WRF[v,3],1]+ zrainsh[sel_WRF[v,2],sel_WRF[v,3],1] + zrainnc[sel_WRF[v,2],sel_WRF[v,3],1])*(1/3600000) # mm/h to m/s
		} else {
		finalold[v,u+1,1]=((zrainc[sel_WRF[v,2],sel_WRF[v,3],1]+ zrainsh[sel_WRF[v,2],sel_WRF[v,3],1] + zrainnc[sel_WRF[v,2],sel_WRF[v,3],1])-(3600000*sum(as.numeric(final[v,u+1,]))))*(1/3600000) # mm/h to m/s  
		}
    Paccum=zrainc[sel_WRF[v,2],sel_WRF[v,3],]+ zrainsh[sel_WRF[v,2],sel_WRF[v,3],] + zrainnc[sel_WRF[v,2],sel_WRF[v,3],]
			for (vv in 2:TIMES){
			finalold[v,u+1,vv]=(Paccum[vv]-Paccum[vv-1])*(1/3600000) # mm/h to m/s 
			}
		}
	}

	if (Outnames[u]=="FRAC_FROZ_PREC"){
	zsnownc = ncvar_get( ex.nc, "SNOWNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,TIMES))   #\
	zgraupelnc = ncvar_get( ex.nc, "GRAUPELNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,TIMES))   #\
 	zhailnc = ncvar_get( ex.nc, "HAILNC",start=c(WRFL[1],WRFL[2],1), count=c((WRFU[1]-WRFL[1]+1),(WRFU[2]-WRFL[2]+1) ,TIMES))   #\
	cat("variable  FRAC_FROZ_PREC",fill=TRUE)
	for (v in 1:dim(TIN)[1]){
	  if (mi==1){
    finalold[v,u+1,1]=(zsnownc[sel_WRF[v,2],sel_WRF[v,3],1]+ zgraupelnc[sel_WRF[v,2],sel_WRF[v,3],1] + zhailnc[sel_WRF[v,2],sel_WRF[v,3],1])*(1/3600000) # mm/h to m/s
	  } else {
	  finalold[v,u+1,1]=((zsnownc[sel_WRF[v,2],sel_WRF[v,3],1]+ zgraupelnc[sel_WRF[v,2],sel_WRF[v,3],1] + zhailnc[sel_WRF[v,2],sel_WRF[v,3],1])-(3600000*sum(as.numeric(final[v,u+1,]))))*(1/3600000) # mm/h to m/s    
    }
    Saccum=zsnownc[sel_WRF[v,2],sel_WRF[v,3],]+ zgraupelnc[sel_WRF[v,2],sel_WRF[v,3],] + zhailnc[sel_WRF[v,2],sel_WRF[v,3],]
			for (vv in 2:TIMES){
			finalold[v,u+1,vv]=(Saccum[vv]-Saccum[vv-1])*(1/3600000) # mm/h to m/s # mm/h to m/s
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
## Computing the MAXVEGFRAC column before writing the final output

  dmv=which(Outnames=="MAXVEGFRA")
  dv=which(Outnames=="VEGFRA")
  final[,(dv+1),]=as.numeric(final[,(dv+1),])/100   # values for ADHydro ranging between 0 and 1
  varia=Outnames[dmv]
  cat("variable ",varia,fill=TRUE)
  anos=rep(NA,dim(final)[3])
  # identify how many different years involved in the simulation
  for (yi in 1:dim(final)[3]){
    pa= substring(final[1,2,yi], 1:nchar(final[1,2,yi]), 1:nchar(final[1,2,yi]))
    anos[yi]=paste(pa[1],pa[2],pa[3],pa[4],sep="")    
  }

  difano=unique(anos) # how many unique anos

  for (da in 1:length(difano)){
  dondeigual=grep(difano[da],final[1,2,])
    for (nodis in 1:dim(final)[1]){
    final[nodis,(dmv+1),dondeigual]=max(final[nodis,(dv+1),dondeigual])  
    }
  }



##### Exporting to NETCDF files #################################################################
# define the netcdf coordinate variables -- note these have values!
cat("--Exporting final NETCDF file to Outputfolder",fill=TRUE)
dim1 = ncdim_def( "Nodes","", as.integer(final[,1,1]))
dim2= ncdim_def( "Time",paste("Hours since ",final[1,2,1]),seq(0,(dim(final)[3]-1))) #final[1,2,]))


# define the EMPTY (elevation) netcdf variable
varNODES=ncvar_def("NODES","", list(dim1), -99999, longname="NODES")
varJULTIME=ncvar_def("JULTIME","secs", list(dim2), -99999, longname="JULTIME")
varT2 = ncvar_def("T2","Celsius", list(dim1,dim2), -99999, longname="T2")
varQ2 = ncvar_def("Q2","kg kg-1", list(dim1,dim2), -99999, longname="Q2")
varQVAPOR = ncvar_def("QVAPOR","kg kg-1", list(dim1,dim2), -99999, longname="QVAPOR")
varQCLOUD = ncvar_def("QCLOUD","kg kg-1", list(dim1,dim2), -99999, longname="QCLOUD")
varPSFC = ncvar_def("PSFC","Pa", list(dim1,dim2), -99999, longname="PSFC")
varU10 = ncvar_def("U10","ms-1", list(dim1,dim2), -99999, longname="U10")
varV10 = ncvar_def("V10","ms-1", list(dim1,dim2), -99999, longname="V10")
varU = ncvar_def("U","ms-1", list(dim1,dim2), -99999, longname="U")
varV = ncvar_def("V","ms-1", list(dim1,dim2), -99999, longname="V")
varVEGFRA = ncvar_def("VEGFRA","", list(dim1,dim2), -99999, longname="VEGFRA")
varMAXVEGFRA = ncvar_def("MAXVEGFRA","", list(dim1,dim2), -99999, longname="MAXVEGFRA")
varTPREC = ncvar_def("TPREC","ms-1", list(dim1,dim2), -99999, longname="TPREC")
varFRAC_FROZ_PREC = ncvar_def("FRAC_FROZ_PREC","ms-1", list(dim1,dim2), -99999, longname="FRAC_FROZ_PREC")
varSWDOWN = ncvar_def("SWDOWN","W m-2", list(dim1,dim2), -99999, longname="SWDOWN")
varGLW = ncvar_def("GLW","W m-2", list(dim1,dim2), -99999, longname="GLW")
varPBLH = ncvar_def("PBLH","m", list(dim1,dim2), -99999, longname="PBLH")
varSOILTB = ncvar_def("SOILTB","Celsius", list(dim1,dim2), -99999, longname="SOILTB")
varTSLB = ncvar_def("TSLB","Celsius", list(dim1,dim2), -99999, longname="TSLB")
# associate the netcdf variable with a netcdf file   
# put the variable into the file, and
# close

nc.ex = nc_create(paste(Outfolder,Outncfile,"_",final[1,2,1],".nc",sep=""),list(varNODES, varJULTIME, varT2, varQ2, varQVAPOR, varQCLOUD,
        varPSFC, varU10, varV10, varU, varV, varVEGFRA, varMAXVEGFRA, varTPREC, varFRAC_FROZ_PREC, varSWDOWN, varGLW, varPBLH, varSOILTB, varTSLB),force_v4=TRUE)

ncvar_put(nc.ex, varNODES, final[,1,1])
ncvar_put(nc.ex, varJULTIME, final[1,3,])
ncvar_put(nc.ex, varT2, final[,4,])
ncvar_put(nc.ex, varQ2, final[,5,])
ncvar_put(nc.ex, varQVAPOR, final[,6,])
ncvar_put(nc.ex, varQCLOUD, as.double(final[,7,]))
ncvar_put(nc.ex, varPSFC, final[,8,])
ncvar_put(nc.ex, varU10, final[,9,])
ncvar_put(nc.ex, varV10, final[,10,])
ncvar_put(nc.ex, varU, final[,11,])
ncvar_put(nc.ex, varV, final[,12,])
ncvar_put(nc.ex, varVEGFRA, final[,13,])
ncvar_put(nc.ex, varMAXVEGFRA, final[,14,])
ncvar_put(nc.ex, varTPREC, final[,15,])
ncvar_put(nc.ex, varFRAC_FROZ_PREC, final[,16,])
ncvar_put(nc.ex, varSWDOWN, final[,17,])
ncvar_put(nc.ex, varGLW, final[,18,])
ncvar_put(nc.ex, varPBLH, final[,19,])
ncvar_put(nc.ex, varSOILTB, final[,20,])
ncvar_put(nc.ex, varTSLB, final[,21,])
nc_close(nc.ex)



### basin average for verification ##
cat("**********Averaging basin values for verification",fill=TRUE)
Avefinal=matrix(NA,dim(final)[3],(dim(final)[2]-3)) ## matrix containing the variables in thes column (not includign  node or time) and times in the rows
colnames(Avefinal)=Outnames[3:length(Outnames)]
for (vi in 1:(dim(final)[2]-3)){  # by columns
	for (ti in 1:dim(final)[3]){	
	Avefinal[ti,vi]=mean(as.numeric(final[,(vi+3),ti])) 	
        }
}

### PLotting basin average values# 

if (dim(final)[2]<=9){
x11()
par(mar = rep(2, 4),mfrow=c(dim(final)[2],1))
for (pli in 4:length(Outnames)){
plot(Avefinal[,pli],type="l",main=Outnames[pli+2])
}
}

if (dim(final)[2]<=15 & dim(final)[2]>9){
x11()
par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 1:6){
plot(Avefinal[,pli],type="l",main=Outnames[pli+2])
}
x11()
par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 7:dim(Avefinal)[2]){
plot(Avefinal[,pli],type="l",main=Outnames[pli+2])
}
}

if (dim(final)[2]<=21 & dim(final)[2]>15){
x11()
par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 1:6){
plot(Avefinal[,pli],type="l",main=Outnames[pli+2])
}
x11()
par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 7:12){
plot(Avefinal[,pli],type="l",main=Outnames[pli+2])
}
x11()
par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 13:dim(Avefinal)[2]){
plot(Avefinal[,pli],type="l",main=Outnames[pli+2])
}
}	
	

cat("End of processes at ",date(),fill=TRUE)


#### verification of generated nc file:
ex.nco = nc_open(paste(Outfolder,Outncfile,"_",final[1,2,1],".nc",sep="")) # it opens the first file
Rdumpo=capture.output(print(ex.nco), file = NULL, append = FALSE)
write.table(Rdumpo,paste(Outfolder,"Rncdumpo.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   ## dumps netcdf headers to a file in outfolder  

z = ncvar_get( ex.nco, "JULTIME")   # variable





