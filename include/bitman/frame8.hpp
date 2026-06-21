#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace bitman {

// One byte per row. Bit 7 is the left-most pixel and bit 0 is the right-most.
class Frame8 {
public:
    using Rows = std::array<std::uint8_t, 8>;

    constexpr Frame8() : rows_{} {}
    constexpr explicit Frame8(const Rows& rows) : rows_(rows) {}

    constexpr const Rows& rows() const { return rows_; }
    constexpr std::uint8_t row(std::size_t y) const { return rows_[y]; }

    bool pixel(std::size_t x, std::size_t y) const;
    std::size_t litPixelCount() const;
    Frame8 mirrored() const;
    Frame8 rotatedQuarterTurns(std::uint8_t clockwiseQuarterTurns) const;
    Frame8 translated(int deltaX, int deltaY) const;
    std::string toAscii(char on = '#', char off = '.') const;

    bool operator==(const Frame8& other) const { return rows_ == other.rows_; }
    bool operator!=(const Frame8& other) const { return !(*this == other); }

private:
    Rows rows_;
};

}  // namespace bitman
