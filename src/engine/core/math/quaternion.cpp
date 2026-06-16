/// @file quaternion.cpp
/// @brief Implementation file for Quaternion mathematics

#include "quaternion.h"
#include <cmath>

// Constructors
Quaternion::Quaternion() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
Quaternion::Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

Quaternion::Quaternion(const Vec3& axis, float angle) {
  fromAxisAngle(axis, angle);
}

float Quaternion::length() const {
  return std::sqrt(w * w + x * x + y * y + z * z);
}

float Quaternion::lengthSquared() const {
  return w * w + x * x + y * y + z * z;
}

void Quaternion::normalize() {
  float len = length();
  if(len > 0.0f) {
    float invLen = 1.0f / len;
    w *= invLen;
    x *= invLen;
    y *= invLen;
    z *= invLen;
  }
}

Quaternion Quaternion::normalized() const {
  Quaternion result(*this);
  result.normalize();
  return result;
}

Quaternion Quaternion::conjugate() const {
  return Quaternion(w, -x, -y, -z);
}

Quaternion Quaternion::inverse() const {
  float norm = lengthSquared();
  if(norm > 0.0f) {
    float invNorm = 1.0f / norm;
    return Quaternion(w * invNorm, -x * invNorm, -y * invNorm, -z * invNorm);
  }
  return Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
}

// ────── ⋆⋅☆⋅⋆ ────────
//      Operations
// ────── ⋆⋅☆⋅⋆ ────────

Quaternion Quaternion::operator+(const Quaternion& other) const {
  return Quaternion(w + other.w, x + other.x, y + other.y, z + other.z);
}

Quaternion Quaternion::operator-(const Quaternion& other) const {
  return Quaternion(w - other.w, x - other.x, y - other.y, z - other.z);
}

Quaternion Quaternion::operator*(const Quaternion& other) const {
  return Quaternion(
    w * other.w - x * other.x - y * other.y - z * other.z,
    w * other.x + x * other.w + y * other.z - z * other.y,
    w * other.y - x * other.z + y * other.w + z * other.x,
    w * other.z + x * other.y - y * other.x + z * other.w
  );
}

Quaternion Quaternion::operator*(float scalar) const {
  return Quaternion(w * scalar, x * scalar, y * scalar, z * scalar);
}

Vec3 Quaternion::rotateVector(const Vec3& vector) const {
  // Convert vector to Quaternion
  Quaternion vecQuat(0.0f, vector.x, vector.y, vector.z);

  // Rotate: q * v * q^-1
  Quaternion result = *this * vecQuat * inverse();

  return Vec3(result.x, result.y, result.z);
}
// ────── ⋆⋅☆⋅⋆ ────────
// Conversion operations
// ────── ⋆⋅☆⋅⋆ ────────

Mat4 Quaternion::toMatrix() const {
  Quaternion q = normalized();

  float xx = q.x * q.x;
  float yy = q.y * q.y;
  float zz = q.z * q.z;
  float xy = q.x * q.y;
  float xz = q.x * q.z;
  float yz = q.y * q.z;
  float wx = q.w * q.x;
  float wy = q.w * q.y;
  float wz = q.w * q.z;

  Mat4 matrix;
  matrix.m[0][0] = 1.0f - 2.0f * (yy + zz);
  matrix.m[0][1] = 2.0f * (xy - wz);
  matrix.m[0][2] = 2.0f * (xz + wy);
  matrix.m[0][3] = 0.0f;

  matrix.m[1][0] = 2.0f * (xy + wz);
  matrix.m[1][1] = 1.0f - 2.0f * (xx + zz);
  matrix.m[1][2] = 2.0f * (yz - wx);
  matrix.m[1][3] = 0.0f;

  matrix.m[2][0] = 2.0f * (xz - wy);
  matrix.m[2][1] = 2.0f * (yz + wx);
  matrix.m[2][2] = 1.0f - 2.0f * (xx + yy);
  matrix.m[2][3] = 0.0f;

  matrix.m[3][0] = 0.0f;
  matrix.m[3][1] = 0.0f;
  matrix.m[3][2] = 0.0f;
  matrix.m[3][3] = 1.0f;

  return matrix;
}

void Quaternion::fromAxisAngle(const Vec3& axis, float angle) {
  Vec3 normalizedAxis = axis.normalized();
  float halfAngle = angle * 0.5f;
  float sinHalfAngle = std::sin(halfAngle);

  w = std::cos(halfAngle);
  x = normalizedAxis.x * sinHalfAngle;
  y = normalizedAxis.y * sinHalfAngle;
  z = normalizedAxis.z * sinHalfAngle;
}

void Quaternion::fromEulerAngles(float roll, float pitch, float yaw) {
  float cr = std::cos(roll * 0.5f);
  float sr = std::sin(roll * 0.5f);
  float cp = std::cos(pitch * 0.5f);
  float sp = std::sin(pitch * 0.5f);
  float cy = std::cos(yaw * 0.5f);
  float sy = std::sin(yaw * 0.5f);

  w = cr * cp * cy + sr * sp * sy;
  x = sr * cp * cy - cr * sp * sy;
  y = cr * sp * cy + sr * cp * sy;
  z = cr * cp * sy - sr * sp * cy;
}

Vec3 Quaternion::toEulerAngles() const {
  float sinr_cosp = 2.0f * (w * x + y * z);
  float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
  float roll = std::atan2(sinr_cosp, cosr_cosp);

  float sinp = 2.0f * (w * y - z * x);
  float pitch;
  if(std::abs(sinp) >= 1) {
    pitch = std::copysign(3.14159265358979323846f / 2, sinp);
  } else {
    pitch = std::asin(sinp);
  }

  float siny_cosp = 2.0f * (w * z + x * y);
  float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
  float yaw = std::atan2(siny_cosp, cosy_cosp);

  return Vec3(roll, pitch, yaw);
}

// ────── ⋆⋅☆⋅⋆ ────────
//   Utility functions
// ────── ⋆⋅☆⋅⋆ ────────

Quaternion Quaternion::slerp(const Quaternion& a, const Quaternion& b, float t) {
  Quaternion qa = a.normalized();
  Quaternion qb = b.normalized();

  // calculate dot product
  float dot = qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z;

  // If dot product is negative, flip one Quaternion
  if(dot < 0.0f) {
    qb = Quaternion(-qb.w, -qb.x, -qb.y, -qb.z);
    dot = -dot;
  }

  // If quaternions are very close, use lerp
  if(dot > 0.9995f) {
    return (qa + t * (qb - qa)).normalized();
  }

  // Slerp (spherical linear interpolation)
  float theta_0 = std::acos(dot);
  float theta = theta_0 * t;

  Quaternion qPerp = (qb - qa * dot).normalized();
  return qa * std::cos(theta) + qPerp * std::sin(theta);
}

Quaternion Quaternion::identity() {
  return Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
}

Quaternion Quaternion::fromToRotation(const Vec3& from, const Vec3& to) {
  // Normalize input vectors
  Vec3 fromNorm = from.normalized();
  Vec3 toNorm = to.normalized();

  // if vectors are parallel, return identity
  if(fromNorm == toNorm) return identity();

  // if vectos are opposite, return 180 degree rotation around perpendicular axis
  if(fromNorm == -toNorm) {
    Vec3 axis = Vec3(1.0f, 0.0f, 0.0f);
    if(std::abs(fromNorm.x) <= std::abs(fromNorm.y) && std::abs(fromNorm.x) <= std::abs(fromNorm.z))
      axis = Vec3(1.0f, 0.0f, 0.0f);
    else if(std::abs(fromNorm.y) <= std::abs(fromNorm.z))
      axis = Vec3(0.0f, 1.0f, 0.0f);
    else
      axis = Vec3(0.0f, 0.0f, 1.0f);

    axis = axis.cross(fromNorm).normalized();
    return Quaternion(axis, 3.14159265358979323846f);
  }

  // General case
  Vec3 axis = fromNorm.cross(toNorm).normalized();
  float angle = std::acos(fromNorm.dot(toNorm));

  return Quaternion(axis, angle);
}

// non-member operators
Quaternion operator*(float scalar, const Quaternion& q) {
  return q * scalar;
}