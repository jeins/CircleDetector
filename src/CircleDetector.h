#include <QImage>
#include <QVector>
#include <mpi.h>

typedef QVector<unsigned int> IntArray;
typedef QVector<IntArray>     Image;

class CircleDetector
{
  public: 
    void detect(int min_r, int max_r, const QImage &binary, QImage &detection, int rank, int size);
    QImage edges(const QImage &source);
    void draw_circle(QImage &image, const QPoint &position, int radius, const QColor &color);
    void accum_circle(Image &image, const QPoint &position, int radius);
    void accum_pixel(Image &image, const QPoint &position);
    void draw_pixel(QImage &image, const QPoint &position, const QColor &color);
    
};
