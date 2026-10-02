#include "Image.h"
using namespace std;

void stairs(GrayscaleImage &img, int steps, int scale) {
  int j = 0;
  for (int i = 0; i < steps * scale; i++) {
    if (i % scale == 0)
      for (; j < scale + i; j++)
        img(i, j) = 255;
    img(i, j) = 255;
  }
}

int main() {
  int size = 255;
  GrayscaleImage img(size, size);

  stairs(img, 3, 5);

  img.Save("Transformation.png");
  return 0;
}