#include "Image.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// Colors for 4-bit nibbles (Base-16)
// Palette chosen to be distinct and avoid White (used for padding)
struct ColorEntry {
  int r, g, b;
};

ColorEntry palette[16] = {
    {0, 0, 0},       // 0: Black
    {0, 0, 170},     // 1: Blue
    {0, 170, 0},     // 2: Green
    {0, 170, 170},   // 3: Cyan
    {170, 0, 0},     // 4: Red
    {170, 0, 170},   // 5: Magenta
    {170, 85, 0},    // 6: Brown
    {170, 170, 170}, // 7: Light Gray
    {85, 85, 85},    // 8: Dark Gray
    {85, 85, 255},   // 9: Light Blue
    {85, 255, 85},   // 10: Light Green
    {85, 255, 255},  // 11: Light Cyan
    {255, 85, 85},   // 12: Light Red
    {255, 85, 255},  // 13: Light Magenta
    {255, 255, 85},  // 14: Yellow
    {255, 128, 0}    // 15: Orange
};

RGBA getColor(int val) {
  if (val >= 0 && val < 16) {
    return RGBA(palette[val].r, palette[val].g, palette[val].b);
  }
  return RGBA(255, 255, 255); // White (Padding)
}

void drawRect(ColorImage &img, int x, int y, int w, int h, RGBA color) {
  for (int j = y; j < y + h; j++) {
    for (int i = x; i < x + w; i++) {
      if (i >= 0 && i < img.GetWidth() && j >= 0 && j < img.GetHeight()) {
        img(i, j) = color;
      }
    }
  }
}

void drawFinderPattern(ColorImage &img, int modX, int modY, int scale) {
  // 7x7 Finder Pattern structure
  // Outer Black: 7x7
  // Inner White: 5x5
  // Center Black: 3x3

  int px = modX * scale;
  int py = modY * scale;

  // 1. Outer Black Box (7x7)
  drawRect(img, px, py, 7 * scale, 7 * scale, RGBA(0, 0, 0));

  // 2. Inner White Box (5x5) - Offset by 1 module
  drawRect(img, px + 1 * scale, py + 1 * scale, 5 * scale, 5 * scale,
           RGBA(255, 255, 255));

  // 3. Center Black Box (3x3) - Offset by 2 modules
  drawRect(img, px + 2 * scale, py + 2 * scale, 3 * scale, 3 * scale,
           RGBA(0, 0, 0));
}

bool isFinderPatternZone(int x, int y, int size) {
  // Reserve 8x8 area for finder patterns (7x7 + 1 separator)
  // Top Left
  if (x < 8 && y < 8)
    return true;
  // Top Right
  if (x >= size - 8 && y < 8)
    return true;
  // Bottom Left
  if (x < 8 && y >= size - 8)
    return true;
  return false;
}

// Helper to find closest color match
int matchColor(RGBA c) {
  int minDist = 100000000;
  int bestVal = -1;

  // Check palette
  for (int i = 0; i < 16; i++) {
    int dr = c.r - palette[i].r;
    int dg = c.g - palette[i].g;
    int db = c.b - palette[i].b;
    int dist = dr * dr + dg * dg + db * db;

    if (dist < minDist) {
      minDist = dist;
      bestVal = i;
    }
  }

  // Check White (Padding)
  int dw = c.r - 255;
  int dh = c.g - 255;
  int dz = c.b - 255; // dz for blue z
  int distWhite = dw * dw + dh * dh + dz * dz;

  if (distWhite < minDist) {
    return -1; // Padding
  }

  return bestVal;
}

string decodeQRImpl(string filename) {
  // Check if file exists
  FILE *f = fopen(filename.c_str(), "rb");
  if (!f) {
    // Try appending .png
    string altFilename = filename + ".png";
    f = fopen(altFilename.c_str(), "rb");
    if (f) {
      filename = altFilename;
    } else {
      cout << "Error: File not found: " << filename << endl;
      return "";
    }
  }
  fclose(f);

  ColorImage img;
  img.Load(filename);

  if (img.GetWidth() == 0 || img.GetHeight() == 0) {
    cout << "Failed to load image or image is empty." << endl;
    return "";
  }

  // Detect scale
  int scale = 0;
  for (int i = 0; i < img.GetWidth(); i++) {
    RGBA c = img(i, i);
    // Check if not black (allow some tolerance)
    if (c.r > 128 || c.g > 128 || c.b > 128) {
      scale = i;
      break;
    }
  }

  if (scale == 0) {
    cout << "Could not detect QR code scale. Is this a valid image?" << endl;
    return "";
  }

  int size = img.GetWidth() / scale;
  cout << "Detected Scale: " << scale << " px/module" << endl;
  cout << "Detected Grid: " << size << "x" << size << endl;

  vector<int> symbols;

  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      if (isFinderPatternZone(x, y, size))
        continue;

      // Sample center of module
      int sx = x * scale + scale / 2;
      int sy = y * scale + scale / 2;

      if (sx >= img.GetWidth() || sy >= img.GetHeight())
        continue;

      RGBA c = img(sx, sy);
      int val = matchColor(c);

      if (val == -1) {
        goto decoding_done;
      }
      symbols.push_back(val);
    }
  }

decoding_done:
  // Reconstruct string
  string decoded = "";
  for (size_t i = 0; i + 1 < symbols.size(); i += 2) {
    char c = 0;
    c |= (symbols[i] << 4);
    c |= (symbols[i + 1] << 0);
    decoded += c;
  }
  return decoded;
}

void decodeToText() {
  string filename;
  cout << "Enter filename to decode: ";
  getline(cin, filename);

  string decoded = decodeQRImpl(filename);
  if (!decoded.empty()) {
    cout << "Decoded Text: " << endl;
    cout << "----------------------------------------" << endl;
    cout << decoded << endl;
    cout << "----------------------------------------" << endl;
  }
}

void decodeToFile() {
  string filename;
  cout << "Enter filename to decode: ";
  getline(cin, filename);

  string decoded = decodeQRImpl(filename);
  if (!decoded.empty()) {
    string outFile;
    cout << "Enter output filename to save data: ";
    getline(cin, outFile);

    ofstream f(outFile, ios::binary);
    if (f) {
      f << decoded;
      cout << "Saved decoded data to " << outFile << endl;
    } else {
      cout << "Error opening output file." << endl;
    }
  }
}

void generateQRImpl(const string &input, const string &outFilename) {
  if (input.empty()) {
    cout << "Error: Input cannot be empty." << endl;
    return;
  }

  // Convert string to 4-bit symbols
  vector<int> symbols;
  for (char c : input) {
    // Extract 4-bit nibbles from each byte (MSB to LSB)
    symbols.push_back((c >> 4) & 0x0F);
    symbols.push_back((c >> 0) & 0x0F);
  }

  // Determine grid size
  // Start at 21x21 (Version 1 standard QR size)
  int size = 21;
  while (true) {
    int totalModules = size * size;
    // Subtract finder pattern areas (approx 3 * 8*8 = 192 modules)
    int availableModules = totalModules - 192;

    if (availableModules >= symbols.size())
      break;
    size += 4; // Grow by 4 modules (standard QR version step)
  }

  int scale = 20; // Pixels per module
  ColorImage qr(size * scale, size * scale);

  // Initialize canvas to White
  for (int y = 0; y < qr.GetHeight(); y++)
    for (int x = 0; x < qr.GetWidth(); x++)
      qr(x, y) = RGBA(255, 255, 255);

  // Draw Finder Patterns
  drawFinderPattern(qr, 0, 0, scale);        // Top Left
  drawFinderPattern(qr, size - 7, 0, scale); // Top Right
  drawFinderPattern(qr, 0, size - 7, scale); // Bottom Left

  // Fill Data
  int symbolIdx = 0;
  // Simple raster scan filling (skipping finder zones)
  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      // Skip Finder Patterns
      if (isFinderPatternZone(x, y, size))
        continue;

      if (symbolIdx < symbols.size()) {
        RGBA color = getColor(symbols[symbolIdx++]);
        drawRect(qr, x * scale, y * scale, scale, scale, color);
      } else {
        // Padding: Leave white or use a neutral pattern
        // We'll leave it white as initialized
      }
    }
  }

  qr.Save(outFilename);

  cout << "Successfully generated " << outFilename << endl;
  cout << "Grid Size: " << size << "x" << size << endl;
  cout << "Data Length: " << input.length() << " bytes" << endl;
  cout << "Symbols Encoded: " << symbols.size() << endl;
}

void generateQRText() {
  string input;
  cout << "Enter text to encode: ";
  getline(cin, input);
  generateQRImpl(input, "MultiColorQR.png");
}

void generateQRFile() {
  string filename;
  cout << "Enter file to encode: ";
  getline(cin, filename);

  ifstream f(filename, ios::binary);
  if (!f) {
    cout << "Error: File not found." << endl;
    return;
  }

  // Read entire file into string
  string data((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
  generateQRImpl(data, "MultiColorQR.png");
}

int main() {
  cout << "=== Multi-Color QR Code Tool ===" << endl;
  cout << "Encoding: 4 bits per symbol (16 colors)" << endl;
  cout << "1. Generate QR Code from Text" << endl;
  cout << "2. Generate QR Code from File" << endl;
  cout << "3. Decode QR Code to Text" << endl;
  cout << "4. Decode QR Code to File" << endl;
  cout << "Choice (1-4): ";

  string choice;
  getline(cin, choice);

  if (choice == "1") {
    generateQRText();
  } else if (choice == "2") {
    generateQRFile();
  } else if (choice == "3") {
    decodeToText();
  } else if (choice == "4") {
    decodeToFile();
  } else {
    cout << "Invalid choice." << endl;
  }

  return 0;
}
