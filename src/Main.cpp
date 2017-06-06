#include <QString>
#include <QRegExp>
#include <QDebug>
#include <QImageReader>
#include <time.h>
#include <omp.h>

#include "CircleDetector.h"

int main()
{
  QImage source("test.gif");
  QString output("result.jpg");
  unsigned int min_r = 50, max_r = 200;
  
  CircleDetector circleDetector;  
  //clock_t start_time = clock();
  double start_time = omp_get_wtime();

  QImage result = circleDetector.detect(source, min_r, max_r);
  result.save(output);

  printf("Time taken: %f\n", (omp_get_wtime() - start_time));
  //printf("Time taken: %.2fs\n", (double)(clock() - start_time)/CLOCKS_PER_SEC);
}
