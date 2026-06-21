#pragma once

#include <cstdint>

#include "bitman/frame8.hpp"
#include "bitman/sprites.hpp"
#include "bitman/types.hpp"

namespace bitman {

class BitmanEngine {
public:
    BitmanEngine();

    void reset(std::uint32_t nowMs = 0);
    void setMode(BitmanMode mode, std::uint32_t nowMs);
    void playDemo(std::uint32_t nowMs);

    // Returns true only when the 8x8 output changed.
    bool update(std::uint32_t nowMs, const MotionResult& motion);

    BitmanMode mode() const { return mode_; }
    MotionState motion() const { return motion_; }
    GroundDirection ground() const { return renderGround_; }
    Pose pose() const { return pose_; }
    const Frame8& frame() const { return frame_; }

private:
    Pose dancePose(bool walking, bool clockwise, std::uint32_t phase) const;
    bool setPose(Pose pose, GroundDirection ground, int offsetX = 0, int offsetY = 0);
    bool renderTransitionFrame(std::uint32_t phase, GroundDirection oldGround,
                               GroundDirection newGround, int direction);
    bool updateTransition(std::uint32_t nowMs);
    void startNextTransition(std::uint32_t nowMs, bool clockwise);
    bool updateDance(std::uint32_t nowMs, const MotionResult& motion);
    bool updateDemo(std::uint32_t nowMs);

    BitmanMode mode_ = BitmanMode::Dance;
    MotionState motion_ = MotionState::Neutral;
    GroundDirection displayedGround_ = GroundDirection::Down;
    GroundDirection desiredGround_ = GroundDirection::Down;
    GroundDirection transitionTarget_ = GroundDirection::Down;
    GroundDirection renderGround_ = GroundDirection::Down;
    GroundDirection demoStartGround_ = GroundDirection::Down;
    bool clockwise_ = true;
    bool transitioning_ = false;
    int transitionDirection_ = 1;
    int offsetX_ = 0;
    int offsetY_ = 0;
    Pose pose_ = Pose::Blank;
    Frame8 frame_{};
    std::uint32_t modeStartedMs_ = 0;
    std::uint32_t motionStartedMs_ = 0;
    std::uint32_t walkingUntilMs_ = 0;
    std::uint32_t transitionStartedMs_ = 0;
};

}  // namespace bitman
