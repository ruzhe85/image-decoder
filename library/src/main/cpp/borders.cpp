//
// Created by len on 25/12/20.
//

#include "borders.h"
#include <cmath>
#include <cstdint>
#include <vector>

bool inline isBlackPixel(const uint8_t* pixels, uint32_t width, uint32_t x,
                         uint32_t y) {
  const uint8_t pixel = *((uint8_t*)pixels + (y * width + x));
  return pixel < thresholdForBlack;
}

bool inline isWhitePixel(const uint8_t* pixels, uint32_t width, uint32_t x,
                         uint32_t y) {
  const uint8_t pixel = *((uint8_t*)pixels + (y * width + x));
  return pixel > thresholdForWhite;
}

// ---------------------------------------------------------------------------
// Komiho: stock line-scan (upstream algorithm, unchanged).
// ---------------------------------------------------------------------------

/** Return the first x position where there is a substantial amount of fill,
 * starting the search from the left. */
uint32_t findBorderLeft(uint8_t* pixels, uint32_t width, uint32_t height,
                        uint32_t top, uint32_t bottom) {
  int x, y;
  const auto filledLimit = (uint32_t)round(height * filledRatioLimit / 2);

  // Scan first line to detect dominant color
  uint32_t whitePixels = 0;
  uint32_t blackPixels = 0;

  for (y = top; y < bottom; y += 2) {
    if (isBlackPixel(pixels, width, 0, y)) {
      blackPixels++;
    } else if (isWhitePixel(pixels, width, 0, y)) {
      whitePixels++;
    }
  }

  auto detectFunc = isBlackPixel;
  if (whitePixels > filledLimit && blackPixels > filledLimit) {
    // Mixed fill found... don't crop anything
    return 0;
  } else if (blackPixels > filledLimit) {
    detectFunc = isWhitePixel;
  }

  // Scan vertical lines in search of filled lines
  for (x = 1; x < width; x++) {
    uint32_t filledCount = 0;

    for (y = top; y < bottom; y += 2) {
      if (detectFunc(pixels, width, x, y)) {
        filledCount++;
      }
    }

    if (filledCount > filledLimit) {
      // This line contains enough fill
      return x;
    }
  }

  // No fill found... don't crop anything
  return 0;
}

/** Return the first x position where there is a substantial amount of fill,
 * starting the search from the right. */
uint32_t findBorderRight(uint8_t* pixels, uint32_t width, uint32_t height,
                         uint32_t top, uint32_t bottom) {
  int x, y;
  const auto filledLimit = (uint32_t)round(height * filledRatioLimit / 2);

  // Scan first line to detect dominant color
  uint32_t whitePixels = 0;
  uint32_t blackPixels = 0;

  uint32_t lastX = width - 1;
  for (y = top; y < bottom; y += 2) {
    if (isBlackPixel(pixels, width, lastX, y)) {
      blackPixels++;
    } else if (isWhitePixel(pixels, width, lastX, y)) {
      whitePixels++;
    }
  }

  auto detectFunc = isBlackPixel;
  if (whitePixels > filledLimit && blackPixels > filledLimit) {
    // Mixed fill found... don't crop anything
    return width;
  } else if (blackPixels > filledLimit) {
    detectFunc = isWhitePixel;
  }

  // Scan vertical lines in search of filled lines
  for (x = width - 2; x > 0; x--) {
    uint32_t filledCount = 0;

    for (y = top; y < bottom; y += 2) {
      if (detectFunc(pixels, width, x, y)) {
        filledCount++;
      }
    }

    if (filledCount > filledLimit) {
      // This line contains enough fill
      return x + 1;
    }
  }

  // No fill found... don't crop anything
  return width;
}

/** Return the first y position where there is a substantial amount of fill,
 * starting the search from the top. */
uint32_t findBorderTop(uint8_t* pixels, uint32_t width, uint32_t height) {
  int x, y;
  const auto filledLimit = (uint32_t)round(width * filledRatioLimit / 2);

  // Scan first line to detect dominant color
  uint32_t whitePixels = 0;
  uint32_t blackPixels = 0;

  for (x = 0; x < width; x += 2) {
    if (isBlackPixel(pixels, width, x, 0)) {
      blackPixels++;
    } else if (isWhitePixel(pixels, width, x, 0)) {
      whitePixels++;
    }
  }

  auto detectFunc = isBlackPixel;
  if (whitePixels > filledLimit && blackPixels > filledLimit) {
    // Mixed fill found... don't crop anything
    return 0;
  } else if (blackPixels > filledLimit) {
    detectFunc = isWhitePixel;
  }

  // Scan horizontal lines in search of filled lines
  for (y = 1; y < height; y++) {
    uint32_t filledCount = 0;

    for (x = 0; x < width; x += 2) {
      if (detectFunc(pixels, width, x, y)) {
        filledCount++;
      }
    }

    if (filledCount > filledLimit) {
      // This line contains enough fill
      return y;
    }
  }

  // No fill found... don't crop anything
  return 0;
}

/** Return the first y position where there is a substantial amount of fill,
 * starting the search from the bottom. */
uint32_t findBorderBottom(uint8_t* pixels, uint32_t width, uint32_t height) {
  int x, y;
  const auto filledLimit = (uint32_t)round(width * filledRatioLimit / 2);

  // Scan first line to detect dominant color
  uint32_t whitePixels = 0;
  uint32_t blackPixels = 0;
  uint32_t lastY = height - 1;

  for (x = 0; x < width; x += 2) {
    if (isBlackPixel(pixels, width, x, lastY)) {
      blackPixels++;
    } else if (isWhitePixel(pixels, width, x, lastY)) {
      whitePixels++;
    }
  }

  auto detectFunc = isBlackPixel;
  if (whitePixels > filledLimit && blackPixels > filledLimit) {
    // Mixed fill found... don't crop anything
    return height;
  } else if (blackPixels > filledLimit) {
    detectFunc = isWhitePixel;
  }

  // Scan horizontal lines in search of filled lines
  for (y = height - 2; y > 0; y--) {
    uint32_t filledCount = 0;

    for (x = 0; x < width; x += 2) {
      if (detectFunc(pixels, width, x, y)) {
        filledCount++;
      }
    }

    if (filledCount > filledLimit) {
      // This line contains enough fill
      return y + 1;
    }
  }

  // No fill found... don't crop anything
  return height;
}

// ---------------------------------------------------------------------------
// Komiho: aggressive mode — connected-component content bounding box.
//
// Real-world margins are layered: [thin black scanner bar][white margin with a
// watermark / page number][sparse-edged content]. Any single-line scan with a
// fill threshold stops on the wrong layer (the bar, the watermark, or cuts
// into sparse art). Instead, label the ink components and take the bounding
// box of the *content* components:
//   - components smaller than aggressiveContentAreaRatio of the image are
//     specks / watermarks / page numbers -> dropped;
//   - components touching the image edge whose thickness (min of bbox width /
//     height) is below aggressiveEdgeBarThicknessRatio are scanner bars /
//     baked-in frames -> dropped; thicker edge components are full-bleed art
//     -> kept.
// Runs on a 2x downsampled mask (any dark pixel in a 2x2 block) — component
// geometry is preserved for margins measured in tens of pixels, memory stays
// at ~1/4 of the grayscale buffer, and precision is ±2px at full resolution.
// ---------------------------------------------------------------------------

struct AggComponent {
  uint32_t area = 0;
  uint32_t minX = 0, maxX = 0, minY = 0, maxY = 0;
  bool touchesEdge = false;
};

Rect findBordersAggressive(const uint8_t* pixels, uint32_t width,
                           uint32_t height) {
  const uint32_t sw = width / 2;
  const uint32_t sh = height / 2;
  if (sw == 0 || sh == 0) {
    return {.x = 0, .y = 0, .width = width, .height = height};
  }

  // 2x downsampled ink mask.
  std::vector<uint8_t> ink(sw * sh);
  for (uint32_t y = 0; y < sh; y++) {
    const uint8_t* row0 = pixels + (2 * y) * width;
    const uint8_t* row1 = pixels + (2 * y + 1) * width;
    for (uint32_t x = 0; x < sw; x++) {
      const uint32_t sx = 2 * x;
      const bool dark = row0[sx] < thresholdForBlack ||
                        row0[sx + 1] < thresholdForBlack ||
                        row1[sx] < thresholdForBlack ||
                        row1[sx + 1] < thresholdForBlack;
      ink[y * sw + x] = dark ? 1 : 0;
    }
  }

  // Connected components (4-connectivity) via iterative flood fill.
  std::vector<int32_t> labels(sw * sh, -1);
  std::vector<AggComponent> comps;
  std::vector<uint32_t> stack;
  const auto imageArea = (float)(sw * sh);
  const float areaFloor = imageArea * aggressiveContentAreaRatio;
  const float barThickness = (sw < sh ? sw : sh) * aggressiveEdgeBarThicknessRatio;

  for (uint32_t start = 0; start < sw * sh; start++) {
    if (ink[start] == 0 || labels[start] != -1) {
      continue;
    }
    const int32_t id = (int32_t)comps.size();
    AggComponent comp;
    comp.minX = start % sw;
    comp.maxX = comp.minX;
    comp.minY = start / sw;
    comp.maxY = comp.minY;

    stack.push_back(start);
    labels[start] = id;
    while (!stack.empty()) {
      const uint32_t p = stack.back();
      stack.pop_back();
      const uint32_t px = p % sw;
      const uint32_t py = p / sw;
      comp.area++;
      if (px < comp.minX) comp.minX = px;
      if (px > comp.maxX) comp.maxX = px;
      if (py < comp.minY) comp.minY = py;
      if (py > comp.maxY) comp.maxY = py;
      if (px == 0 || py == 0 || px == sw - 1 || py == sh - 1) {
        comp.touchesEdge = true;
      }
      if (px > 0 && ink[p - 1] && labels[p - 1] == -1) {
        labels[p - 1] = id;
        stack.push_back(p - 1);
      }
      if (px < sw - 1 && ink[p + 1] && labels[p + 1] == -1) {
        labels[p + 1] = id;
        stack.push_back(p + 1);
      }
      if (py > 0 && ink[p - sw] && labels[p - sw] == -1) {
        labels[p - sw] = id;
        stack.push_back(p - sw);
      }
      if (py < sh - 1 && ink[p + sw] && labels[p + sw] == -1) {
        labels[p + sw] = id;
        stack.push_back(p + sw);
      }
    }
    comps.push_back(comp);
  }

  // Content bbox over the components that survive the filters.
  bool any = false;
  uint32_t minX = sw, maxX = 0, minY = sh, maxY = 0;
  for (const auto& comp : comps) {
    if ((float)comp.area < areaFloor) {
      continue; // speck / watermark / page number
    }
    if (comp.touchesEdge) {
      const float w = (float)(comp.maxX - comp.minX + 1);
      const float h = (float)(comp.maxY - comp.minY + 1);
      if ((w < barThickness ? w : h) < barThickness) {
        continue; // thin scanner bar / baked-in frame
      }
    }
    any = true;
    if (comp.minX < minX) minX = comp.minX;
    if (comp.maxX > maxX) maxX = comp.maxX;
    if (comp.minY < minY) minY = comp.minY;
    if (comp.maxY > maxY) maxY = comp.maxY;
  }

  if (!any) {
    return {.x = 0, .y = 0, .width = width, .height = height};
  }

  // Map back to full resolution (inclusive max -> +1), scaled by 2.
  uint32_t left = minX * 2;
  uint32_t top = minY * 2;
  uint32_t right = (maxX + 1) * 2;
  uint32_t bottom = (maxY + 1) * 2;
  if (right > width) right = width;
  if (bottom > height) bottom = height;

  // Rim refinement: skip lines at the bbox rim while they are solid dark
  // (>= aggressiveSolidLineRatio) — an attached scanner bar. The first
  // non-solid line stops the walk (sparse included), so art edges survive.
  // Bounded to 10% of the dimension per edge.
  const uint32_t solidY = (uint32_t)((bottom - top) / 2 * aggressiveSolidLineRatio);
  const uint32_t solidX = (uint32_t)((right - left) / 2 * aggressiveSolidLineRatio);
  const uint32_t maxRefineX = width / 10;
  const uint32_t maxRefineY = height / 10;

  for (uint32_t x = left; x < right && x - left < maxRefineX; x++) {
    uint32_t filled = 0;
    for (uint32_t y = top; y < bottom; y += 2) {
      if (isBlackPixel(pixels, width, x, y)) filled++;
    }
    if (filled < solidY) {
      left = x;
      break;
    }
  }
  for (uint32_t x = right; x > left && right - x < maxRefineX; x--) {
    uint32_t filled = 0;
    for (uint32_t y = top; y < bottom; y += 2) {
      if (isBlackPixel(pixels, width, x - 1, y)) filled++;
    }
    if (filled < solidY) {
      right = x;
      break;
    }
  }
  for (uint32_t y = top; y < bottom && y - top < maxRefineY; y++) {
    uint32_t filled = 0;
    for (uint32_t x = left; x < right; x += 2) {
      if (isBlackPixel(pixels, width, x, y)) filled++;
    }
    if (filled < solidX) {
      top = y;
      break;
    }
  }
  for (uint32_t y = bottom; y > top && bottom - y < maxRefineY; y--) {
    uint32_t filled = 0;
    for (uint32_t x = left; x < right; x += 2) {
      if (isBlackPixel(pixels, width, x, y - 1)) filled++;
    }
    if (filled < solidX) {
      bottom = y;
      break;
    }
  }

  return {.x = left,
          .y = top,
          .width = right > left ? right - left : 0,
          .height = bottom > top ? bottom - top : 0};
}

Rect findBorders(uint8_t* pixels, uint32_t width, uint32_t height,
                 bool aggressive) {
  if (aggressive) {
    return findBordersAggressive(pixels, width, height);
  }

  uint32_t top = findBorderTop(pixels, width, height);
  uint32_t bottom = findBorderBottom(pixels, width, height);
  uint32_t left = findBorderLeft(pixels, width, height, top, bottom);
  uint32_t right =
      findBorderRight(pixels, width, height, top, bottom);

  return {.x = left, .y = top, .width = right - left, .height = bottom - top};
}
