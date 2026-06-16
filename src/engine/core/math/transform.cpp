/// @file transform.cpp
/// @brief Implementation file for 3D transformations

#include "transform.h"
#include <cmath>

Mat4 Transform::getLocalToWorldMatrix() const {
  // Create transformation matrix: T * R * S
  Mat4 translationMatrix = Mat4::translate(position);
  Mat4 rotationMatrix = rotation.toMatrix();
  Mat4 scaleMatrix = Mat4::scale(v3Scale);

  return translationMatrix * rotationMatrix * scaleMatrix;
}

Mat4 Transform::getWorldToLocalMatrix() const {
  // Create inverse transformation matrix
  Mat4 translationMatrix = Mat4::translate(-position);
  Mat4 rotationMatrix = rotation.inverse().toMatrix();
  Mat4 scaleMatrix = Mat4::scale(Vec3(1.0f / v3Scale.x, 1.0f / v3Scale.y, 1.0f / v3Scale.z));

  // Combine transformations: S^-1 * R^-1 * T^-1
  return scaleMatrix * rotationMatrix * translationMatrix;
}

// ────── ⋆⋅☆⋅⋆ ────────
//  Transformation Ops
// ────── ⋆⋅☆⋅⋆ ────────
void Transform::translate(const Vec3& translation) {
  position += translation;
}

void Transform::rotate(const Quaternion& deltaRotation) {
  rotation = deltaRotation * rotation;
  rotation.normalize();
}

void Transform::rotate(const Vec3& axis, float angle) {
  Quaternion deltaRotation(axis, angle);
  rotate(deltaRotation);
}

void Transform::scale(const Vec3& factor) {
  v3Scale.x *= factor.x;
  v3Scale.y *= factor.y;
  v3Scale.z *= factor.z;
}

Vec3 Transform::transformPoint(const Vec3& point) const {
  // Apply transformation: T * R * S * point
  Vec3 scaledPoint(point.x * v3Scale.x, point.y * v3Scale.y, point.z * v3Scale.z);
  Vec3 rotatedPoint = rotation.rotateVector(scaledPoint);
  return rotatedPoint + position;
}

Vec3 Transform::transformVector(const Vec3& vector) const {
  // Apply only rotation and scale
  Vec3 scaledVector(vector.x * v3Scale.x, vector.y * v3Scale.y, vector.z * v3Scale.z);
  return rotation.rotateVector(scaledVector);
}
// ────── ⋆⋅☆⋅⋆ ────────
//    Get local axes
// ────── ⋆⋅☆⋅⋆ ────────
Vec3 Transform::getForward() const {
  // Forward is along the negative Z axis in local space
  return rotation.rotateVector(Vec3(0.0f, 0.0f, -1.0f));
}

Vec3 Transform::getUp() const {
  // Up is along the Y axis in local space
  return rotation.rotateVector(Vec3(0.0f, 1.0f, 0.0f));
}

Vec3 Transform::getRight() const {
  // Right is along the X axis in local space
  return rotation.rotateVector(Vec3(1.0f, 0.0f, 0.0f));
}

// Reset to identity
void Transform::reset() {
  position = Vec3(0.0f, 0.0f, 0.0f);
  rotation = Quaternion::identity();
  v3Scale = Vec3(1.0f, 1.0f, 1.0f);
}

// ────── ⋆⋅☆⋅⋆ ────────
//    Static methods
// ────── ⋆⋅☆⋅⋆ ────────
Transform Transform::fromPosition(const Vec3& position) {
  return Transform(position, Quaternion::identity(), Vec3(1.0f, 1.0f, 1.0f));
}

Transform Transform::fromRotation(const Quaternion& rotation) {
  return Transform(Vec3(0.0f, 0.0f, 0.0f), rotation, Vec3(1.0f, 1.0f, 1.0f));
}

Transform Transform::fromScale(const Vec3& scale) {
  return Transform(Vec3(0.0f, 0.0f, 0.0f), Quaternion::identity(), scale);
}

// ────── ⋆⋅☆⋅⋆ ────────
// Non-member operators
// ────── ⋆⋅☆⋅⋆ ────────
Transform operator*(const Transform& a, const Transform& b) {
  // Combine two transforms
  // This applis b's transformation first, the a's transformation
  Vec3 position = a.position + a.rotation.rotateVector(Vec3(b.position.x * a.v3Scale.x, b.position.y * a.v3Scale.y, b.position.z * a.v3Scale.z));
  Quaternion rotation = a.rotation * b.rotation;
  Vec3 scale(a.v3Scale.x * b.v3Scale.x, a.v3Scale.y * b.v3Scale.y, a.v3Scale.z * b.v3Scale.z);

  return Transform(position, rotation, scale);
}