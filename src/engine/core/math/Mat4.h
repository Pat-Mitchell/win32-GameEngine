#pragma once

struct Vec3;

struct Mat4
{
  float m[4][4];

  Mat4();
  Mat4(float diagonal);

  static Mat4 identity();
  static Mat4 translate(const Vec3& translation);
  static Mat4 rotate(float angle, const Vec3& axis);
  static Mat4 scale(const Vec3& scale);
  static Mat4 perspective(float fov, float aspect, float near, float far);
  static Mat4 lookAt(const Vec3& position, const Vec3& target, const Vec3& up);

  Mat4 operator*(const Mat4& other) const;
};
