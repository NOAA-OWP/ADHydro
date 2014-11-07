## Reads and joins state.nc outputs to be read in Paraview #####################################################################
## Hernan Moreno
## Nov 5 2014
## 

rm(list=ls(all=TRUE))
require(playwith)
require(tcltk)
require(ncdf4)
## INPUT DATA ###################################################################################################################

outpath="/localstore/output/"   ## Path to read output data
filen="state.nc"                # root name of a set of files to read in sequential order
nout=c(1,2,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,24,25,26)                   # number of outputs to read
ex.ncp = nc_open(paste(outpath, nout[1],filen, sep="")) # it opens the first file
Rdump=capture.output(print(ex.ncp), file = NULL, append = FALSE)
write.table(Rdump,paste(outpath,"Pncdump.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   ## dumps netcdf headers to a file in outfolder  
meshel<<-36861  #This value can be extracted from line 21
chanel<<-14974  #This value can be extracted from line 21
inst<<-9        # initial number of time steps output in the 1state.nc file

# define the netcdf coordinate variables -- note that these have values!
  dim1 = ncdim_def( "instances","",seq(1,inst), unlim=TRUE)
  dim2= ncdim_def( "meshElements","",seq(1,meshel),unlim=TRUE) #
  dim3= ncdim_def( "channelElements","",seq(1,chanel),unlim=TRUE) #

  # define the EMPTY netcdf variables
  varITERATION=ncvar_def("iteration","", list(dim1), -99999, longname="iteration",prec="integer")
  varCURRENTTIME=ncvar_def("currentTime","", list(dim1),longname="currentTime",prec="double")
  varDT=ncvar_def("dt","", list(dim1), -99999, longname="dt",prec="double")
  varGEOMETRYINSTANCE=ncvar_def("geometryInstance","", list(dim1), -99999, longname="geometryInstance",prec="integer")
  varPARAMETERINSTANCE=ncvar_def("parameterInstance","", list(dim1), -99999, longname="parameterInstance",prec="integer")
  varMESHSURFACEWATERDEPTH = ncvar_def("meshSurfacewaterDepth","", list(dim2,dim1), -99999, longname="meshSurfacewaterDepth",prec="double")
  varMESHSURFACEWATERERROR = ncvar_def("meshSurfacewaterEror","", list(dim2,dim1), -99999, longname="meshSurfacewaterEror",prec="double")
  varMESHGROUNDWATERHEAD = ncvar_def("meshGroundwaterHead","", list(dim2,dim1), -99999, longname="meshGroundwaterHead",prec="double")
  varMESHGROUNDWATERERROR = ncvar_def("meshGroundwaterError","", list(dim2,dim1), -99999, longname="meshGroundwaterError",prec="double")
  varCHANNELSURFACEWATERDEPTH = ncvar_def("channelSurfacewaterDepth","", list(dim3,dim1), -99999, longname="channelSurfacewaterDepth",prec="double")
  varCHANNELSURFACEWATERERROR = ncvar_def("channelSurfacewaterError","", list(dim3,dim1), -99999, longname="channelSurfacewaterError",prec="double")

  nc.ex = nc_create(paste(outpath,"Allstate.nc",sep=""),list(varITERATION, varCURRENTTIME, varDT, varGEOMETRYINSTANCE, varPARAMETERINSTANCE, varMESHSURFACEWATERDEPTH,
          varMESHSURFACEWATERERROR, varMESHGROUNDWATERHEAD, varMESHGROUNDWATERERROR, varCHANNELSURFACEWATERDEPTH, varCHANNELSURFACEWATERERROR),force_v4=TRUE)

tini=1
for (ii in 1:length(nout)){
  cat("Processing file # ",nout[ii],filen, sep="",fill=TRUE)
  ex.ncp = nc_open(paste(outpath, nout[ii],filen, sep="")) # it opens the first file
  if (ii==1){
  uno=ncvar_get(ex.ncp, "iteration")
  dos=ncvar_get(ex.ncp, "currentTime")
  tres=ncvar_get(ex.ncp, "dt")
  cuatro=ncvar_get(ex.ncp, "geometryInstance")
  cinco=ncvar_get(ex.ncp, "parameterInstance")
  seis=ncvar_get(ex.ncp, "meshSurfacewaterDepth")
  siete=ncvar_get(ex.ncp, "meshSurfacewaterError")
  ocho=ncvar_get(ex.ncp, "meshGroundwaterHead")
  nueve=ncvar_get(ex.ncp, "meshGroundwaterError")
  diez=ncvar_get(ex.ncp, "channelSurfacewaterDepth")
  once=ncvar_get(ex.ncp, "channelSurfacewaterError")
    } else {
    uno=ncvar_get(ex.ncp, "iteration",start=2)
    dos=ncvar_get(ex.ncp, "currentTime",start=2)
    tres=ncvar_get(ex.ncp, "dt",start=2)
    cuatro=ncvar_get(ex.ncp, "geometryInstance",start=2)
    cinco=ncvar_get(ex.ncp, "parameterInstance",start=2)
    seis=ncvar_get(ex.ncp, "meshSurfacewaterDepth",start=c(1,2))
    siete=ncvar_get(ex.ncp, "meshSurfacewaterError",start=c(1,2))
    ocho=ncvar_get(ex.ncp, "meshGroundwaterHead",start=c(1,2))
    nueve=ncvar_get(ex.ncp, "meshGroundwaterError",start=c(1,2))
    diez=ncvar_get(ex.ncp, "channelSurfacewaterDepth",start=c(1,2))
    once=ncvar_get(ex.ncp, "channelSurfacewaterError",start=c(1,2))
   }
  
  if (ii==1){
    tini=1
    tf=length(uno)
  } else {
    tini=tf+1
    tf=tini+length(uno)-1
  }
    
  ncvar_put(nc.ex, varITERATION, uno, start=tini, count=length(uno))
  ncvar_put(nc.ex, varCURRENTTIME, dos, start=tini, count=length(uno))
  ncvar_put(nc.ex, varDT, tres, start=tini, count=length(uno))
  ncvar_put(nc.ex, varGEOMETRYINSTANCE, cuatro, start=tini, count=length(uno))
  ncvar_put(nc.ex, varPARAMETERINSTANCE, cinco, start=tini, count=length(uno))
  ncvar_put(nc.ex, varMESHSURFACEWATERDEPTH, seis,start=c(1,tini),count=c(meshel,length(uno)))
  ncvar_put(nc.ex, varMESHSURFACEWATERERROR, siete,start=c(1,tini),count=c(meshel,length(uno)))
  ncvar_put(nc.ex, varMESHGROUNDWATERHEAD, ocho,start=c(1,tini),count=c(meshel,length(uno)))
  ncvar_put(nc.ex, varMESHGROUNDWATERERROR, nueve,start=c(1,tini),count=c(meshel,length(uno)))
  ncvar_put(nc.ex, varCHANNELSURFACEWATERDEPTH, diez,start=c(1,tini),count=c(chanel,length(uno)))
  ncvar_put(nc.ex, varCHANNELSURFACEWATERERROR, once,start=c(1,tini),count=c(chanel,length(uno)))
  
}

nc_close(nc.ex)


## Verificatoin of written files
ex.ncpo = nc_open(paste(outpath,"Allstate.nc", sep="")) # it opens the first file
Rdumpo=capture.output(print(ex.ncp), file = NULL, append = FALSE)
write.table(Rdumpo,paste(outpath,"Allncdumpo.txt",sep=""),quote=FALSE,col.names=FALSE,row.names=FALSE)   ## dumps netcdf headers to a file in outfolder  


