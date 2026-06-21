#include "bitman/frame8.hpp"

#include <sstream>

namespace bitman {

bool Frame8::pixel(std::size_t x, std::size_t y) const
{
    if (x >= 8 || y >= 8) {
        return false;
    }
    return (rows_[y] & static_cast<std::uint8_t>(0x80U >> x)) != 0;
}

std::size_t Frame8::litPixelCount() const
{
    std::size_t count = 0;
    for (const auto rowValue : rows_) {
        std::uint8_t row = rowValue;
        while (row != 0) {
            count += row & 1U;
            row >>= 1U;
        }
    }
    return count;
}

Frame8 Frame8::mirrored() const
{
    Rows result{};
    for (std::size_t y = 0; y < rows_.size(); ++y) {
        std::uint8_t source = rows_[y];
        std::uint8_t reversed = 0;
        for (int bit = 0; bit < 8; ++bit) {
            reversed = static_cast<std::uint8_t>((reversed << 1U) | (source & 1U));
            source >>= 1U;
        }
        result[y] = reversed;
    }
    return Frame8(result);
}

Frame8 Frame8::rotatedQuarterTurns(std::uint8_t clockwiseQuarterTurns) const
{
    Frame8 result = *this;
    clockwiseQuarterTurns %= 4;
    for (std::uint8_t turn = 0; turn < clockwiseQuarterTurns; ++turn) {
        Rows rotated{};
        for (std::size_t y = 0; y < 8; ++y) {
            for (std::size_t x = 0; x < 8; ++x) {
                if (result.pixel(x, y)) {
                    const std::size_t rotatedX = 7 - y;
                    const std::size_t rotatedY = x;
                    rotated[rotatedY] |= static_cast<std::uint8_t>(0x80U >> rotatedX);
                }
            }
        }
        result = Frame8(rotated);
    }
    return result;
}

Frame8 Frame8::translated(int deltaX, int deltaY) const
{
    Rows translated{};
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            if (!pixel(static_cast<std::size_t>(x), static_cast<std::size_t>(y))) {
                continue;
            }
            const int movedX = x + deltaX;
            const int movedY = y + deltaY;
            if (movedX >= 0 && movedX < 8 && movedY >= 0 && movedY < 8) {
                translated[static_cast<std::size_t>(movedY)] |=
                    static_cast<std::uint8_t>(0x80U >> movedX);
            }
        }
    }
    return Frame8(translated);
}

std::string Frame8::toAscii(char on, char off) const
{
    std::ostringstream output;
    for (std::size_t y = 0; y < 8; ++y) {
        for (std::size_t x = 0; x < 8; ++x) {
            output << (pixel(x, y) ? on : off);
        }
        if (y != 7) {
            output << '\n';
        }
    }
    return output.str();
}

}  // namespace bitman
