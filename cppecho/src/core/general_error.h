// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <string>
#include <system_error>

namespace rms {
namespace core {

/**
 * Enum which describes general Engine errors.
 */
enum class GeneralError {
  Success,
  InternalError,
  WrongCommandLine,
  StartupFailed,
};

/**
 * Error category for general errors. Required be std::error_code facility.
 */
class ErrorCategory : public std::error_category {
 public:
  /**
   * Gets category name.
   * @return Name of the category.
   */
  const char* name() const noexcept override;

  /**
   * Convert error code to corresponding message string.
   * @param error_value Error code
   * @return Error massage
   */
  std::string message(int error_value) const override;

  /**
   * Allows to get access to single instance of this category.
   * @return This Category.
   */
  static const std::error_category& get();

 protected:
  ErrorCategory() = default;
};

std::error_condition make_error_condition(GeneralError error) noexcept;
std::error_code make_error_code(GeneralError error) noexcept;

}  // namespace core
}  // namespace rms

// Register for implicit conversion to error_condition
namespace std {
template <>
struct is_error_condition_enum<rms::core::GeneralError> : public true_type {};
}  // namespace std
