//
// Created by len on 25/12/20.
//

#include "borders.h"
#include <cmath>

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

/**
 * Komiho: dominant-color detection samples a thin edge band instead of a
 * single line. Pages with a hairline dark frame baked into the very edge
 * (scanner artifact / source border) read as "dark background" under
 * single-line detection; the scan then hunts white lines, stops at the first
 * one — which is the start of the white margin itself — and the margin never
 * gets cropped. A band vote (dimension/100 lines, simple majority) classifies
 * the hairline frame as the outlier it is. The failure mode of a wrong
 * majority is a boundary at the edge itself (no crop), never cropping into
 * content.
 */
uint32_t edgeBandWidth(uint32_t dimension) {
  const uint32_t band = dimension / 100;
  return band > 0 ? band : 1;
}

/** Return the first x position where there is a substantial amount of fill,
 * starting the search from the left. */
uint32_t findBorderLeft(uint8_t* pixels, uint32_t width, uint32_t height,
                        uint32_t top, uint32_t bottom, bool aggressive) {
  int x, y;
  // Komiho: in aggressive mode a line must be ~10% filled to count as the
  // content edge — watermark / page-number lines are only a few percent
  // filled and must not count. Stock mode keeps the 0.25% limit.
  const auto filledLimit = (uint32_t)round(
      height * (aggressive ? aggressiveFilledRatioLimit : filledRatioLimit) / 2);
  const uint32_t band = edgeBandWidth(width);

  // Scan the first lines to detect dominant color
  uint32_t whitePixels = 0;
  uint32_t blackPixels = 0;

  for (y = top; y < bottom; y += 2) {
    for (x = 0; x < (int)band; x++) {
      if (isBlackPixel(pixels, width, x, y)) {
        blackPixels++;
      } else if (isWhitePixel(pixels, width, x, y)) {
        whitePixels++;
      }
    }
  }

  auto detectFunc = isBlackPixel;
  if (whitePixels > blackPixels && whitePixels > filledLimit) {
    detectFunc = isBlackPixel;
  } else if (blackPixels > filledLimit) {
    detectFunc = isWhitePixel;
  } else {
    // Neither color present in sufficient amount... don't crop anything
    return 0;
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
                         uint32_t top, uint32_t bottom, bool aggressive) {
  int x, y;
  const auto filledLimit = (uint32_t)round(
      height * (aggressive ? aggressiveFilledRatioLimit : filledRatioLimit) / 2);
  const uint32_t band = edgeBandWidth(width);

  // Scan the last lines to detect dominant color
  uint32_t whitePixels = 0;
  uint32_t blackPixels = 0;

  for (y = top; y < bottom; y += 2) {
    for (x = 0; x < (int)band; x++) {
      if (isBlackPixel(pixels, width, width - 1 - x, y)) {
        blackPixels++;
      } else if (isWhitePixel(pixels, width, width - 1 - x, y)) {
        whitePixels++;
      }
    }
  }

  auto detectFunc = isBlackPixel;
  if (whitePixels > blackPixels && whitePixels > filledLimit) {
    detectFunc = isBlackPixel;
  } else if (blackPixels > filledLimit) {
    detectFunc = isWhitePixel;
  } else {
    // Neither color present in sufficient amount... don't crop anything
    return width;
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
uint32_t findBorderTop(uint8_t* pixels, uint32_t width, uint32_t height,
                       bool aggressive) {
  int x, y;
  const auto filledLimit = (uint32_t)round(
      width * (aggressive ? aggressiveFilledRatioLimit : filledRatioLimit) / 2);
  const uint32_t band = edgeBandWidth(height);

  // Scan the first lines to detect dominant color
  uint32_t whitePixels = 0;
  uint32_t blackPixels = 0;

  for (x = 0; x < width; x += 2) {
    for (y = 0; y < (int)band; y++) {
      if (isBlackPixel(pixels, width, x, y)) {
        blackPixels++;
      } else if (isWhitePixel(pixels, width, x, y)) {
        whitePixels++;
      }
    }
  }

  auto detectFunc = isBlackPixel;
  if (whitePixels > blackPixels && whitePixels > filledLimit) {
    detectFunc = isBlackPixel;
  } else if (blackPixels > filledLimit) {
    detectFunc = isWhitePixel;
  } else {
    // Neither color present in sufficient amount... don't crop anything
    return 0;
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
uint32_t findBorderBottom(uint8_t* pixels, uint32_t width, uint32_t height,
                          bool aggressive) {
  int x, y;
  const auto filledLimit = (uint32_t)round(
      width * (aggressive ? aggressiveFilledRatioLimit : filledRatioLimit) / 2);
  const uint32_t band = edgeBandWidth(height);

  // Scan the last lines to detect dominant color
  uint32_t whitePixels = 0;
  uint32_t blackPixels = 0;

  for (x = 0; x < width; x += 2) {
    for (y = 0; y < (int)band; y++) {
      if (isBlackPixel(pixels, width, x, height - 1 - y)) {
        blackPixels++;
      } else if (isWhitePixel(pixels, width, x, height - 1 - y)) {
        whitePixels++;
      }
    }
  }

  auto detectFunc = isBlackPixel;
  if (whitePixels > blackPixels && whitePixels > filledLimit) {
    detectFunc = isBlackPixel;
  } else if (blackPixels > filledLimit) {
    detectFunc = isWhitePixel;
  } else {
    // Neither color present in sufficient amount... don't crop anything
    return height;
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

Rect findBorders(uint8_t* pixels, uint32_t width, uint32_t height,
                 bool aggressive) {
  uint32_t top = findBorderTop(pixels, width, height, aggressive);
  uint32_t bottom = findBorderBottom(pixels, width, height, aggressive);
  uint32_t left = findBorderLeft(pixels, width, height, top, bottom, aggressive);
  uint32_t right =
      findBorderRight(pixels, width, height, top, bottom, aggressive);

  return {.x = left, .y = top, .width = right - left, .height = bottom - top};
}
