#include <Arduino.h>

#include "bitman/bitman_core.hpp"
#include "bitman/sprites.hpp"
#include "controllers/atoms3r_controller.hpp"
#include "drivers/chain_mono_display.hpp"

namespace {

struct ChainConnectionProfile {
    const char* label;
    int rxPin;
    int txPin;
};

#if defined(BITMAN_CHAIN_RX_PIN) && defined(BITMAN_CHAIN_TX_PIN)
#ifdef BITMAN_CHAIN_ATOMIC_TOCHAIN
constexpr ChainConnectionProfile kChainProfiles[] = {{"TOCHAIN", BITMAN_CHAIN_RX_PIN,
                                                       BITMAN_CHAIN_TX_PIN}};
#elif defined(BITMAN_CHAIN_ATOMIC_MOTION_V12)
constexpr ChainConnectionProfile kChainProfiles[] = {{"PORT B", BITMAN_CHAIN_RX_PIN,
                                                       BITMAN_CHAIN_TX_PIN}};
#else
constexpr ChainConnectionProfile kChainProfiles[] = {{"DIRECT", BITMAN_CHAIN_RX_PIN,
                                                       BITMAN_CHAIN_TX_PIN}};
#endif
#else
constexpr ChainConnectionProfile kChainProfiles[] = {
    {"TOCHAIN", 6, 5},
    {"PORT B", 7, 8},
    {"DIRECT", 1, 2},
};
#endif

constexpr std::size_t kChainProfileCount = sizeof(kChainProfiles) / sizeof(kChainProfiles[0]);
constexpr std::uint32_t kReconnectIntervalMs = 2000;

bitman::AtomS3RController controller;
bitman::ChainMonoDisplay display;
bitman::BitmanCore core;
std::uint32_t lastReconnectMs = 0;
std::uint32_t demoStartedMs = 0;
int activeChainProfile = -1;
bitman::GroundDirection lastGround = bitman::GroundDirection::Down;

const char* groundName(bitman::GroundDirection ground)
{
    switch (ground) {
        case bitman::GroundDirection::Right:
            return "ground right";
        case bitman::GroundDirection::Up:
            return "ground up";
        case bitman::GroundDirection::Left:
            return "ground left";
        case bitman::GroundDirection::Down:
        default:
            return "ground down";
    }
}

void showChainConnection()
{
    if (activeChainProfile < 0) {
        return;
    }
    char text[24];
    snprintf(text, sizeof(text), "%u Mono %s", display.panelCount(),
             kChainProfiles[activeChainProfile].label);
    controller.showStatus("ATOM BITMAN", text);
    Serial.println(text);
}

bool tryChainProfile(std::size_t profileIndex)
{
    const auto& profile = kChainProfiles[profileIndex];
    Serial.printf("Trying Chain %s: UART2 RX=G%d TX=G%d\n", profile.label, profile.rxPin,
                  profile.txPin);
    if (!display.begin(Serial2, profile.rxPin, profile.txPin, display.brightness())) {
        return false;
    }
    activeChainProfile = static_cast<int>(profileIndex);
    return true;
}

bool connectChain()
{
    const int preferred = activeChainProfile;
    if (preferred >= 0 && tryChainProfile(static_cast<std::size_t>(preferred))) {
        return true;
    }
    for (std::size_t i = 0; i < kChainProfileCount; ++i) {
        if (static_cast<int>(i) != preferred && tryChainProfile(i)) {
            return true;
        }
    }
    activeChainProfile = -1;
    return false;
}

void handleEvent(bitman::ControlEvent event, std::uint32_t now)
{
    switch (event) {
        case bitman::ControlEvent::NextMode:
        case bitman::ControlEvent::PlayDemo:
            core.playDemo(now);
            demoStartedMs = now;
            controller.showStatus("ATOM BITMAN", "demo");
            break;
        case bitman::ControlEvent::CycleBrightness: {
            std::uint8_t level = static_cast<std::uint8_t>(display.brightness() + 1U);
            if (level > 7) {
                level = 1;
            }
            display.setBrightness(level);
            char text[16];
            snprintf(text, sizeof(text), "brightness %u", level);
            controller.showStatus("ATOM BITMAN", text);
            break;
        }
        case bitman::ControlEvent::None:
        default:
            break;
    }
}

}  // namespace

void setup()
{
    Serial.begin(115200);
    delay(200);
    controller.begin();
    core.reset(millis());
    controller.showStatus("ATOM BITMAN", "Mono scanning");

    if (connectChain()) {
        showChainConnection();
    } else {
        controller.showStatus("ATOM BITMAN", "Mono scanning");
        Serial.println("Chain Mono not found; auto-scanning in background");
    }
}

void loop()
{
    const std::uint32_t now = millis();
    controller.update();
    handleEvent(controller.pollEvent(), now);

    bitman::ImuSample sample;
    const bitman::ImuSample* samplePtr = controller.latestImuSample(sample) ? &sample : nullptr;
    const bool frameChanged = core.update(now, samplePtr);
    if (display.connected()) {
        const bool traveling = core.mode() == bitman::BitmanMode::Demo &&
                               core.pose() != bitman::Pose::Blank &&
                               core.pose() != bitman::Pose::BoxOuter &&
                               core.pose() != bitman::Pose::BoxMiddle &&
                               core.pose() != bitman::Pose::BoxInner;
        const bool shouldRefresh = frameChanged || traveling;
        const bool shown = !shouldRefresh ||
                           (traveling ? display.showTraveling(core.frame(), now - demoStartedMs)
                                      : display.show(core.frame()));
        if (!shown) {
            Serial.println("Chain Mono frame write failed");
        }
    }

    if (core.motion().ground != lastGround) {
        lastGround = core.motion().ground;
        controller.showStatus("ATOM BITMAN", groundName(lastGround));
        Serial.println(groundName(lastGround));
    }

    if (!display.connected() && now - lastReconnectMs >= kReconnectIntervalMs) {
        lastReconnectMs = now;
        if (connectChain()) {
            display.show(core.frame());
            showChainConnection();
            Serial.println("Chain Mono reconnected");
        }
    }

    delay(5);
}
