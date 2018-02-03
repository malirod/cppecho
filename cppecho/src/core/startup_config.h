// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <stdint.h>
#include <string>

namespace rms {
namespace core {

class StartupConfig {
 public:
  bool Parse(int argc, char** argv);

  bool GetIsShowHelp() const;

  bool GetIsShowVersion() const;

  const std::string& GetAddress() const;

  std::uint32_t GetPort() const;

  const std::string& GetHelp() const;

 private:
  bool is_show_help_ = false;

  bool is_show_version_ = false;

  std::string address_;

  std::uint32_t port_ = 0u;

  std::string help_;
};

}  // namespace core
}  // namespace rms
