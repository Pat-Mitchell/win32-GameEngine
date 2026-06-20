/// @file SystemManager.cpp

#include "System.h"
#include <algorithm>

void SystemManager::registerSystem(System* system) {
  if(system != nullptr) {
    m_Systems.push_back(system);
  }
}

void SystemManager::updateSystems(float deltaTime) {
  for(auto* system : m_Systems) {
    if(system != nullptr) {
      system->update(deltaTime);
    }
  }
}

void SystemManager::removeSystem(System* system) {
  auto it = std::find(m_Systems.begin(), m_Systems.end(), system);
  if(it != m_Systems.end()) {
    m_Systems.erase(it);
  }
}