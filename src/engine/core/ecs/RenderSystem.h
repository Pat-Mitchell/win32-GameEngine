/// @file RenderSystem.h

#pragma once

#include "System.h"
#include "Entity.h"

class World;
class Renderer;

// Draws every entity that has both a Transform and a MeshRenderer, using the
// view/projection taken from the camera entity. Registered with the World, so its
// update() runs each frame from World::update(). The app still owns the GL frame
// (clear/SwapBuffers); this system only issues the per-entity draws.
class RenderSystem : public System {
  public:
    RenderSystem(World& world, Renderer& renderer, EntityID cameraEntity);
    void update(float fDeltaTime) override;

  private:
    World& m_World;
    Renderer& m_Renderer;
    EntityID m_CameraEntity;
};
