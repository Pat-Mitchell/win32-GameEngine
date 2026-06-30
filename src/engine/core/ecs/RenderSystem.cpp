/// @file RenderSystem.cpp

#include "RenderSystem.h"

#include "World.h"
#include "MeshRenderer.h"
#include "CameraComponent.h"
#include "../math/transform.h"
#include "../graphics/Renderer.h"
#include "../graphics/Shader.h"
#include "../graphics/Mesh.h"

RenderSystem::RenderSystem(World& world, Renderer& renderer, EntityID cameraEntity)
  : m_World(world), m_Renderer(renderer), m_CameraEntity(cameraEntity) {}

void RenderSystem::update(float /*fDeltaTime*/) {
  // View + projection come from the camera entity (Transform + CameraComponent).
  if(!m_World.hasComponent<Transform>(m_CameraEntity) ||
     !m_World.hasComponent<CameraComponent>(m_CameraEntity)) {
    return; // no camera to render from
  }
  Transform& camTransform = m_World.getComponent<Transform>(m_CameraEntity);
  CameraComponent& cam = m_World.getComponent<CameraComponent>(m_CameraEntity);
  Mat4 view = camTransform.getWorldToLocalMatrix();
  Mat4 proj = cam.getProjection();

  // Draw each entity that carries both a Transform and a MeshRenderer.
  for(EntityID e : m_World.getActiveEntities()) {
    if(!m_World.hasComponent<Transform>(e) || !m_World.hasComponent<MeshRenderer>(e)) {
      continue;
    }
    MeshRenderer& mr = m_World.getComponent<MeshRenderer>(e);
    if(mr.mesh == nullptr || mr.shader == nullptr) {
      continue;
    }
    Transform& t = m_World.getComponent<Transform>(e);

    Mat4 model = t.getLocalToWorldMatrix();
    Mat4 mvp = proj * view * model;

    // Active program before glUniform*; setUniformMatrix4fv passes GL_TRUE for
    // Mat4's row-major storage. render() re-binds the program and draws.
    mr.shader->use();
    mr.shader->setUniformMatrix4fv("uMVP", &mvp.m[0][0]);
    m_Renderer.render(*mr.mesh, *mr.shader);
  }
}
