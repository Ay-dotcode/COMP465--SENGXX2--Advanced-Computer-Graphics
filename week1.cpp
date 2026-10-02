#include <fstream>
#include <iostream>

using std::endl, std::cerr, std::ofstream;

void wall(ofstream &svg, int x, int y, int width, int height, int n = 10) {
  svg << "<path d='M" << x - 20 << "," << y + height << " V" << y + 20 << " ";
  for (int i = 0; i < n; i++, x += 40)
    svg << "H" << x << " V" << y << " H" << x + 20 << " V" << y + 20 << " ";
  svg << "H" << x << " V" << y + height
      << " Z' fill='grey' stroke='black' stroke-width='2' />";
}

void rail(ofstream &svg, int x, int y, int width, int height, int n = 5) {
  svg << "<path d= 'M" << x << "," << y;
  for (int i = 0; i < n; i++, x += width)
    svg << "H" << width + x << " M" << x << "," << y + height << " H"
        << width + x << " M" << width / 2 + x << "," << y << " V" << height + y
        << "M" << width + x << "," << y;
  svg << "' stroke='red' stroke-width='5' />\n";
}

void stairs(ofstream &svg, int x, int y, int n = 5, int size = 15) {
  svg << "<path d=' M" << x << "," << y;
  for (int i = 0; i < n; i++, x += size, y -= size)
    svg << "L" << x << "," << y << " " << x + size << "," << y << " L"
        << x + size << "," << y << " " << x + size << "," << y;
  svg << "V" << y + size * (n + 1) << "H" << x - size * n;
  svg << " Z' fill='lightblue' stroke='black' stroke-width='5' />\n";
}

void tower(ofstream &svg, int x, int y, int width, int height) {
  svg << "<path d='M" << x << "," << y;
  svg << "V" << y - height * .8 << " L" << x + width / 3 << ","
      << y - height * .9 << "V" << y - height << "H" << x - width * 2 / 3
      << " V" << y - height * .9 << " L" << x - width / 3 << ","
      << y - height * .8 << " V" << y;
  svg << " Z' stroke='black' stroke-width='2' fill='grey' />\n";
}

int main() {

  ofstream svg("castle.svg");
  if (!svg.is_open()) {
    cerr << "Error opening file" << endl;
    return 1;
  }
  svg << "<?xml version='1.0' encoding='UTF-8' standalone='no' ?>\n";
  svg << "<svg width='500' height='500' xmlns='http://www.w3.org/2000/svg'>\n";

  wall(svg, 100, 300, 200, 150, 8);

  rail(svg, 15, 15, 50, 30, 5);

  stairs(svg, 300, 100, 7);

  tower(svg, 80, 450, 100, 250);
  tower(svg, 450, 450, 100, 250);

  svg << "<rect x='225' y='370' width='50' height='80' />\n";
  svg << "<rect x='110' y='350' width='40' height='40' fill='white' "
         "stroke = 'black' stroke-width='1'/>\n";
  svg << "<line x1='130' y1='350' x2='130' y2='390' stroke='black' "
         "stroke-width='1'/>\n";
  svg << "<line x1='110' y1='370' x2='150' y2='370' stroke='black' "
         "stroke-width='1'/>\n";
  svg << "<rect x='340' y='350' width='40' height='40' fill='white' "
         "stroke = 'black' stroke-width='1'/>\n";
  svg << "<line x1='360' y1='350' x2='360' y2='390' stroke='black' "
         "stroke-width='1'/>\n";
  svg << "<line x1='340' y1='370' x2='380' y2='370' stroke='black' "
         "stroke-width='1'/>\n";

  svg << "</svg>";

  return 0;
}