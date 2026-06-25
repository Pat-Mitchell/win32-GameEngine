/// @file Entity.h

#pragma once

#include <cstdint>
#include <vector>

using EntityID = uint32_t;
constexpr EntityID INVALID_ENTITY = 0;

class EntityManager {
  public:
    EntityManager();
    EntityID createEntity();
    void destroyEntity(EntityID entity);
    bool isValid(EntityID entity) const;
    void reset();

    std::vector<bool> getEntityRegistry() const { return m_EntityRegistry; };

  private:
    std::vector<bool> m_EntityRegistry;
    std::vector<EntityID> m_FreeEntities;
    EntityID m_NextEntityID;
};