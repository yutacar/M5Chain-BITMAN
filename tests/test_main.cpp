#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

#include "bitman/bitman_core.hpp"
#include "bitman/frame8.hpp"
#include "bitman/motion_classifier.hpp"
#include "bitman/panel_composer.hpp"
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

void testPanelComposer()
{
    const bitman::Frame8 bar({0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    CHECK(bitman::composePanelFrame(bar, 0, 0).row(0) == 0xFF);
    CHECK(bitman::composePanelFrame(bar, 1, 0).row(0) == 0x00);
    CHECK(bitman::composePanelFrame(bar, 0, 4).row(0) == 0x0F);
    CHECK(bitman::composePanelFrame(bar, 1, 4).row(0) == 0xF0);
    CHECK(bitman::composePanelFrame(bar, 0, 8).row(0) == 0x00);
    CHECK(bitman::composePanelFrame(bar, 1, 8).row(0) == 0xFF);
    CHECK(bitman::composePanelFrame(bar, 0, -4).row(0) == 0xF0);

    CHECK(bitman::demoTravelOriginX(1, 7000) == 0);
    CHECK(bitman::demoTravelOriginX(3, 0) == 0);
    CHECK(bitman::demoTravelOriginX(3, 1000) == 0);
    CHECK(bitman::demoTravelOriginX(3, 1450) == 8);
    CHECK(bitman::demoTravelOriginX(3, 1900) == 16);
    CHECK(bitman::demoTravelOriginX(3, 3979) == 16);
    CHECK(bitman::demoTravelOriginX(3, 4880) == 16);
    CHECK(bitman::demoTravelOriginX(3, 6959) == 16);
    CHECK(bitman::demoTravelOriginX(3, 6960) == 16);
    CHECK(bitman::demoTravelOriginX(3, 7410) == 8);
    CHECK(bitman::demoTravelOriginX(3, 7860) == 0);
    CHECK(bitman::demoTravelOriginX(3, 10840) == 0);
    CHECK(bitman::demoTravelOriginX(3, 13820) == 0);
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
    CHECK(bitman::sprite(bitman::Pose::StraddleClockwiseA).mirrored() ==
          bitman::sprite(bitman::Pose::StraddleCounterClockwiseA));
    CHECK(bitman::sprite(bitman::Pose::StraddleClockwiseB).mirrored() ==
          bitman::sprite(bitman::Pose::StraddleCounterClockwiseB));
    CHECK(bitman::sprite(bitman::Pose::StraddleClockwiseA).row(7) != 0);
    CHECK((bitman::sprite(bitman::Pose::StraddleClockwiseA).row(5) & 0x01U) != 0);
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
    CHECK(engine.frame() == bitman::sprite(bitman::Pose::CrouchA).rotatedQuarterTurns(1));
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
    CHECK(engine.frame() ==
          bitman::sprite(bitman::Pose::CrouchB).rotatedQuarterTurns(1).translated(-1, 0));

    CHECK(engine.update(1120, motion));
    CHECK(engine.pose() == bitman::Pose::StraddleCounterClockwiseA);
    CHECK(engine.ground() == bitman::GroundDirection::Down);

    CHECK(engine.update(1380, motion));
    CHECK(engine.pose() == bitman::Pose::StraddleCounterClockwiseB);
    CHECK(engine.ground() == bitman::GroundDirection::Down);

    CHECK(engine.update(1640, motion));
    CHECK(engine.pose() == bitman::Pose::StraddleClockwiseB);
    CHECK(engine.ground() == bitman::GroundDirection::Right);
    CHECK(engine.frame() ==
          bitman::sprite(bitman::Pose::StraddleClockwiseB).rotatedQuarterTurns(2));
    CHECK(engine.update(1900, motion));
    CHECK(engine.pose() == bitman::Pose::StraddleClockwiseA);
    CHECK(engine.ground() == bitman::GroundDirection::Right);
    CHECK(engine.update(2160, motion));
    CHECK(engine.pose() == bitman::Pose::CrouchB);
    CHECK(engine.update(2420, motion));
    CHECK(engine.pose() == bitman::Pose::CrouchA);
    CHECK(!engine.update(2680, motion));

    engine.playDemo(3000);
    engine.update(3000, motion);
    CHECK(engine.mode() == bitman::BitmanMode::Demo);
    CHECK(engine.pose() == bitman::Pose::Blank);
    engine.update(4100, motion);
    CHECK(engine.pose() == bitman::Pose::CrouchA);
    engine.update(5680, motion);
    CHECK(engine.pose() == bitman::Pose::StraddleCounterClockwiseB);
    CHECK(engine.ground() == bitman::GroundDirection::Right);
    engine.update(16820, motion);
    CHECK(engine.pose() == bitman::Pose::BoxOuter);
    CHECK(engine.ground() == bitman::GroundDirection::Right);
    engine.update(22000, motion);
    CHECK(engine.mode() == bitman::BitmanMode::Dance);

    bitman::BitmanEngine idleEngine;
    idleEngine.reset(0);
    CHECK(idleEngine.update(0, bitman::MotionResult{}));
    idleEngine.update(7000, bitman::MotionResult{});
    idleEngine.update(7520, bitman::MotionResult{});
    CHECK(idleEngine.pose() != bitman::Pose::CrouchA);
    CHECK(idleEngine.pose() != bitman::Pose::CrouchB);

    bool sawWave = false;
    bool sawStep = false;
    bool sawJump = false;
    bool sawSurprise = false;
    bool sawHeadstand = false;
    idleEngine.reset(0);
    for (std::uint32_t now = 0; now <= 60000; now += 130) {
        idleEngine.update(now, bitman::MotionResult{});
        const auto pose = idleEngine.pose();
        sawWave = sawWave || pose == bitman::Pose::ShakeA || pose == bitman::Pose::ShakeB;
        sawStep = sawStep || pose == bitman::Pose::StepLeftA || pose == bitman::Pose::StepLeftB ||
                  pose == bitman::Pose::StepRightA || pose == bitman::Pose::StepRightB;
        sawJump = sawJump || pose == bitman::Pose::JumpA || pose == bitman::Pose::JumpB;
        sawSurprise = sawSurprise || pose == bitman::Pose::Surprise;
        sawHeadstand =
            sawHeadstand || pose == bitman::Pose::HeadstandA || pose == bitman::Pose::HeadstandB;
    }
    CHECK(static_cast<int>(sawWave) + static_cast<int>(sawStep) + static_cast<int>(sawJump) +
              static_cast<int>(sawSurprise) + static_cast<int>(sawHeadstand) >=
          4);
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
    for (std::uint32_t now = 520; now <= 3300; now += 20) {
        core.update(now, &sample);
    }
    CHECK(core.motion().ground == bitman::GroundDirection::Right);
    CHECK(core.pose() == bitman::Pose::CrouchA || core.pose() == bitman::Pose::CrouchB);
}

}  // namespace

int main()
{
    testFrame8();
    testPanelComposer();
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
