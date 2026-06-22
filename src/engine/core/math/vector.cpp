/// @file vector.cpp
/// @brief Structs for vec2, vec3 and vec4.

#include "vector.h"
#include <cmath>

// ────── ⋆⋅☆⋅⋆ ────────
//        Vec2
// ────── ⋆⋅☆⋅⋆ ────────

// Vector addition
Vec2 Vec2::operator+(const Vec2& other) const {
  return Vec2(x + other.x, y + other.y);
}

// Vector subtraction
Vec2 Vec2::operator-(const Vec2& other) const {
  return Vec2(x - other.x, y - other.y);
}

// Scalar multiplication
Vec2 Vec2::operator*(float scalar) const {
  return Vec2(x * scalar, y * scalar);
}

// Scalar division
Vec2 Vec2::operator/(float scalar) const {
  if(scalar != 0.0f) {
    return Vec2(x / scalar, y / scalar);
  }
  return Vec2(0.0f, 0.0f);
}

// Vector addition assignment
Vec2& Vec2::operator+=(const Vec2& other) {
  x += other.x;
  y += other.y;
  return *this;
}

// Scalar division assignment
Vec2& Vec2::operator/=(float scalar) {
  if(scalar != 0.0f) {
    x /= scalar;
    y /= scalar;
  }
  return *this;
}

// Equivalence
bool Vec2::operator==(const Vec2& other) const {
  if(other.x == x && other.y == y) return true;
  return false;
}

// Dot product
float Vec2::dot(const Vec2& other) const {
  return x * other.x + y * other.y;
}

// Vector length
float Vec2::length() const {
  return std::sqrt(x * x + y * y);
}

// Normalize vector
void Vec2::normalize() {
  float len = length();
  if(len > 0.0f) {
    x /= len;
    y /= len;
  } else {
    x = 0.0f;
    y = 0.0f;
  }
}

Vec2 Vec2::normalized() const {
  float len = length();
  if(len > 0.0f) {
    return Vec2(x / len, y / len);
  }
  return Vec2(0.0f, 0.0f);
}

// Distance between two vectors
float Vec2::distance(const Vec2& other) const {
  float dx = x - other.x;
  float dy = y - other.y;
  return std::sqrt(dx * dx + dy * dy);
}

// ────── ⋆⋅☆⋅⋆ ────────
//        Vec3
// ────── ⋆⋅☆⋅⋆ ────────

// Vector addition
Vec3 Vec3::operator+(const Vec3& other) const {
  return Vec3(x + other.x, y + other.y, z + other.z);
}

// Vector subtraction
Vec3 Vec3::operator-(const Vec3& other) const {
  return Vec3(x - other.x, y - other.y, z - other.z);
}

// Scalar multiplication
Vec3 Vec3::operator*(float scalar) const {
  return Vec3(x * scalar, y * scalar, z * scalar);
}

// Scalar division
Vec3 Vec3::operator/(float scalar) const {
  if(scalar != 0.0f) {
    return Vec3(x / scalar, y / scalar, z / scalar);
  }
  return Vec3(0.0f, 0.0f, 0.0f);
}

// Negation operator
Vec3 Vec3::operator-() const {
  return Vec3(-x, -y, -z);
}

// Vector addition assignment
Vec3& Vec3::operator+=(const Vec3& other) {
  x += other.x;
  y += other.y;
  z += other.z;
  return *this;
}

// Scalar division assignment
Vec3& Vec3::operator/=(float scalar) {
  if(scalar != 0.0f) {
    x /= scalar;
    y /= scalar;
    z /= scalar;
  }
  return *this;
}

// Equivalence
bool Vec3::operator==(const Vec3& other) const {
  if(other.x == x && other.y == y && other.z == z) return true;
  return false;
}

// Dot product
float Vec3::dot(const Vec3& other) const {
  return x * other.x + y * other.y + z * other.z;
}

// Cross product
Vec3 Vec3::cross(const Vec3& other) const {
  return Vec3(
    y * other.z - z * other.y,
    z * other.x - x * other.z,
    x * other.y - y * other.x
  );
}

// Vector length
float Vec3::length() const {
  return std::sqrt(x * x + y * y + z * z);
}

// Normalize vector
void Vec3::normalize() {
  float len = length();
  if(len > 0.0f) {
    x /= len;
    y /= len; 
    z /= len;
  } else {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
  }
}

Vec3 Vec3::normalized() const {
  float len = length();
  if(len > 0.0f) {
    return Vec3(x / len, y / len, z / len);
  }
  return Vec3(0.0f, 0.0f, 0.0f);
}

// Distance between two vectors
float Vec3::distance(const Vec3& other) const {
  float dx = x - other.x;
  float dy = y - other.y;
  float dz = z - other.z;
  return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// ────── ⋆⋅☆⋅⋆ ────────
//        Vec4
// ────── ⋆⋅☆⋅⋆ ────────

// Vector addition
Vec4 Vec4::operator+(const Vec4& other) const {
  return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
}

// Vector subtraction
Vec4 Vec4::operator-(const Vec4& other) const {
  return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
}

// Scalar multiplication
Vec4 Vec4::operator*(float scalar) const {
  return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
}

// Scalar division
Vec4 Vec4::operator/(float scalar) const {
  if(scalar != 0.0f) {
    return Vec4(x / scalar, y / scalar, z / scalar, w / scalar);
  }
  return Vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

// Vector addition assignment
Vec4& Vec4::operator+=(const Vec4& other) {
  x += other.x;
  y += other.y;
  z += other.z;
  w += other.w;
  return *this;
}

// Scalar division assignment
Vec4& Vec4::operator/=(float scalar) {
  if(scalar != 0.0f) {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
  }
  return *this;
}