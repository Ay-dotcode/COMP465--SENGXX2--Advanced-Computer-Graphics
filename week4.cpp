#include "Image.h"
#include <cmath>
#include <utility>
using namespace std;

void Midpoint(GrayscaleImage &img, int x1, int y1, int x2, int y2, int color) {
  if (x1 > x2) { // ensure left to right
    swap(x1, x2);
    swap(y1, y2);
  }
  int a = y1 - y2;
  int b = x2 - x1;
  int c = x1 * y2 - x2 * y1;

  double m = (float)-a / b;
  double d1, d2;

  if (m >= 0 && m <= 1) {
    for (; x1 <= x2; x1++) {
      img(x1, y1) = color;
      d1 = a * (x1) + b * (y1) + c;
      d2 = a * (x1) + b * (y1 + 1) + c;
      if (abs(d2) < abs(d1))
        y1++;
    }
  } else if (m > 1) {
    for (; y1 <= y2; y1++) {
      img(x1, y1) = color;
      d1 = a * (x1) + b * (y1) + c;
      d2 = a * (x1 + 1) + b * (y1) + c;
      if (abs(d2) < abs(d1))
        x1++;
    }
  } else if (m >= -1 && m < 0) {
    for (; x1 <= x2; x1++) {
      img(x1, y1) = color;
      d1 = a * (x1) + b * (y1) + c;
      d2 = a * (x1) + b * (y1 - 1) + c;
      if (abs(d2) < abs(d1))
        y1--;
    }
  } else if (m < -1) {
    for (; y1 >= y2; y1--) {
      img(x1, y1) = color;
      d1 = a * (x1) + b * (y1) + c;
      d2 = a * (x1 - 1) + b * (y1) + c;
      if (abs(d2) < abs(d1))
        x1--;
    }
  }
}

void DDA(GrayscaleImage &img, int x1, int y1, int x2, int y2, int color) {
  double m = (double)(y2 - y1) / (x2 - x1);

  if (fabs(m) < 1) {
    if (x1 > x2) {
      swap(x1, x2);
      swap(y1, y2);
    }
    for (; x1 <= x2; x1++, y1 += m)
      img(x1, round(y1)) = color;

  } else if (fabs(m) >= 1) {
    if (y1 > y2) {
      swap(x1, x2);
      swap(y1, y2);
    }
    for (; y1 <= y2; y1++, x1 += 1 / m)
      img(round(x1), y1) = color;
  }
}

void Bresenham(GrayscaleImage &img, int x1, int y1, int x2, int y2, int color) {
  int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);
  int sx = (x2 > x1) ? 1 : -1;
  int sy = (y2 > y1) ? 1 : -1;

  int d;
  if (dx > dy) {
    d = 2 * dy - dx;
    while (true) {
      img(x1, y1) = color;
      if (x1 == x2)
        break;
      if (d >= 0) {
        d += 2 * dy - 2 * dx;
        y1 += sy;
      } else
        d += 2 * dy;
      x1 += sx;
    }
  } else if (dy >= dx) {
    d = 2 * dx - dy;
    while (true) {
      img(x1, y1) = color;
      if (y1 == y2)
        break;
      if (d >= 0) {
        d += 2 * dx - 2 * dy;
        x1 += sx;
      } else
        d += 2 * dx;
      y1 += sy;
    }
  }
}

void drawShape(GrayscaleImage &img,
               void (*func)(GrayscaleImage &, int, int, int, int, int)) {
  int x = 115, y = 115;
  func(img, x, y, x, y - 100, 255);
  func(img, x, y, x + 71, y - 71, 255);
  func(img, x, y, x + 100, y, 255);
  func(img, x, y, x + 71, y + 71, 255);
  func(img, x, y, x, y + 100, 255);
  func(img, x, y, x - 71, y + 71, 255);
  func(img, x, y, x - 100, y, 255);
  func(img, x, y, x - 71, y - 71, 255);
}

int main() {

  int height = 230, width = 230;
  GrayscaleImage img(height, width);

  // drawShape(img, DDA);
  drawShape(img, Midpoint);
  // drawShape(img, Bresenham);

  img.Save("Lines.png");
  return 0;
}