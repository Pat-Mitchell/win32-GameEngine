/// @file System.h

#pragma once

#include "Entity.h"
#include "Component.h"

class System{
  public:
    virtual ~System() = default;
    virtual void update(float fDeltaTime) = 0;
};

class SystemManager {
  public:
    void registerSystem(System* system);
    void updateSystems(float fDeltaTime);
    void removeSystem(System* system);

  private: 
    std::vector<System*> m_Systems;

};