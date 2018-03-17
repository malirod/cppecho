// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <stdint.h>
#include <string>

namespace rms {
namespace core {

/**
 * Command line parameters parser.
 */
class StartupConfig {
 public:
  /**
   * Parse specified command line parameters.
   * @param argc Count command line of parameters.
   * @param argv Command line parameters.
   * @return True if parsed, False otherwise.
   */
  bool Parse(int argc, char** argv);

  /**
   * Get parsed "ShowHelp" parameter.
   * @return True if show help was requested. False otherwise.
   */
  bool GetIsShowHelp() const;

  /**
   * Get parsed "ShowVersion" parameter.
   * @return True if show version was requested. False otherwise.
   */
  bool GetIsShowVersion() const;

  /**
   * Get parsed "Server Address" parameter.
   * @return Server address string.
   */
  const std::string& GetAddress() const;

  /**
   * Get parsed "Server Port" parameter.
   * @return Server port.
   */
  std::uint32_t GetPort() const;

  /**
   * Get help string with description of command line parameters.
   * @return Help string.
   */
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
