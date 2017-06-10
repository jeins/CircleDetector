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
  MPI_Init(NULL, NULL);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  QImage source("test.gif");
  QString output("result.jpg");
  unsigned int min_r = 10, max_r = 100;
  
  CircleDetector cd;  
  //clock_t start_time = clock();
  double start_time = MPI_Wtime();

  // QImage result = circleDetector.detect(source, min_r, max_r);


  int* min_max_process;
  int *min_max_local = (int*) malloc(sizeof(int) * 2);
  
  if (world_rank == 0) {
    min_max_process = generateMinMaxEachProcess(min_r, max_r, world_size);
  }

  MPI_Scatter(min_max_process, 2, MPI_INT, min_max_local, 2, MPI_INT, 0, MPI_COMM_WORLD);

  QImage binary = cd.edges(source);
  QImage detection = source.convertToFormat(QImage::Format_RGB888);


  cd.tester(min_max_local[0], min_max_local[1], binary, detection, world_rank);

  //MPI_Gather(detection, 1, MPI_INT, detections, 1, MPI_INT, 0, MPI_COMM_WORLD);

printf("rank: %d | min: %d | max: %d", world_rank, min_max_local[0], min_max_local[1]);



  printf("Time taken: %f\n", (MPI_Wtime() - start_time));
  MPI_Finalize();

  //printf("Time taken: %.2fs\n", (double)(clock() - start_time)/CLOCKS_PER_SEC);
}
