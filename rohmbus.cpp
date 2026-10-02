#include "Image.h"
using namespace std;

void rohmbus(GrayscaleImage &img, int cx, int cy, int r) {
  for (int i = 0; i <= r; i++) {
      img(cx - i, cy - r + i) = 255;
      img(cx + i, cy - r + i) = 255;
      img(cx + i, cy + r - i) = 255;
      img(cx - i, cy + r - i) = 255;
  }
}

int main() {
  int size = 200;
  GrayscaleImage img(size, size);

  rohmbus(img, 100, 100, 20);

  img.Save("rohmus.png");
  return 0;
}