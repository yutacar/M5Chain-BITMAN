#pragma once

#include <cstdint>

#include "bitman/bitman_engine.hpp"
#include "bitman/motion_classifier.hpp"

namespace bitman {

class BitmanCore {
public:
    explicit BitmanCore(const MotionConfig& config = MotionConfig{});

    void reset(std::uint32_t nowMs = 0);
    void setMode(BitmanMode mode, std::uint32_t nowMs);
    void playDemo(std::uint32_t nowMs);

    // sample may be null for controllers without an IMU.
    bool update(std::uint32_t nowMs, const ImuSample* sample);

    const Frame8& frame() const { return engine_.frame(); }
    Pose pose() const { return engine_.pose(); }
    BitmanMode mode() const { return engine_.mode(); }
    const MotionResult& motion() const { return motion_; }

private:
    MotionClassifier classifier_;
    BitmanEngine engine_;
    MotionResult motion_{};
    bool clockInitialized_ = false;
    std::uint32_t previousUpdateMs_ = 0;
};

}  // namespace bitman

