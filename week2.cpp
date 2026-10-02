#include <fstream>
#include <iostream>

using std::endl, std::cerr, std::ofstream;

int main() {
  ofstream svg("castle2.svg");
  if (!svg.is_open()) {
    cerr << "Error opening file" << endl;
    return 1;
  }
  svg << "<?xml version='1.0' encoding='UTF-8' standalone='no' ?>\n";
  svg << "<svg width='500' height='500' xmlns='http://www.w3.org/2000/svg'>\n";

  svg << "  ";

  svg << "</svg>";

  return 0;
}