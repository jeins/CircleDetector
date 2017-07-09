#include "CircleDetector.h"

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

/*
* detect circle with hough transform
* draw founded circle on output image
*/
void CircleDetector::detect(int min_r, int max_r, const QImage &binary, QImage &detection, int rank, int size)
{
  printf("rank %d start | min: %d | max: %d\n", rank, min_r, max_r);
  QVector<Image> houghs(max_r - min_r);

  MPI_Status status;
  MPI_Datatype rtype;
  MPI_Request request = MPI_REQUEST_NULL;
  MPI_Request req = MPI_REQUEST_NULL;
  MPI_Type_contiguous(3, MPI_INT, &rtype);
  MPI_Type_commit(&rtype);

#pragma omp parallel for
  for (int i = min_r; i < max_r; i++)
  {
    Image &hough = houghs[i - min_r];
    hough.resize(binary.width());

#pragma omp parallel for
    for (int x = 0; x < hough.size(); x++)
    {
      hough[x].resize(binary.height());

#pragma omp parallel for
      for (int y = 0; y < hough[x].size(); y++)
      {
        hough[x][y] = 0;
      }
    }

#pragma omp parallel for collapse(2)
    for (int x = 0; x < binary.width(); x++)
    {
      for (int y = 0; y < binary.height(); y++)
      {
        if (binary.pixelIndex(x, y) == 1)
        {
          accum_circle(hough, QPoint(x, y), i);
        }
      }
    }

    int threshold = 4.9 * i;
#pragma omp parallel for collapse(2)
    for (int x = 0; x < hough.size(); x++)
    {
      for (int y = 0; y < hough[x].size(); y++)
      {
        int local_data[] = {i, x, y, hough[x][y]};
        int *global_data = NULL;
        if (rank == 0)
        {
          global_data = (int *)malloc(sizeof(int) * size * 3);
        }

        if (hough[x][y] > threshold)
        {
          MPI_Isend(local_data, 3, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
        }

        if (rank == 0)
        {
          int flag = 0;
          MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, &status);

          if (flag)
          {
            MPI_Irecv(global_data, 1, rtype, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &req);

            int rVal = global_data[0];
            int xVal = global_data[1];
            int yVal = global_data[2];

            draw_circle(detection, QPoint(xVal, yVal), rVal, Qt::yellow);
          }
        }

        free(global_data);
      }
    }
  }

  MPI_Wait(&req, &status);
  MPI_Wait(&request, &status);
  printf("rank %d done | min: %d | max: %d\n", rank, min_r, max_r);
}

/*
* accumulates a circle on the specified image at the specified position with
* the specified radius, using the midpoint circle drawing algorithm
*/
void CircleDetector::accum_circle(Image &image, const QPoint &position, int radius)
{
  int f = 1 - radius;
  int ddF_x = 1;
  int ddF_y = -2 * radius;
  int x = 0;
  int y = radius;

  accum_pixel(image, QPoint(position.x(), position.y() + radius));
  accum_pixel(image, QPoint(position.x(), position.y() - radius));
  accum_pixel(image, QPoint(position.x() + radius, position.y()));
  accum_pixel(image, QPoint(position.x() - radius, position.y()));

  while (x < y)
  {
    if (f >= 0)
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }

    x++;
    ddF_x += 2;
    f += ddF_x;

    accum_pixel(image, QPoint(position.x() + x, position.y() + y));
    accum_pixel(image, QPoint(position.x() - x, position.y() + y));
    accum_pixel(image, QPoint(position.x() + x, position.y() - y));
    accum_pixel(image, QPoint(position.x() - x, position.y() - y));
    accum_pixel(image, QPoint(position.x() + y, position.y() + x));
    accum_pixel(image, QPoint(position.x() - y, position.y() + x));
    accum_pixel(image, QPoint(position.x() + y, position.y() - x));
    accum_pixel(image, QPoint(position.x() - y, position.y() - x));
  }
}

/*
* accumulates at the specified position
*/
void CircleDetector::accum_pixel(Image &image, const QPoint &position)
{
  if (position.x() < 0 || position.x() >= image.size() ||
      position.y() < 0 || position.y() >= image[position.x()].size())
  {
    return;
  }

  image[position.x()][position.y()]++;
}

/*
* draws a circle on the output image, using the midpoint circle drawing algorithm
*/ 
void CircleDetector::draw_circle(QImage &image, const QPoint &position, int radius, const QColor &color)
{
  int f = 1 - radius;
  int ddF_x = 1;
  int ddF_y = -2 * radius;
  int x = 0;
  int y = radius;

  draw_pixel(image, QPoint(position.x(), position.y() + radius), color);
  draw_pixel(image, QPoint(position.x(), position.y() - radius), color);
  draw_pixel(image, QPoint(position.x() + radius, position.y()), color);
  draw_pixel(image, QPoint(position.x() - radius, position.y()), color);

  while (x < y)
  {
    if (f >= 0)
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }

    x++;
    ddF_x += 2;
    f += ddF_x;

    draw_pixel(image, QPoint(position.x() + x, position.y() + y), color);
    draw_pixel(image, QPoint(position.x() - x, position.y() + y), color);
    draw_pixel(image, QPoint(position.x() + x, position.y() - y), color);
    draw_pixel(image, QPoint(position.x() - x, position.y() - y), color);
    draw_pixel(image, QPoint(position.x() + y, position.y() + x), color);
    draw_pixel(image, QPoint(position.x() - y, position.y() + x), color);
    draw_pixel(image, QPoint(position.x() + y, position.y() - x), color);
    draw_pixel(image, QPoint(position.x() - y, position.y() - x), color);
  }
}

/*
* draws at the specified position
*/
void CircleDetector::draw_pixel(QImage &image, const QPoint &position, const QColor &color)
{
  if (position.x() < 0 || position.x() >= image.width() ||
      position.y() < 0 || position.y() >= image.height())
  {
    return;
  }

  image.setPixel(position, color.rgb());
}

/*
* detects edges in the specified QImage
*/
QImage CircleDetector::edges(const QImage &source)
{
  QImage binary = QImage(source.size(), QImage::Format_Mono);

  /*** Sobel edge detection ***/
  QVector<QByteArray> Lx(3), Ly(3);

  Lx[0][0] = -1;
  Lx[0][1] = +0;
  Lx[0][2] = +1;
  Lx[1][0] = -2;
  Lx[1][1] = +0;
  Lx[1][2] = +2;
  Lx[2][0] = -1;
  Lx[2][1] = +0;
  Lx[2][2] = +1;

  Ly[0][0] = +1;
  Ly[0][1] = +2;
  Ly[0][2] = +1;
  Ly[1][0] = +0;
  Ly[1][1] = +0;
  Ly[1][2] = +0;
  Ly[2][0] = -1;
  Ly[2][1] = -2;
  Ly[2][2] = -1;

#pragma omp parallel for collapse(2)
  for (int x = 0; x < source.width(); x++)
  {
    for (int y = 0; y < source.height(); y++)
    {
      double new_x = 0.0, new_y = 0.0;

#pragma omp parallel for collapse(2)
      for (int i = -1; i <= 1; i++)
      {
        for (int j = -1; j <= 1; j++)
        {
          int _x = x + i;
          int _y = y + j;

          if (_x < 0)
            _x = -_x;
          else if (_x >= source.width())
            _x = 2 * source.width() - _x - 2;

          if (_y < 0)
            _y = -_y;
          else if (_y >= source.height())
            _y = 2 * source.height() - _y - 2;

          int gray = qGray(source.pixel(_x, _y));
          new_x += Lx[i + 1][j + 1] * gray;
          new_y += Ly[i + 1][j + 1] * gray;
        }
      }

      // using 128 as a threshold, decide if the steepness is sufficient (= edge = 1)
      int pixel = sqrt(pow(new_x, 2) + pow(new_y, 2)) > 128 ? 1 : 0;
      binary.setPixel(x, y, pixel);
    }
  }

  return binary;
}
