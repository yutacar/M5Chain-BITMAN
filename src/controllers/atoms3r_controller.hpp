#pragma once

#include <M5Unified.h>

#include "bitman/controller.hpp"

namespace bitman {

class AtomS3RController final : public IBitmanController {
public:
    bool begin() override;
    void update() override;
    ControllerCapabilities capabilities() const override;
    bool latestImuSample(ImuSample& sample) const override;
    ControlEvent pollEvent() override;
    void showStatus(const char* title, const char* detail) override;

private:
    ImuSample latest_{};
    ControlEvent pendingEvent_ = ControlEvent::None;
};

}  // namespace bitman

