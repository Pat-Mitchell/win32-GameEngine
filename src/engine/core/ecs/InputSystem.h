/// @file InputSystem.h

#pragma once

#include "System.h"
#include "Entity.h"

class World;
class Keyboard;
class Mouse;

// Reads the global Keyboard/Mouse and writes the camera entity's Transform:
// right-mouse turns the camera (yaw/pitch), WASD + Q/E fly it. This is the
// "device -> intent -> transform" layer -- nothing downstream touches the raw
// devices, only the Transform this system produces.
class InputSystem : public System {
  public:
    InputSystem(World& world, Keyboard& keyboard, Mouse& mouse, EntityID cameraEntity);
    void update(float fDeltaTime) override;

  private:
    World& m_World;
    Keyboard& m_Keyboard;
    Mouse& m_Mouse;
    EntityID m_CameraEntity;

    float m_Yaw = 0.0f;          // accumulated, degrees
    float m_Pitch = 0.0f;        // accumulated, degrees (clamped +/-89)
    float m_MoveSpeed = 3.0f;    // units per second
    float m_Sensitivity = 0.15f; // degrees per pixel of mouse motion
};
