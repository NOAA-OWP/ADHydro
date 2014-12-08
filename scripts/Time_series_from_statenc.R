## Time_series_from_statenc.R file ##############################################################################################
## Hernan Moreno
## Oct 24, 2014
## This script reads the state.nc file from ADHydro and plots times series of average basin values of prescribed variables

rm(list=ls(all=TRUE))
require(playwith)
require(tcltk)
require(ncdf4)
## INPUT DATA ###################################################################################################################

outpath="/localstore/output/"   ## Path to read output data
filen="state.nc"                # root name of a set of files to read in sequential order
nout=c(1,2) #c(1,2,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,24,25,26,27,29,31,33,34,35,36,37,38,39,40,41,42,43,44,45,47,48,50,52,53,54,55,56,58,59,
       #60,61,62,64,65,66,67,68,69,71,72,73,75,76,77,78,79,80)                   # number of outputs to read
toplot=c("meshSurfacewaterDepth","meshGroundwaterHead","channelSurfacewaterDepth")

##################################################################################################################################

ex.ncp = nc_open(paste(outpath, nout[1],filen, sep="")) # it opens the first file
Rdump=capture.output(print(ex.ncp), file = NULL, append = FALSE)
write.table(Rdump,paste(outpath,"Pncdump.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   ## dumps netcdf headers to a file in outfolder  

for (j in 1:length(nout)){
    cat("Reading state.nc # ", nout[j], fill=TRUE)
    ex.ncp = nc_open(paste(outpath, nout[j],filen, sep="")) # it opens the first file 
    dt=ncvar_get(ex.ncp, "dt")
    ctime=ncvar_get(ex.ncp, "currentTime")
    basinave=matrix(NA,length(ctime),length(toplot))  # Rows are the time steps, columns are the basin-averaged variables
  
    for (pli in 1: length(toplot)){
    cat("Averaging column ", pli, " of ", length(toplot), fill=TRUE)
    cual=toplot[pli]
    ordenada=cual
    ordenada=as.name(ordenada)
    Ind = ncvar_get(ex.ncp, toplot[pli]) # rows are the number of nodes. Columns are the outpout time steps.
      for (ti in 1:length(ctime)){
      basinave[ti,pli]=mean(Ind[,ti])
      }
    }
    if (j==1) {
      colnames(basinave)=toplot
      basinaveold=basinave
      dtold=dt
      ctimeold=ctime
    } else {
      basinaveold=rbind(basinaveold,basinave)
      dtold=c(dtold,dt)
      ctimeold=c(ctimeold,ctime)
    }
}

dt=dtold
ctime=ctimeold/3600
basinave=basinaveold
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
postscript(paste(outpath,"basin_vars_figure1.ps",sep=""),width=8000,height=5500)
mat=matrix(1:(length(toplot)+1),length(toplot)+1,1)
layout(mat)
par(mar=c(3.9,4,3,3.5))
plot(ctime,dt,type ="l",xlab="time [hr]",ylab="value",main="Time step",
     pch=22, col="red", cex.axis=1,panel.first = grid(),cex.lab=1)
for (i in 1:length(toplot)) {
cual=toplot[i]
ordenada=as.name(cual)
plot(ctime,basinave[,cual],type ="l",xlab="time [hr]",ylab="value",main=cual,
pch=22, col="blue", cex.axis=1,panel.first = grid(),cex.lab=1)
cat("mean of ",cual,"=",mean(basinave[,cual]),fill=TRUE)
}
dev.off()
#},new = TRUE)



### Plots individual variables
#for (i in 1:length(toplot)){
#    cuyo=paste("a",as.character(i),sep="")
#    playwith(plot(ctime,basinave[,i],type ="l",xlab="Time (h)",ylab=toplot[i],
#    pch=22, col="purple", cex.axis=1,panel.first = grid()),new = TRUE)
#    }


	msg <- "A figure with variables has been exported in JPEG to the outpath folder"
	tkmessageBox(message=msg)

  #write.table(vectorr,file=name_s,quote=FALSE,row.names=FALSE,col.names=FALSE)
  #msg <- "The matrix has been finished"
	#tkmessageBox(message=msg)
