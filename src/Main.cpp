#include <QString>
#include <QRegExp>
#include <QDebug>
#include <QImageReader>

#include "CircleDetector.h"

int main()
{
  QImage source("test.gif");
  QString output("result.jpg");
  unsigned int min_r = 50, max_r = 200;
  
  CircleDetector circleDetector;  
  QImage result = circleDetector.detect(source, min_r, max_r);
  result.save(output);
}
