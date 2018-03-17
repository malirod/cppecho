// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <cassert>

namespace rms {
namespace util {

/**
 * Class which allows to create singletons for the same class or replace singleton implementation. This class makes
 * singleton pattern unit testable.
 * @tparam T Class which should be accessed as singleton.
 * @tparam Tag Allows to create different singletons for the same class. By default equals to the same class as T.
 */
template <typename T, typename Tag = T>
class SingleAccessor {
 public:
  /**
   * Assign singleton implementation.
   * @param single Object which should be accessed as singleton.
   */
  void Attach(T& single);

  /**
   * Unlink singleton.
   */
  void Detach();

  /**
   * Check whether some object is attached so getters can be used.
   * @return True is some object is attached. False otherwise.
   */
  bool GetIsAttached() const;

  /**
   * Get implicit reference to attached object.
   * @return Reference to attached object.
   */
  operator T&();  // NOLINT

  /**
   * Get explicit reference to attached object.
   * @return Reference to attached object.
   */
  T& GetRef();

 private:
  T* ptr_single_ = nullptr;
};

/**
 * Singleton implementation based on SingleAccessor.
 * @tparam T Class which should be accessed as singleton.
 * @tparam Tag Allows to create different singletons for the same class. By default equals to the same class as T.
 * @return Reference to singleton.
 */
template <typename T, typename Tag = T>
T& single() {
  static T singleton;
  return singleton;
}

template <typename T, typename Tag>
void SingleAccessor<T, Tag>::Attach(T& single) {
  ptr_single_ = &single;
}

template <typename T, typename Tag>
void SingleAccessor<T, Tag>::Detach() {
  ptr_single_ = nullptr;
}

template <typename T, typename Tag>
SingleAccessor<T, Tag>::operator T&() {
  return GetRef();
}

template <typename T, typename Tag>
T& SingleAccessor<T, Tag>::GetRef() {
  assert(ptr_single_ != nullptr && "Single is not attached");
  return *ptr_single_;
}

template <typename T, typename Tag>
bool SingleAccessor<T, Tag>::GetIsAttached() const {
  return ptr_single_ != nullptr;
}

}  // namespace util
}  // namespace rms
