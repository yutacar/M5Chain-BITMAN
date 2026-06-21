#include "bitman/motion_classifier.hpp"

#include <algorithm>
#include <cmath>

namespace bitman {
namespace {

constexpr float kRadiansToDegrees = 57.29577951308232F;

float magnitude(float x, float y, float z)
{
    return std::sqrt(x * x + y * y + z * z);
}

float normalizeDegrees(float degrees)
{
    while (degrees < 0.0F) {
        degrees += 360.0F;
    }
    while (degrees >= 360.0F) {
        degrees -= 360.0F;
    }
    return degrees;
}

float shortestAngleDelta(float from, float to)
{
    float delta = normalizeDegrees(to) - normalizeDegrees(from);
    if (delta > 180.0F) {
        delta -= 360.0F;
    } else if (delta < -180.0F) {
        delta += 360.0F;
    }
    return delta;
}

float groundCenterDegrees(GroundDirection ground)
{
    switch (ground) {
        case GroundDirection::Right:
            return 90.0F;
        case GroundDirection::Up:
            return 180.0F;
        case GroundDirection::Left:
            return 270.0F;
        case GroundDirection::Down:
        default:
            return 0.0F;
    }
}

}  // namespace

MotionClassifier::MotionClassifier(const MotionConfig& config) : config_(config)
{
    reset();
}

void MotionClassifier::reset()
{
    initialized_ = false;
    fax_ = 0.0F;
    fay_ = 0.0F;
    faz_ = 1.0F;
    previousAx_ = 0.0F;
    previousAy_ = 0.0F;
    previousAz_ = 1.0F;
    shakeRemainingSeconds_ = 0.0F;
    groundInitialized_ = false;
    ground_ = GroundDirection::Down;
    previousGroundAngleDegrees_ = 0.0F;
    result_ = MotionResult{};
}

GroundDirection MotionClassifier::classifyGround(float angleDegrees) const
{
    const int quadrant = static_cast<int>((normalizeDegrees(angleDegrees) + 45.0F) / 90.0F) & 3;
    switch (quadrant) {
        case 1:
            return GroundDirection::Right;
        case 2:
            return GroundDirection::Up;
        case 3:
            return GroundDirection::Left;
        case 0:
        default:
            return GroundDirection::Down;
    }
}

MotionState MotionClassifier::classifyTilt(float roll, float pitch, float filteredZ) const
{
    if (filteredZ < config_.upsideDownThresholdG) {
        return MotionState::UpsideDown;
    }

    const float threshold = result_.state == MotionState::Neutral ? config_.tiltEnterDegrees
                                                                  : config_.tiltReleaseDegrees;

    // Pitch takes priority over roll so diagonal tilts remain deterministic.
    if (pitch >= threshold) {
        return MotionState::LeanLeft;
    }
    if (pitch <= -threshold) {
        return MotionState::LeanRight;
    }
    if (roll >= threshold) {
        return MotionState::Crouch;
    }
    if (roll <= -threshold) {
        return MotionState::Jump;
    }
    return MotionState::Neutral;
}

MotionResult MotionClassifier::update(const ImuSample& sample, float deltaSeconds)
{
    if (!sample.valid) {
        return result_;
    }

    deltaSeconds = std::max(0.001F, std::min(deltaSeconds, 0.25F));

    if (!initialized_) {
        fax_ = previousAx_ = sample.ax;
        fay_ = previousAy_ = sample.ay;
        faz_ = previousAz_ = sample.az;
        initialized_ = true;
    }

    const float alpha = deltaSeconds / (config_.filterTimeConstantSeconds + deltaSeconds);
    fax_ += alpha * (sample.ax - fax_);
    fay_ += alpha * (sample.ay - fay_);
    faz_ += alpha * (sample.az - faz_);

    const float jerk = magnitude(sample.ax - previousAx_, sample.ay - previousAy_, sample.az - previousAz_) /
                       deltaSeconds;
    const float gyro = magnitude(sample.gx, sample.gy, sample.gz);
    previousAx_ = sample.ax;
    previousAy_ = sample.ay;
    previousAz_ = sample.az;

    const float roll = std::atan2(fay_, faz_) * kRadiansToDegrees;
    const float pitch = std::atan2(-fax_, std::sqrt(fay_ * fay_ + faz_ * faz_)) * kRadiansToDegrees;
    const float groundProjection = std::sqrt(fax_ * fax_ + fay_ * fay_);
    float groundAngle = previousGroundAngleDegrees_;
    float angularRate = 0.0F;
    bool groundChanged = false;

    if (groundProjection >= config_.groundProjectionMinG) {
        groundAngle = normalizeDegrees(std::atan2(fax_, fay_) * kRadiansToDegrees);
        if (!groundInitialized_) {
            ground_ = classifyGround(groundAngle);
            previousGroundAngleDegrees_ = groundAngle;
            groundInitialized_ = true;
        } else {
            const float delta = shortestAngleDelta(previousGroundAngleDegrees_, groundAngle);
            angularRate = delta / deltaSeconds;
            previousGroundAngleDegrees_ = groundAngle;

            const float fromCurrentGround =
                std::fabs(shortestAngleDelta(groundCenterDegrees(ground_), groundAngle));
            if (fromCurrentGround >= config_.groundSwitchDegrees) {
                const auto nextGround = classifyGround(groundAngle);
                groundChanged = nextGround != ground_;
                ground_ = nextGround;
            }
        }
    }

    if (jerk >= config_.shakeJerkThresholdGps || gyro >= config_.shakeGyroThresholdDps) {
        shakeRemainingSeconds_ = config_.shakeHoldSeconds;
    } else {
        shakeRemainingSeconds_ = std::max(0.0F, shakeRemainingSeconds_ - deltaSeconds);
    }

    result_.rollDegrees = roll;
    result_.pitchDegrees = pitch;
    result_.ground = ground_;
    result_.groundAngleDegrees = groundAngle;
    result_.shakeStrength = std::max(jerk / config_.shakeJerkThresholdGps,
                                     gyro / config_.shakeGyroThresholdDps);
    result_.clockwise = angularRate >= 0.0F;
    result_.rotating = groundChanged || std::fabs(angularRate) >= config_.rotationThresholdDps ||
                       std::fabs(sample.gz) >= config_.rotationThresholdDps;
    result_.state = shakeRemainingSeconds_ > 0.0F && !result_.rotating
                        ? MotionState::Shake
                        : classifyTilt(roll, pitch, faz_);
    return result_;
}

}  // namespace bitman
