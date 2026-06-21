#include "bitman/bitman_engine.hpp"

#include <array>

namespace bitman {
namespace {

struct DemoStep {
    Pose pose;
    std::uint16_t durationMs;
};

// The closing pulse preserves the original demo's final box animation.
constexpr std::array<DemoStep, 8> kDemoEnding = {{
    {Pose::BoxOuter, 500}, {Pose::BoxMiddle, 500}, {Pose::BoxInner, 500},
    {Pose::BoxMiddle, 500}, {Pose::BoxOuter, 500}, {Pose::BoxMiddle, 350},
    {Pose::BoxInner, 350}, {Pose::Blank, 1800},
}};

constexpr std::uint32_t kWalkIntervalMs = 450;
constexpr std::uint32_t kCrouchIntervalMs = 450;
constexpr std::uint32_t kWalkHoldMs = 650;
constexpr std::uint32_t kTransitionFrameMs = 260;
constexpr std::uint32_t kTransitionFrameCount = 6;
constexpr std::uint32_t kTransitionDurationMs = kTransitionFrameMs * kTransitionFrameCount;
constexpr std::uint32_t kDemoBlankMs = 1000;
constexpr std::uint32_t kDemoCrouchMs = 900;
constexpr std::uint32_t kDemoGroundPauseMs = 900;

std::uint8_t rotationForGround(GroundDirection ground)
{
    // The source sprite has its feet at the bottom edge.
    switch (ground) {
        case GroundDirection::Right:
            return 3;  // Counter-clockwise: feet move to the right edge.
        case GroundDirection::Up:
            return 2;
        case GroundDirection::Left:
            return 1;
        case GroundDirection::Down:
        default:
            return 0;
    }
}

int groundIndex(GroundDirection ground)
{
    return static_cast<int>(ground);
}

GroundDirection groundFromIndex(int index)
{
    index %= 4;
    if (index < 0) {
        index += 4;
    }
    return static_cast<GroundDirection>(index);
}

void tangentTowardNextCorner(GroundDirection ground, int direction, int& x, int& y)
{
    static constexpr int kClockwiseX[4] = {1, 0, -1, 0};
    static constexpr int kClockwiseY[4] = {0, -1, 0, 1};
    const int index = groundIndex(ground);
    x = kClockwiseX[index] * direction;
    y = kClockwiseY[index] * direction;
}

}  // namespace

BitmanEngine::BitmanEngine()
{
    reset();
}

void BitmanEngine::reset(std::uint32_t nowMs)
{
    mode_ = BitmanMode::Dance;
    motion_ = MotionState::Neutral;
    displayedGround_ = GroundDirection::Down;
    desiredGround_ = GroundDirection::Down;
    transitionTarget_ = GroundDirection::Down;
    renderGround_ = GroundDirection::Down;
    demoStartGround_ = GroundDirection::Down;
    clockwise_ = true;
    transitioning_ = false;
    transitionDirection_ = 1;
    offsetX_ = 0;
    offsetY_ = 0;
    pose_ = Pose::Blank;
    frame_ = sprite(Pose::Blank);
    modeStartedMs_ = nowMs;
    motionStartedMs_ = nowMs;
    walkingUntilMs_ = nowMs;
    transitionStartedMs_ = nowMs;
}

void BitmanEngine::setMode(BitmanMode mode, std::uint32_t nowMs)
{
    mode_ = mode;
    modeStartedMs_ = nowMs;
    motionStartedMs_ = nowMs;
    transitioning_ = false;
    offsetX_ = 0;
    offsetY_ = 0;
    if (mode == BitmanMode::Dance) {
        motion_ = MotionState::Neutral;
    }
}

void BitmanEngine::playDemo(std::uint32_t nowMs)
{
    demoStartGround_ = renderGround_;
    setMode(BitmanMode::Demo, nowMs);
}

bool BitmanEngine::setPose(Pose pose, GroundDirection ground, int offsetX, int offsetY)
{
    if (pose == pose_ && ground == renderGround_ && offsetX == offsetX_ && offsetY == offsetY_) {
        return false;
    }
    pose_ = pose;
    renderGround_ = ground;
    offsetX_ = offsetX;
    offsetY_ = offsetY;
    frame_ = sprite(pose).rotatedQuarterTurns(rotationForGround(ground)).translated(offsetX, offsetY);
    return true;
}

Pose BitmanEngine::dancePose(bool walking, bool clockwise, std::uint32_t phase) const
{
    (void)walking;
    (void)clockwise;
    const bool alternate = (phase & 1U) != 0;
    // Movement is a calm bend-and-shift, never a running pose.
    return alternate ? Pose::CrouchB : Pose::CrouchA;
}

void BitmanEngine::startNextTransition(std::uint32_t nowMs, bool clockwise)
{
    const int current = groundIndex(displayedGround_);
    const int desired = groundIndex(desiredGround_);
    const int delta = (desired - current + 4) % 4;
    if (delta == 0) {
        return;
    }

    if (delta == 1) {
        transitionDirection_ = 1;
    } else if (delta == 3) {
        transitionDirection_ = -1;
    } else {
        transitionDirection_ = clockwise ? 1 : -1;
    }
    clockwise_ = transitionDirection_ > 0;
    transitionTarget_ = groundFromIndex(current + transitionDirection_);
    transitionStartedMs_ = nowMs;
    motionStartedMs_ = nowMs;
    transitioning_ = true;
}

bool BitmanEngine::updateTransition(std::uint32_t nowMs)
{
    const std::uint32_t elapsed = nowMs - transitionStartedMs_;
    const std::uint32_t phase = elapsed / kTransitionFrameMs;
    if (phase >= kTransitionFrameCount) {
        displayedGround_ = transitionTarget_;
        transitioning_ = false;
        motionStartedMs_ = nowMs;
        walkingUntilMs_ = nowMs;
        return setPose(Pose::CrouchA, displayedGround_);
    }

    return renderTransitionFrame(phase, displayedGround_, transitionTarget_, transitionDirection_);
}

bool BitmanEngine::renderTransitionFrame(std::uint32_t phase, GroundDirection oldGround,
                                         GroundDirection newGround, int direction)
{
    int oldOffsetX = 0;
    int oldOffsetY = 0;
    int newOffsetX = 0;
    int newOffsetY = 0;
    tangentTowardNextCorner(oldGround, direction, oldOffsetX, oldOffsetY);
    tangentTowardNextCorner(newGround, -direction, newOffsetX, newOffsetY);
    switch (phase) {
        case 0:
            return setPose(Pose::CrouchA, oldGround);
        case 1:
            return setPose(Pose::CrouchB, oldGround, oldOffsetX, oldOffsetY);
        case 2:
            return setPose(Pose::CrouchA, oldGround, oldOffsetX, oldOffsetY);
        case 3:
            return setPose(Pose::CrouchA, newGround, newOffsetX, newOffsetY);
        case 4:
            return setPose(Pose::CrouchB, newGround, newOffsetX, newOffsetY);
        case 5:
        default:
            return setPose(Pose::CrouchA, newGround);
    }
}

bool BitmanEngine::updateDance(std::uint32_t nowMs, const MotionResult& motion)
{
    desiredGround_ = motion.ground;
    if (motion.state != motion_) {
        motion_ = motion.state;
        motionStartedMs_ = nowMs;
    }

    if (transitioning_) {
        return updateTransition(nowMs);
    }
    if (desiredGround_ != displayedGround_) {
        startNextTransition(nowMs, motion.clockwise);
        return updateTransition(nowMs);
    }

    if (motion.rotating) {
        walkingUntilMs_ = nowMs + kWalkHoldMs;
        clockwise_ = motion.clockwise;
    }
    const bool walking = static_cast<std::int32_t>(walkingUntilMs_ - nowMs) > 0;
    const std::uint32_t interval = walking ? kWalkIntervalMs : kCrouchIntervalMs;
    const std::uint32_t phase = (nowMs - motionStartedMs_) / interval;
    return setPose(dancePose(walking, clockwise_, phase), displayedGround_);
}

bool BitmanEngine::updateDemo(std::uint32_t nowMs)
{
    std::uint32_t elapsed = nowMs - modeStartedMs_;

    if (elapsed < kDemoBlankMs) {
        return setPose(Pose::Blank, demoStartGround_);
    }
    elapsed -= kDemoBlankMs;

    if (elapsed < kDemoCrouchMs) {
        const auto phase = elapsed / kCrouchIntervalMs;
        return setPose(phase == 0 ? Pose::CrouchA : Pose::CrouchB, demoStartGround_);
    }
    elapsed -= kDemoCrouchMs;

    for (int turn = 0; turn < 4; ++turn) {
        const auto oldGround = groundFromIndex(groundIndex(demoStartGround_) + turn);
        const auto newGround = groundFromIndex(groundIndex(demoStartGround_) + turn + 1);
        if (elapsed < kTransitionDurationMs) {
            return renderTransitionFrame(elapsed / kTransitionFrameMs, oldGround, newGround, 1);
        }
        elapsed -= kTransitionDurationMs;

        if (elapsed < kDemoGroundPauseMs) {
            const auto phase = elapsed / kCrouchIntervalMs;
            return setPose(phase == 0 ? Pose::CrouchA : Pose::CrouchB, newGround);
        }
        elapsed -= kDemoGroundPauseMs;
    }

    for (const auto& step : kDemoEnding) {
        if (elapsed < step.durationMs) {
            return setPose(step.pose, demoStartGround_);
        }
        elapsed -= step.durationMs;
    }

    displayedGround_ = demoStartGround_;
    desiredGround_ = demoStartGround_;
    transitioning_ = false;
    setMode(BitmanMode::Dance, nowMs);
    return setPose(Pose::CrouchA, displayedGround_);
}

bool BitmanEngine::update(std::uint32_t nowMs, const MotionResult& motion)
{
    return mode_ == BitmanMode::Demo ? updateDemo(nowMs) : updateDance(nowMs, motion);
}

}  // namespace bitman
