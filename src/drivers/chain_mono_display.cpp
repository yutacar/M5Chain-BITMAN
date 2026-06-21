#include "drivers/chain_mono_display.hpp"

#include <algorithm>

#include "bitman/panel_composer.hpp"

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
    monoCount_ = 0;
    monoIds_.fill(0);
    cacheValid_ = false;
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
        if (devices[i].device_type == CHAIN_MONO_TYPE_CODE && monoCount_ < kMaximumPanels) {
            monoIds_[monoCount_++] = static_cast<std::uint8_t>(devices[i].id);
        }
    }
    if (monoCount_ == 0) {
        return false;
    }

    for (std::size_t i = 0; i < monoCount_; ++i) {
        if (!configurePanel(monoIds_[i])) {
            monoCount_ = 0;
            return false;
        }
    }
    connected_ = true;
    return clear();
}

bool ChainMonoDisplay::configurePanel(std::uint8_t id)
{
    std::uint8_t operation = 0;
    auto status = chain_.setMonoMode(id, MONO_PIXEL_MODE, &operation);
    if (!operationSucceeded(status, operation)) {
        return false;
    }
    operation = 0;
    status = chain_.setMonoRotation(id, MONO_ROTATION_0, &operation);
    if (!operationSucceeded(status, operation)) {
        return false;
    }
    operation = 0;
    status = chain_.setMonoBrightness(
        id, static_cast<mono_brightness_level_t>(brightness_), &operation);
    return operationSucceeded(status, operation);
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
    if (cacheValid_ && !cacheTraveling_ && sameFrame(cachedFrame_, frame)) {
        return true;
    }
    for (std::size_t i = 0; i < monoCount_; ++i) {
        if (!writePanel(i, frame)) {
            connected_ = false;
            return false;
        }
    }
    cachedFrame_ = frame;
    cacheValid_ = true;
    cacheTraveling_ = false;
    cachedTravelX_ = -1;
    return true;
}

bool ChainMonoDisplay::showTraveling(const Frame8& frame, std::uint32_t elapsedMs)
{
    if (!connected_) {
        return false;
    }
    if (monoCount_ <= 1) {
        return show(frame);
    }

    const int travelX = demoTravelOriginX(monoCount_, elapsedMs);
    if (cacheValid_ && cacheTraveling_ && cachedTravelX_ == travelX &&
        sameFrame(cachedFrame_, frame)) {
        return true;
    }

    for (std::size_t i = 0; i < monoCount_; ++i) {
        const Frame8 panelFrame = composePanelFrame(frame, i, travelX);
        if (!writePanel(i, panelFrame)) {
            connected_ = false;
            return false;
        }
    }
    cachedFrame_ = frame;
    cacheValid_ = true;
    cacheTraveling_ = true;
    cachedTravelX_ = travelX;
    return true;
}

bool ChainMonoDisplay::writePanel(std::size_t panelIndex, const Frame8& frame)
{
    if (panelIndex >= monoCount_) {
        return false;
    }
    std::uint8_t buffer[8]{};
    for (std::size_t i = 0; i < 8; ++i) {
        buffer[i] = frame.row(i);
    }
    std::uint8_t operation = 0;
    const auto status = chain_.setMonoBufferRefresh(monoIds_[panelIndex], buffer, &operation);
    return operationSucceeded(status, operation);
}

bool ChainMonoDisplay::clear()
{
    if (!connected_) {
        return false;
    }
    for (std::size_t i = 0; i < monoCount_; ++i) {
        std::uint8_t operation = 0;
        const auto status = chain_.setMonoClear(monoIds_[i], &operation);
        if (!operationSucceeded(status, operation)) {
            connected_ = false;
            return false;
        }
    }
    cacheValid_ = false;
    return true;
}

bool ChainMonoDisplay::setBrightness(std::uint8_t brightness)
{
    brightness_ = std::min<std::uint8_t>(brightness, 7);
    if (!connected_) {
        return false;
    }
    for (std::size_t i = 0; i < monoCount_; ++i) {
        std::uint8_t operation = 0;
        const auto status = chain_.setMonoBrightness(
            monoIds_[i], static_cast<mono_brightness_level_t>(brightness_), &operation);
        if (!operationSucceeded(status, operation)) {
            connected_ = false;
            return false;
        }
    }
    return true;
}

}  // namespace bitman
