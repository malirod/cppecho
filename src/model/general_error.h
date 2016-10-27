// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <string>
#include <system_error>

namespace cppecho {
namespace model {

enum class GeneralError {
  Success,
  InternalError,
  WrongCommandLine,
  StartupFailed,
};

class ErrorCategory : public std::error_category {
 public:
  const char* name() const noexcept override;

  std::string message(int errorValue) const override;

  static const std::error_category& get();

 protected:
  ErrorCategory() = default;
};

std::error_condition make_error_condition(GeneralError error) noexcept;
std::error_code make_error_code(GeneralError error) noexcept;

}  // namespace model
}  // namespace cppecho

// Register for implicit conversion to error_condition
namespace std {
template <>
struct is_error_condition_enum<cppecho::model::GeneralError>
    : public true_type {};
}  // namespace std
