/// @file ecs_smoke_test.cpp
/// @brief Console smoke test for the ECS core (Entity / Component / System / World).
///
/// No OpenGL or Win32 — pure console. Exercises the public World API end to end
/// and exits non-zero if any check fails, so it can double as a CI gate.

#include <iostream>

#include "../../src/engine/core/ecs/World.h"

// ────── ⋆⋅☆⋅⋆ ────────
//  Tiny CHECK harness
// ────── ⋆⋅☆⋅⋆ ────────

static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond)                                                         \
  do {                                                                      \
    ++g_checks;                                                             \
    if(!(cond)) {                                                          \
      ++g_failures;                                                         \
      std::cout << "  [FAIL] " << #cond << "  (line " << __LINE__ << ")\n"; \
    }                                                                       \
  } while(0)

static void section(const char* name) {
  std::cout << "[ " << name << " ]\n";
}

// ────── ⋆⋅☆⋅⋆ ────────
//    Test fixtures 
// ────── ⋆⋅☆⋅⋆ ────────

struct Position { float x, y, z; };
struct Velocity { float dx, dy, dz; };

// A system that integrates one entity's Position by its Velocity each update.
// Systems have no built-in query, so for the smoke test it operates on a single
// entity handed to it at construction.
class MovementSystem : public System {
  public:
    MovementSystem(World& world, EntityID entity) : m_World(world), m_Entity(entity) {}

    int updateCount = 0;
    float lastDelta = 0.0f;

    void update(float fDeltaTime) override {
      ++updateCount;
      lastDelta = fDeltaTime;
      if(m_World.hasComponent<Position>(m_Entity) && m_World.hasComponent<Velocity>(m_Entity)) {
        Position& p = m_World.getComponent<Position>(m_Entity);
        const Velocity& v = m_World.getComponent<Velocity>(m_Entity);
        p.x += v.dx * fDeltaTime;
        p.y += v.dy * fDeltaTime;
        p.z += v.dz * fDeltaTime;
      }
    }

  private:
    World& m_World;
    EntityID m_Entity;
};

// ────── ⋆⋅☆⋅⋆ ────────
//        Tests 
// ────── ⋆⋅☆⋅⋆ ────────

static void testEntityLifecycle(World& world) {
  section("Entity lifecycle");

  EntityID a = world.createEntity();
  EntityID b = world.createEntity();

  CHECK(a != INVALID_ENTITY);
  CHECK(b != INVALID_ENTITY);
  CHECK(a != b);
  CHECK(world.isValid(a));
  CHECK(world.isValid(b));

  world.destroyEntity(a);
  CHECK(!world.isValid(a));
  CHECK(world.isValid(b));

  // Destroyed ids are recycled by the free list.
  EntityID c = world.createEntity();
  CHECK(c == a);
  CHECK(world.isValid(c));
}

static void testComponentStorage(World& world) {
  section("Component add / has / get / overwrite / remove");

  EntityID e = world.createEntity();

  CHECK(!world.hasComponent<Position>(e));

  world.addComponent<Position>(e, Position{1.0f, 2.0f, 3.0f});
  CHECK(world.hasComponent<Position>(e));
  CHECK(world.getComponent<Position>(e).y == 2.0f);

  // Re-adding overwrites rather than duplicating.
  world.addComponent<Position>(e, Position{9.0f, 9.0f, 9.0f});
  CHECK(world.getComponent<Position>(e).x == 9.0f);

  // Distinct component types are independent.
  world.addComponent<Velocity>(e, Velocity{0.5f, 0.0f, 0.0f});
  CHECK(world.hasComponent<Velocity>(e));
  CHECK(world.hasComponent<Position>(e));

  world.removeComponent<Position>(e);
  CHECK(!world.hasComponent<Position>(e));
  CHECK(world.hasComponent<Velocity>(e)); // unaffected

  // Removing a component the entity doesn't have is a safe no-op.
  world.removeComponent<Position>(e);
  CHECK(!world.hasComponent<Position>(e));
}

static void testDestroyDropsComponents(World& world) {
  section("Destroying an entity drops its components");

  EntityID e = world.createEntity();
  world.addComponent<Position>(e, Position{1, 1, 1});
  world.addComponent<Velocity>(e, Velocity{2, 2, 2});
  CHECK(world.hasComponent<Position>(e));
  CHECK(world.hasComponent<Velocity>(e));

  world.destroyEntity(e);
  CHECK(!world.isValid(e));
  CHECK(!world.hasComponent<Position>(e));
  CHECK(!world.hasComponent<Velocity>(e));
}

static void testPackedStorageStaysConsistent(World& world) {
  section("Packed storage stays consistent after middle removal");

  // Force a swap-and-pop: removing the middle entity backfills it with the last.
  EntityID e1 = world.createEntity();
  EntityID e2 = world.createEntity();
  EntityID e3 = world.createEntity();
  world.addComponent<Position>(e1, Position{10, 0, 0});
  world.addComponent<Position>(e2, Position{20, 0, 0});
  world.addComponent<Position>(e3, Position{30, 0, 0});

  world.removeComponent<Position>(e2);

  CHECK(!world.hasComponent<Position>(e2));
  CHECK(world.hasComponent<Position>(e1));
  CHECK(world.hasComponent<Position>(e3));
  CHECK(world.getComponent<Position>(e1).x == 10.0f);
  CHECK(world.getComponent<Position>(e3).x == 30.0f); // moved into e2's old slot, value intact
}

static void testSystems(World& world) {
  section("System registration and update");

  EntityID mover = world.createEntity();
  world.addComponent<Position>(mover, Position{0, 0, 0});
  world.addComponent<Velocity>(mover, Velocity{1.0f, 2.0f, 0.0f});

  MovementSystem movement(world, mover);
  world.registerSystem(&movement);

  world.update(0.5f);
  CHECK(movement.updateCount == 1);
  CHECK(movement.lastDelta == 0.5f);
  CHECK(world.getComponent<Position>(mover).x == 0.5f); // 0 + 1.0 * 0.5
  CHECK(world.getComponent<Position>(mover).y == 1.0f); // 0 + 2.0 * 0.5

  world.update(0.5f);
  CHECK(movement.updateCount == 2);
  CHECK(world.getComponent<Position>(mover).x == 1.0f);

  // After removal the system is no longer ticked.
  world.removeSystem(&movement);
  world.update(0.5f);
  CHECK(movement.updateCount == 2);
}

static void testReset() {
  section("World reset invalidates entities");

  World world;
  EntityID e = world.createEntity();
  CHECK(world.isValid(e));

  world.reset();
  CHECK(!world.isValid(e));

  // Id allocation restarts from the beginning after a reset.
  EntityID first = world.createEntity();
  CHECK(first == e);
}

// ────── ⋆⋅☆⋅⋆ ────────
//     Entry point 
// ────── ⋆⋅☆⋅⋆ ────────

int main() {
  std::cout << "ECS smoke test\n==============\n";

  World world;
  testEntityLifecycle(world);
  testComponentStorage(world);
  testDestroyDropsComponents(world);
  testPackedStorageStaysConsistent(world);
  testSystems(world);
  testReset();

  std::cout << "\n--------------------------------\n";
  std::cout << "checks: " << g_checks
            << "   failures: " << g_failures << "\n";

  if (g_failures == 0) {
    std::cout << "RESULT: PASS\n";
    return 0;
  }
  std::cout << "RESULT: FAIL\n";
  return 1;
}
