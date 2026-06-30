/// @file CameraComponent.h
/// @brief Component: the camera "lens" (projection parameters).

#pragma once

#include "../math/Mat4.h"

// Holds only the projection parameters. The camera's position and orientation
// live on the same entity's Transform; the view matrix is that Transform's
// getWorldToLocalMatrix(). Keeping the lens and the placement separate is the
// standard ECS split -- a camera is just an entity with a Transform + this.
struct CameraComponent {
  float fov = 55.0f;            // vertical field of view, in degrees
  float aspect = 16.0f / 9.0f;  // viewport width / height (updated on resize)
  float nearPlane = 0.1f;
  float farPlane = 100.0f;

  Mat4 getProjection() const {
    const float DEG2RAD = 3.14159265f / 180.0f;
    return Mat4::perspective(fov * DEG2RAD, aspect, nearPlane, farPlane);
  }
};
