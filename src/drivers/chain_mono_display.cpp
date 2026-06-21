#include "drivers/chain_mono_display.hpp"

#include <algorithm>

namespace bitman {

bool ChainMonoDisplay::operationSucceeded(chain_status_t status, std::uint8_t operationStatus) const
{
    return status == CHAIN_OK && operationStatus == 1;
}

bool ChainMonoDisplay::begin(HardwareSerial& serial, int rxPin, int txPin, std::uint8_t brightness)
{
    serial_ = &serial;
    rxPin_ = rxPin;
    txPin_ = txPin;
    brightness_ = std::min<std::uint8_t>(brightness, 7);
    chain_.begin(serial_, 115200, rxPin_, txPin_);
    return discover();
}

bool ChainMonoDisplay::discover()
{
    connected_ = false;
    monoId_ = 0;
    if (!chain_.isDeviceConnected(2, 120)) {
        return false;
    }

    std::uint16_t count = 0;
    if (chain_.getDeviceNum(&count, 120) != CHAIN_OK || count == 0 || count > 16) {
        return false;
    }

    device_info_t devices[16]{};
    device_list_t list{count, devices};
    if (!chain_.getDeviceList(&list, 120)) {
        return false;
    }
    for (std::uint16_t i = 0; i < count; ++i) {
        if (devices[i].device_type == CHAIN_MONO_TYPE_CODE) {
            monoId_ = static_cast<std::uint8_t>(devices[i].id);
            break;
        }
    }
    if (monoId_ == 0) {
        return false;
    }

    std::uint8_t operation = 0;
    auto status = chain_.setMonoMode(monoId_, MONO_PIXEL_MODE, &operation);
    if (!operationSucceeded(status, operation)) {
        return false;
    }
    operation = 0;
    status = chain_.setMonoRotation(monoId_, MONO_ROTATION_0, &operation);
    if (!operationSucceeded(status, operation)) {
        return false;
    }
    operation = 0;
    status = chain_.setMonoBrightness(
        monoId_, static_cast<mono_brightness_level_t>(brightness_), &operation);
    if (!operationSucceeded(status, operation)) {
        return false;
    }
    connected_ = true;
    return clear();
}

bool ChainMonoDisplay::reconnect()
{
    return serial_ != nullptr && discover();
}

bool ChainMonoDisplay::show(const Frame8& frame)
{
    if (!connected_) {
        return false;
    }
    std::uint8_t buffer[8]{};
    for (std::size_t i = 0; i < 8; ++i) {
        buffer[i] = frame.row(i);
    }
    std::uint8_t operation = 0;
    const auto status = chain_.setMonoBufferRefresh(monoId_, buffer, &operation);
    connected_ = operationSucceeded(status, operation);
    return connected_;
}

bool ChainMonoDisplay::clear()
{
    if (!connected_) {
        return false;
    }
    std::uint8_t operation = 0;
    const auto status = chain_.setMonoClear(monoId_, &operation);
    connected_ = operationSucceeded(status, operation);
    return connected_;
}

bool ChainMonoDisplay::setBrightness(std::uint8_t brightness)
{
    brightness_ = std::min<std::uint8_t>(brightness, 7);
    if (!connected_) {
        return false;
    }
    std::uint8_t operation = 0;
    const auto status = chain_.setMonoBrightness(
        monoId_, static_cast<mono_brightness_level_t>(brightness_), &operation);
    connected_ = operationSucceeded(status, operation);
    return connected_;
}

}  // namespace bitman
