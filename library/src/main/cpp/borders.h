//
// Created by len on 25/12/20.
//

#ifndef IMAGEDECODER_BORDERS_H
#define IMAGEDECODER_BORDERS_H

#include "rect.h"

/** A line will be considered as having content if 0.25% of it is filled. */
const float filledRatioLimit = 0.0025;

/**
 * Komiho: aggressive mode threshold. Watermarks / page numbers sit inside the
 * margin but their line coverage is only a few percent, so the stock limit
 * (0.25%) treats their lines as content and the margin is never cropped. In
 * aggressive mode a line must be ~10% filled to count as the content edge,
 * which skips such small margin artefacts while still stopping at real panel
 * frame lines (~50%+ coverage). The same limit also governs the edge-line
 * dominant-color check: at the stock limit a couple of dark samples on the
 * very edge line (scanner noise, watermark bleed) read as "mixed fill" and
 * silently abort the whole edge.
 */
const float aggressiveFilledRatioLimit = 0.10;

/** When the threshold is closer to 1, less content will be cropped. **/
#define THRESHOLD 0.75

const uint8_t thresholdForBlack = (uint8_t)(255.0 * THRESHOLD);

const uint8_t thresholdForWhite = (uint8_t)(255.0 - 255.0 * THRESHOLD);

/** Finds the borders of the image. This only works on bitmaps of a single
 * component (grayscale) **/
Rect findBorders(uint8_t* pixels, uint32_t width, uint32_t height,
                 bool aggressive = false);

#endif // IMAGEDECODER_BORDERS_H
