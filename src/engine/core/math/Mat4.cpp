/// @file Mat4.cpp
/// @brief struct for 4x4 matrices

#include "Mat4.h"
#include <cmath>

// Default contructor creates an identity matrix
Mat4::Mat4() {
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      m[i][j] = (i == j) ? 1.0f : 0.0f;
    }
  }
}

// Constructor with diagonal value
Mat4::Mat4(float fDiagonal) {
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      m[i][j] = (i == j) ? fDiagonal : 0.0f;
    }
  }
}

// Identity matrix
Mat4 Mat4::identity() {
  return Mat4(1.0f);
}

// Translation matrix
Mat4 Mat4::translate(const Vec3& v3Translation) {
  Mat4 result(1.0f);
  result.m[0][3] = v3Translation.x;
  result.m[1][3] = v3Translation.y;
  result.m[2][3] = v3Translation.z;
  return result;
}

// ────── ⋆⋅☆⋅⋆ ────────
//      Rotations
// ────── ⋆⋅☆⋅⋆ ────────

// Rotation matrix around X axis
Mat4 Mat4::rotateX(float fAngle) {
  float cos_a = std::cos(fAngle);
  float sin_a = std::sin(fAngle);

  Mat4 result(1.0f);
  result.m[1][1] = cos_a;
  result.m[1][2] = -sin_a;
  result.m[2][1] = sin_a;
  result.m[2][2] = cos_a;
  return result;
}

// Rotation matix around Y axis
Mat4 Mat4::rotateY(float fAngle) {
  float cos_a = std::cos(fAngle);
  float sin_a = std::sin(fAngle);

  Mat4 result(1.0f);
  result.m[0][0] = cos_a;
  result.m[0][2] = sin_a;
  result.m[2][0] = -sin_a;
  result.m[2][2] = cos_a;
  return result;
}

// Rotation matrix around Z axis
Mat4 Mat4::rotateZ(float fAngle) {
  float cos_a = std::cos(fAngle);
  float sin_a = std::sin(fAngle);

  Mat4 result(1.0f);
  result.m[0][0] = cos_a;
  result.m[0][1] = -sin_a;
  result.m[1][0] = sin_a;
  result.m[1][1] = cos_a;
  return result;
}

// Rotation matrix around arbitrary axis
Mat4 Mat4::rotate(float fAngle, const Vec3& v3Axis) {
  Vec3 normalized_axis = v3Axis.normalized();
  float cos_a = std::cos(fAngle);
  float sin_a = std::sin(fAngle);
  float one_minus_cos = 1.0f - cos_a;

  Mat4 result(1.0f);
  result.m[0][0] = cos_a + normalized_axis.x * normalized_axis.x * one_minus_cos;
  result.m[0][1] = normalized_axis.x * normalized_axis.y * one_minus_cos - normalized_axis.z * sin_a;
  result.m[0][2] = normalized_axis.x * normalized_axis.z * one_minus_cos + normalized_axis.y * sin_a;
    
  result.m[1][0] = normalized_axis.y * normalized_axis.x * one_minus_cos + normalized_axis.z * sin_a;
  result.m[1][1] = cos_a + normalized_axis.y * normalized_axis.y * one_minus_cos;
  result.m[1][2] = normalized_axis.y * normalized_axis.z * one_minus_cos - normalized_axis.x * sin_a;
    
  result.m[2][0] = normalized_axis.z * normalized_axis.x * one_minus_cos - normalized_axis.y * sin_a;
  result.m[2][1] = normalized_axis.z * normalized_axis.y * one_minus_cos + normalized_axis.x * sin_a;
  result.m[2][2] = cos_a + normalized_axis.z * normalized_axis.z * one_minus_cos;
    
  return result;
}

// Scale matrix
Mat4 Mat4::scale(const Vec3& v3Scale) {
  Mat4 result(1.0f);
  result.m[0][0] = v3Scale.x;
  result.m[1][1] = v3Scale.y;
  result.m[2][2] = v3Scale.z;
  return result;
}

// Uniform scale matrix
Mat4 Mat4::scale(float v3Scale) {
  Mat4 result(1.0f);
  result.m[0][0] = result.m[1][1] = result.m[2][2] = v3Scale;
  return result;
}

// Perspective projection matrix
Mat4 Mat4::perspective(float fov, float fAspect, float fNear, float fFar) {
  float f = 1.0f / std::tan(fov * 0.5f);

  Mat4 result(0.0f);
  result.m[0][0] = f / fAspect;
  result.m[1][1] = f;
  result.m[2][2] = (fFar + fNear) / (fNear - fFar);
  result.m[2][3] = (2.0f * fFar * fNear) / (fNear - fFar);
  result.m[3][2] = -1.0f;

  return result;
}

// Look at matrix
Mat4 Mat4::lookAt(const Vec3& v3Position, const Vec3& v3Target, const Vec3& v3Up) {
  Vec3 forward = (v3Target - v3Position).normalized();
  Vec3 side = forward.cross(v3Up).normalized();
  Vec3 newUp = side.cross(forward);

  Mat4 result(1.0f);
  result.m[0][0] = side.x;
  result.m[0][1] = side.y;
  result.m[0][2] = side.z;
  
  result.m[1][0] = newUp.x;
  result.m[1][1] = newUp.y;
  result.m[1][2] = newUp.z;
  
  result.m[2][0] = -forward.x;
  result.m[2][1] = -forward.y;
  result.m[2][2] = -forward.z;
  
  result.m[0][3] = -side.dot(v3Position);
  result.m[1][3] = -newUp.dot(v3Position);
  result.m[2][3] = forward.dot(v3Position);
  
  return result;
}

// Matrix multiplication
Mat4 Mat4::operator*(const Mat4& m4Other) const {
  Mat4 result;
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      result.m[i][j] = 0.0f;
      for(int k = 0; k < 4; k++) {
        result.m[i][j] += m[i][k] * m4Other.m[k][j];
      }
    }
  }
  return result;
}

// Matrix-vector multiplication
Vec3 Mat4::operator*(const Vec3& v3Vector) const {
  float x = m[0][0] * v3Vector.x + m[0][1] * v3Vector.y + m[0][2] * v3Vector.z + m[0][3];
  float y = m[1][0] * v3Vector.x + m[1][1] * v3Vector.y + m[1][2] * v3Vector.z + m[1][3];
  float z = m[2][0] * v3Vector.x + m[2][1] * v3Vector.y + m[2][2] * v3Vector.z + m[2][3];
  return Vec3(x, y, z);
}

// Get a specific row
Vec4 Mat4::getRow(int iIndex) const {
  return Vec4(m[iIndex][0], m[iIndex][1], m[iIndex][2], m[iIndex][3]);
}

// Get a specific column
Vec4 Mat4::getColumn(int iIndex) const {
  return Vec4(m[0][iIndex], m[1][iIndex], m[2][iIndex], m[3][iIndex]);
}

// Transpose matrix
Mat4 Mat4::transpose() const {
  Mat4 result;
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      result.m[i][j] = m[j][i];
    }
  }
  return result;
}

// Determinant calcualtion (using cofactor expansion)
float Mat4::determinant() const {
  float det = 0.0f;

  for(int i = 0; i < 4; i++) {
    // create minor matrix by removing row 0 and column i
    float minor[3][3];
    int minor_row = 0;
    for(int r = 1; r < 4; r++) {
      int minor_col = 0;
      for(int c = 0; c < 4; c++) {
        if(c != i) {
          minor[minor_row][minor_col] = m[r][c];
          minor_col++;
        }
      }
      minor_row++;
    }

    // Calculate 3x3 determinant
    float det_3x3 = 
      minor[0][0] * (minor[1][1] * minor[2][2] - minor[1][2] * minor[2][1]) -
      minor[0][1] * (minor[1][0] * minor[2][2] - minor[1][2] * minor[2][0]) +
      minor[0][2] * (minor[1][0] * minor[2][1] - minor[1][1] * minor[2][0]);

    det += m[0][i] * det_3x3 * ((i % 2) == 0 ? 1.0f : -1.0f);
  }

  return det;
}

Mat4 Mat4::inverse() const {
  Mat4 result;
  
  float s0 = m[0][0] * m[1][1] - m[0][1] * m[1][0];
  float s1 = m[0][0] * m[1][2] - m[0][2] * m[1][0];
  float s2 = m[0][0] * m[1][3] - m[0][3] * m[1][0];
  float s3 = m[0][1] * m[1][2] - m[0][2] * m[1][1];
  float s4 = m[0][1] * m[1][3] - m[0][3] * m[1][1];
  float s5 = m[0][2] * m[1][3] - m[0][3] * m[1][2];

  float c5 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
  float c4 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
  float c3 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
  float c2 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
  float c1 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
  float c0 = m[2][0] * m[3][1] - m[2][1] * m[3][0];

  // Compute the full 4x4 determinant using the 2x2 sub-determinants
  float det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;

  // Check if matrix is singular (non-invertible)
  if(std::abs(det) < 1e-9f) {
    return Mat4(0.0f);
  }

  // Compute adjugate matrix entries scaled by cofactor signs
  result.m[0][0] =  m[1][1] * c5 - m[1][2] * c4 + m[1][3] * c3;
  result.m[0][1] = -m[0][1] * c5 + m[0][2] * c4 - m[0][3] * c3;
  result.m[0][2] =  m[3][1] * s5 - m[3][2] * s4 + m[3][3] * s3;
  result.m[0][3] = -m[2][1] * s5 + m[2][2] * s4 - m[2][3] * s3;

  result.m[1][0] = -m[1][0] * c5 + m[1][2] * c2 - m[1][3] * c1;
  result.m[1][1] =  m[0][0] * c5 - m[0][2] * c2 + m[0][3] * c1;
  result.m[1][2] = -m[3][0] * s5 + m[3][2] * s2 - m[3][3] * s1;
  result.m[1][3] =  m[2][0] * s5 - m[2][2] * s2 + m[2][3] * s1;

  result.m[2][0] =  m[1][0] * c4 - m[1][1] * c2 + m[1][3] * c0;
  result.m[2][1] = -m[0][0] * c4 + m[0][1] * c2 - m[0][3] * c0;
  result.m[2][2] =  m[3][0] * s4 - m[3][1] * s2 + m[3][3] * s0;
  result.m[2][3] = -m[2][0] * s4 + m[2][1] * s2 - m[2][3] * s0;

  result.m[3][0] = -m[1][0] * c3 + m[1][1] * c1 - m[1][2] * c0;
  result.m[3][1] =  m[0][0] * c3 - m[0][1] * c1 + m[0][2] * c0;
  result.m[3][2] = -m[3][0] * s3 + m[3][1] * s1 - m[3][2] * s0;
  result.m[3][3] =  m[2][0] * s3 - m[2][1] * s1 + m[2][2] * s0;

  // Multiply the adjugate matrix by 1 / determinant
  float invDet = 1.0f / det;
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      result.m[i][j] *= invDet;
    }
  }

  return result;
}