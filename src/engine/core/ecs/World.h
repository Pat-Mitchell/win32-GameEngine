/// @file World.h

#pragma once

#include "Entity.h"
#include "Component.h"
#include "System.h"

class World {
  public:
    EntityID createEntity();
    void destroyEntity(EntityID entity);
    void update(float fDeltaTime);

  private:
    EntityManager m_EntityManager;
    ComponentManager m_ComponentManager;
    SystemManager m_SystemManager;
};