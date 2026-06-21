#pragma once

#include <cstddef>
#include <cstdint>

#include "bitman/frame8.hpp"

namespace bitman {

// Crops an 8x8 sprite from a horizontal virtual canvas into one 8x8 panel.
// sourceOriginX is the sprite's left edge in virtual-canvas pixels.
Frame8 composePanelFrame(const Frame8& source, std::size_t panelIndex, int sourceOriginX);

// Returns the horizontal origin for one complete perimeter lap. The sprite
// reaches an end panel before each corner animation begins.
int demoTravelOriginX(std::size_t panelCount, std::uint32_t elapsedMs);

}  // namespace bitman
