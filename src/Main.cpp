#include <QString>
#include <QRegExp>
#include <QDebug>
#include <QImageReader>
#include <time.h>
#include <mpi.h>
#include <math.h>

#include "CircleDetector.h"

int* generateMinMaxEachProcess(int min, int max, int comSize)
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
  unsigned int min_r = 10, max_r = 100;

  CircleDetector cd;

  // QImage result = circleDetector.detect(source, min_r, max_r);

  int* min_max_process;
  int *min_max_local = (int*) malloc(sizeof(int) * 2);

  if (world_rank == 0) {
    min_max_process = generateMinMaxEachProcess(min_r, max_r, world_size);
  }

  //get processor name
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

  MPI_Scatter(min_max_process, 2, MPI_INT, min_max_local, 2, MPI_INT, 0, MPI_COMM_WORLD);

  MPI_Barrier(MPI_COMM_WORLD);
  double start_time = MPI_Wtime();
 
  QImage binary = cd.edges(source);
  QImage detection = source.convertToFormat(QImage::Format_RGB888);

  cd.tester(min_max_local[0], min_max_local[1], binary, detection, MPI_DOUBLE, MPI_COMM_WORLD);
  
  MPI_Barrier(MPI_COMM_WORLD);
  double duration_time = MPI_Wtime() - start_time;

  printf("PName: %s| Rank: %d | min: %d | max: %d | Elapsed time: %g\n",processor_name, world_rank,min_max_local[0], min_max_local[1],duration_time);

  endtime = MPI_Wtime();
  duration = endtime - starttime;

  MPI_Finalize();
  if(world_rank == 0){
 	 printf("GLobal time %g\n",duration);
  }
  return 0;
}
