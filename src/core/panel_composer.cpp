#include "bitman/panel_composer.hpp"

#include <cstdint>

namespace bitman {

namespace {

// Matches the human-animation portion of BitmanEngine::updateDemo.
constexpr std::uint32_t kDemoTravelStartMs = 1000;
constexpr std::uint32_t kDemoEndingStartMs = 13820;
constexpr std::uint32_t kDemoCrouchDurationMs = 900;
constexpr std::uint32_t kDemoTransitionDurationMs = 2080;
constexpr std::uint32_t kDemoGroundPauseDurationMs = 900;

// The first horizontal run uses the opening crouch sequence. After turning
// Down -> Right -> Up at the far end, the second ground pause carries BITMAN
// back along the top edge. All corner transitions therefore happen with the
// complete sprite parked on an end panel.
constexpr std::uint32_t kDemoFarEndMs = kDemoTravelStartMs + kDemoCrouchDurationMs;
constexpr std::uint32_t kDemoReturnStartMs =
    kDemoFarEndMs + kDemoTransitionDurationMs + kDemoGroundPauseDurationMs +
    kDemoTransitionDurationMs;
constexpr std::uint32_t kDemoReturnEndMs =
    kDemoReturnStartMs + kDemoGroundPauseDurationMs;

int interpolate(int from, int to, std::uint32_t elapsed, std::uint32_t duration)
{
    const auto distance = static_cast<std::int64_t>(to - from);
    const auto rounded = distance >= 0 ? static_cast<std::int64_t>(duration / 2U)
                                       : -static_cast<std::int64_t>(duration / 2U);
    return from + static_cast<int>(
                      (static_cast<std::int64_t>(elapsed) * distance + rounded) /
                      static_cast<std::int64_t>(duration));
}

}  // namespace

Frame8 composePanelFrame(const Frame8& source, std::size_t panelIndex, int sourceOriginX)
{
    Frame8::Rows rows{};
    const int panelOriginX = static_cast<int>(panelIndex * 8U);
    for (std::size_t y = 0; y < 8; ++y) {
        for (std::size_t sourceX = 0; sourceX < 8; ++sourceX) {
            if (!source.pixel(sourceX, y)) {
                continue;
            }
            const int localX = sourceOriginX + static_cast<int>(sourceX) - panelOriginX;
            if (localX >= 0 && localX < 8) {
                rows[y] |= static_cast<std::uint8_t>(0x80U >> localX);
            }
        }
    }
    return Frame8(rows);
}

int demoTravelOriginX(std::size_t panelCount, std::uint32_t elapsedMs)
{
    if (panelCount <= 1) {
        return 0;
    }
    if (elapsedMs <= kDemoTravelStartMs || elapsedMs >= kDemoEndingStartMs) {
        return 0;
    }

    const int maximumX = static_cast<int>((panelCount - 1U) * 8U);
    if (elapsedMs < kDemoFarEndMs) {
        return interpolate(0, maximumX, elapsedMs - kDemoTravelStartMs,
                           kDemoCrouchDurationMs);
    }
    if (elapsedMs < kDemoReturnStartMs) {
        return maximumX;
    }
    if (elapsedMs < kDemoReturnEndMs) {
        return interpolate(maximumX, 0, elapsedMs - kDemoReturnStartMs,
                           kDemoGroundPauseDurationMs);
    }
    return 0;
}

}  // namespace bitman
