# This script runs ADHydro iteratively in the cluster by using a pre-built jobscript and using 
# the last state.nc file to re-start from the point it was stooped.  CAVEATS:
# 1. Need to connect the cluster link to a folder as a disk in your computer called using sshfs. 
#    See: http://embraceubuntu.com/2005/10/28/how-to-mount-a-remote-ssh-filesystem-using-sshfs/ 
# 2. Need to generate automatic password for ssh through ssh key authentication: keygen, etc: 
#    http://www.youtube.com/watch?v=R65HTJeObkI (SSH logins without passwords). In a terminal type: 
#    1) ssh-keygen. Then hit enter twice when asked for file and authentication; type: 
#    2) ssh-copy-id -i .ssh/id_rsa.pub hamoreno@saguaro.fulton.asu.edu and its done!
# 3. The present script needs to be saved on a local (not in cluster) folder.
# 4. You can run this script by typing on a terminal: R CMD BATCH Run_ADHydro_iteratively.R, 
#    or simply by openning R and typing: source("/media/D/Sourc.........")
# 5. The first iteration this program assumes everything is ready to submit a new job 
 

rm(list=ls(all=TRUE))   # INITIAL RUN..
require(audio)

#***********************************INPUT *********************************************************************************************************************
inpath<<-"/localstore/MtMoranLink/project/CI-WATER/tools/build/ADHydro/input/"   # link to input folder 
outpath<<-"/localstore/MtMoranLink/project/CI-WATER/tools/build/ADHydro/output/" # link to output folder
copypath<<-"/localstore/output/"                                                 # Backup folder
homepath<<-"/localstore/MtMoranLink/home/hmorenor/"                               # link to home folder 
parallel_jobscript<<-"job_script"
iterations<<-1000                                                                # Total number of times joscripts are expected to be sumbitted
user<<-"hmorenor"
mtmoran_sen<<-"ssh hmorenor@mtmoran.uwyo.edu qsub /project/CI-WATER/tools/build/ADHydro/input/job_script"
mtmoran_sta<<-"ssh hmorenor@mtmoran.uwyo.edu qstat -u "
mtmoran_del<<-"ssh hmorenor@mtmoran.uwyo.edu qdel"
nchecks<<-800000  # number of iterative checks separated by about 30 seconds that the program makes during a run  
checko<<-20       # use the last checko iterations to check that the model does not get stopped at the beggining of the simulation
num=27  #  Consecutive number of state.nc outputs... careful when restarting this script to avoid overwriting outputs
#*************************************************************************************************************************************************************

# A new job is submitted
wait(system(mtmoran_sen,intern=FALSE, ignore.stderr = FALSE,wait=FALSE),timeout=10) # do  not run this line if cluster is already running a jobscript

checkpoints=seq(checko,nchecks,checko)
inforjobold=0
# Program begins checking when "C" signal is found from the qstat command
statusold="Q"
for (to in 1:nchecks){   # checks for 1000000 minutes ! 
  
  # Checking status of simulation until it gets "C"
  
  sj<-wait(system(paste(mtmoran_sta,user," >", copypath,"getex.txt",sep=""),intern=FALSE, ignore.stderr = FALSE,wait=FALSE),timeout=10)
  infor<-file.info(paste(copypath,"getex.txt",sep=""))
  SAL=read.table(paste(copypath,"getex.txt",sep=""),skip=5)
  status=as.character(SAL[dim(SAL)[1],10])
  cat("Status = ",status,fill=TRUE)
  
  # Checking that ADHydro.oxxxxxx file shows some change every X checking iterations 
  if (status=="R"){
  deco=as.character(SAL[dim(SAL)[1],1])
  pa=as.numeric(substring(deco, 1:nchar(deco),1:nchar(deco)))
  dondenas=which(is.na(pa)==TRUE)
  jobid=paste(pa[1:(dondenas[1]-1)],collapse="")
  filejob=paste(homepath,"ADHydro.o",jobid,sep="")
  inforjob<-file.info(filejob)[1,1]
  if (is.element(to,checkpoints)==TRUE){
    cat("checking size of ADHydro.o file to determine if runs are stopped",fill=TRUE)
    if (inforjob == inforjobold) {
      status="C"
      cat("Status = ",status," due to early stop",fill=TRUE)  
      wait(system(paste(mtmoran_del,jobid,sep=" "),intern=FALSE, ignore.stderr = FALSE,wait=FALSE),timeout=10) # delete jobscript
      lista=dir(outpath)
      cat("Removing files in ",outpath,fill=TRUE)
      cat(lista,fill=TRUE)
      file.remove(paste(outpath,lista,sep=""))
      cat("Removing ADHydro.oxxxx  file",fill=TRUE)
      file.remove(paste(homepath,"ADHydro.o",jobid,sep=""))
      Sys.sleep(60)  
    }
    inforjobold=inforjob
  }
  }  
  
  if (statusold=="R" & status=="C"){
      cat("Copying state.nc to ",copypath,fill=TRUE)
      file.copy(paste(outpath,"state.nc",sep=""), paste(copypath,num,"state.nc",sep=""),overwrite=FALSE)
      lista=dir(outpath)
      #donde=grep("ADHydro",lista)
      #cat("Copying ADHydro.o file to ",copypath,fill=TRUE)
      #file.copy(paste(outpath,lista[donde],sep=""), paste(copypath,to,"_",lista[donde],sep=""),overwrite=FALSE)
      cat("Copying state.nc to ",inpath,fill=TRUE)
      file.copy(paste(outpath,"state.nc",sep=""), paste(inpath,"state.nc",sep=""),overwrite=TRUE)
      cat("Removing files in ",outpath,fill=TRUE)
      cat(lista,fill=TRUE)
      file.remove(paste(outpath,lista,sep=""))
      cat("Removing ADHydro.oxxxx  file",fill=TRUE)
      file.remove(paste(homepath,"ADHydro.o",jobid,sep=""))
      num=num+1
      Sys.sleep(60)
      # Re-submitting a new job  
      wait(system(mtmoran_sen,intern=FALSE, ignore.stderr = FALSE,wait=FALSE),timeout=10)
      Sys.sleep(30)
    }
  
  if (statusold=="Q" & status=="C"){
    cat("Statusold=Q and status=C not copying files and re-submitting job",fill=TRUE)
    num=num+1
    Sys.sleep(60)
    # Re-submitting a new job  
    wait(system(mtmoran_sen,intern=FALSE, ignore.stderr = FALSE,wait=FALSE),timeout=10)
    Sys.sleep(30)
  }
  
tiempo<<-as.character(Sys.time())
cat(tiempo,fill=TRUE)
Sys.sleep(20) # every 30 secodns check qstat -u
statusold=status
}



