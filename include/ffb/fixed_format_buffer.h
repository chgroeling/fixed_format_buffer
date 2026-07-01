#pragma once

#include <array>
#include <cstddef>
#include <cstdio>
#include <string_view>

namespace ffb {

/// Allocation-free fixed-capacity formatting buffer.
///
/// Usable on the stack, as a class member, or statically allocated.
/// Formatted string views are transient and valid only until the buffer
/// is modified, reused, or destroyed.
///
/// @tparam N Maximum number of characters (excluding null terminator).
template <std::size_t N>
class FixedFormatBuffer {
public:
    /// Maximum number of characters the buffer can hold (excluding null terminator).
    static constexpr std::size_t CAPACITY = N;

    FixedFormatBuffer() noexcept { buffer_[0] = '\0'; }

    /// Write a plain string into the buffer (no format expansion).
    /// Truncates silently if the result exceeds capacity.
    /// @return Number of characters written (excluding null terminator).
    std::size_t Format(const char* str) noexcept {
        return FormatImpl("%s", str);
    }

    /// Format a string into the buffer using printf-style format specifiers.
    /// Truncates silently if the result exceeds capacity.
    /// @return Number of characters written (excluding null terminator).
    template <typename Arg, typename... Args>
    std::size_t Format(const char* fmt, Arg&& arg, Args&&... args) noexcept {
        return FormatImpl(fmt, std::forward<Arg>(arg), std::forward<Args>(args)...);
    }

    /// Clear the buffer contents.
    void Clear() noexcept {
        buffer_[0] = '\0';
        size_ = 0;
    }

    /// Return a string_view over the current contents.
    /// Valid only until the buffer is modified or destroyed.
    [[nodiscard]] std::string_view View() const noexcept {
        return std::string_view{buffer_.data(), size_};
    }

    /// Return the number of characters currently stored.
    [[nodiscard]] std::size_t Size() const noexcept { return size_; }

    /// Return true if the buffer is empty.
    [[nodiscard]] bool Empty() const noexcept { return size_ == 0; }

private:
    template <typename... Args>
    std::size_t FormatImpl(const char* fmt, Args&&... args) noexcept {
        int written = std::snprintf(buffer_.data(), buffer_.size(), fmt, std::forward<Args>(args)...);
        if (written < 0) {
            buffer_[0] = '\0';
            size_ = 0;
        } else {
            size_ = static_cast<std::size_t>(written) < N ? static_cast<std::size_t>(written) : N;
        }
        return size_;
    }

    std::array<char, N + 1> buffer_{};
    std::size_t size_{0};
};

}  // namespace ffb
