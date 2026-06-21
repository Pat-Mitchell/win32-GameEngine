/// @file World.cpp

#include "World.h"
#include <algorithm>

World::World() {

}

EntityID World::createEntity() {
  return m_EntityManager.createEntity();
}

void World::destroyEntity(EntityID entity) {
  if(m_EntityManager.isValid(entity)) {
    m_ComponentManager.entityDestroyed(entity); // Notify component manager that this entity is being destroyed
    m_EntityManager.destroyEntity(entity); // Destroy the entity
  }
}

void World::update(float fDeltaTime) {
  // Update all systems in the system manager
  m_SystemManager.updateSystems(fDeltaTime);
}

// Component management methods are templates defined inline in World.h.

//System argument systems

void World::registerSystem(System* system) {
  m_SystemManager.registerSystem(system);
}

void World::removeSystem(System* system) {
  m_SystemManager.removeSystem(system);
}

// Utility methods

bool World::isValid(EntityID entity) const {
  return m_EntityManager.isValid(entity);
}

void World::reset() {
  m_EntityManager.reset();
  m_SystemManager = SystemManager();
}