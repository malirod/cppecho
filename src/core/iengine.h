// Copyright [2016] <Malinovsky Rodion>

#pragma once

namespace cppecho {
namespace core {

class IEngine {
 public:
  virtual ~IEngine() = default;

  virtual bool Launch() = 0;

  virtual bool Init() = 0;
};

}  // namespace core
}  // namespace cppecho
