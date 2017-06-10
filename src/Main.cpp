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
  unsigned int min_r = 50, max_r = 200;
  
  CircleDetector cd;  
  //clock_t start_time = clock();
  // double start_time = omp_get_wtime();

  // QImage result = circleDetector.detect(source, min_r, max_r);

  QImage binary = cd.edges(source);
  QImage detection = source.convertToFormat(QImage::Format_RGB888);

  int* min_max_process;
  int *min_max_local = (int*) malloc(sizeof(int) * 2);;
  
  if (world_rank == 0) {
    min_max_process = generateMinMaxEachProcess(50, 200, world_size);
  }

  MPI_Scatter(min_max_process, 2, MPI_INT, min_max_local, 2, MPI_INT, 0, MPI_COMM_WORLD);

//cd.tester(min_max_local[0], min_max_local[1], binary, detection);



  MPI_Finalize();

  // printf("Time taken: %f\n", (omp_get_wtime() - start_time));
  //printf("Time taken: %.2fs\n", (double)(clock() - start_time)/CLOCKS_PER_SEC);
}
