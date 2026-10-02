#include "Image.h"
#include <cmath>
#include <iostream>
#include <limits>

struct LineVariables {
  float a, b, c, m;
};

LineVariables calculateLineVariables(float x1, float y1, float x2, float y2) {
  LineVariables values;

  values.a = y1 - y2;
  values.b = x2 - x1;
  values.c = x1 * y2 - x2 * y1;

  if (values.b == 0) {
    values.m = std::numeric_limits<float>::infinity();
  } else if (values.b > 0) {
    values.m = -(values.a) / values.b;
  } else {
    values.m = -std::numeric_limits<float>::infinity();
  }

  return values;
}

float calculateD(LineVariables v, float x, float y) {
  return std::fabs(v.a * x + v.b * y + v.c);
}

void calculateMidpoint(GrayscaleImage &img, float x1, float y1, float x2,
                       float y2) {
  LineVariables values = calculateLineVariables(x1, y1, x2, y2);
  std::cout << "a: " << values.a << "\n";
  std::cout << "b: " << values.b << "\n";
  std::cout << "c: " << values.c << "\n";

  if (values.m >= 0 && values.m <= 1) {
    float x, y = y1;
    for (x = x1; x <= x2; x++) {
      float d1 = calculateD(values, x, y);
      float d2 = calculateD(values, x, y + 1);
      if (d1 <= d2) {
        img(x, y) = 255;
        y = y;
      } else {
        img(x, y + 1) = 255;
        y = y + 1;
      }
    }
  }

  else if (values.m > 1 && values.m < std::numeric_limits<float>::infinity()) {
    float y, x = x1;
    for (y = y1; y <= y2; y++) {
      float d1 = calculateD(values, x, y);
      float d2 = calculateD(values, x + 1, y);
      if (d1 <= d2) {
        img(x, y) = 255;
        x = x;
      } else {
        img(x + 1, y) = 255;
        x = x + 1;
      }
    }
  }

  else if (values.m >= -1 && values.m < 0) {
    float x, y = y1;
    for (x = x1; x <= x2; x++) {
      float d1 = calculateD(values, x, y);
      float d2 = calculateD(values, x, y - 1);
      if (d1 <= d2) {
        img(x, y) = 255;
        y = y;
      } else {
        img(x, y - 1) = 255;
        y = y - 1;
      }
    }
  }

  else if (values.m > -std::numeric_limits<float>::infinity() &&
           values.m < -1) {
    float y, x = x1;
    for (y = y1; y <= y2; y++) {
      float d1 = calculateD(values, x, y);
      float d2 = calculateD(values, x - 1, y);
      if (d1 <= d2) {
        img(x, y) = 255;
        x = x;
      } else {
        img(x - 1, y) = 255;
        x = x - 1;
      }
    }
  }
}

void calculateDda(GrayscaleImage &img, float x1, float y1, float x2, float y2) {
  float dx = x2 - x1;
  float dy = y2 - y1;
  float steps = std::max(std::fabs(dx), std::fabs(dy));

  float xInc = dx / steps;
  float yInc = dy / steps;

  float x = x1;
  float y = y1;

  for (int i = 0; i <= steps; i++) {
    img(std::round(x), std::round(y)) = 255;
    x += xInc;
    y += yInc;
  }
}

int main() {
  int width = 128;
  int height = 128;
  GrayscaleImage img(width, height);

  // calculateMidpoint(img, 10, 10, 110, 100);
  calculateDda(img, 10, 10, 110, 100);

  img.Save("output.png");
  return 0;
}