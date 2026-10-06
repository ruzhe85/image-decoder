//
// Created by len on 25/12/20.
//

#ifndef IMAGEDECODER_BORDERS_H
#define IMAGEDECODER_BORDERS_H

#include "rect.h"

/** A line will be considered as having content if 0.25% of it is filled. */
const float filledRatioLimit = 0.0025;

/**
 * Komiho: aggressive mode content-floor. A connected component counts as
 * content if its area is at least this fraction of the (downsampled) image
 * area — watermarks, page numbers and specks fall below it, panels are far
 * above.
 */
const float aggressiveContentAreaRatio = 0.0002f;

/**
 * Komiho: aggressive mode edge-bar rule. A component touching the image edge
 * is only kept if it is at least this thick (min of its width/height) — that
 * keeps full-bleed art while discarding the thin black scanner bars / baked-in
 * frames that sit between the margin and the content.
 */
const float aggressiveEdgeBarThicknessRatio = 0.03f;

/**
 * Komiho: aggressive mode rim refinement. A scanner bar 4-connected to the
 * content merges into a kept component and drags the content bbox to the
 * image edge; walking inward from each rim while lines are >= this fraction
 * dark removes such attached bars. The walk stops at the first non-solid
 * line, so sparse art edges are never eaten.
 */
const float aggressiveSolidLineRatio = 0.85f;

/** When the threshold is closer to 1, less content will be cropped. **/
#define THRESHOLD 0.75

const uint8_t thresholdForBlack = (uint8_t)(255.0 * THRESHOLD);

const uint8_t thresholdForWhite = (uint8_t)(255.0 - 255.0 * THRESHOLD);

/** Finds the borders of the image. This only works on bitmaps of a single
 * component (grayscale) **/
Rect findBorders(uint8_t* pixels, uint32_t width, uint32_t height,
                 bool aggressive = false);

#endif // IMAGEDECODER_BORDERS_H
