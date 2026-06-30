/// @file ObjLoader.h

#pragma once

#include <string>
#include "../graphics/Mesh.h"

// Load a Wavefront OBJ file into outMesh's CPU-side vertex/index arrays.
//
// Does NOT call initialize()/uploadData() -- the caller does that once a GL
// context is current, exactly like the procedural mesh builders. Returns false
// only if the file cannot be opened.
//
// Supports v / vt / vn, triangulates n-gon faces with a triangle fan,
// deduplicates unique position/uv/normal combinations into an index buffer,
// and generates smooth normals if the file provides none.
bool loadOBJ(const std::string& path, Mesh& outMesh);
