/// @file Camera.h

#pragma once 

#include <GL/glew.h>
#include "../math/Mat4.h"

class Camera {
  public:
    Camera();

    // Update internal matrices based on position/orientation
    void update();

    Mat4 getViewMatrix() const { return viewMatrix; }
    Mat4 getProjectionMatrix() const { return projectionMatrix; }

    // Read-only state for input-driven movement (fly camera, etc.).
    Vec3 getPosition() const { return position; }
    Vec3 getFront() const { return front; }

    void setPosition(Vec3 pos);
    void setRotation(float pitch, float yaw);
    void setProjection(float fov, float aspect, float near, float far);
    void setAspect(float aspect); // Updates aspect only, preserving fov/near/far

    float getFov() { return fov; }

  private:
    Vec3 position;
    Vec3 front;
    Vec3 up;
    Vec3 right;

    Mat4 viewMatrix;
    Mat4 projectionMatrix;

    float pitch;
    float yaw;

    float fov;
    float aspect;
    float nearPlane;
    float farPlane;
};