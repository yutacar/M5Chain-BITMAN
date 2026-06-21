#pragma once

#include "bitman/types.hpp"

namespace bitman {

struct MotionConfig {
    float filterTimeConstantSeconds = 0.10F;
    float tiltEnterDegrees = 35.0F;
    float tiltReleaseDegrees = 22.0F;
    float upsideDownThresholdG = -0.65F;
    float groundProjectionMinG = 0.25F;
    float groundSwitchDegrees = 55.0F;
    float rotationThresholdDps = 20.0F;
    float shakeJerkThresholdGps = 8.0F;
    float shakeGyroThresholdDps = 220.0F;
    float shakeHoldSeconds = 0.35F;
};

class MotionClassifier {
public:
    explicit MotionClassifier(const MotionConfig& config = MotionConfig{});

    void reset();
    MotionResult update(const ImuSample& sample, float deltaSeconds);
    const MotionResult& result() const { return result_; }

private:
    MotionState classifyTilt(float roll, float pitch, float filteredZ) const;
    GroundDirection classifyGround(float angleDegrees) const;

    MotionConfig config_;
    bool initialized_ = false;
    float fax_ = 0.0F;
    float fay_ = 0.0F;
    float faz_ = 1.0F;
    float previousAx_ = 0.0F;
    float previousAy_ = 0.0F;
    float previousAz_ = 1.0F;
    float shakeRemainingSeconds_ = 0.0F;
    bool groundInitialized_ = false;
    GroundDirection ground_ = GroundDirection::Down;
    float previousGroundAngleDegrees_ = 0.0F;
    MotionResult result_{};
};

}  // namespace bitman
