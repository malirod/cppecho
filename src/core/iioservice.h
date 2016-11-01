// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include "core/alias.h"

namespace cppecho {
namespace core {

class IIoService {
 public:
  virtual ~IIoService() = default;

  virtual AsioServiceType& GetAsioService() = 0;
};

}  // namespace core
}  // namespace cppecho
