#include <QImage>
#include <QVector>

typedef QVector<unsigned int> IntArray;
typedef QVector<IntArray>     Image;

class CircleDetector
{ 
  public: 
    QImage detect(const QImage &source, int min_r, int max_r);
    void tester(int min_r, int max_r, const QImage &binary, QImage &detection, int rank);
    QImage edges(const QImage &source);
  
  private:
    void accum_circle(Image &image, const QPoint &position, int radius);
    void accum_pixel(Image &image, const QPoint &position);
  
    void draw_circle(QImage &image, const QPoint &position, int radius, const QColor &color);
    void draw_pixel(QImage &image, const QPoint &position, const QColor &color);
    
};