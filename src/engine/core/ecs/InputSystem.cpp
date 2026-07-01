/// @file InputSystem.cpp

#include "InputSystem.h"

#include "World.h"
#include "../math/transform.h" // brings in Vec3, Quaternion, Transform
#include "../input/Keyboard.h"
#include "../input/Mouse.h"

namespace {
  const float DEG2RAD = 3.14159265f / 180.0f;
}

InputSystem::InputSystem(World& world, Keyboard& keyboard, Mouse& mouse, EntityID cameraEntity)
  : m_World(world), m_Keyboard(keyboard), m_Mouse(mouse), m_CameraEntity(cameraEntity) {}

void InputSystem::update(float fDeltaTime) {
  if(!m_World.hasComponent<Transform>(m_CameraEntity)) {
    return;
  }
  Transform& t = m_World.getComponent<Transform>(m_CameraEntity);

  // Mouse-look while the cursor is locked (Tab toggle) or the right button is held.
  if(m_Mouse.isLocked() || m_Mouse.isButtonDown(Mouse::Right)) {
    m_Yaw += m_Mouse.getDeltaX() * m_Sensitivity;
    m_Pitch -= m_Mouse.getDeltaY() * m_Sensitivity; // screen-down -> look down
    if(m_Pitch > 89.0f) m_Pitch = 89.0f;
    if(m_Pitch < -89.0f) m_Pitch = -89.0f;
  }
  m_Mouse.consumeDelta(); // reset accumulated motion every frame

  // Rebuild orientation: yaw about world up, then pitch about local right. Using
  // axis-angle quaternions avoids Euler-order ambiguity; pitch about X keeps the
  // right vector horizontal so strafing stays level.
  Quaternion qYaw(Vec3(0.0f, 1.0f, 0.0f), m_Yaw * DEG2RAD);
  Quaternion qPitch(Vec3(1.0f, 0.0f, 0.0f), m_Pitch * DEG2RAD);
  t.rotation = qYaw * qPitch;

  // Keyboard fly movement, relative to the new orientation
  Vec3 forward = t.getForward();
  Vec3 right = t.getRight();
  Vec3 worldUp(0.0f, 1.0f, 0.0f);
  float step = m_MoveSpeed * fDeltaTime;
  if(m_Keyboard.isKeyDown('W')) t.position += forward * step;
  if(m_Keyboard.isKeyDown('S')) t.position += forward * (-step);
  if(m_Keyboard.isKeyDown('D')) t.position += right * step;
  if(m_Keyboard.isKeyDown('A')) t.position += right * (-step);
  if(m_Keyboard.isKeyDown('E')) t.position += worldUp * step;
  if(m_Keyboard.isKeyDown('Q')) t.position += worldUp * (-step);
}
