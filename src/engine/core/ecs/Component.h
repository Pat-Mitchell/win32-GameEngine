/// @file Component.h
/// @brief Component type identification and per-type component storage.
///
/// Two responsibilities live here:
///   1. getType<T>() assigns a unique, process-global id to each component
///      type the first time it is seen.
///   2. ComponentArray<T> / ComponentManager store the actual component data,
///      one packed array per component type, keyed by EntityID.

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

#include "Entity.h"

using ComponentType = uint32_t;
constexpr ComponentType MAX_COMPONENTS = 64;

// Type-erased handle so ComponentManager can hold arrays of differing T
//    together and notify them when an entity is destroyed.
class IComponentArray {
  public:
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(EntityID entity) = 0;
};

// Packed storage for a single component type.
//
// Components live contiguously in m_Components. Two maps translate between an
//    entity and its slot in that dense array, so iteration stays cache-friendly
//    and removal is O(1) via swap-and-pop (the last element backfills the hole).
template<typename T>
class ComponentArray : public IComponentArray {
  public:
    // Add (or overwrite) the component attached to an entity.
    void insert(EntityID entity, const T& component) {
      auto it = m_EntityToIndex.find(entity);
      if(it != m_EntityToIndex.end()) {
        m_Components[it->second] = component; // already present: overwrite
        return;
      }

      size_t newIndex = m_Components.size();
      m_EntityToIndex[entity] = newIndex;
      m_IndexToEntity[newIndex] = entity;
      m_Components.push_back(component);
    }

    // Remove an entity's component, backfilling the slot with the last
    //    element so the array stays packed. No-op if the entity has no T.
    void remove(EntityID entity) {
      auto it = m_EntityToIndex.find(entity);
      if(it == m_EntityToIndex.end()) {
        return;
      }

      size_t removedIndex = it->second;
      size_t lastIndex = m_Components.size() - 1;
      EntityID lastEntity = m_IndexToEntity[lastIndex];

      // Move the last element into the removed slot.
      m_Components[removedIndex] = m_Components[lastIndex];
      m_EntityToIndex[lastEntity] = removedIndex;
      m_IndexToEntity[removedIndex] = lastEntity;

      // Drop the now-duplicated tail.
      m_EntityToIndex.erase(entity);
      m_IndexToEntity.erase(lastIndex);
      m_Components.pop_back();
    }

    // Access an entity's component. Throws std::out_of_range if absent.
    T& get(EntityID entity) {
      return m_Components[m_EntityToIndex.at(entity)];
    }

    bool has(EntityID entity) const {
      return m_EntityToIndex.find(entity) != m_EntityToIndex.end();
    }

    void entityDestroyed(EntityID entity) override {
      remove(entity); // remove() already no-ops when the entity has no T
    }

  private:
    std::vector<T> m_Components; // packed dense storage
    std::unordered_map<EntityID, size_t> m_EntityToIndex;
    std::unordered_map<size_t, EntityID> m_IndexToEntity;
};

// Owns one ComponentArray per component type and routes typed calls to it.
class ComponentManager {
  public:
    // Unique id for component type T, assigned on first use. Static, so the
    // id is shared across every ComponentManager instance.
    template<typename T>
    static ComponentType getType() {
      static ComponentType type = m_NextComponentType++;
      return type;
    }

    template<typename T>
    void addComponent(EntityID entity, const T& component) {
      getComponentArray<T>()->insert(entity, component);
    }

    template<typename T>
    void removeComponent(EntityID entity) {
      getComponentArray<T>()->remove(entity);
    }

    template<typename T>
    T& getComponent(EntityID entity) {
      return getComponentArray<T>()->get(entity);
    }

    template<typename T>
    bool hasComponent(EntityID entity) {
      auto it = m_ComponentArrays.find(getType<T>());
      if(it == m_ComponentArrays.end()) {
        return false; // no array for T means no entity has one
      }
      return std::static_pointer_cast<ComponentArray<T>>(it->second)->has(entity);
    }

    // Drop every component belonging to a destroyed entity. Defined in the
    // .cpp because it is not templated.
    void entityDestroyed(EntityID entity);

    // Drop all component arrays, returning the manager to its empty state.
    void reset();

  private:
    static ComponentType m_NextComponentType;
    std::unordered_map<ComponentType, std::shared_ptr<IComponentArray>> m_ComponentArrays;

    /// Fetch the array for T, creating it on first use.
    template<typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray() {
      ComponentType type = getType<T>();
      auto it = m_ComponentArrays.find(type);
      if (it == m_ComponentArrays.end()) {
        auto array = std::make_shared<ComponentArray<T>>();
        m_ComponentArrays[type] = array;
        return array;
      }
      return std::static_pointer_cast<ComponentArray<T>>(it->second);
    }
};
