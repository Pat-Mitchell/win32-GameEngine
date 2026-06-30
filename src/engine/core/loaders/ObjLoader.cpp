/// @file ObjLoader.cpp

#include "ObjLoader.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>

namespace {
  // Resolve a 1-based OBJ index (negative = relative to the end of the list)
  // into a 0-based array index. Returns -1 for 0 (OBJ's "absent" sentinel).
  int resolveIndex(int idx, size_t count) {
    if(idx > 0) return idx - 1;
    if(idx < 0) return static_cast<int>(count) + idx;
    return -1;
  }
}

bool loadOBJ(const std::string& path, Mesh& outMesh) {
  std::ifstream file(path);
  if(!file.is_open()) {
    return false;
  }

  // Raw attribute pools, indexed independently by the face definitions.
  std::vector<Vec3> positions;
  std::vector<Vec2> texCoords;
  std::vector<Vec3> normals;

  // Final interleaved vertices + the index buffer that references them.
  std::vector<Vertex> verts;
  std::vector<GLuint> indices;
  // Maps a raw face token ("v/vt/vn") to its slot in `verts` so identical
  // corners are emitted once and shared via the index buffer.
  std::unordered_map<std::string, GLuint> uniqueVerts;

  bool fileHasNormals = false;

  std::string line;
  while(std::getline(file, line)) {
    std::istringstream ls(line);
    std::string tag;
    ls >> tag;

    if(tag == "v") {
      Vec3 p;
      ls >> p.x >> p.y >> p.z;
      positions.push_back(p);
    } else if(tag == "vt") {
      Vec2 t;
      ls >> t.x >> t.y; // ignore an optional 3rd (w) component
      texCoords.push_back(t);
    } else if(tag == "vn") {
      Vec3 n;
      ls >> n.x >> n.y >> n.z;
      normals.push_back(n);
      fileHasNormals = true;
    } else if(tag == "f") {
      // Collect this face's (possibly >3) corner indices, then fan-triangulate.
      std::vector<GLuint> faceIdx;
      std::string token;
      while(ls >> token) {
        // Reuse the vertex if we've seen this exact token before.
        auto it = uniqueVerts.find(token);
        if(it != uniqueVerts.end()) {
          faceIdx.push_back(it->second);
          continue;
        }

        // Parse "v", "v/vt", "v//vn", or "v/vt/vn". 0 means "not present".
        int vi = 0, ti = 0, ni = 0;
        size_t slash1 = token.find('/');
        if(slash1 == std::string::npos) {
          vi = std::stoi(token);
        } else {
          vi = std::stoi(token.substr(0, slash1));
          size_t slash2 = token.find('/', slash1 + 1);
          if(slash2 == std::string::npos) {
            ti = std::stoi(token.substr(slash1 + 1));
          } else {
            if(slash2 > slash1 + 1) {
              ti = std::stoi(token.substr(slash1 + 1, slash2 - slash1 - 1));
            }
            std::string nStr = token.substr(slash2 + 1);
            if(!nStr.empty()) ni = std::stoi(nStr);
          }
        }

        Vertex v;
        int rvi = resolveIndex(vi, positions.size());
        if(rvi >= 0 && rvi < static_cast<int>(positions.size())) v.position = positions[rvi];
        int rti = resolveIndex(ti, texCoords.size());
        if(rti >= 0 && rti < static_cast<int>(texCoords.size())) v.texCoord = texCoords[rti];
        int rni = resolveIndex(ni, normals.size());
        if(rni >= 0 && rni < static_cast<int>(normals.size())) v.normal = normals[rni];

        GLuint newIndex = static_cast<GLuint>(verts.size());
        verts.push_back(v);
        uniqueVerts.emplace(token, newIndex);
        faceIdx.push_back(newIndex);
      }

      // Triangle fan: (0, i, i+1) for i in 1..n-2. A triangle yields one tri,
      // a quad two, etc. Assumes the polygon is convex (true for OBJ exports).
      for(size_t i = 1; i + 1 < faceIdx.size(); ++i) {
        indices.push_back(faceIdx[0]);
        indices.push_back(faceIdx[i]);
        indices.push_back(faceIdx[i + 1]);
      }
    }
    // All other tags (#, mtllib, usemtl, o, g, s, ...) are ignored for now.
  }

  // If the file carried no normals, synthesize smooth ones: accumulate each
  // triangle's face normal onto its three vertices, then normalize.
  if(!fileHasNormals && !verts.empty()) {
    for(auto& v : verts) v.normal = Vec3(0.0f, 0.0f, 0.0f);
    for(size_t i = 0; i + 2 < indices.size(); i += 3) {
      GLuint ia = indices[i], ib = indices[i + 1], ic = indices[i + 2];
      Vec3 e1 = verts[ib].position - verts[ia].position;
      Vec3 e2 = verts[ic].position - verts[ia].position;
      Vec3 faceNormal = e1.cross(e2); // CCW winding -> outward normal
      verts[ia].normal += faceNormal;
      verts[ib].normal += faceNormal;
      verts[ic].normal += faceNormal;
    }
    for(auto& v : verts) v.normal = v.normal.normalized();
  }

  // Hand the assembled geometry to the mesh (still CPU-side; caller uploads).
  for(const auto& v : verts) outMesh.addVertex(v);
  for(GLuint idx : indices) outMesh.addIndex(idx);

  return true;
}
