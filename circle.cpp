#include "Image.h"
#include <cmath>
using namespace std;

void draw(GrayscaleImage &img, int r, int x, int y) {
  img(r + x, r + y) = 255;
  img(r + x, r - y) = 255;
  img(r - x, r + y) = 255;
  img(r - x, r - y) = 255;

  img(r + y, r + x) = 255;
  img(r + y, r - x) = 255;
  img(r - y, r + x) = 255;
  img(r - y, r - x) = 255;
}

void fill(GrayscaleImage &img, int r, int x, int y) {
  for (int X = r - x; X <= r + x; X++) {
    img(X, r + y) = 255; // octants 1 -> 4
    img(X, r - y) = 150; // octants 8 -> 5
  }
  for (int X = r - y; X <= r + y; X++) {
    img(X, r + x) = 200; // octants 2 -> 3
    img(X, r - x) = 100; // octants 7 -> 6
  }
}

void midpoint(GrayscaleImage &img, int r) {
  int x = r, y = 0, d1, d2;
  while (y <= x) {
    draw(img, r, x, y);
    fill(img, r, x, y);

    d1 = r * r - x * x - (y + 1) * (y + 1);
    d2 = r * r - (x - 1) * (x - 1) - (y + 1) * (y + 1);

    x = abs(d1) < abs(d2) ? x : x - 1;
    y = y + 1;
  }
}

void Bresenham(GrayscaleImage &img, int r) {
  int x = r, y = 0, d = 3 - 2 * r;
  while (y <= x) {
    draw(img, r, x, y);
    if (d <= 0)
      d += 4 * y + 6;
    else if (d > 0) {
      d += 4 * (y - x) + 10;
      x--;
    }
    y++;
  }
}

void spiral(GrayscaleImage &img, int r1, int r2, int loops) {
  double maxAngle = 2 * M_PI * loops;
  double angleStep = 0.01;

  for (double angle = 0; angle <= maxAngle; angle += angleStep) {
    double R = r1 + (r2 - r1) * angle / maxAngle;

    // Convert to cartesian
    int x = (int)(R * cos(angle));
    int y = (int)(R * sin(angle));
    img(r2 + x, r2 + y) = 255;
  }
}

void roseCurve(GrayscaleImage &img, int loops, int radius, int c) {
  double maxAngle = 2 * M_PI;
  double angleStep = 0.001;

  for (double angle = 0; angle <= maxAngle; angle += angleStep) {
    double R = radius * cos(loops * angle);

    int x = (int)(R * cos(angle));
    int y = (int)(R * sin(angle));
    img(c + x, c + y) = 255;
  }
}

int main() {
  int height = 200, width = 200;
  GrayscaleImage img(height, width);

  // midpoint(img, 99);
  // Bresenham(img, 99);
  spiral(img, 0, 99, 5);
  // roseCurve(img, 4, 99, 100);

  img.Save("Circle.png");
  return 0;
}
