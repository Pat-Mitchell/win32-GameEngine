/// @file Component.cpp
/// @brief Out-of-line ComponentManager state and non-templated methods.
///
/// ComponentManager::getType<T>() and the typed add/get/remove helpers are
/// templates and live entirely in the header. What lands here is the single
/// translation unit's worth of state and behaviour that is not templated:
///   - the static type counter (starts at 0; first type queried gets id 0,
///     the next gets 1, and so on, up to MAX_COMPONENTS).
///   - entityDestroyed(), which must visit every component array regardless
///     of its element type.

#include "Component.h"

ComponentType ComponentManager::m_NextComponentType = 0;

void ComponentManager::entityDestroyed(EntityID entity) {
  // Notify every component array so each can drop this entity's component
  // (if it holds one). The IComponentArray interface lets us do this without
  // knowing the concrete element type of each array.
  for (auto const& pair : m_ComponentArrays) {
    pair.second->entityDestroyed(entity);
  }
}

void ComponentManager::reset() {
  // Releasing the shared_ptrs destroys every ComponentArray and all the
  // component data they hold. m_NextComponentType is intentionally left
  // alone: type ids are process-global and stay stable across resets.
  m_ComponentArrays.clear();
}
