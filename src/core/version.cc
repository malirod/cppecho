// Copyright [2016] <Malinovsky Rodion>

#include "core/version.h"

#include <iomanip>
#include <sstream>

std::string cppecho::core::version::GetVersion() {
  std::stringstream version_stream;
  version_stream << "CppEcho version: " << kMajor << "." << kMinor << "."
                 << kPatch;
  return version_stream.str();
}
