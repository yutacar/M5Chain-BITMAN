#pragma once

#include <array>
#include <cstddef>

#include <Arduino.h>
#include <M5Chain.h>

#include "bitman/frame8.hpp"

namespace bitman {

class ChainMonoDisplay {
public:
    bool begin(HardwareSerial& serial, int rxPin, int txPin, std::uint8_t brightness = 4);
    bool reconnect();
    bool show(const Frame8& frame);
    bool showTraveling(const Frame8& frame, std::uint32_t elapsedMs);
    bool clear();
    bool setBrightness(std::uint8_t brightness);

    bool connected() const { return connected_; }
    std::uint8_t brightness() const { return brightness_; }
    std::uint8_t panelCount() const { return monoCount_; }

private:
    static constexpr std::size_t kMaximumPanels = 16;

    bool discover();
    bool operationSucceeded(chain_status_t status, std::uint8_t operationStatus) const;
    bool configurePanel(std::uint8_t id);
    bool writePanel(std::size_t panelIndex, const Frame8& frame);
    bool sameFrame(const Frame8& left, const Frame8& right) const { return left == right; }

    Chain chain_;
    HardwareSerial* serial_ = nullptr;
    int rxPin_ = -1;
    int txPin_ = -1;
    std::array<std::uint8_t, kMaximumPanels> monoIds_{};
    std::uint8_t monoCount_ = 0;
    std::uint8_t brightness_ = 4;
    bool connected_ = false;
    bool cacheValid_ = false;
    bool cacheTraveling_ = false;
    int cachedTravelX_ = -1;
    Frame8 cachedFrame_{};
};

}  // namespace bitman
