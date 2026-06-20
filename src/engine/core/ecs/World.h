/// @file World.h

#pragma once

#include "Entity.h"
#include "Component.h"
#include "System.h"

class World {
  public:
    World();
    EntityID createEntity();
    void destroyEntity(EntityID entity);
    void update(float fDeltaTime);

    template<typename T>
    void addComponent(EntityID entity, const T& component);

    template<typename T>
    void removeComponent(EntityID entity);

    template<typename T>
    T& getComponent(EntityID entity);

    template<typename T>
    bool hasComponent(EntityID entity);

    void registerSystem(System* system);
    void removeSystem(System* system);
    bool isValid(EntityID entity) const;
    void reset();

  private:
    EntityManager m_EntityManager;
    ComponentManager m_ComponentManager;
    SystemManager m_SystemManager;
};