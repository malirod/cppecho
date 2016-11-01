// Copyright [2016] <Malinovsky Rodion>

#include "core/startup_config.h"
#include <iostream>
#include "boost/program_options.hpp"

bool cppecho::core::StartupConfig::Parse(int argc, char** argv) {
  namespace po = boost::program_options;

  is_show_help_ = false;
  is_show_version_ = false;
  address_ = "";
  port_ = 0u;

  help_.clear();
  po::options_description desc("Options");

  try {
    desc.add_options()("help,h", "Print help")("version,v", "Print version")(
        "address,a", po::value<std::string>(), "Set listen address")(
        "port,p", po::value<std::uint32_t>(), "Set listen port");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    std::stringstream desc_sstream;
    desc_sstream << desc << std::endl;
    help_ = desc_sstream.str();

    if (vm.count("help")) {
      is_show_help_ = true;
    }

    if (vm.count("version")) {
      is_show_version_ = true;
    }

    if (vm.count("address")) {
      auto opt_value = vm["address"].as<std::string>();
      if (opt_value.empty()) {
        std::cerr << "Address must not be empty" << std::endl;
        return false;
      }
      address_ = opt_value;
    }

    if (vm.count("port")) {
      port_ = vm["port"].as<std::uint32_t>();
    }
  } catch (std::exception const& e) {
    std::cerr << "Failed to parse command line options: " << e.what()
              << std::endl;
    std::cerr << "Pass --help to get more information" << std::endl;
    return false;
  }
  return true;
}

bool cppecho::core::StartupConfig::GetIsShowHelp() const {
  return is_show_help_;
}

bool cppecho::core::StartupConfig::GetIsShowVersion() const {
  return is_show_version_;
}

const std::string& cppecho::core::StartupConfig::GetAddress() const {
  return address_;
}

std::uint32_t cppecho::core::StartupConfig::GetPort() const {
  return port_;
}

const std::string& cppecho::core::StartupConfig::GetHelp() const {
  return help_;
}
