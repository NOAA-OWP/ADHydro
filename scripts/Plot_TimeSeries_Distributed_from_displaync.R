## Time_series_from_statenc.R file ############################################################################################################
## Hernan Moreno
## Oct 24, 2014
## This script reads the state.nc file from ADHydro and plots times series of average basin values of prescribed variables

rm(list=ls(all=TRUE))
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
## INPUT DATA #################################################################################################################################

outpath="/localstore/MtMoranLink/project/CI-WATER/tools/build/ADHydro/output/"       # Path to read output data
filen="display.nc"                                                                   # root name of a set of files to read in sequential order
nout=c("")                                                                           # c(1,2) # a list with the serial number of outputs to read
output_dir<<-"/localstore/MtMoranLink/project/CI-WATER/tools/build/ADHydro/output/"  # directory where figures will be exported
toplotmeshm=c("meshGroundwaterHead","meshSurfacewaterDepth","meshSnEqv")             # List of variables to plot in meters per unit of time
toplotmeshmm=c("meshPrecipitation","meshEvaporation")                                # List of variables to plot in mm per unit of time
toplotchannel=c("channelSurfacewaterDepth")					     # List of variables to plot on channel nodes
#toplotneighbors=c("channelMeshNeighborsGroundwaterFlowRate")                        # List of variables to plot with neighbors
meshplot<<- 1                                                                        # 1 if mesh element time series must be plotted; 0 if no.
meshlist<<-c(32288,32278,32266, 32273, 32283, 32066, 32281, 32282)                  #List of mesh elements whose time series must be plotted only if meshplot=1		     
chanplot<<-1                                                                        # 1 if channel element time series must be plotted; 0 if not.
chanlist<<-c(100,102)                                                               # List of channel elements whose time series must be plotted only if meshplot=1  								          
## This module is optional only is spatial <<-1 ################################################################################################
spatial<<-0   				                                             # Plot spatially distributed variables for toplotmesh? 0 no, 1 yes
TINF<<-"/share/CI-WATER_Simulation_Data/small_green_mesh/geometry.nc"                # Netcdf file with the AD-Hydro mesh elements for which the WRF 
celda<<- 100                                                                         # Desired output cellsize
divisoria<<-"/user2/hmoreno/Documents/DEMS/small_green_mesh/XY_Border_points.csv"    # csv points file with the watershed divide
nodata<<- -9999                                                                      # nodata value
xtifflim<<-c(2260,  0.0,  0,          0)                                             # optional scale limits for the SPATIAL tiff plots only for toplotmeshm and toplotmeshmm
ytifflim<<-c(4062,  0.2,  5.2e-4,    65)
plotchannels<<-1                                                                     # optional do you want to add channels? 0=no, 1=yes
canales="/user2/hmoreno/Documents/DEMS/small_green_mesh/projectednet.shp"            # optional path and name of channels file
BB<<-100                                                                             # BB=2 always to start but can increase in multiples of 2 until divide looks good
################################################################################################################################################

cat("Reading display.nc netcdf files", fill=TRUE)
ex.ncp = nc_open(paste(outpath, nout[1],filen, sep=""))                                              # it opens the first file
Rdump=capture.output(print(ex.ncp), file = NULL, append = FALSE)
write.table(Rdump,paste(outpath,"Pncdump.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   # dumps netcdf headers to a file in outfolder  

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
  
  ## reading topography
  mesh.nc = nc_open(TINF) # it opens the geometry file
  Rdump_mesh=capture.output(print(mesh.nc), file = NULL, append = FALSE)
  #write.table(Rdump_mesh,paste(Outfolder,"Rncdump_mesh.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   ## dumps netcdf headers of geometry to a file in outfolder  
  TINX = ncvar_get(mesh.nc, "meshElementX") 
  TINY = ncvar_get(mesh.nc, "meshElementY")
  TINZ = ncvar_get(mesh.nc, "meshElementZSurface")
  TIN= cbind(seq(0,(length(TINX)-1)),TINX,TINY,TINZ)
  xtt=TINX
  ytt=TINY
  # PLotting topography
  ## interpolating and converting to ASCII
  x_min = min(TINX)-(20*celda)
  x_max = max(TINX)+(20*celda)
  
  y_min = min(TINY)-(20*celda)
  y_max = max(TINY)+(20*celda)
  
    
  ### Creates temporally averaged matrices for each variable
  meshm_dist=array(NA,c(length(TINX),length(toplotmeshm),length(nout)))
  colnames(meshm_dist)=toplotmeshm
  meshmm_dist=array(NA,c(length(TINX),length(toplotmeshmm),length(nout)))
  colnames(meshmm_dist)=toplotmeshmm
} 



for (j in 1:length(nout)){
    cat("Reading display.nc # ", nout[j], fill=TRUE)
    ex.ncp = nc_open(paste(outpath, nout[j],filen, sep="")) # it opens the first file 
    dt=ncvar_get(ex.ncp, "dt")
    ctime=ncvar_get(ex.ncp, "currentTime")
    cat("Number of output times = ", length(ctime),fill=TRUE)
    basinavemeshm=matrix(NA,length(ctime),length(toplotmeshm))  # Rows are the time steps, columns are the basin-averaged variables
    basinavemeshmm=matrix(NA,length(ctime),length(toplotmeshmm))  # Rows are the time steps, columns are the basin-averaged variables
    basinavechannel=matrix(NA,length(ctime),length(toplotchannel))  # Rows are the time steps, columns are the basin-averaged variables
    #basinaveneighbors=matrix(NA,length(ctime),length(toplotneighbors))  # Rows are the time steps, columns are the basin-averaged variables
    
        if (meshplot==1){
        meshavemeshm=array(NA,c(length(ctime),length(toplotmeshm),length(meshlist)))  # Rows are the time steps, columns are the basin-averaged variables,thickness is the number of mesh elements 
        meshavemeshmm=array(NA,c(length(ctime),length(toplotmeshmm),length(meshlist)))  # Rows are the time steps, columns are the basin-averaged variables,thickness is the number of mesh elements
        #meshaveneighbors=array(NA,c(length(ctime),length(toplotneighbors),length(meshlist)))  # Rows are the time steps, columns are the basin-averaged variables,thickness is the number of mesh elements
        }   
    
        if (chanplot==1){  
        meshavechannel=array(NA,c(length(ctime),length(toplotchannel),length(meshlist)))  # Rows are the time steps, columns are the basin-averaged variables,thickness is the number of mesh elements
        }    
    
    
    for (plim in 1: length(toplotmeshm)){
    cat("Averaging column ", toplotmeshm[plim], fill=TRUE)
    Ind = ncvar_get(ex.ncp, toplotmeshm[plim]) # rows are the number of nodes. Columns are the outpout time steps.
      for (ti in 1:length(ctime)){
      basinavemeshm[ti,plim]=mean(Ind[,ti])
         if (meshplot==1){
         meshavemeshm[ti,plim,]=Ind[meshlist+1,ti]    
         }
      }
      if (spatial==1){
      meshm_dist[,plim,j]=rowMeans(Ind)
      }    
    }

    for (plimm in 1: length(toplotmeshmm)){
    cat("Averaging column ", toplotmeshmm[plimm], fill=TRUE)
     Ind = ncvar_get(ex.ncp, toplotmeshmm[plimm]) # rows are the number of nodes. Columns are the outpout time steps.
      for (ti in 1:length(ctime)){
      basinavemeshmm[ti,plimm]=mean(Ind[,ti])
        if (meshplot==1){
        meshavemeshmm[ti,plimm,]=Ind[meshlist+1,ti]    
        }
      }
      if (spatial==1){
      meshmm_dist[,plimm,j]=rowMeans(Ind)
      }    
    }

    for (plic in 1: length(toplotchannel)){
    cat("Averaging column ", toplotchannel[plic], fill=TRUE)
    Ind = ncvar_get(ex.ncp, toplotchannel[plic]) # rows are the number of nodes. Columns are the outpout time steps.
      for (ti in 1:length(ctime)){
       basinavechannel[ti,plic]=mean(Ind[,ti])
         if (chanplot==1){
         meshavechannel[ti,plic,]=Ind[chanlist+1,ti]    
         }
      }
    }

    #for (plin in 1: length(toplotneighbors)){
    #cat("Averaging column ", toplotneighbors[plin], fill=TRUE)
    #Ind = ncvar_get(ex.ncp, toplotneighbors[plin]) # rows are the number of nodes. Columns are the outpout time steps.
      #for (ti in 1:length(ctime)){
       #basinaveneighbors[ti,plin]=mean(Ind[,,ti])
      #}
    #}


    if (j==1) {
      colnames(basinavemeshm)=toplotmeshm
      colnames(basinavemeshmm)=toplotmeshmm
      colnames(basinavechannel)=toplotchannel
      #colnames(basinaveneighbors)=toplotneighbors
      basinavemeshmold=basinavemeshm
      basinavemeshmmold=basinavemeshmm
      basinavechannelold=basinavechannel
      #basinaveneighborsold=basinaveneighbors
      if (meshplot==1){
      colnames(meshavemeshm)=toplotmeshm
      colnames(meshavemeshmm)=toplotmeshmm
      meshavemeshmold=meshavemeshm
      meshavemeshmmold=meshavemeshmm
      }
      if (chanplot==1){
      colnames(meshavechannel)=toplotchannel
      meshavechannelold=meshavechannel
      }
      #meshaveneighborsold=basinaveneighbors
      dtold=dt
      ctimeold=ctime
    } else {
      basinavemeshmold=rbind(basinavemeshmold,basinavemeshm)
      basinavemeshmmold=rbind(basinavemeshmmold,basinavemeshmm)
      basinavechannelold=rbind(basinavechannelold,basinavechannel)
      #basinaveneighborsold=rbind(basinaveneighborsold,basinaveneighbors)
      if (meshplot==1){
      meshavemeshmold=rbind(meshavemeshmold,meshavemeshm)
      meshavemeshmmold=rbind(meshavemeshmmold,meshavemeshmm)
      }
      if (chanplot==1){
      meshavechannelold=rbind(meshavechannelold,meshavechannel)
      }
      #basinaveneighborsold=rbind(basinaveneighborsold,basinaveneighbors)
      dtold=c(dtold,dt)
      ctimeold=c(ctimeold,ctime)
    }
}

dt=dtold
ctime=ctimeold/3600
basinavemeshm=basinavemeshmold
basinavemeshmm=basinavemeshmmold
basinavechannel=basinavechannelold
#basinaveneighbors=basinaveneighborsold
if (meshplot==1){
meshavemeshm=meshavemeshmold
meshavemeshmm=meshavemeshmmold
}
if (chanplot==1){
meshavechannel=meshavechannelold
}
#meshaveneighbors=meshaveneighborsold

#for (ploi in 1:length(toplot)) {
#cual=toplot[ploi]
#playwith(plot(ctime,basinave[,ploi],type ="l",xlab="time",ylab=cual,
#pch=22, col="black", cex.axis=1,panel.first = grid()),new = TRUE)
#}

## plot mosaic ##########################################################
#setwd(outpath)
#jpeg("ADHydro_state.jpg", width=750, height=900)
#mat=matrix(1:(length(toplot)+1),3,length(toplot)/3)
#layout(mat)
#par(mar=c(3.9,4,3,3.5))
#plot(ctime,dt,type ="l",xlab="time [hr]",ylab="value",main=cual,
#     pch=22, col="red", cex.axis=1,panel.first = grid(),cex.lab=1)
#for (i in 1:length(toplot)) {
#cual=toplot[i]
#ordenada=as.name(cual)
#plot(ctime,basinave[,cual],type ="l",xlab="time [hr]",ylab=cual,
#pch=22, col="red", cex.axis=2,panel.first = grid(),cex.lab=1.8)
#}
#dev.off()

#playwith({
postscript(paste(output_dir,"basin_vars_meshm.ps",sep=""),width=8000,height=5500)
mat=matrix(1:(length(toplotmeshm)+1),length(toplotmeshm)+1,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
plot(ctime,dt,type ="l",xlab="time [hr]",ylab="value",main="Time step",
     pch=22, col="red", cex.axis=1,panel.first = grid(),cex.lab=1)
for (i in 1:length(toplotmeshm)) {
cual=toplotmeshm[i]
ordenada=as.name(cual)
plot(ctime,basinavemeshm[,cual],type ="l",xlab="time [hr]",ylab="value",main=cual,col="blue", cex.axis=1,panel.first = grid(),cex.lab=1,
xlim=c(ctime[1],ctime[length(ctime)]),ylim=c(min(basinavemeshm[,cual]),max(basinavemeshm[,cual])))
polygon(c(ctime[1],ctime,ctime[length(ctime)],ctime[1]),c(min(basinavemeshm[,cual]),basinavemeshm[,cual],min(basinavemeshm[,cual]),min(basinavemeshm[,cual])),
col='skyblue',xlim=c(ctime[1],ctime[length(ctime)]),ylim=c(min(basinavemeshm[,cual]),max(basinavemeshm[,cual])))
cat("mean of ",cual,"=",mean(basinavemeshm[,cual]),fill=TRUE)
}
dev.off()
#},new = TRUE)
colores=c("blue","red","green","orange","purple")
#playwith({
postscript(paste(output_dir,"basin_vars_meshmm.ps",sep=""),width=8000,height=5500)
mat=matrix(1:(length(toplotmeshmm)+1),length(toplotmeshmm)+1,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
plot(ctime,dt,type ="l",xlab="time [hr]",ylab="value [m]",main="Time step",
     pch=22, col="red", cex.axis=1,panel.first = grid(),cex.lab=1)
for (i in 1:length(toplotmeshmm)) {
cual=toplotmeshmm[i]
ordenada=as.name(cual)
#plot(ctime,3600000*basinavemeshmm[,cual],type ="h",xlab="time [hr]",ylab="value [mm]",main=cual,
barplot(3600000*basinavemeshmm[,cual],,xlab="time [hr]",ylab="value [mm]", names.arg=round(ctime,digits=0),panel.first = grid(),main=cual,col=colores[i])
box()
cat("mean of ",cual,"=",mean(basinavemeshmm[,cual]),fill=TRUE)
}
dev.off()
#},new = TRUE)

#playwith({
postscript(paste(output_dir,"basin_vars_channel.ps",sep=""),width=8000,height=5500)
mat=matrix(1:(length(toplotchannel)+1),length(toplotchannel)+1,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
plot(ctime,dt,type ="l",xlab="time [hr]",ylab="value [m]",main="Time step",
     pch=22, col="red", cex.axis=1,panel.first = grid(),cex.lab=1)
for (i in 1:length(toplotchannel)) {
cual=toplotchannel[i]
ordenada=as.name(cual)
plot(ctime,basinavechannel[,cual],type ="l",xlab="time [hr]",ylab="value ",main=cual,
pch=22, col="darkgreen", cex.axis=1,panel.first = grid(),cex.lab=1)
points(ctime,basinavechannel[,cual],pch=19,cex=0.6,col="darkgreen")
cat("mean of ",cual,"=",mean(basinavechannel[,cual]),fill=TRUE)
}
dev.off()
#},new = TRUE)

##playwith({
#postscript(paste(output_dir,"basin_vars_neighbor.ps",sep=""),width=8000,height=5500)
#mat=matrix(1:(length(toplotneighbors)+1),length(toplotneighbors)+1,1)
#layout(mat)
#par(mar=c(3.9,4,3,3.5))
#plot(ctime,dt,type ="l",xlab="time [hr]",ylab="value [m]",main="Time step",
#     pch=22, col="red", cex.axis=1,panel.first = grid(),cex.lab=1)
#for (i in 1:length(toplotneighbors)) {
#cual=toplotneighbors[i]
#ordenada=as.name(cual)
#plot(ctime,basinaveneighbors[,cual],type ="l",xlab="time [hr]",ylab="value ",main=cual,
#pch=22, col="orange", cex.axis=1,panel.first = grid(),cex.lab=1)
#points(ctime,basinaveneighbors[,cual],pch=19,cex=0.6,col="orange")
#cat("mean of ",cual,"=",mean(basinaveneighbors[,cual]),fill=TRUE)
#}
#dev.off()
##},new = TRUE)

if (meshplot==1){
  for (mi in 1:length(meshlist)){
  postscript(paste(output_dir,"MeshEl_",meshlist[mi],"_vars_meshm.ps",sep=""),width=8000,height=5500)
  mat=matrix(1:(length(toplotmeshm)+1),length(toplotmeshm)+1,1)
  layout(mat)
  par(mar=c(3.9,4,3,3.5))
  plot(ctime,dt,type ="l",xlab="time [hr]",ylab="value",main="Time step",
       pch=22, col="red", cex.axis=1,panel.first = grid(),cex.lab=1)
    for (i in 1:length(toplotmeshm)) {
    cual=toplotmeshm[i]
    ordenada=as.name(cual)
    plot(ctime,meshavemeshm[,cual,mi],type ="l",xlab="time [hr]",ylab="value",main=cual,col="blue", cex.axis=1,panel.first = grid(),cex.lab=1,
         xlim=c(ctime[1],ctime[length(ctime)]),ylim=c(min(meshavemeshm[,cual,mi]),max(meshavemeshm[,cual,mi])))
    polygon(c(ctime[1],ctime,ctime[length(ctime)],ctime[1]),c(min(meshavemeshm[,cual,mi]),meshavemeshm[,cual,mi],min(meshavemeshm[,cual,mi]),min(meshavemeshm[,cual,mi])),
            col='skyblue',xlim=c(ctime[1],ctime[length(ctime)]),ylim=c(min(meshavemeshm[,cual,mi]),max(meshavemeshm[,cual,mi])))
    cat("mean of ",cual,"=",mean(meshavemeshm[,cual,mi]),fill=TRUE)
  }
  dev.off()
  }
  
  for (mi in 1:length(meshlist)){
    postscript(paste(output_dir,"MeshEl_",meshlist[mi],"_vars_meshmm.ps",sep=""),width=8000,height=5500)
    mat=matrix(1:(length(toplotmeshmm)+1),length(toplotmeshmm)+1,1)
    layout(mat)
    par(mar=c(3.9,4,3,3.5))
    plot(ctime,dt,type ="l",xlab="time [hr]",ylab="value",main="Time step",
         pch=22, col="red", cex.axis=1,panel.first = grid(),cex.lab=1)

     for (i in 1:length(toplotmeshmm)) {
     cual=toplotmeshmm[i]
     ordenada=as.name(cual)
     #plot(ctime,3600000*basinavemeshmm[,cual],type ="h",xlab="time [hr]",ylab="value [mm]",main=cual,
     barplot(3600000*meshavemeshmm[,cual,mi],xlab="time [hr]",ylab="value [mm]", names.arg=round(ctime,digits=0),panel.first = grid(),main=cual,col=colores[i])
     box()
     cat("mean of ",cual,"=",mean(meshavemeshmm[,cual,mi]),fill=TRUE)
     }
    dev.off()
  }
   
}

if (meshplot==1){
  for (mi in 1:length(meshlist)){
    postscript(paste(output_dir,"MeshEl_",meshlist[mi],"_vars_meshm.ps",sep=""),width=8000,height=5500)
    mat=matrix(1:(length(toplotmeshm)+1),length(toplotmeshm)+1,1)
    layout(mat)
    par(mar=c(3.9,4,3,3.5))
    plot(ctime,dt,type ="l",xlab="time [hr]",ylab="value",main="Time step",
         pch=22, col="red", cex.axis=1,panel.first = grid(),cex.lab=1)
    for (i in 1:length(toplotmeshm)) {
      cual=toplotmeshm[i]
      ordenada=as.name(cual)
      plot(ctime,meshavemeshm[,cual,mi],type ="l",xlab="time [hr]",ylab="value",main=cual,col="blue", cex.axis=1,panel.first = grid(),cex.lab=1,
           xlim=c(ctime[1],ctime[length(ctime)]),ylim=c(min(meshavemeshm[,cual,mi]),max(meshavemeshm[,cual,mi])))
      polygon(c(ctime[1],ctime,ctime[length(ctime)],ctime[1]),c(min(meshavemeshm[,cual,mi]),meshavemeshm[,cual,mi],min(meshavemeshm[,cual,mi]),min(meshavemeshm[,cual,mi])),
              col='skyblue',xlim=c(ctime[1],ctime[length(ctime)]),ylim=c(min(meshavemeshm[,cual,mi]),max(meshavemeshm[,cual,mi])))
      cat("mean of ",cual,"=",mean(meshavemeshm[,cual,mi]),fill=TRUE)
    }
    dev.off()
  }
}
  



## averaging individual nout files in meshm_dist and meshmm_dist
if (spatial==1){
  meshm_avenodes=matrix(NA,length(TINX),length(toplotmeshm))
  colnames(meshm_avenodes)=toplotmeshm
  for (pi in 1:length(toplotmeshm)){
   meshm_avenodes[,pi]=rowMeans(meshm_dist[,pi,])
   }
  meshm_dist<<-0
}

if (spatial==1){
  meshmm_avenodes=matrix(NA,length(TINX),length(toplotmeshmm))
  colnames(meshmm_avenodes)=toplotmeshmm
  for (pi in 1:length(toplotmeshmm)){
    meshmm_avenodes[,pi]=rowMeans(meshmm_dist[,pi,])
  }
  meshmm_dist<<-0
}



## Plotting spatial variables
# toplotmeshmm_dist
if (spatial==1){
join=cbind(meshm_avenodes,meshmm_avenodes)

for (bi in 1:dim(join)[2]){
  varia=c(toplotmeshm,toplotmeshmm)
  cat("Processing ",varia[bi],fill=TRUE)  
  zlimites<<-c(xtifflim[bi],ytifflim[bi]) 
  TINZ = join[,bi]
  TIN= cbind(seq(0,(length(TINX)-1)),TINX,TINY,TINZ)
  ztt=TINZ  

  cat("interpolating... may take some minutes",fill=TRUE)
  akima.li <- interp(xtt, ytt, ztt,xo=seq(x_min, x_max, length = (x_max-x_min)/celda),
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
                  border=NA,main=varia[bi],na.color=NA)
  
  
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
                  border=NA,na.color=NA,main=varia[bi])
  
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
  write.asciigrid(TT, varia[bi],sep =" ")  
  
  ### PLOTTING THE DEM WITH MAPTOOLS READASCIIGRID ##
  colores=gray(100:0 / 100)
  DEMIM=readAsciiGrid(paste(output_dir,varia[bi],sep = ""), as.image = TRUE, plot.image = FALSE,
                      colname = varia[bi], proj4string = CRS(as.character(NA)),
                      dec=options()$OutDec)
  playwith({
    image.plot(DEMIM,col=colores,zlim=zlimites,grid=NULL)    # using the library fields
    if (plotchannels == 1) {
      plot(channels,add=TRUE)
    }
    polygon(xconstrain,yconstrain)
  },new=TRUE)
  
  
  png(paste(output_dir,varia[bi],".png",sep=''))
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
  
  png(paste(output_dir,varia[bi],"_color.png",sep=''))
  image.plot(DEMIM)   # using the library fields
  if (plotchannels==1) plot(channels,add=TRUE)
  polygon(xconstrain,yconstrain)
  dev.off()
  
  tiff(paste(output_dir,varia[bi],"_color.tiff",sep=''), 
       res = 550, pointsize = 8,height=3000,width=3000) #res = 300, pointsize = 5,height=1000,width=1000)
  image.plot(DEMIM,axes=FALSE,zlim=zlimites,col=tim.colors())   # using the library fields  col=rev(tim.colors())
  if (plotchannels==1) plot(channels,add=TRUE,lwd=1)
  polygon(xconstrain,yconstrain,lwd=1)
  dev.off() 
  
}
}
  

