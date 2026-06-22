// Note: storage is row-major
//   gluniformMatrix4fv expects column-major
//   GL_TRUE must be passed for the transpose argument
//
// Example:
//   Mat4 mvp = projection * view * model;
//   glUniformMatrix4fv(mvpLocation, 1, GL_TRUE, &mvp.m[0][0]);
//                                      ^^^^^^^ transpose on, because Mat4 is row-major

#pragma once

#include "vector.h"

struct Mat4
{
  float m[4][4];

  Mat4();
  Mat4(float fDiagonal);

  static Mat4 identity();
  static Mat4 translate(const Vec3& v3Translation);
  static Mat4 rotateX(float fAngle);
  static Mat4 rotateY(float fAngle);
  static Mat4 rotateZ(float fAngle);
  static Mat4 rotate(float fAngle, const Vec3& v3Axis);
  static Mat4 scale(const Vec3& scale);
  static Mat4 scale(float scale); // Uniform scale
  static Mat4 perspective(float fov, float aspect, float fNear, float fFar);
  static Mat4 lookAt(const Vec3& position, const Vec3& target, const Vec3& up);

  Mat4 transpose() const;
  float determinant() const;
  Mat4 inverse() const;

  Vec4 getRow(int index) const;
  Vec4 getColumn(int index) const;

  Mat4 operator*(const Mat4& other) const;
  Vec3 operator*(const Vec3& vector) const;
};
