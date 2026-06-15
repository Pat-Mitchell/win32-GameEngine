/// @file vector.cpp
/// @brief Struct for vector3. 

#include "vector.h"
#include <cmath>

// Constructor
Vec3::Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

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
Vec3 Vec3::normalize() const {
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