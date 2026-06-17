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
    std::vector<System*> m_Systems;

  private:
    void registerSystem(System* system);
    void updateSystem(float fDeltaTime);
};