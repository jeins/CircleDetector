#include <QString>
#include <QRegExp>
#include <QDebug>
#include <QImageReader>
#include <time.h>
#include <mpi.h>
#include <math.h>
#include <QColor>

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
/*
//test new impl
int main()
{
  QImage source("test.gif");
  CircleDetector cd;  

  clock_t start_time = clock();

  QImage binary = cd.edges(source);
  QImage detection = source.convertToFormat(QImage::Format_RGB888);

  int ***tmpsA;
  int ***tmpsB;
  int ***tmpsC;

  cd.detect(10, 30, binary, detection, 1, tmpsA);
  cd.detect(30, 60, binary, detection, 2, tmpsB);
  cd.detect(60, 100, binary, detection, 3, tmpsC);

  int id = 0;
  for(int i=10; i<100; i++){
    if(i == 30 || i == 60){
      id = 0;
    }
    int threshold = 4.9 * i;
    #pragma omp parallel for collapse(2)
    for(int x = 0; x < binary.width(); x++)
    {
      for(int y = 0; y < binary.height(); y++)
      {
        int hough;

        if(i < 30){
          hough = tmpsA[id][x][y];
        } else if(i > 30 && i < 60){
          hough = tmpsB[id][x][y];
        }else{
          hough = tmpsC[id][x][y];
        }

        if(hough > threshold)
        {
          cd.draw_circle(detection, QPoint(x, y), i, Qt::yellow);
        }
      }
    }
    id++;
  }
  
  QString output("tester.jpg");
  detection.save(output);
  printf("Time taken: %.2fs\n", (double)(clock() - start_time)/CLOCKS_PER_SEC);
}
*/
int ***alloc3d(int l, int m, int n) {
    int *data = new int [l*m*n];
    int ***array = new int **[l];
    for (int i=0; i<l; i++) {
        array[i] = new int *[m];
        for (int j=0; j<m; j++) {
            array[i][j] = &(data[(i*m+j)*n]);
        }
    }
    return array;
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
 
  QImage binary = cd.edges(source);
  QImage detection = source.convertToFormat(QImage::Format_RGB888);

  // definition custom mpi datatype for 3d array
  int sizes[3]    = {max_r - min_r, binary.width(), binary.height()};        
  int subsizes[3] = {ceil((max_r - min_r) / world_size), binary.width(), binary.height()};     
  int starts[3]   = {0,0,0};                       
  MPI_Datatype type, subarrtype;
  MPI_Type_create_subarray(3, sizes, subsizes, starts, MPI_ORDER_C, MPI_INT, &type);
  MPI_Type_create_resized(type, 0, ceil((max_r - min_r) / world_size)*sizeof(int), &subarrtype);
  MPI_Type_commit(&subarrtype);

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

  int ***tmps;
  cd.detect(min_max_local[0], min_max_local[1], binary, detection, world_rank, tmps);

  // TODO:// fix mpi gather receive 3d array
  // int ***tmps;
  // int ***stmps = NULL;
  // if (world_rank == 0){
  //   stmps = alloc3d(max_r - min_r, binary.width(), binary.height());
  // }

  // cd.detect(min_max_local[0], min_max_local[1], binary, detection, world_rank, tmps);

  // MPI_Gather(&tmps[0][0][0], binary.width()*binary.height()*ceil((max_r - min_r) / world_size), MPI_INT, 
  //             stmps, 1, subarrtype,
  //             0, MPI_COMM_WORLD);
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