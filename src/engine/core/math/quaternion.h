/// @file quaternion.h
/// @brief Quaternion mathematics

#pragma once

#include "vector.h"
#include "Mat4.h"

struct quaternion {
  float w, x, y, z;

  // Constructors
  quaternion();
  quaternion(float w, float x, float y, float z);
  quaternion(const Vec3& axis, float angle); // From axis-angle

  float length() const;
  float lengthSquared() const;
  void normalize();
  quaternion normalized() const;
  quaternion conjugate() const;
  quaternion inverse() const;

  quaternion operator+(const quaternion& other) const;
  quaternion operator-(const quaternion& other) const;
  quaternion operator*(const quaternion& other) const;
  quaternion operator*(float scalar) const;
  Vec3 rotateVector(const Vec3& vector) const;

  Mat4 toMatrix() const;
  void fromAxisAngle(const Vec3& axis, float angle);
  void fromEulerAngles(float roll, float pitch, float yaw);
  Vec3 toEulerAngles() const;

  static quaternion slerp(const quaternion& a, const quaternion& b, float t);
  static quaternion identity();
  static quaternion fromToRotation(const Vec3& from, const Vec3& to);
};

// non-member operator
quaternion operator*(float scalar, const quaternion& q);