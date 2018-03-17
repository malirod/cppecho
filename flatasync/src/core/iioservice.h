// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include "core/alias.h"

namespace rms {
namespace core {

/**
 * Interface accessor to underlying asio io service.
 */
class IIoService {
 public:
  /**
   * Destructor of the io service.
   */
  virtual ~IIoService() = default;

  /**
   * Gets reference to asio io service.
   * @return Reference to asio io service.
   */
  virtual AsioServiceType& GetAsioService() = 0;
};

}  // namespace core
}  // namespace rms
