#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include "ffb/buffer_policy.h"

namespace ffb {

/// Allocation-free fixed-capacity formatting buffer.
///
/// Usable on the stack, as a class member, or statically allocated.
/// Formatted string views are transient and valid only until the buffer
/// is modified, reused, or destroyed.
///
/// @tparam N      Maximum number of characters (excluding null terminator).
/// @tparam Policy Feature-flag policy struct (see buffer_policy.h).
///                Defaults to AllFeatures (all features enabled).
template <std::size_t N, typename Policy = AllFeatures>
class FixedFormatBuffer {
public:
    /// Maximum number of characters the buffer can hold (excluding null terminator).
    static constexpr std::size_t CAPACITY = N;

    FixedFormatBuffer() noexcept { buffer_[0] = '\0'; }

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
    std::array<char, N + 1> buffer_{};
    std::size_t size_{0};
};

}  // namespace ffb
