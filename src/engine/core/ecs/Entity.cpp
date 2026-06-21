/// @file Entity.cpp
/// @brief Provides a basic entity management system with:
///   - Entity ID generation and recycling
///   - Validation of entity IDs
///   - Efficient storage using a registry
///   - Memory reservation for performance
///   - Proper cleanup and reset functionality

#include "Entity.h"
#include <stdexcept>

EntityManager::EntityManager() : m_NextEntityID(1) {
  m_EntityRegistry.reserve(1000); // Reserve space for 1000 entities
}

EntityID EntityManager::createEntity() {
  EntityID id;
  if(!m_FreeEntities.empty()) {
    // Reuse a freed entity ID. Fall through so it gets marked active below.
    id = m_FreeEntities.back();
    m_FreeEntities.pop_back();
  } else {
    // Create a new entity ID
    id = m_NextEntityID++;
    if(id >= m_EntityRegistry.size()) {
      m_EntityRegistry.resize(id + 1, false); // ensure index "id" exists
    }
  }
  m_EntityRegistry[id] = true;
  return id;
}

void EntityManager::destroyEntity(EntityID entity) {
  if(entity == INVALID_ENTITY || entity >= m_EntityRegistry.size() || !m_EntityRegistry[entity]) {
    return; // Invalid or already destroyed entity
  }

  // Mark entity as inactive
  m_EntityRegistry[entity] = false;
  m_FreeEntities.push_back(entity);
}

bool EntityManager::isValid(EntityID entity) const {
  return entity != INVALID_ENTITY &&
         entity < m_EntityRegistry.size() &&
         m_EntityRegistry[entity];
}

void EntityManager::reset() {
  m_EntityRegistry.clear();
  m_FreeEntities.clear();
  m_NextEntityID = 1;
}