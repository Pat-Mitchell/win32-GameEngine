/// @file graphics_cpu_smoke_test.cpp
/// @brief CPU-side smoke test for the graphics module (no GL context).
///
/// Covers the parts of the graphics code that do not touch the GPU:
///   - Vec2 math (the type backing Vertex::texCoord)
///   - Vertex memory layout, which uploadData()'s glVertexAttribPointer offsets
///     and strides depend on
///   - Mesh's CPU-side vertex/index buffers (addVertex / addIndex / counts)

#include <iostream>
#include <cmath>
#include <cstddef>

#include "../../src/engine/core/math/vector.h"
#include "../../src/engine/core/graphics/Mesh.h"

// ────── ⋆⋅☆⋅⋆ ────────
//    CHECK harness
// ────── ⋆⋅☆⋅⋆ ────────

static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond)                                                         \
  do {                                                                      \
    ++g_checks;                                                             \
    if(!(cond)) {                                                           \
      ++g_failures;                                                         \
      std::cout << "  [FAIL] " << #cond << "  (line " << __LINE__ << ")\n"; \
    }                                                                       \
  } while(0)

static void section(const char* name) {
  std::cout << "[ " << name << " ]\n";
}

// Float comparison with a small epsilon for derived values.
static bool approx(float a, float b, float eps = 1e-5f) {
  return std::fabs(a - b) <= eps;
}

// ────── ⋆⋅☆⋅⋆ ────────
//      Vec2 math
// ────── ⋆⋅☆⋅⋆ ────────

static void testVec2() {
  section("Vec2 math");

  Vec2 a(3.0f, 4.0f);
  Vec2 b(1.0f, 2.0f);

  CHECK((a + b) == Vec2(4.0f, 6.0f));
  CHECK((a - b) == Vec2(2.0f, 2.0f));
  CHECK((a * 2.0f) == Vec2(6.0f, 8.0f));
  CHECK((a / 2.0f) == Vec2(1.5f, 2.0f));

  // Division by zero is guarded (returns zero vector, matching Vec3/Vec4).
  CHECK((a / 0.0f) == Vec2(0.0f, 0.0f));

  Vec2 c = a;
  c += b;
  CHECK(c == Vec2(4.0f, 6.0f));

  Vec2 d = a;
  d /= 2.0f;
  CHECK(d == Vec2(1.5f, 2.0f));

  CHECK(approx(a.dot(b), 11.0f)); // 3*1 + 4*2
  CHECK(approx(a.length(), 5.0f)); // 3-4-5 triangle
  CHECK(approx(a.distance(b), std::sqrt(8.0f)));

  Vec2 n = a.normalized();
  CHECK(approx(n.length(), 1.0f));
  CHECK(approx(n.x, 0.6f));
  CHECK(approx(n.y, 0.8f));

  // normalize() mutates in place; normalizing a zero vector is safe.
  Vec2 z(0.0f, 0.0f);
  z.normalize();
  CHECK(z == Vec2(0.0f, 0.0f));
}

// ────── ⋆⋅☆⋅⋆ ────────
//    Vertex layout
// ────── ⋆⋅☆⋅⋆ ────────

static void testVertexLayout() {
  section("Vertex memory layout");

  CHECK(sizeof(Vec2) == 2 * sizeof(float));
  CHECK(sizeof(Vec3) == 3 * sizeof(float));

  CHECK(offsetof(Vertex, position) == 0);
  CHECK(offsetof(Vertex, normal) == sizeof(Vec3));
  CHECK(offsetof(Vertex, texCoord) == 2 * sizeof(Vec3));

  // texCoord must be 2 floats wide. The attribute uploads exactly 2.
  CHECK(sizeof(Vertex().texCoord) == 2 * sizeof(float));

  // No padding between the three tightly packed float members.
  CHECK(sizeof(Vertex) == 2 * sizeof(Vec3) + sizeof(Vec2));
}

// ────── ⋆⋅☆⋅⋆ ────────
//   Mesh CPU buffers
// ────── ⋆⋅☆⋅⋆ ────────

static void testMeshCpuData() {
  section("Mesh CPU-side vertex/index buffers");

  Mesh mesh;

  CHECK(mesh.getVertexCount() == 0);
  CHECK(mesh.getIndexConst() == 0);

  Vertex v{};
  v.position = Vec3(1.0f, 0.0f, 0.0f);
  v.normal = Vec3(0.0f, 1.0f, 0.0f);
  v.texCoord = Vec2(0.5f, 0.5f);

  mesh.addVertex(v);
  mesh.addVertex(v);
  mesh.addVertex(v);
  CHECK(mesh.getVertexCount() == 3);

  mesh.addIndex(0);
  mesh.addIndex(1);
  mesh.addIndex(2);
  CHECK(mesh.getIndexConst() == 3);
}

// ────── ⋆⋅☆⋅⋆ ────────
//     Entry point
// ────── ⋆⋅☆⋅⋆ ────────

int main() {
  std::cout << "Graphics CPU smoke test\n=======================\n";

  testVec2();
  testVertexLayout();
  testMeshCpuData();

  std::cout << "\n--------------------------------\n";
  std::cout << "checks: " << g_checks
            << "   failures: " << g_failures << "\n";

  if(g_failures == 0) {
    std::cout << "RESULT: PASS\n";
    return 0;
  }
  std::cout << "RESULT: FAIL\n";
  return 1;
}
