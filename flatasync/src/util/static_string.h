// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <cstddef>
#include <string>

namespace rms {
namespace util {

/**
 * Class which allows to perform simple manipulations with strings at compile time.
 */
class StaticString {
 public:
  /**
   * Creates from const string literal.
   * @tparam N Size of the string. Calculated automatically.
   * @param data String itself.
   */
  template <std::size_t N>
  constexpr explicit StaticString(const char (&data)[N]) : data_(data), size_(N - 1) {
    static_assert(N > 0, "Invalid string literal! Length is zero!");
  }

  /**
   * Shallow copy of the string.
   * @param other Source string.
   * @param start_pos Offset.
   * @param number Number of chars to copy.
   */
  constexpr StaticString(const StaticString& other, const std::size_t start_pos, const std::size_t number)
      : data_(other.data_ + start_pos), size_(other.Size() - number) {}

  /**
   * Shallow copy of the string.
   * @param other Source string.
   * @param start_pos Offset.
   */
  constexpr StaticString(const StaticString& other, const std::size_t start_pos)
      : data_(other.data_ + start_pos), size_(other.Size() - start_pos) {}

  /**
   * Access single char.
   * @param n position.
   * @return Char at position.
   */
  constexpr char operator[](const std::size_t n) const {
    return n < size_ ? data_[n] : '\0';
  }

  /**
   * Get size of string.
   * @return Size of string.
   */
  constexpr std::size_t Size() const {
    return size_;
  }

  /**
   * Get pointer to char array which holds string.
   * @return Pointer to char array with string.
   */
  constexpr const char* Data() const {
    return data_;
  }

  /**
   * Compares strings.
   * @param other String to compare with.
   * @return True if strings are the same. False otherwise.
   */
  constexpr bool IsEqual(const StaticString& other) const {
    return (this == &other) || ((Size() == other.Size()) && (IsStringsEqual(Data(), other.Data(), Size(), 0u)));
  }

  /**
   * Gets copy of the string.
   * @return String copy.
   */
  std::string ToString() const {
    return std::string{data_, size_};
  }

 private:
  constexpr bool IsStringsEqual(const char* rhs,
                                const char* lhs,
                                const std::size_t size,
                                const std::size_t index) const {
    return (*lhs == *rhs) && ((index == size - 1) ? true : IsStringsEqual(lhs + 1, rhs + 1, size, index + 1));
  }

  const char* const data_;

  const std::size_t size_;
};

}  // namespace util
}  // namespace rms
