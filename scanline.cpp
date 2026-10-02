#include "Image.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

struct Point {
  int x;
  int y;
};

struct ScanlineTable {
  int y_min, y_max, x;
  double inv_slope;
};

struct ScanlineActive {
  int y_max;
  double x, inv_slope;
};

// Simple Bresenham line to render polygon edges.
void drawLine(GrayscaleImage &img, Point a, Point b, Byte color = 255) {
  int dx = std::abs(b.x - a.x);
  int dy = -std::abs(b.y - a.y);
  int sx = (a.x < b.x) ? 1 : -1;
  int sy = (a.y < b.y) ? 1 : -1;
  int err = dx + dy;

  int x = a.x;
  int y = a.y;
  while (true) {
    if (x >= 0 && x < img.GetWidth() && y >= 0 && y < img.GetHeight())
      img(x, y) = color;

    if (x == b.x && y == b.y)
      break;
    int e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y += sy;
    }
  }
}

void drawPolygonOutline(GrayscaleImage &img, const std::vector<Point> &points,
                        Byte color = 255) {
  for (size_t i = 0; i < points.size(); ++i) {
    drawLine(img, points[i], points[(i + 1) % points.size()], color);
  }
}

void fillPolygon(GrayscaleImage &img, const std::vector<Point> &points,
                 Byte color = 255) {
  std::vector<ScanlineTable> table;
  std::vector<ScanlineActive> active;
  int min_y = img.GetHeight(), max_y = 0;

  // Build edge table
  for (size_t i = 0; i < points.size(); i++) {
    Point p1 = points[i];
    Point p2 = points[(i + 1) % points.size()];
    
    // Skip horizontal edges
    if (p1.y == p2.y)
      continue;
    
    // Ensure p1 is the lower point (smaller y)
    if (p1.y > p2.y)
      std::swap(p1, p2);

    ScanlineTable e;
    e.y_min = p1.y;
    e.y_max = p2.y;
    e.x = p1.x;
    e.inv_slope = double(p2.x - p1.x) / double(p2.y - p1.y);

    // Clip to image bounds
    if (e.y_max > img.GetHeight())
      e.y_max = img.GetHeight();
    if (e.y_min < 0) {
      e.x += e.inv_slope * (-e.y_min);
      e.y_min = 0;
    }

    table.push_back(e);
    min_y = std::min(min_y, e.y_min);
    max_y = std::max(max_y, e.y_max);
  }

  // Sort edge table by y_min
  std::sort(table.begin(), table.end(),
            [](const ScanlineTable &a, const ScanlineTable &b) {
              return a.y_min < b.y_min;
            });

  std::cout << "Edge table sorted by y_min:\n";
  std::cout << "idx  y_min  y_max   x   inv_slope\n";
  for (size_t i = 0; i < table.size(); ++i) {
    const auto &e = table[i];
    std::cout << std::setw(3) << i << std::setw(7) << e.y_min
              << std::setw(7) << e.y_max << std::setw(5) << e.x
              << std::setw(12) << std::fixed << std::setprecision(3)
              << e.inv_slope << '\n';
  }

  // Process each scanline
  size_t table_idx = 0;
  for (int y = min_y; y < max_y; y++) {
    // Add new edges that start at this scanline
    while (table_idx < table.size() && table[table_idx].y_min == y) {
      const auto &e = table[table_idx];
      active.push_back({e.y_max, (double)e.x, e.inv_slope});
      table_idx++;
    }

    // Remove edges that end at this scanline
    active.erase(
        std::remove_if(active.begin(), active.end(),
                       [y](const ScanlineActive &e) { return y >= e.y_max; }),
        active.end());

    if (active.size() < 2)
      continue;

    // Sort active edges by x coordinate
    std::sort(active.begin(), active.end(),
              [](const ScanlineActive &a, const ScanlineActive &b) {
                return a.x < b.x;
              });

    std::cout << "Active table at y=" << y << "\n";
    std::cout << "idx  y_max      x   inv_slope\n";
    for (size_t i = 0; i < active.size(); ++i) {
      const auto &e = active[i];
      std::cout << std::setw(3) << i << std::setw(7) << e.y_max
                << std::setw(9) << std::fixed << std::setprecision(3)
                << e.x << std::setw(12) << e.inv_slope << '\n';
    }

    std::vector<int> drawn_points;

    // Fill between pairs of edges (even-odd rule)
    for (size_t i = 0; i + 1 < active.size(); i += 2) {
      // Round left inward (floor), right inward (ceil-1) to keep coverage tight
      int x0 = std::max(0, int(std::floor(active[i].x)));
      int x1 = std::min(img.GetWidth() - 1, int(std::ceil(active[i + 1].x)) - 1);

      // Fill pixels inclusively from x0 to x1
      for (int x = x0; x <= x1; ++x) {
        drawn_points.push_back(x);
        img(x, y) = color;
      }
    }

    if (!drawn_points.empty()) {
      std::cout << "Points drawn at y=" << y << ": ";
      for (size_t i = 0; i < drawn_points.size(); ++i) {
        if (i)
          std::cout << ' ';
        std::cout << '(' << drawn_points[i] << ',' << y << ')';
      }
      std::cout << '\n';
    } else {
      std::cout << "No points drawn at y=" << y << '\n';
    }

    std::cout << '\n';

    // Update x coordinates for next scanline
    for (auto &e : active)
      e.x += e.inv_slope;
  }
}

int main() {
  GrayscaleImage img(11, 9);
  
  std::vector<Point> points = {{1, 1}, {7, 1}, {9, 4}, {7, 7}, {1, 7},
                               {4, 3}, {6, 4}, {4, 6}};

  fillPolygon(img, points, 100);
  
  // Optionally mark vertices
  // for (const auto &p : points)
  //   img(p.x, p.y) = 255;

  img.Save("scanline.png");
  return 0;
}