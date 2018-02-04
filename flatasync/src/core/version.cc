// Copyright [2018] <Malinovsky Rodion>

#include "core/version.h"

#include <sstream>

std::string rms::core::version::GetVersion() {
  std::stringstream version_stream;
  version_stream << "Version: " << kMajor << "." << kMinor << "." << kPatch;
  return version_stream.str();
}
