#include <QString>
#include <QRegExp>
#include <QDebug>
#include <QImageReader>
#include <time.h>
#include <mpi.h>
#include <math.h>
#include <QColor>

#include "CircleDetector.h"

int* generate_min_max_radius(int min, int max, int comSize)
{
  int sumEachProcess = ceil((max - min) / comSize);
  int n = comSize * 2;
  int* arrProcess = new int[n];
  int tmpN = min;

  bool r = false;
  for(int i=0; i < n; i++){
    if(i != 0 && i%2 != 0){
      arrProcess[i] = tmpN;
      continue;
    }

    if(i == (n-1) || tmpN > max){
      arrProcess[i] = max;
    } else{
      arrProcess[i] = tmpN;
    }

    tmpN += sumEachProcess;
  }

  return arrProcess;
}

int main()
{ 
  double duration, starttime, endtime;
  MPI_Init(NULL, NULL);
  starttime = MPI_Wtime();

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  QImage source("test.gif");
  QString output("result.jpg");
  unsigned int min_rad = 30, max_rad = 100;

  CircleDetector cd;
  QImage binary = cd.edges(source);
  QImage detection;

  int* min_max_radius;
  int *min_max__rad_local = (int*) malloc(sizeof(int) * 2);

  if (world_rank == 0) {
    min_max_radius = generate_min_max_radius(min_rad, max_rad, world_size);
    detection = source.convertToFormat(QImage::Format_RGB888);
  }

  //get processor name
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);
  
  // MPI_Request req;
  // MPI_Iscatter(min_max_radius, 2, MPI_INT, 
  //             min_max__rad_local, 2, MPI_INT, 
  //             0, MPI_COMM_WORLD, &req);
  // MPI_Wait(&req, MPI_STATUS_IGNORE);
  MPI_Scatter(min_max_radius, 2, MPI_INT, 
              min_max__rad_local, 2, MPI_INT, 
              0, MPI_COMM_WORLD);

  double start_time = MPI_Wtime();
  
  cd.detect(min_max__rad_local[0], min_max__rad_local[1], binary, detection, world_rank, world_size);
  
  //calc exec time
  MPI_Barrier(MPI_COMM_WORLD);
  double duration_time = MPI_Wtime() - start_time;

  printf("Done! PName: %s| Rank: %d | min: %d | max: %d | Elapsed time: %g\n",processor_name, world_rank,min_max__rad_local[0], min_max__rad_local[1],duration_time);

  //save image
  if(world_rank == 0){
    detection.save(output);
    free(min_max__rad_local);

 	  printf("GLobal time %g\n", MPI_Wtime() - starttime);
  }

  MPI_Finalize();
  
  return 0;
}