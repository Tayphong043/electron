// Copyright (c) 2024 Salesforce, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "shell/renderer/electron_smooth_round_rect.h"

#include <numbers>
#include "base/check_op.h"

namespace electron {

namespace {

constexpr float PI_DIV_4 = std::numbers::pi / 4.0f;
constexpr float EDGE_CURVE_POINT_RATIO = 2.0f / 3.0f;

inline constexpr SkPoint MakeXPoint(float x) {
  return {x, 0.0f};
}
inline constexpr SkPoint MakeYPoint(float y) {
  return {0.0f, y};
}

}  // namespace

// ASSUMPTIONS:
// - Radii are positive and not zero
// - Size (width, height) fits full radius + extended rounding
SkPath CalculateSmoothRoundRect(float x,
                                float y,
                                float width,
                                float height,
                                float smoothness,
                                float radius) {
  DCHECK_GT(width, 0.0f);
  DCHECK_GT(height, 0.0f);
  DCHECK_GT(smoothness,
            0.0f);  // smoothness == 0 should call an optimized procedure
  DCHECK_LE(smoothness, 1.0f);
  DCHECK_GT(radius, 0.0f);

  //
  // Calculations
  //

  // ξ = smoothness
  // R = radius

  // "parallel" (+X axis) and "perpendicular" (+Y axis) are in reference to the
  // direction of the edge.

  // The edge length used by simple rounding.
  //
  // Originally:
  //   `q = R * sqrt((1 + cos θ) / (1 - cos θ))`
  //
  // Since the corner of a rectangle is exactly 90 degrees, `cos θ = 0`, thus
  // we have:
  //   `q = R`
  float rounding_segment_length = radius;

  // The edge length used by rounding and smoothing.
  //   `p = (1 + ξ) * q`
  float smoothing_rounding_segment_length =
      (1.0f + smoothness) * rounding_segment_length;

  float edge_connecting_offset = smoothing_rounding_segment_length;

  // The angle where the curve connects to the arc.
  //   `45º * ξ`
  float arc_connecting_angle = PI_DIV_4 * smoothness;

  // The offset from the center of rounding where the curve connects to the arc.
  SkVector arc_connecting_vector =
      SkVector::Make(1.0f - sin(arc_connecting_angle),
                     1.0f - cos(arc_connecting_angle)) *
      radius;

  float arc_curve_offset_from_connecting =
      tan(arc_connecting_angle / 2.0f) * cos(arc_connecting_angle) * radius;
  // The offset from |arc_connecting_point| to the edge axis in the parallel
  // direction.
  float arc_curve_offset =
      arc_connecting_vector.x() + arc_curve_offset_from_connecting;

  // The offset from the edge connector point to corner in the parallel
  // direction.
  float edge_curve_offset =
      smoothing_rounding_segment_length -
      ((smoothing_rounding_segment_length - arc_curve_offset) *
       EDGE_CURVE_POINT_RATIO);

  //
  // Drawing the path
  //
  SkPath path;

  //
  // Upper left corner
  //
  SkPoint upper_left_corner = SkPoint::Make(x, y);
  path.moveTo(upper_left_corner + MakeYPoint(edge_connecting_offset));
  path.cubicTo(upper_left_corner + MakeYPoint(edge_curve_offset),
               upper_left_corner + MakeYPoint(arc_curve_offset),
               upper_left_corner + SkPoint::Make(arc_connecting_vector.y(),
                                                 arc_connecting_vector.x()));
  path.arcTo(SkPoint::Make(radius, radius), 0.0f, SkPath::kSmall_ArcSize,
             SkPathDirection::kCW, upper_left_corner + arc_connecting_vector);
  path.cubicTo(upper_left_corner + MakeXPoint(arc_curve_offset),
               upper_left_corner + MakeXPoint(edge_curve_offset),
               upper_left_corner + MakeXPoint(edge_connecting_offset));

  //
  // Upper right corner
  //
  SkPoint upper_right_corner = SkPoint::Make(x + width, y);
  path.lineTo(upper_right_corner - MakeXPoint(edge_connecting_offset));
  path.cubicTo(upper_right_corner - MakeXPoint(edge_curve_offset),
               upper_right_corner - MakeXPoint(arc_curve_offset),
               upper_right_corner - SkPoint::Make(arc_connecting_vector.x(),
                                                  -arc_connecting_vector.y()));
  path.arcTo(SkPoint::Make(radius, radius), 0.0f, SkPath::kSmall_ArcSize,
             SkPathDirection::kCW,
             upper_right_corner - SkPoint::Make(arc_connecting_vector.y(),
                                                -arc_connecting_vector.x()));
  path.cubicTo(upper_right_corner + MakeYPoint(arc_curve_offset),
               upper_right_corner + MakeYPoint(edge_curve_offset),
               upper_right_corner + MakeYPoint(edge_connecting_offset));

  //
  // Bottom right corner
  //
  SkPoint bottom_right_corner = SkPoint::Make(x + width, y + height);
  path.lineTo(bottom_right_corner - MakeYPoint(edge_connecting_offset));
  path.cubicTo(bottom_right_corner - MakeYPoint(edge_curve_offset),
               bottom_right_corner - MakeYPoint(arc_curve_offset),
               bottom_right_corner - SkPoint::Make(arc_connecting_vector.y(),
                                                   arc_connecting_vector.x()));
  path.arcTo(SkPoint::Make(radius, radius), 0.0f, SkPath::kSmall_ArcSize,
             SkPathDirection::kCW, bottom_right_corner - arc_connecting_vector);
  path.cubicTo(bottom_right_corner - MakeXPoint(arc_curve_offset),
               bottom_right_corner - MakeXPoint(edge_curve_offset),
               bottom_right_corner - MakeXPoint(edge_connecting_offset));

  //
  // Bottom left corner
  //
  SkPoint bottom_left_corner = SkPoint::Make(x, y + height);
  path.lineTo(bottom_left_corner + MakeXPoint(edge_connecting_offset));
  path.cubicTo(bottom_left_corner + MakeXPoint(edge_curve_offset),
               bottom_left_corner + MakeXPoint(arc_curve_offset),
               bottom_left_corner + SkPoint::Make(arc_connecting_vector.x(),
                                                  -arc_connecting_vector.y()));
  path.arcTo(SkPoint::Make(radius, radius), 0.0f, SkPath::kSmall_ArcSize,
             SkPathDirection::kCW,
             bottom_left_corner + SkPoint::Make(arc_connecting_vector.y(),
                                                -arc_connecting_vector.x()));
  path.cubicTo(bottom_left_corner - MakeYPoint(arc_curve_offset),
               bottom_left_corner - MakeYPoint(edge_curve_offset),
               bottom_left_corner - MakeYPoint(edge_connecting_offset));

  //
  // Done
  //
  path.close();
  return path;
}

}  // namespace electron
