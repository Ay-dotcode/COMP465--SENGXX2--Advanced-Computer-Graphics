#include "Image.h"
using namespace std;

int main() {
  int height = 255, width = 255;
  ColorImage img(height, width);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      img(i, j) = RGBA(log(i * j) * i * j, log(i * j) * i * j / 20,
                       log(i * j) * i * j * j);
      // img(i, j) = log(i * j) * i * j * j;
    }
  }

  img.Save("pattern.png");
  return 0;
}