/// @file Quaternion.h
/// @brief Quaternion mathematics

#pragma once

#include "vector.h"
#include "Mat4.h"

struct Quaternion {
  float w, x, y, z;

  // Constructors
  Quaternion();
  Quaternion(float w, float x, float y, float z);
  Quaternion(const Vec3& axis, float angle); // From axis-angle

  float length() const;
  float lengthSquared() const;
  void normalize();
  Quaternion normalized() const;
  Quaternion conjugate() const;
  Quaternion inverse() const;

  Quaternion operator+(const Quaternion& other) const;
  Quaternion operator-(const Quaternion& other) const;
  Quaternion operator*(const Quaternion& other) const;
  Quaternion operator*(float scalar) const;
  Vec3 rotateVector(const Vec3& vector) const;

  Mat4 toMatrix() const;
  void fromAxisAngle(const Vec3& axis, float angle);
  void fromEulerAngles(float roll, float pitch, float yaw);
  Vec3 toEulerAngles() const;

  static Quaternion slerp(const Quaternion& a, const Quaternion& b, float t);
  static Quaternion identity();
  static Quaternion fromToRotation(const Vec3& from, const Vec3& to);
};

// non-member operator
Quaternion operator*(float scalar, const Quaternion& q);