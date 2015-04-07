###################################################################################################################################################################################
# Code to plot time series and distributed basin values from the WRF files, clipping it using a points file containing the basin divide
# Hernan Moreno
# Dec 04 2014
################# LOAD LIBRARIES ##################################################################################################################################################

rm(list=ls())  # remove all objects before starting
library(ncdf4)
library(abind)
library(timeDate)
library(akima)
library(plotrix)
library(rgl)
library(sp)
library(maptools)
library(fields)
options(digits=16)
################ INPUT MODULE #####################################################################################################################################################
Outfolder<<-"/localstore/MtMoranLink/project/CI-WATER/data/WRFtoADHydro/"         # Path to the nc folder
Outncfile<<-"Forcing10Y_VEGFRA.nc"                                                # Name of the nc file
Outnames<<-c("T2","QVAPOR","QCLOUD","PSFC","U","V","VEGFRA","MAXVEGFRA","TPREC","SWDOWN","GLW","PBLH","TSLB",
             "T2_C","QVAPOR_C","QCLOUD_C","PSFC_C","U_C","V_C","VEGFRA_C","MAXVEGFRA_C","TPREC_C","SWDOWN_C","GLW_C","PBLH_C","TSLB_C")  # names of the nc variables to read
timeseries<<-1                                                                    # 0 if time series are not desired. 1 if time series are desired
TINF<<-"/share/CI-WATER_Simulation_Data/small_green_mesh/geometry.nc"             # netcdf file with the AD-Hydro mesh elements for which the WRF data will be extracted
nodata<<- -9999                                                                   # nodata value
output_dir<<-"/localstore/MtMoranLink/project/CI-WATER/data/WRFtoADHydro/"        # directory where figures will be exported
chunksTime<<-3                                                                    # Number of chunks to be split the read for number of TIN pints in the nc file
cluster<<-0                                                                       # 0 if you are not running this script in the cluster; 1 if yes.                                                               
spatial<<-0                                                                       # 0 if should not plot spatially-averaged data, 1 if yes. 
celda<<-100                                                                       # Desired output cellsize for plots
divisoria<<-"/user2/hmoreno/Documents/DEMS/small_green_mesh/XY_Border_points.csv" # csv points file with the watershed divide
plotchannels<<-1                                                                  # optional do you want to add channels? 0=no, 1=yes
canales="/user2/hmoreno/Documents/DEMS/small_green_mesh/projectednet.shp"         # optional path and name of channels file
xtifflim<<-c(2260,-5.0, 0.0030, 1e-8, -1.3,-2.2)                                  # optional scale limits for the tiff plots in order of Topography + Outnames
ytifflim<<-c(4066,4.5,  0.0038, 3.7e-8,1.9, 0.9)
BB<<-2                                                                            # BB=2 always to start but can increase in multiples of 2 until divide looks good
################################################################################################################################################################
#### Verification of generated nc file:
#### Basin average for verification. Will have to read values from written matrices

ex.nco = nc_open(paste(Outfolder,Outncfile,sep=""),readunlim=FALSE) # it opens the recently created nc file
Rdumpo=capture.output(print(ex.nco), file = NULL, append = FALSE)
write.table(Rdumpo,paste(Outfolder,"Rncdumpo.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   ## dumps netcdf headers to a file in outfolder  

### Number of hours
if (timeseries==1){
z = ncvar_get(ex.nco, "JULTIME")   # variable
rowsfinal=length(z)

cat("**********Averaging basin values for verification",fill=TRUE)
Avefinal=matrix(NA,rowsfinal,length(Outnames)) ## matrix containing the variables in the columns (not includign  node, WRFHOUR or JULTIME) and times in the rows
colnames(Avefinal)=Outnames


if (chunksTime > 1){ 
  startpedazos=floor(seq(from=1,to=rowsfinal,length.out=chunksTime+1)) #by=floor(dim(TINX)[1]/chunks)
  startpedazos[2:length(startpedazos)]=startpedazos[2:length(startpedazos)]+1
  sp1=c(startpedazos[2:length(startpedazos)],startpedazos[length(startpedazos)])  
  countpedazos=sp1-startpedazos
  
} else {
  startpedazos=1  
  countpedazos=rowsfinal  
}

##### Reading geometry properties from Rdumpo to find number of mesh and channel elements
dondeMeshEl=grep("Mesh_Elements  Size",Rdumpo)  # Creful when running this line because this might change depending on nc output programming
paMeshEl=substring(Rdumpo[dondeMeshEl],1:nchar(Rdumpo[dondeMeshEl]),1:nchar(Rdumpo[dondeMeshEl]))
if (cluster==0) {
MeshEl=as.numeric(paste(paMeshEl[(which(paMeshEl==":")+1):(which(paMeshEl=="\"")[2]-1)],collapse=""))
} else {
MeshEl=as.numeric(paste(paMeshEl[(which(paMeshEl==":")+1):length(paMeshEl)],collapse=""))
}

dondeChannelEl=grep("Channel_Elements  Size",Rdumpo)  # Creful when running this line because this might change depending on nc output programming
paChannEl=substring(Rdumpo[dondeChannelEl],1:nchar(Rdumpo[dondeChannelEl]),1:nchar(Rdumpo[dondeChannelEl]))
if (cluster==0) {
ChannelEl=as.numeric(paste(paChannEl[(which(paChannEl==":")+1):(which(paChannEl=="\"")[2]-1)],collapse=""))
} else {
ChannelEl=as.numeric(paste(paChannEl[(which(paChannEl==":")+1):length(paChannEl)],collapse=""))
}


for (vi in 1:length(Outnames)){  # by columns
cat("Averaging variable ",Outnames[vi], " of ",length(Outnames)," Variables", fill=TRUE)
  for (wi in 1:chunksTime){
  cat("Chunk ",wi, " of ",chunksTime,fill=TRUE)
     if (Outnames[vi]=="T2" | Outnames[vi]=="QVAPOR" | Outnames[vi]=="QCLOUD" | Outnames[vi]=="PSFC" | Outnames[vi]=="U" | Outnames[vi]=="V"
         | Outnames[vi]=="VEGFRA" | Outnames[vi]=="MAXVEGFRA" | Outnames[vi]=="TPREC" | Outnames[vi]=="SWDOWN" | Outnames[vi]=="GLW" | Outnames[vi]=="PBLH"
         | Outnames[vi]=="TSLB") { 
    cat("Using MeshEl for elements", fill=TRUE) 
    z = ncvar_get( ex.nco, Outnames[vi], start=c(1,startpedazos[wi]), count=c(MeshEl,countpedazos[wi]))   # variable
    } else {
    cat("Using ChannelEl for elements", fill=TRUE) 
    z = ncvar_get( ex.nco, Outnames[vi], start=c(1,startpedazos[wi]), count=c(ChannelEl,countpedazos[wi]))   # variable
    }
    
  for (ti in 1:countpedazos[wi]){	
	  cui=ti+startpedazos[wi]-1
    Avefinal[cui,vi]=mean(as.numeric(z[,ti]))  	
    }
  }
}

### PLotting basin average values# 

if (length(Outnames)<=5){
postscript(paste(Outfolder,"Basin_Average_TS.ps",sep=""),width=8000,height=5500)
mat=matrix(1:5,5,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 1:length(Outnames)){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
}

if (length(Outnames)<=10 & length(Outnames)>5){
postscript(paste(Outfolder,"Basin_Average_TS1.ps",sep=""),width=8000,height=5500)
mat=matrix(1:5,5,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 1:5){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
postscript(paste(Outfolder,"Basin_Average_TS2.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 6:dim(Avefinal)[2]){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
}

if (length(Outnames)<=15 & length(Outnames)>10){
postscript(paste(Outfolder,"Basin_Average_TS1.ps",sep=""),width=8000,height=5500)
mat=matrix(1:5,5,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 1:5){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS2.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 6:10){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS3.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 11:dim(Avefinal)[2]){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
}


if (length(Outnames)<=25 & length(Outnames)>20){
postscript(paste(Outfolder,"Basin_Average_TS1.ps",sep=""),width=8000,height=5500)
mat=matrix(1:5,5,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 1:5){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS2.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 6:10){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS3.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 11:15){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS4.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 16:20){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS6.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 21:dim(Avefinal)[2]){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
}


if (length(Outnames)<=30 & length(Outnames)>25){
postscript(paste(Outfolder,"Basin_Average_TS1.ps",sep=""),width=8000,height=5500)
mat=matrix(1:5,5,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 1:5){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS2.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 6:10){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS3.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
#x11()
#par(mar = rep(2, 4),mfrow=c(6,1))
for (pli in 11:15){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS4.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 16:20){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS5.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 21:25){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
postscript(paste(Outfolder,"Basin_Average_TS6.ps",sep=""),width=8000,height=5500)
layout(mat)
par(mar=c(3.9,4,3,3.5))
for (pli in 26:dim(Avefinal)[2]){
plot(Avefinal[,pli],type="l",main=Outnames[pli])
}
dev.off()
}
}


if (spatial == 1){

  if (plotchannels==1) {
    channels=readShapeLines(canales) #optional channels
  }
  
  ## New module... organized points from the points.file so that they form a convex hull in consecutive order
  divi=read.csv(divisoria)
  xconstrain=divi[,1]
  yconstrain=divi[,2]
  divisoriaold=cbind(xconstrain,yconstrain)
  divis=divisoriaold
  
  nueva_divi=matrix(NA,dim(divisoriaold)[1],2)
  nueva_divi[1,1]=divis[1,1]
  nueva_divi[1,2]=divis[1,2]
  divis=divis[-1,]
  dele=0
  for (ui in 2:dim(divisoriaold)[1]){
    dista=sqrt(((nueva_divi[(ui-1),1]-divis[,1])^2)+((nueva_divi[(ui-1),2]-divis[,2])^2))
    mindis=min(dista)
    dondemin=which(dista==mindis)
    if (length(dondemin)>1) {
      dele=dele+length(dondemin)
      cat("Warning more than one divide point deleted (",length(dondemin), ") Total points deleted are ",dele+ui+1,fill=TRUE)
    }
    nueva_divi[ui,1]=divis[dondemin[1],1]  
    nueva_divi[ui,2]=divis[dondemin[1],2]
    divis=divis[-dondemin,]
    #Sys.sleep(0.05)
    #plot(nueva_divi,pch=19,cex=0.2)
    #cat(mindis,fill=TRUE)
    #cat(dondemin,fill=TRUE)
    if (length(divis)==BB) {    # change this number to any number larger than 2 (multiples of 2) if the watershed divide and raster figure looks weird  
      #nueva_divi[dim(divisoriaold)[1],1]=divis[1]
      #nueva_divi[dim(divisoriaold)[1],2]=divis[2]
      cat("Module stopped due to empty matrix",fill=TRUE)
      break
    }
  }
  
  nueva_divis=unique(nueva_divi) # removing identical rows
  playwith(plot(nueva_divis,pch=19,cex=0.2),new=TRUE)
  narows=na.omit(nueva_divis)  # odered watershed divide removing NAs
  xconstrain=narows[,1]
  yconstrain=narows[,2]
  
    
  ############## MODULE COORDINATES CONVERSION AND BOUNDING BOX SELECTION ###########################################################################################################
  varia=c("Topography",Outnames)
  for (wi in 1:(length(Outnames)+1)){
  cat("Processing ",varia[wi],fill=TRUE)  
  zlimites<<-c(xtifflim[wi],ytifflim[wi]) 
  
  if (wi==1){
  mesh.nc = nc_open(TINF) # it opens the geometry file
  Rdump_mesh=capture.output(print(mesh.nc), file = NULL, append = FALSE)
  write.table(Rdump_mesh,paste(Outfolder,"Rncdump_mesh.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   ## dumps netcdf headers of geometry to a file in outfolder  
  TINX = ncvar_get(mesh.nc, "meshElementX") 
  TINY = ncvar_get(mesh.nc, "meshElementY")
  TINZ = ncvar_get(mesh.nc, "meshElementZSurface")
  TIN= cbind(seq(0,(length(TINX)-1)),TINX,TINY,TINZ)
  } else {
  var.nc = nc_open(paste(Outfolder,Outncfile,sep="")) # it opens the geometry file
  TINZ = rowMeans(ncvar_get(var.nc, Outnames[wi-1]))
  TIN= cbind(seq(0,(length(TINX)-1)),TINX,TINY,TINZ)
  }
  
# PLotting topography
## interpolating and converting to ASCII
 x_min = min(TINX)-(20*celda)
 x_max = max(TINX)+(20*celda)
 
 y_min = min(TINY)-(20*celda)
 y_max = max(TINY)+(20*celda)
 
 ## using akima
 ## linear interpolation
 x=TINX
 y=TINY
 z=TINZ
 
 #d <- deldir(x,y)
 #td <- tile.list(d)
 #plot(td,polycol=heat.colors(z),close=TRUE,showpoints=FALSE)
 #polygon(xconstrain,yconstrain,lwd=4)
 
 #xmin=min(x)
 #xmax=max(x)
 #ymin=min(y)
 #ymax=max(y)
 
 #require(latticeExtra)
 #xyz <- data.frame(x = runif(1000), y =runif(1000) , z = z)  #((y-ymin)/(ymax-ymin))
 #tileplot(z ~ x * y, xyz, use.tripack = TRUE,points=FALSE,col.regions = grey.colors(100))
 #polygon(xconstrain,yconstrain,lwd=4)
 
 
 cat("interpolating... may take some minutes",fill=TRUE)
 akima.li <- interp(x, y, z,xo=seq(x_min, x_max, length = (x_max-x_min)/celda),
                    yo=seq(y_min, y_max, length =(y_max-y_min)/celda),
                    linear = TRUE, extrap=FALSE, duplicate = "error", dupfun = NULL, ncp = NULL)
 cat("interpolation done",fill=TRUE)
 
 SEQ_Yi=as.matrix(as.data.frame(akima.li[2]))
 SEQ_YY=rev(SEQ_Yi)
 SEQ_XX=as.matrix(as.data.frame(akima.li[1]))
 SEQ_Zi=as.matrix(as.data.frame(akima.li[3]))
 
 ## when extracting elements from a list 1) transpose   2) reverse rows
 SEQ_ZZ=t(SEQ_Zi)                  ## 1. TRanspose
 SEQ_ZZZ= SEQ_ZZ[ nrow(SEQ_ZZ):1, ]## 2. reverse the rows
 
 ### image of SEQ_ZZ ##########  uncomment it. It plots a matrix without transposing it
 
 X11()
 color2D.matplot(SEQ_ZZZ,c(0,1),c(0,1),c(0,1),
                 extremes=NA,cellcolors=NA,show.legend=TRUE,nslices=20,xlab="Column",
                 ylab="Row",do.hex=FALSE,axes=TRUE,show.values=FALSE,vcol="white",vcex=1,
                 border=NA,main=varia[wi],na.color=NA)
 
 
 YII=matrix(SEQ_YY,length(SEQ_YY),length(SEQ_XX))  ## NORTES
 XII=matrix(SEQ_XX,length(SEQ_YY),length(SEQ_XX),byrow=T)  ## Estes
 
 
 ######### Dtermines if couple of coordinates XII,YII fall inside a polygon (points with denomination 1)
 ## define los puntos inside del polygon decrito por las coordenadas xconstranin, yconstrain
 cualesII=point.in.polygon(XII,YII,xconstrain,yconstrain)
 ng2=length(which(cualesII==1))+length(xconstrain)  ## number of cells 30 m inside the watershed divide
 MATII=matrix(cualesII, length(SEQ_YY))   ## conviertE en matrix el vector cuales##length(SEQ_Y) is # ofrows
 MATII[MATII==0]=NA
 INSIDE=MATII*SEQ_ZZZ
 
   X11()
   color2D.matplot(INSIDE,c(0,1),c(0,1),c(0,1),
                   extremes=NA,cellcolors=NA,show.legend=TRUE,nslices=20,xlab="Column",
                   ylab="Row",do.hex=FALSE,axes=TRUE,show.values=FALSE,vcol="white",vcex=1,
                   border=NA,na.color=NA,main=varia[wi])
 
 #### Generate ASCII grids
 INSIDE[INSIDE==NA]=nodata
 CELL<<-celda
 XLLCORNER=x_min
 YLLCORNER=y_min
 n_cols=dim(XII)[2]
 n_rows=dim(XII)[1]
 
 setwd(output_dir)
 
 x = GridTopology(c(XLLCORNER+(0.5*CELL),YLLCORNER+(0.5*CELL)), c(CELL,CELL), c(n_cols,n_rows))   #(XLLCORNER-YLLCORNER, CELLSIZE-CELLSIZE, NCOLS-NROWS)
 class(x)                                                               ## THIS GRIDTOPOLOGY WORKS CELL CENTERED
 summary(x)
 y = SpatialGrid(grid = x)
 class(y)
 YY= as.data.frame(y)
 for (i in 1:n_rows) {
   YY[(((i-1)*n_cols)+1):(i*n_cols),1]=INSIDE[i,]
 }
 
 TT=SpatialGridDataFrame(x, YY)
 setwd(output_dir)
 write.asciigrid(TT, varia[wi],sep =" ")  
 
 
   ### PLOTTING THE DEM WITH MAPTOOLS READASCIIGRID ##
   colores=gray(100:0 / 100)
   DEMIM=readAsciiGrid(paste(output_dir,varia[wi],sep = ""), as.image = TRUE, plot.image = FALSE,
                       colname = varia[wi], proj4string = CRS(as.character(NA)),
                       dec=options()$OutDec)
   playwith({
     image.plot(DEMIM,col=colores,zlim=zlimites,grid=NULL)    # using the library fields
     if (plotchannels == 1) {
       plot(channels,add=TRUE)
     }
     polygon(xconstrain,yconstrain)
   },new=TRUE)
   


png(paste(output_dir,varia[wi],".png",sep=''))
image.plot(DEMIM,col=colores,zlim=zlimites)    # using the library fields
if (plotchannels==1) plot(channels,add=TRUE)
polygon(xconstrain,yconstrain)
dev.off()


playwith({
  image.plot(DEMIM)   # using the library fields
  if (plotchannels==1) {
    plot(channels,add=TRUE)
  }
  polygon(xconstrain,yconstrain)
},new=TRUE)

png(paste(output_dir,varia[wi],"_color.png",sep=''))
image.plot(DEMIM)   # using the library fields
if (plotchannels==1) plot(channels,add=TRUE)
polygon(xconstrain,yconstrain)
dev.off()

  tiff(paste(output_dir,varia[wi],"_color.tiff",sep=''), 
       res = 550, pointsize = 8,height=3000,width=3000) #res = 300, pointsize = 5,height=1000,width=1000)
  image.plot(DEMIM,axes=FALSE,zlim=zlimites,col=tim.colors())   # using the library fields  col=rev(tim.colors())
  if (plotchannels==1) plot(channels,add=TRUE,lwd=1)
  polygon(xconstrain,yconstrain,lwd=1)
  dev.off() 
  
}
}

cat("End of processes at ",date(),fill=TRUE)


