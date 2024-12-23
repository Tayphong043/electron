// Copyright (c) 2024 Salesforce, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ELECTRON_SHELL_RENDERER_API_ELECTRON_SMOOTH_ROUND_RECT_H_
#define ELECTRON_SHELL_RENDERER_API_ELECTRON_SMOOTH_ROUND_RECT_H_

#include "third_party/skia/include/core/SkPath.h"

namespace electron {

SkPath CalculateSmoothRoundRect(float x,
                                float y,
                                float width,
                                float height,
                                float roundness,
                                float radius);

}  // namespace electron

#endif  // ELECTRON_SHELL_RENDERER_API_ELECTRON_SMOOTH_ROUND_RECT_H_
