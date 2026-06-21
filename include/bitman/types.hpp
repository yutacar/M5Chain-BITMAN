#pragma once

#include <cstdint>

namespace bitman {

struct ImuSample {
    float ax = 0.0F;
    float ay = 0.0F;
    float az = 1.0F;
    float gx = 0.0F;
    float gy = 0.0F;
    float gz = 0.0F;
    bool valid = false;
};

enum class GroundDirection : std::uint8_t {
    Down,
    Right,
    Up,
    Left,
};

enum class MotionState : std::uint8_t {
    Neutral,
    LeanLeft,
    LeanRight,
    Crouch,
    Jump,
    Shake,
    UpsideDown,
};

struct MotionResult {
    MotionState state = MotionState::Neutral;
    GroundDirection ground = GroundDirection::Down;
    float rollDegrees = 0.0F;
    float pitchDegrees = 0.0F;
    float groundAngleDegrees = 0.0F;
    float shakeStrength = 0.0F;
    bool rotating = false;
    bool clockwise = true;
};

enum class BitmanMode : std::uint8_t {
    Dance,
    Demo,
};

enum class ControlEvent : std::uint8_t {
    None,
    NextMode,
    PlayDemo,
    CycleBrightness,
};

struct ControllerCapabilities {
    bool imu = false;
    bool rtc = false;
    bool screen = false;
    bool touch = false;
    bool vibration = false;
    bool externalMotion = false;
};

}  // namespace bitman
