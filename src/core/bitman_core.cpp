#include "bitman/bitman_core.hpp"

namespace bitman {

BitmanCore::BitmanCore(const MotionConfig& config) : classifier_(config)
{
    reset();
}

void BitmanCore::reset(std::uint32_t nowMs)
{
    classifier_.reset();
    engine_.reset(nowMs);
    motion_ = MotionResult{};
    clockInitialized_ = false;
    previousUpdateMs_ = nowMs;
}

void BitmanCore::setMode(BitmanMode mode, std::uint32_t nowMs)
{
    engine_.setMode(mode, nowMs);
}

void BitmanCore::playDemo(std::uint32_t nowMs)
{
    engine_.playDemo(nowMs);
}

bool BitmanCore::update(std::uint32_t nowMs, const ImuSample* sample)
{
    float deltaSeconds = 0.02F;
    if (clockInitialized_) {
        deltaSeconds = static_cast<float>(nowMs - previousUpdateMs_) / 1000.0F;
    } else {
        clockInitialized_ = true;
    }
    previousUpdateMs_ = nowMs;

    if (sample != nullptr && sample->valid) {
        motion_ = classifier_.update(*sample, deltaSeconds);
    }
    return engine_.update(nowMs, motion_);
}

}  // namespace bitman
