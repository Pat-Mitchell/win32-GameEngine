/// @file World.h

#pragma once

#include "Entity.h"
#include "Component.h"
#include "System.h"

#include <vector>

class World {
  public:
    World();
    EntityID createEntity();
    void destroyEntity(EntityID entity);
    void update(float fDeltaTime);

    std::vector<EntityID> getActiveEntities() const;

    // Defined in the header: these are templates, so their bodies must be
    //    visible to every translation unit that instantiates them.
    template<typename T>
    void addComponent(EntityID entity, const T& component) {
      m_ComponentManager.addComponent<T>(entity, component);
    }

    template<typename T>
    void removeComponent(EntityID entity) {
      m_ComponentManager.removeComponent<T>(entity);
    }

    template<typename T>
    T& getComponent(EntityID entity) {
      return m_ComponentManager.getComponent<T>(entity);
    }

    template<typename T>
    bool hasComponent(EntityID entity) {
      return m_ComponentManager.hasComponent<T>(entity);
    }

    void registerSystem(System* system);
    void removeSystem(System* system);
    bool isValid(EntityID entity) const;
    void reset();

  private:
    EntityManager m_EntityManager;
    ComponentManager m_ComponentManager;
    SystemManager m_SystemManager;
};