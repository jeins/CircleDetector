# CircleDetector
circle detector with hough-transformation algorithm on parallel computing.

## Environment
* Ubuntu 16.04(Master) & mini ubuntu(Slave)
* OpenMP & OpenMPI
* Qt5
* NFS (Network File System)
* OpenSSH

## Compile 
* $ qmake -project ##first time to create .pro
* $ qmake
* $ make
* $ export OMP_NUM_THREADS=4

## Setup Cluster
* $ sudo /etc/init.d/nfs-kernel-server start ##Master
* $ sudo mount ip_master:/parsys /parsys ##Slave

## Running
* add slave ip address to hosts file
* $ mpirun -n 3 -hostfile host ./CircleDetector --src=test.gif --out=test.out.jpg --minr=50 --maxr=100