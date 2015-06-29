#!/bin/bash

#EXEC=/home/zork/loclib/mpich-3.1.4/bin/mpiexec
EXEC=mpiexec
HOST=machines

#export LD_LIBRARY_PATH=/home/zork/loclib/mpich-3.1.4/lib/

APP=./main
#SZ=1073741824
#SZ=1024
#SZ=104857600 # 400 MB of int
#SZ=524288000 # 2GB of int
SZ=268435456 # 1GB of int
#${EXEC} --mca btl openib,self,sm --hostfile $HOST ${APP} --num_int=${SZ}
${EXEC} --mca btl ^tcp --hostfile $HOST ${APP} --num_int=${SZ}
