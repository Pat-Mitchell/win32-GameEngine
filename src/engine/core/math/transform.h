/// @file transform.h
/// @brief 3D transformations

#pragma once
#include "vector.h"
#include "quaternion.h"
#include "Mat4.h"

struct Transform {
  Vec3 position;
  Quaternion rotation;
  Vec3 v3Scale;

  Transform() : position(0.0f, 0.0f, 0.0f), rotation(Quaternion::identity()), v3Scale(1.0f, 1.0f, 1.0f) {}
  Transform(const Vec3& position, const Quaternion& rotation, const Vec3& v3Scale)
    : position(position), rotation(rotation), v3Scale(v3Scale) {}

  Mat4 getLocalToWorldMatrix() const;
  Mat4 getWorldToLocalMatrix() const;

  void translate(const Vec3& translation);
  void rotate(const Quaternion& deltaRotation);
  void rotate(const Vec3& axis, float angle);
  void scale(const Vec3& factor);

  Vec3 transformPoint(const Vec3& point) const;
  Vec3 transformVector(const Vec3& vector) const;

  Vec3 getForward() const;
  Vec3 getUp() const;
  Vec3 getRight() const;

  void reset();

  static Transform fromPosition(const Vec3& position);
  static Transform fromRotation(const Quaternion& rotation);
  static Transform fromScale(const Vec3& scale);
};

Transform operator*(const Transform& a, const Transform& b);