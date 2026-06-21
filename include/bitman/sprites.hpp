#pragma once

#include <cstdint>

#include "bitman/frame8.hpp"

namespace bitman {

enum class Pose : std::uint8_t {
    Blank,
    IdleA,
    IdleB,
    StepLeftA,
    StepLeftB,
    StepRightA,
    StepRightB,
    CrouchA,
    CrouchB,
    JumpA,
    JumpB,
    ShakeA,
    ShakeB,
    HeadstandA,
    HeadstandB,
    Surprise,
    BoxOuter,
    BoxMiddle,
    BoxInner,
};

const Frame8& sprite(Pose pose);
const char* poseName(Pose pose);

}  // namespace bitman
