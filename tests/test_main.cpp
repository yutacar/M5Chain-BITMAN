#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

#include "bitman/bitman_core.hpp"
#include "bitman/frame8.hpp"
#include "bitman/motion_classifier.hpp"
#include "bitman/sprites.hpp"

namespace {

int failures = 0;

#define CHECK(condition)                                                                            \
    do {                                                                                            \
        if (!(condition)) {                                                                         \
            std::cerr << __FILE__ << ':' << __LINE__ << " CHECK failed: " #condition << '\n';       \
            ++failures;                                                                             \
        }                                                                                           \
    } while (false)

bitman::ImuSample imu(float ax, float ay, float az, float gx = 0, float gy = 0, float gz = 0)
{
    bitman::ImuSample sample;
    sample.ax = ax;
    sample.ay = ay;
    sample.az = az;
    sample.gx = gx;
    sample.gy = gy;
    sample.gz = gz;
    sample.valid = true;
    return sample;
}

bitman::MotionResult settle(bitman::MotionClassifier& classifier, const bitman::ImuSample& sample,
                            int iterations = 40)
{
    bitman::MotionResult result;
    for (int i = 0; i < iterations; ++i) {
        result = classifier.update(sample, 0.02F);
    }
    return result;
}

void testFrame8()
{
    const bitman::Frame8 frame({0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    CHECK(frame.pixel(0, 0));
    CHECK(frame.pixel(7, 1));
    CHECK(!frame.pixel(7, 0));
    CHECK(!frame.pixel(8, 0));
    CHECK(frame.litPixelCount() == 2);
    CHECK(frame.mirrored().row(0) == 0x01);
    CHECK(frame.mirrored().row(1) == 0x80);
    CHECK(frame.rotatedQuarterTurns(1).row(0) == 0x01);
    CHECK(frame.rotatedQuarterTurns(1).row(7) == 0x02);
    CHECK(frame.rotatedQuarterTurns(4) == frame);
    CHECK(frame.translated(1, 0).row(0) == 0x40);
    CHECK(frame.translated(1, 0).row(1) == 0x00);
    CHECK(frame.toAscii().substr(0, 8) == "#.......");
}

void testSpriteCatalog()
{
    const bitman::Frame8 originalBitman(
        {0x18, 0x18, 0x3C, 0x5A, 0x5A, 0x24, 0x24, 0x66});
    CHECK(bitman::sprite(bitman::Pose::Blank).litPixelCount() == 0);
    CHECK(bitman::sprite(bitman::Pose::IdleA) == originalBitman);
    CHECK(bitman::sprite(bitman::Pose::CrouchA) == originalBitman);
    CHECK(bitman::sprite(bitman::Pose::IdleA) != bitman::sprite(bitman::Pose::IdleB));
    CHECK(bitman::sprite(bitman::Pose::StepLeftA).mirrored() ==
          bitman::sprite(bitman::Pose::StepRightA));
    CHECK(std::string(bitman::poseName(bitman::Pose::Surprise)) == "surprise");
}

void testGroundDirections()
{
    bitman::MotionClassifier classifier;
    CHECK(settle(classifier, imu(0, 1, 0)).ground == bitman::GroundDirection::Down);

    classifier.reset();
    CHECK(settle(classifier, imu(1, 0, 0)).ground == bitman::GroundDirection::Right);

    classifier.reset();
    CHECK(settle(classifier, imu(0, -1, 0)).ground == bitman::GroundDirection::Up);

    classifier.reset();
    CHECK(settle(classifier, imu(-1, 0, 0)).ground == bitman::GroundDirection::Left);
}

void testGroundHysteresisAndRotation()
{
    bitman::MotionClassifier classifier;
    CHECK(settle(classifier, imu(0, 1, 0)).ground == bitman::GroundDirection::Down);

    constexpr float kSin50 = 0.7660444F;
    constexpr float kCos50 = 0.6427876F;
    CHECK(settle(classifier, imu(kSin50, kCos50, 0)).ground == bitman::GroundDirection::Down);

    constexpr float kSin70 = 0.9396926F;
    constexpr float kCos70 = 0.3420201F;
    CHECK(settle(classifier, imu(kSin70, kCos70, 0)).ground == bitman::GroundDirection::Right);

    classifier.reset();
    settle(classifier, imu(0, 1, 0), 10);
    const auto rotating = classifier.update(imu(0.2F, 0.98F, 0, 0, 0, 60), 0.02F);
    CHECK(rotating.rotating);
    CHECK(rotating.clockwise);
}

void testAnimationEngine()
{
    bitman::BitmanEngine engine;
    engine.reset(0);
    bitman::MotionResult motion;
    motion.ground = bitman::GroundDirection::Down;
    CHECK(engine.update(0, motion));
    CHECK(engine.pose() == bitman::Pose::CrouchA);
    CHECK(!engine.update(100, motion));
    CHECK(engine.update(500, motion));
    CHECK(engine.pose() == bitman::Pose::CrouchB);

    motion.ground = bitman::GroundDirection::Right;
    motion.rotating = true;
    motion.clockwise = true;
    CHECK(engine.update(600, motion));
    CHECK(engine.pose() == bitman::Pose::CrouchA);
    CHECK(engine.ground() == bitman::GroundDirection::Down);

    motion.rotating = false;
    CHECK(engine.update(860, motion));
    CHECK(engine.pose() == bitman::Pose::CrouchB);
    CHECK(engine.ground() == bitman::GroundDirection::Down);
    CHECK(engine.frame() == bitman::sprite(bitman::Pose::CrouchB).translated(1, 0));

    CHECK(engine.update(1120, motion));
    CHECK(engine.pose() == bitman::Pose::CrouchA);
    CHECK(engine.ground() == bitman::GroundDirection::Down);

    CHECK(engine.update(1380, motion));
    CHECK(engine.pose() == bitman::Pose::CrouchA);
    CHECK(engine.ground() == bitman::GroundDirection::Right);
    CHECK(engine.frame() ==
          bitman::sprite(bitman::Pose::CrouchA).rotatedQuarterTurns(3).translated(0, 1));

    CHECK(engine.update(1640, motion));
    CHECK(engine.pose() == bitman::Pose::CrouchB);
    CHECK(engine.update(1900, motion));
    CHECK(engine.pose() == bitman::Pose::CrouchA);
    CHECK(engine.ground() == bitman::GroundDirection::Right);
    CHECK(!engine.update(2160, motion));

    engine.playDemo(3000);
    engine.update(3000, motion);
    CHECK(engine.mode() == bitman::BitmanMode::Demo);
    CHECK(engine.pose() == bitman::Pose::Blank);
    engine.update(4100, motion);
    CHECK(engine.pose() == bitman::Pose::CrouchA);
    engine.update(5680, motion);
    CHECK(engine.pose() == bitman::Pose::CrouchA);
    CHECK(engine.ground() == bitman::GroundDirection::Up);
    engine.update(14740, motion);
    CHECK(engine.pose() == bitman::Pose::BoxOuter);
    CHECK(engine.ground() == bitman::GroundDirection::Right);
    engine.update(22000, motion);
    CHECK(engine.mode() == bitman::BitmanMode::Dance);
}

void testCorePipeline()
{
    bitman::BitmanCore core;
    core.reset(0);
    auto sample = imu(0, 1, 0);
    CHECK(core.update(0, &sample));
    CHECK(core.motion().ground == bitman::GroundDirection::Down);

    sample = imu(1, 0, 0, 0, 0, 60);
    for (std::uint32_t now = 20; now <= 500; now += 20) {
        core.update(now, &sample);
    }
    sample = imu(1, 0, 0);
    for (std::uint32_t now = 520; now <= 2500; now += 20) {
        core.update(now, &sample);
    }
    CHECK(core.motion().ground == bitman::GroundDirection::Right);
    CHECK(core.pose() == bitman::Pose::CrouchA || core.pose() == bitman::Pose::CrouchB);
}

}  // namespace

int main()
{
    testFrame8();
    testSpriteCatalog();
    testGroundDirections();
    testGroundHysteresisAndRotation();
    testAnimationEngine();
    testCorePipeline();

    if (failures != 0) {
        std::cerr << failures << " test assertion(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "All BITMAN Core tests passed\n";
    return EXIT_SUCCESS;
}
