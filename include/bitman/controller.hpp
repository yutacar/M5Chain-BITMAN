#pragma once

#include "bitman/types.hpp"

namespace bitman {

// Hardware adapters implement this interface. BITMAN Core never calls M5 APIs.
class IBitmanController {
public:
    virtual ~IBitmanController() = default;
    virtual bool begin() = 0;
    virtual void update() = 0;
    virtual ControllerCapabilities capabilities() const = 0;
    virtual bool latestImuSample(ImuSample& sample) const = 0;
    virtual ControlEvent pollEvent() = 0;
    virtual void showStatus(const char* title, const char* detail) = 0;
};

}  // namespace bitman

