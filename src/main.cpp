#include <Arduino.h>

#include "bitman/bitman_core.hpp"
#include "bitman/sprites.hpp"
#include "controllers/atoms3r_controller.hpp"
#include "drivers/chain_mono_display.hpp"

namespace {

constexpr int kChainRxPin = 1;
constexpr int kChainTxPin = 2;
constexpr std::uint32_t kReconnectIntervalMs = 2000;

bitman::AtomS3RController controller;
bitman::ChainMonoDisplay display;
bitman::BitmanCore core;
std::uint32_t lastReconnectMs = 0;
bitman::GroundDirection lastGround = bitman::GroundDirection::Down;

const char* modeName(bitman::BitmanMode mode)
{
    return mode == bitman::BitmanMode::Demo ? "demo" : "dance";
}

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

void handleEvent(bitman::ControlEvent event, std::uint32_t now)
{
    switch (event) {
        case bitman::ControlEvent::NextMode:
        case bitman::ControlEvent::PlayDemo:
            core.playDemo(now);
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

    if (display.begin(Serial2, kChainRxPin, kChainTxPin, 4)) {
        controller.showStatus("ATOM BITMAN", "Chain Mono ready");
        Serial.println("Chain Mono ready");
    } else {
        controller.showStatus("ATOM BITMAN", "Mono not found");
        Serial.println("Chain Mono not found; reconnecting in background");
    }
}

void loop()
{
    const std::uint32_t now = millis();
    controller.update();
    handleEvent(controller.pollEvent(), now);

    bitman::ImuSample sample;
    const bitman::ImuSample* samplePtr = controller.latestImuSample(sample) ? &sample : nullptr;
    if (core.update(now, samplePtr) && display.connected()) {
        if (!display.show(core.frame())) {
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
        if (display.reconnect()) {
            display.show(core.frame());
            controller.showStatus("ATOM BITMAN", modeName(core.mode()));
            Serial.println("Chain Mono reconnected");
        }
    }

    delay(5);
}
