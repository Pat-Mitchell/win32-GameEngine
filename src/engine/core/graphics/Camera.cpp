/// @file Camera.cpp

#include "Camera.h"
#include <cmath>

Camera::Camera() : position(0.0f, 0.0f, 3.0f), front(0.0f, 0.0f, -1.0f), up(0.0f, 1.0f, 0.0f), pitch(0.0f), yaw(0.0f), fov(55.0f), aspect(16.0f/9.0f), nearPlane(0.1f), farPlane(100.0f) {
  update();
}

void Camera::update() {
  // Calculate the view Matrix
  viewMatrix = Mat4::lookAt(position, position + front, up);

  // Calculate the projection matrix
  projectionMatrix = Mat4::perspective(fov * (3.1415926 / 180), aspect, nearPlane, farPlane);
}

void Camera::setPosition(Vec3 pos) {
  position = pos;
  update();
}

void Camera::setRotation(float p, float y) {
  pitch = p;
  yaw = y;

  // Clamp pitch so `front` can't become parallel to world up, which would
  // make Mat4::lookAt degenerate. (A flight sim wanting full vertical
  // rotation should use an incremental quaternion orientation instead.)
  if(pitch > 89.0f) pitch = 89.0f;
  if(pitch < -89.0f) pitch = -89.0f;

  float radPitch = pitch * (3.1415926f / 180.0f);
  float radYaw = yaw * (3.1415926f / 180.0f);

  // yaw = 0, pitch = 0 yields front = (0, 0, -1) (looking down -Z).
  // Increasing yaw turns the camera toward +X.
  front.x = sin(radYaw) * cos(radPitch);
  front.y = sin(radPitch);
  front.z = -cos(radYaw) * cos(radPitch);

  update();
}

void Camera::setProjection(float f, float a, float n, float far) {
  fov = f;
  aspect = a;
  nearPlane = n;
  farPlane = far;
  update();
}

void Camera::setAspect(float a) {
  aspect = a;
  update();
}