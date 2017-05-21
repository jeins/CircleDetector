#include <QImage>
#include <QVector>

typedef QVector<unsigned int> IntArray;
typedef QVector<IntArray>     Image;

class CircleDetector
{ 
  public: 
    QImage detect(const QImage &source, unsigned int min_r, unsigned int max_r);
  
  private:
    void accum_circle(Image &image, const QPoint &position, unsigned int radius);
    void accum_pixel(Image &image, const QPoint &position);
  
    void draw_circle(QImage &image, const QPoint &position, unsigned int radius, const QColor &color);
    void draw_pixel(QImage &image, const QPoint &position, const QColor &color);
    
    QImage edges(const QImage &source);
};