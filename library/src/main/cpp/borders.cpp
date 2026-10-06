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

/** Return the first x position where there is a substantial amount of fill,
 * starting the search from the left. */
uint32_t findBorderLeft(uint8_t* pixels, uint32_t width, uint32_t height,
                        uint32_t top, uint32_t bottom, bool aggressive) {
  int x, y;
  // Komiho: the dominant-color check keeps the stock limit so dark/mixed page
  // detection behaves exactly as before; only the content-edge scan uses the
  // aggressive limit (watermark/page-number lines are a few percent filled and
  // must not count as content).
  const auto detectLimit = (uint32_t)round(height * filledRatioLimit / 2);
  const auto filledLimit = (uint32_t)round(
      height * (aggressive ? aggressiveFilledRatioLimit : filledRatioLimit) / 2);

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
  if (whitePixels > detectLimit && blackPixels > detectLimit) {
    // Mixed fill found... don't crop anything
    return 0;
  } else if (blackPixels > detectLimit) {
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
                         uint32_t top, uint32_t bottom, bool aggressive) {
  int x, y;
  const auto detectLimit = (uint32_t)round(height * filledRatioLimit / 2);
  const auto filledLimit = (uint32_t)round(
      height * (aggressive ? aggressiveFilledRatioLimit : filledRatioLimit) / 2);

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
  if (whitePixels > detectLimit && blackPixels > detectLimit) {
    // Mixed fill found... don't crop anything
    return width;
  } else if (blackPixels > detectLimit) {
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
uint32_t findBorderTop(uint8_t* pixels, uint32_t width, uint32_t height,
                       bool aggressive) {
  int x, y;
  const auto detectLimit = (uint32_t)round(width * filledRatioLimit / 2);
  const auto filledLimit = (uint32_t)round(
      width * (aggressive ? aggressiveFilledRatioLimit : filledRatioLimit) / 2);

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
  if (whitePixels > detectLimit && blackPixels > detectLimit) {
    // Mixed fill found... don't crop anything
    return 0;
  } else if (blackPixels > detectLimit) {
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
uint32_t findBorderBottom(uint8_t* pixels, uint32_t width, uint32_t height,
                          bool aggressive) {
  int x, y;
  const auto detectLimit = (uint32_t)round(width * filledRatioLimit / 2);
  const auto filledLimit = (uint32_t)round(
      width * (aggressive ? aggressiveFilledRatioLimit : filledRatioLimit) / 2);

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
  if (whitePixels > detectLimit && blackPixels > detectLimit) {
    // Mixed fill found... don't crop anything
    return height;
  } else if (blackPixels > detectLimit) {
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

Rect findBorders(uint8_t* pixels, uint32_t width, uint32_t height,
                 bool aggressive) {
  uint32_t top = findBorderTop(pixels, width, height, aggressive);
  uint32_t bottom = findBorderBottom(pixels, width, height, aggressive);
  uint32_t left = findBorderLeft(pixels, width, height, top, bottom, aggressive);
  uint32_t right =
      findBorderRight(pixels, width, height, top, bottom, aggressive);

  return {.x = left, .y = top, .width = right - left, .height = bottom - top};
}
