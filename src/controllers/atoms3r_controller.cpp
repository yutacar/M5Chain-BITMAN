#include "controllers/atoms3r_controller.hpp"

namespace bitman {

bool AtomS3RController::begin()
{
    auto config = M5.config();
    M5.begin(config);
    M5.Display.setRotation(0);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextSize(1);
    showStatus("ATOM BITMAN", "starting");
    return true;
}

void AtomS3RController::update()
{
    M5.update();

    if (M5.Imu.update()) {
        const auto data = M5.Imu.getImuData();
        latest_.ax = data.accel.x;
        latest_.ay = data.accel.y;
        latest_.az = data.accel.z;
        latest_.gx = data.gyro.x;
        latest_.gy = data.gyro.y;
        latest_.gz = data.gyro.z;
        latest_.valid = true;
    }

    if (pendingEvent_ == ControlEvent::None) {
        if (M5.BtnA.wasDoubleClicked()) {
            pendingEvent_ = ControlEvent::PlayDemo;
        } else if (M5.BtnA.wasHold()) {
            pendingEvent_ = ControlEvent::CycleBrightness;
        } else if (M5.BtnA.wasSingleClicked()) {
            pendingEvent_ = ControlEvent::NextMode;
        }
    }
}

ControllerCapabilities AtomS3RController::capabilities() const
{
    ControllerCapabilities result;
    result.imu = true;
    result.screen = true;
    return result;
}

bool AtomS3RController::latestImuSample(ImuSample& sample) const
{
    sample = latest_;
    return sample.valid;
}

ControlEvent AtomS3RController::pollEvent()
{
    const auto event = pendingEvent_;
    pendingEvent_ = ControlEvent::None;
    return event;
}

void AtomS3RController::showStatus(const char* title, const char* detail)
{
    M5.Display.clear(TFT_BLACK);
    M5.Display.drawString(title, 64, 48);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.drawString(detail, 64, 78);
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
}

}  // namespace bitman

