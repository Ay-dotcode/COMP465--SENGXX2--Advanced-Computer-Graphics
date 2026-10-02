#include "Image.h"
#include <cmath>
using namespace std;

void curve(GrayscaleImage &img, double a, double b, double c, int x1, int x2) {
  double x = x1;
  while (x <= x2) {
    double y = a * x * x + b * x + c;
    y = img.GetHeight() - round(y) - 1; // invert y-axis

    img(round(x), round(y)) = 255;

    double m = 2 * a * x + b;
    if (m >= 0 && m <= 1)
      x++;
    else if (m > 1) {
    }
  }
}

void drawParabolaMidpoint(GrayscaleImage &img, float a, float b, float c, int x1, int x2) {
    int py = round(a*x1*x1 + b*x1 + c);

    for (int px = x1; px <= x2; px++) {

        if (px >= 0 && px < 300 && py >= 0 && py < 300) {
            img(px, py) = 255;
        }

        // Compute ideal y for next x
        float ideal = a*(px+1)*(px+1) + b*(px+1) + c;

        // Compute signed error (not absolute!)
        float err_same = py - ideal;
        float err_up   = (py-1) - ideal;
        float err_down = (py+1) - ideal;

        // Choose the step that reduces error magnitude
        if (std::abs(err_down) < std::abs(err_same) && std::abs(err_down) < std::abs(err_up)) {
            py++; // move down
        }
        else if (std::abs(err_up) < std::abs(err_same) && std::abs(err_up) < std::abs(err_down)) {
            py--; // move up
        }
        // otherwise stay same
    }
}

int main() {
  int height = 200, width = 200;
  GrayscaleImage img(height, width);

  curve(img, 0.02, -4, 200, 0, 200);

  img.Save("Curve.png");
  return 0;
}
