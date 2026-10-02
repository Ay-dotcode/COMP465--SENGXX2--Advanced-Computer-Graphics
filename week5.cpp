#include "Image.h"
using namespace std;

void grid(GrayscaleImage &img, int row, int column, int size = 20) {
  for (int i = 1; i < row * size + size; i++) {
    for (int j = 1; j < column * size + size; j++)
      if (i % size == 0 || j % size == 0)
        img(j, i) = 255;
  }
}

void angledGrid(GrayscaleImage &img, int row, int column, int size = 20) {
  for (int i = 0; i <= row * size; i++)
    for (int j = 0; j <= column * size; j++)
      if ((i + j) % size == 0 || (i - j) % size == 0)
        img(j, i) = 255;
}

void x(GrayscaleImage &img, int center, int size) {
  img(center, center) = 255;
  for (int i = 1; i <= size; i++) {
    img(center + i, center + i) = 255;
    img(center + i, center - i) = 255;
    img(center - i, center + i) = 255;
    img(center - i, center - i) = 255;
  }
}
void sign(GrayscaleImage &img, int center, int size) {
  img(center, center) = 255;
  for (int i = 1; i <= size; i++) {
    img(center, center + i) = 255;          
    img(center, center - i) = 255;
    img(center + i, center) = 255;
    img(center - i, center) = 255;
  }
}

void rail(GrayscaleImage &img, int length, int scale) {
  scale += 2;
  for (int i = 1; i < length * scale + scale; i++) {
    img(i, 0) = 255;
    img(i, scale) = 255;
  }
  for (int i = scale; i < length * scale + scale; i += scale)
    for (int j = 0; j <= scale; j++)
      img(i, j) = 255;

  // for (int x = 1; x <= length; x++) {
  //   int rodX = x * scale;
  //   for (int y = 0; y <= scale; y++)
  //     img(rodX, y) = 255;
  // }
}

void stairs(GrayscaleImage &img, int steps, int scale) {
  int j = 0;
  for (int i = 0; i < steps * scale; i++) {
    img(j, i) = 255;
    if (i % scale == 0) {
      for (j = i; j < scale + i; j++)
        img(j, i) = 255;
      img(j, i) = 255;
    }
  }
}

int main() {
  int height = 200, width = 200;
  GrayscaleImage img(height, width);

  grid(img, 7, 5);
  // angledGrid(img, 7, 5);
  // x(img, 100, 50);
  // sign(img, 100, 50);
  // rail(img, 5, 15);
  // stairs(img, 5, 10);

  img.Save("Grid.png");
  return 0;
}
