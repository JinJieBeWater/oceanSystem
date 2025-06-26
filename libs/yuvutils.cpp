#include "yuvutils.h"
#include <QtGlobal>

QRgb yuyv2rgb(int y, int u, int v)
{
  int r = y + 1.4075 * (v - 128);
  int g = y - 0.3455 * (u - 128) - 0.7169 * (v - 128);
  int b = y + 1.779 * (u - 128);
  return qRgb(qBound(0, r, 255), qBound(0, g, 255), qBound(0, b, 255));
}
