// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "flutter/fml/logging.h"
#include "impeller/geometry/gradient.h"

namespace impeller {

static void AppendColor(const Color& color, std::vector<uint8_t>* colors) {
  auto converted = color.ToR8G8B8A8();
  colors->push_back(converted[0]);
  colors->push_back(converted[1]);
  colors->push_back(converted[2]);
  colors->push_back(converted[3]);
}

GradientData CreateGradientBuffer(const std::vector<Color>& colors,
                                  const std::vector<Scalar>& stops) {
  FML_DCHECK(stops.size() == colors.size());

  uint32_t texture_size;
  if (stops.size() == 2) {
    texture_size = colors.size();
  } else {
    auto minimum_delta = 1.0;
    for (size_t i = 1; i < stops.size(); i++) {
      auto value = stops[i] - stops[i - 1];
      // Smaller than kEhCloseEnough
      if (value < 0.0001) {
        continue;
      }
      if (value < minimum_delta) {
        minimum_delta = value;
      }
    }
    // Avoid creating textures that are absurdly large due to stops that are
    // very close together.
    // TODO(jonahwilliams): this should use a platform specific max texture
    // size.
    texture_size =
        std::min((uint32_t)std::round(1.0 / minimum_delta) + 1, 1024u);
  }
  std::vector<uint8_t> color_stop_channels;
  color_stop_channels.reserve(texture_size * 4);

  if (texture_size == colors.size() && colors.size() <= 1024) {
    for (auto i = 0u; i < colors.size(); i++) {
      AppendColor(colors[i], &color_stop_channels);
    }
  } else {
    Color previous_color = colors[0];
    auto previous_stop = 0.0;
    auto previous_color_index = 0;

    // The first index is always equal to the first color, exactly.
    AppendColor(previous_color, &color_stop_channels);

    for (auto i = 1u; i < texture_size - 1; i++) {
      auto scaled_i = i / (texture_size - 1.0);
      Color next_color = colors[previous_color_index + 1];
      auto next_stop = stops[previous_color_index + 1];
      // We're almost exactly equal to the next stop.
      if (ScalarNearlyEqual(scaled_i, next_stop)) {
        AppendColor(next_color, &color_stop_channels);

        previous_color = next_color;
        previous_stop = next_stop;
        previous_color_index += 1;
      } else if (scaled_i < next_stop) {
        // We're still between the current stop and the next stop.
        auto t = (scaled_i - previous_stop) / (next_stop - previous_stop);
        auto mixed_color = Color::lerp(previous_color, next_color, t);

        AppendColor(mixed_color, &color_stop_channels);
      } else {
        // We've slightly overshot the previous stop.
        previous_color = next_color;
        previous_stop = next_stop;
        previous_color_index += 1;
        next_color = colors[previous_color_index + 1];
        auto next_stop = stops[previous_color_index + 1];

        auto t = (scaled_i - previous_stop) / (next_stop - previous_stop);
        auto mixed_color = Color::lerp(previous_color, next_color, t);

        AppendColor(mixed_color, &color_stop_channels);
      }
    }
    // The last index is always equal to the last color, exactly.
    AppendColor(colors.back(), &color_stop_channels);
  }
  return GradientData{
      .color_bytes = std::move(color_stop_channels),
      .texture_size = texture_size,
  };
}

}  // namespace impeller
