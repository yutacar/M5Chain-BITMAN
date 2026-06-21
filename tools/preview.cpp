#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "bitman/bitman_core.hpp"
#include "bitman/sprites.hpp"

namespace {

bitman::ImuSample sample(float ax, float ay, float az, float gx = 0)
{
    bitman::ImuSample result;
    result.ax = ax;
    result.ay = ay;
    result.az = az;
    result.gx = gx;
    result.valid = true;
    return result;
}

struct Segment {
    const char* name;
    bitman::ImuSample imu;
    std::uint32_t durationMs;
};

const char* groundName(bitman::GroundDirection ground)
{
    switch (ground) {
        case bitman::GroundDirection::Right:
            return "right";
        case bitman::GroundDirection::Up:
            return "up";
        case bitman::GroundDirection::Left:
            return "left";
        case bitman::GroundDirection::Down:
        default:
            return "down";
    }
}

}  // namespace

int main()
{
    const std::vector<Segment> script = {
        {"ground down / crouch", sample(0, 1, 0), 1200},
        {"rotate clockwise", sample(0.7F, 0.7F, 0), 500},
        {"ground right", sample(1, 0, 0), 1200},
        {"ground up", sample(0, -1, 0), 1200},
        {"ground left", sample(-1, 0, 0), 1200},
        {"back to ground down", sample(0, 1, 0), 1200},
    };

    bitman::BitmanCore core;
    core.reset(0);
    std::uint32_t now = 0;
    bitman::Pose previous = bitman::Pose::Blank;
    bitman::Frame8 previousFrame;
    bool hasPreviousFrame = false;

    for (const auto& segment : script) {
        std::cout << "\n=== " << segment.name << " ===\n";
        const std::uint32_t end = now + segment.durationMs;
        while (now < end) {
            core.update(now, &segment.imu);
            if (!hasPreviousFrame || core.frame() != previousFrame) {
                previous = core.pose();
                previousFrame = core.frame();
                hasPreviousFrame = true;
                std::cout << '[' << bitman::poseName(previous) << ", ground="
                          << groundName(core.motion().ground) << "]\n"
                          << core.frame().toAscii("##"[0], ' ') << "\n\n";
            }
            now += 20;
        }
    }

    std::cout << "\n=== demo: full turn / crouch / box ending ===\n";
    core.playDemo(now);
    const std::uint32_t demoEnd = now + 19000;
    while (now < demoEnd) {
        core.update(now, nullptr);
        if (!hasPreviousFrame || core.frame() != previousFrame) {
            previous = core.pose();
            previousFrame = core.frame();
            hasPreviousFrame = true;
            std::cout << '[' << bitman::poseName(previous) << "]\n"
                      << core.frame().toAscii("##"[0], ' ') << "\n\n";
        }
        now += 20;
    }
    return 0;
}
