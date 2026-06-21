#pragma once

#include <Arduino.h>
#include <M5Chain.h>

#include "bitman/frame8.hpp"

namespace bitman {

class ChainMonoDisplay {
public:
    bool begin(HardwareSerial& serial, int rxPin, int txPin, std::uint8_t brightness = 4);
    bool reconnect();
    bool show(const Frame8& frame);
    bool clear();
    bool setBrightness(std::uint8_t brightness);

    bool connected() const { return connected_; }
    std::uint8_t brightness() const { return brightness_; }

private:
    bool discover();
    bool operationSucceeded(chain_status_t status, std::uint8_t operationStatus) const;

    Chain chain_;
    HardwareSerial* serial_ = nullptr;
    int rxPin_ = -1;
    int txPin_ = -1;
    std::uint8_t monoId_ = 0;
    std::uint8_t brightness_ = 4;
    bool connected_ = false;
};

}  // namespace bitman

