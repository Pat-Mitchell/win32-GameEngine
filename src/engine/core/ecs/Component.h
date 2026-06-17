/// @file Component.h

#pragma once

#include <cstdint>
#include <type_traits>

using ComponentType = uint32_t;
constexpr ComponentType MAX_COMPONENTS = 64;

class ComponentManager {
  public:
    template<typename T>
    static ComponentType getType() {
      static ComponentType type = m_NextComponentType++;
      return type;
    }
  
  private:
    static ComponentType m_NextComponentType;
};