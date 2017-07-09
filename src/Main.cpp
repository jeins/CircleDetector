#include <QString>
#include <QRegExp>
#include <QDebug>
#include <QImageReader>
#include <time.h>
#include <mpi.h>
#include <math.h>
#include <QColor>
#include <iostream>
#include <stdlib.h>

#include "CircleDetector.h"

int *generate_min_max_radius(int min, int max, int comSize)
{
  int sumEachProcess = ceil((max - min) / comSize);
  int n = comSize * 2;
  int *arrProcess = new int[n];
  int tmpN = min;

  bool r = false;
  for (int i = 0; i < n; i++)
  {
    if (i != 0 && i % 2 != 0)
    {
      arrProcess[i] = tmpN;
      continue;
    }

    if (i == (n - 1) || tmpN > max)
    {
      arrProcess[i] = max;
    }
    else
    {
      arrProcess[i] = tmpN;
    }

    tmpN += sumEachProcess;
  }

  return arrProcess;
}

int main(int argc, char *argv[])
{
  QString out_img, src_img;
  unsigned int min_rad = 10, max_rad = 100;

  QRegExp rx_src_img("^--src=(.*)$");
  QRegExp rx_out_img("^--out=(.*)$");
  QRegExp rx_minr("^--minr=(.*)$");
  QRegExp rx_maxr("^--maxr=(.*)$");

  for (int i = 0; i < argc; i++)
  {
    QString arg = argv[i];

    if (rx_src_img.indexIn(arg) != -1)
    {
      src_img = rx_src_img.cap(1);
    }
    else if (rx_out_img.indexIn(arg) != -1)
    {
      out_img = rx_out_img.cap(1);
    }
    else if (rx_minr.indexIn(arg) != -1)
    {
      min_rad = rx_minr.cap(1).toInt();
    }
    else if (rx_maxr.indexIn(arg) != -1)
    {
      max_rad = rx_maxr.cap(1).toInt();
    }
  }

  QImage source_img(src_img);

  if (out_img.isEmpty())
  {
    out_img = QString("%1.out.jpg").arg(src_img);
  }

  double duration, starttime, endtime;
  MPI_Init(NULL, NULL);
  starttime = MPI_Wtime();

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  CircleDetector cd;
  QImage binary = cd.edges(source_img);
  QImage detection;

  int *min_max_radius;
  int *min_max__rad_local = (int *)malloc(sizeof(int) * 2);

  if (world_rank == 0)
  {
    min_max_radius = generate_min_max_radius(min_rad, max_rad, world_size);
    detection = source_img.convertToFormat(QImage::Format_RGB888);
  }

  //get processor name
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

  MPI_Request req;
  MPI_Iscatter(min_max_radius, 2, MPI_INT,
               min_max__rad_local, 2, MPI_INT,
               0, MPI_COMM_WORLD, &req);
  MPI_Wait(&req, MPI_STATUS_IGNORE);

  double start_time = MPI_Wtime();

  cd.detect(min_max__rad_local[0], min_max__rad_local[1], binary, detection, world_rank, world_size);

  //calc exec time
  double duration_time = MPI_Wtime() - start_time;

  printf("Done! PName: %s| Rank: %d | min: %d | max: %d | Elapsed time: %g\n", processor_name, world_rank, min_max__rad_local[0], min_max__rad_local[1], duration_time);

  //save image
  if (world_rank == 0)
  {
    detection.save(out_img);
    free(min_max__rad_local);

    printf("GLobal time %g\n", MPI_Wtime() - starttime);
  }

  MPI_Finalize();

  return 0;
}