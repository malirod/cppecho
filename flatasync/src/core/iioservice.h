// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include "core/alias.h"

namespace rms {
namespace core {

class IIoService {
 public:
  virtual ~IIoService() = default;

  virtual AsioServiceType& GetAsioService() = 0;
};

}  // namespace core
}  // namespace rms
