#include "Image.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Global variables
ColorImage canvas;
ColorImage tempBuffer; // For double buffering effects
int width = 800;
int height = 600;
bool isRunning = true;

// Mouse state
int lastX = -1, lastY = -1;
bool isMouseDown = false;

// Random helper
float randomFloat() { return (float)rand() / (float)RAND_MAX; }

Byte clampByte(int v) {
  if (v < 0)
    return 0;
  if (v > 255)
    return 255;
  return (Byte)v;
}

// Initialize canvas to black
void initCanvas() {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      canvas(x, y) = RGBA(0, 0, 0);
    }
  }
}

// Splash effect: Random color blob with displacement
void applySplash(int cx, int cy) {
  int radius = 30 + rand() % 40; // Increased size
  RGBA color(rand() % 256, rand() % 256, rand() % 256);

  // 1. Displacement (Push outward)
  int dispRadius = radius * 2.5; // Push affects a wider area
  int minX = max(0, cx - dispRadius);
  int maxX = min(width - 1, cx + dispRadius);
  int minY = max(0, cy - dispRadius);
  int maxY = min(height - 1, cy + dispRadius);

  // Copy to temp to read from stable state
  for (int y = minY; y <= maxY; y++) {
    for (int x = minX; x <= maxX; x++) {
      tempBuffer(x, y) = canvas(x, y);
    }
  }

  for (int y = minY; y <= maxY; y++) {
    for (int x = minX; x <= maxX; x++) {
      float dx = x - cx;
      float dy = y - cy;
      float dist = sqrt(dx * dx + dy * dy);

      if (dist < dispRadius && dist > 0) {
        // Calculate displacement amount
        // Strong push near center
        float force = 1.0f - (dist / dispRadius);
        // Non-linear push to create a "rim" effect
        float displacement = pow(force, 2.0f) * radius * 1.5f;

        // Sample from closer to center (where the fluid is coming FROM)
        float sampleDist = dist - displacement;
        if (sampleDist < 0)
          sampleDist = 0; // Clamp to center

        float ratio = sampleDist / dist;
        float sx = cx + dx * ratio;
        float sy = cy + dy * ratio;

        int isx = (int)sx;
        int isy = (int)sy;

        if (isx >= 0 && isx < width && isy >= 0 && isy < height) {
          canvas(x, y) = tempBuffer(isx, isy);
        }
      }
    }
  }

  // 2. Paint new color in center
  for (int y = cy - radius; y <= cy + radius; y++) {
    for (int x = cx - radius; x <= cx + radius; x++) {
      if (x < 0 || x >= width || y < 0 || y >= height)
        continue;

      float dist = sqrt(pow(x - cx, 2) + pow(y - cy, 2));
      if (dist < radius) {
        float alpha = 1.0f - (dist / radius);
        alpha = pow(alpha, 0.2f); // Very hard edge, solid blob

        RGBA current = canvas(x, y);

        canvas(x, y).r = (Byte)(color.r * alpha + current.r * (1.0f - alpha));
        canvas(x, y).g = (Byte)(color.g * alpha + current.g * (1.0f - alpha));
        canvas(x, y).b = (Byte)(color.b * alpha + current.b * (1.0f - alpha));
      }
    }
  }
}

// Swirl/Smudge effect with mixing
void applySwirl(int cx, int cy, int px, int py) {
  // Direction vector
  float dx = cx - px;
  float dy = cy - py;
  float len = sqrt(dx * dx + dy * dy);
  if (len == 0)
    return;

  int radius = 60; // Brush size

  int minX = max(0, cx - radius);
  int maxX = min(width - 1, cx + radius);
  int minY = max(0, cy - radius);
  int maxY = min(height - 1, cy + radius);

  // Copy patch to temp buffer
  for (int y = minY; y <= maxY; y++) {
    for (int x = minX; x <= maxX; x++) {
      tempBuffer(x, y) = canvas(x, y);
    }
  }

  for (int y = minY; y <= maxY; y++) {
    for (int x = minX; x <= maxX; x++) {
      float dist = sqrt(pow(x - cx, 2) + pow(y - cy, 2));
      if (dist < radius) {
        float influence = 1.0f - (dist / radius);
        influence = pow(influence, 2.0f); // Smooth falloff

        // 1. Drag (Advection)
        // Pull pixels from behind the movement
        float srcX = x - dx * influence * 2.0f;
        float srcY = y - dy * influence * 2.0f;

        // 2. Swirl (Vorticity)
        // Rotate around the brush center
        float rx = srcX - cx;
        float ry = srcY - cy;
        float rAng = atan2(ry, rx);
        float rDist = sqrt(rx * rx + ry * ry);

        // Twist amount depends on speed and influence
        // Higher speed = more twist
        rAng -= 0.2f * len * influence;

        srcX = cx + cos(rAng) * rDist;
        srcY = cy + sin(rAng) * rDist;

        // 3. Mixing (Diffusion/Blur)
        // Sample neighbors and average to simulate liquid mixing
        int sx = (int)srcX;
        int sy = (int)srcY;

        if (sx >= 0 && sx < width - 1 && sy >= 0 && sy < height - 1) {
          RGBA c1 = tempBuffer(sx, sy);
          RGBA c2 = tempBuffer(sx + 1, sy);
          RGBA c3 = tempBuffer(sx, sy + 1);
          RGBA c4 = tempBuffer(sx + 1, sy + 1);

          // Simple box blur / average
          canvas(x, y).r = (c1.r + c2.r + c3.r + c4.r) / 4;
          canvas(x, y).g = (c1.g + c2.g + c3.g + c4.g) / 4;
          canvas(x, y).b = (c1.b + c2.b + c3.b + c4.b) / 4;
        } else if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
          canvas(x, y) = tempBuffer(sx, sy);
        }
      }
    }
  }
}

// X11/GLX Globals
Display *dpy;
Window root;
GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
XVisualInfo *vi;
Colormap cmap;
XSetWindowAttributes swa;
Window win;
GLXContext glc;
XWindowAttributes gwa;
XEvent xev;

void cleanup() {
  glXMakeCurrent(dpy, None, NULL);
  glXDestroyContext(dpy, glc);
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
}

int main(int argc, char **argv) {
  srand(time(NULL));

  // Setup X11
  dpy = XOpenDisplay(NULL);
  if (dpy == NULL) {
    cout << "Cannot connect to X server" << endl;
    return 0;
  }

  root = DefaultRootWindow(dpy);
  vi = glXChooseVisual(dpy, 0, att);

  if (vi == NULL) {
    cout << "No appropriate visual found" << endl;
    return 0;
  }

  cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask |
                   ButtonPressMask | ButtonReleaseMask | ButtonMotionMask;

  win = XCreateWindow(dpy, root, 0, 0, width, height, 0, vi->depth, InputOutput,
                      vi->visual, CWColormap | CWEventMask, &swa);
  XMapWindow(dpy, win);
  XStoreName(dpy, win, "Liquid Canvas");

  glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
  glXMakeCurrent(dpy, win, glc);

  // Init buffers
  canvas = ColorImage(width, height);
  tempBuffer = ColorImage(width, height);
  initCanvas();

  cout << "=== Liquid Canvas ===" << endl;
  cout << "Click to splash paint." << endl;
  cout << "Drag to swirl and mix." << endl;
  cout << "ESC to exit." << endl;

  while (isRunning) {
    while (XPending(dpy) > 0) {
      XNextEvent(dpy, &xev);

      if (xev.type == KeyPress) {
        KeySym key = XLookupKeysym(&xev.xkey, 0);
        if (key == XK_Escape)
          isRunning = false;
      } else if (xev.type == ButtonPress) {
        if (xev.xbutton.button == Button1) {
          isMouseDown = true;
          lastX = xev.xbutton.x;
          lastY = xev.xbutton.y;
          // Apply splash on click
          // Adjust Y for Image coordinate system (0 at top) vs OpenGL (0 at
          // bottom) But here we are just manipulating the image buffer, so we
          // use X11 coords directly X11: 0,0 is top-left. Image.h: usually 0,0
          // is top-left for storage.
          applySplash(lastX, lastY);
        }
      } else if (xev.type == ButtonRelease) {
        if (xev.xbutton.button == Button1) {
          isMouseDown = false;
          lastX = -1;
        }
      } else if (xev.type == MotionNotify) {
        if (isMouseDown) {
          int currX = xev.xmotion.x;
          int currY = xev.xmotion.y;

          if (lastX != -1) {
            // Interpolate steps for smooth continuous flow
            float dist = sqrt(pow(currX - lastX, 2) + pow(currY - lastY, 2));
            int steps = (int)(dist / 5.0f); // One step every 5 pixels
            if (steps < 1)
              steps = 1;

            for (int i = 1; i <= steps; i++) {
              float t = (float)i / steps;
              int ix = lastX + (currX - lastX) * t;
              int iy = lastY + (currY - lastY) * t;
              int ipx = lastX + (currX - lastX) * (float)(i - 1) / steps;
              int ipy = lastY + (currY - lastY) * (float)(i - 1) / steps;
              applySwirl(ix, iy, ipx, ipy);
            }
          }

          lastX = currX;
          lastY = currY;
        }
      } else if (xev.type == ConfigureNotify) {
        if (xev.xconfigure.width != width || xev.xconfigure.height != height) {
          // Resize handling - simplistic (clears canvas)
          // Ideally we would resize and keep content, but for now let's just
          // reset or clip Let's just update dimensions and re-init buffers if
          // needed, but preserving the image is better.

          ColorImage newCanvas(xev.xconfigure.width, xev.xconfigure.height);
          ColorImage newTemp(xev.xconfigure.width, xev.xconfigure.height);

          // Copy old content
          for (int y = 0; y < min(height, xev.xconfigure.height); y++) {
            for (int x = 0; x < min(width, xev.xconfigure.width); x++) {
              newCanvas(x, y) = canvas(x, y);
            }
          }

          width = xev.xconfigure.width;
          height = xev.xconfigure.height;
          canvas = newCanvas;
          tempBuffer = newTemp;

          glViewport(0, 0, width, height);
        }
      }
    }

    // Render
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1,
            1); // Top-left origin to match X11 mouse coords
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glRasterPos2i(0, height); // Start at bottom-left? No, glDrawPixels draws
                              // from current pos.
    // glDrawPixels draws starting from the raster position.
    // If we want 0,0 (top-left of image) to be at 0,0 (top-left of window),
    // And we set Ortho to 0..width, height..0.
    // Then 0,0 is top-left.

    // However, glDrawPixels usually expects data from bottom-up unless we flip
    // it. Let's just draw it and see. If it's upside down, we flip the raster
    // pos or the data loop. Standard OpenGL image data is bottom-to-top. Our
    // Image class stores top-to-bottom (usually).

    // Let's manually flip for display to be safe, or use glPixelZoom(1, -1)

    glPixelZoom(1.0f, -1.0f);
    glRasterPos2i(0, 0); // Top-left

    glDrawPixels(canvas.GetWidth(), canvas.GetHeight(), GL_RGBA,
                 GL_UNSIGNED_BYTE, &canvas(0, 0));

    glPixelZoom(1.0f, 1.0f); // Reset

    glXSwapBuffers(dpy, win);

    usleep(10000); // ~100 FPS cap
  }

  cleanup();
  return 0;
}
