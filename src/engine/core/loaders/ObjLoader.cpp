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

  // Parse a whole-string integer without throwing. Returns false on empty input,
  // non-numeric text, or trailing garbage, so a malformed face token can be
  // skipped instead of crashing the loader (std::stoi throws on bad input).
  bool parseInt(const std::string& s, int& out) {
    if(s.empty()) return false;
    try {
      size_t consumed = 0;
      int value = std::stoi(s, &consumed);
      if(consumed != s.size()) return false; // trailing garbage, e.g. "3x"
      out = value;
      return true;
    } catch(...) {
      return false; // not a number, or out of int range
    }
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
  // Maps a resolved "vi/ti/ni" corner (0-based, absolute) to its slot in
  // verts, so identical corners are emitted once and shared via the index
  // buffer. Keying on the resolved triple rather than the raw token text keeps
  // negative/relative OBJ indices correct: the same token ("-1//-1") can refer
  // to different vertices across interleaved faces, but its resolved indices
  // won't collide.
  std::unordered_map<std::string, GLuint> uniqueVerts;

  // Set true if any emitted vertex lacked a normal (the file had none, or a
  // face omitted them). Drives whether we synthesize smooth normals below.
  bool missingAnyNormal = false;

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
    } else if(tag == "f") {
      // Collect this face's (possibly >3) corner indices, then fan-triangulate.
      std::vector<GLuint> faceIdx;
      std::string token;
      bool faceOk = true;
      while(ls >> token) {
        // Parse "v", "v/vt", "v//vn", or "v/vt/vn". 0 means not present.
        // A malformed token abandons the whole face rather than throwing.
        int vi = 0, ti = 0, ni = 0;
        size_t slash1 = token.find('/');
        if(slash1 == std::string::npos) {
          if(!parseInt(token, vi)) { 
            faceOk = false; 
            break; 
          }
        } else {
          if(!parseInt(token.substr(0, slash1), vi)) { 
            faceOk = false; 
            break; 
          }
          size_t slash2 = token.find('/', slash1 + 1);
          if(slash2 == std::string::npos) {
            if(!parseInt(token.substr(slash1 + 1), ti)) { 
              faceOk = false; 
              break; 
            }
          } else {
            // The texCoord between the slashes is optional ("v//vn").
            if(slash2 > slash1 + 1) {
              if(!parseInt(token.substr(slash1 + 1, slash2 - slash1 - 1), ti)) { 
                faceOk = false; 
                break; 
              }
            }
            std::string nStr = token.substr(slash2 + 1);
            if(!nStr.empty()) {
              if(!parseInt(nStr, ni)) { 
                faceOk = false; 
                break; 
              }
            }
          }
        }

        // Resolve to absolute 0-based indices. A position index that doesn't
        // point at a real vertex makes the face unusable; a missing/invalid
        // uv or normal is tolerated (the default is used, normalized to -1).
        int rvi = resolveIndex(vi, positions.size());
        if(rvi < 0 || rvi >= static_cast<int>(positions.size())) { 
          faceOk = false; 
          break; 
        }
        int rti = resolveIndex(ti, texCoords.size());
        if(rti < 0 || rti >= static_cast<int>(texCoords.size())) rti = -1;
        int rni = resolveIndex(ni, normals.size());
        if(rni < 0 || rni >= static_cast<int>(normals.size())) rni = -1;

        // Reuse the vertex if this resolved corner was already emitted.
        std::string key = std::to_string(rvi) + "/" +
                          std::to_string(rti) + "/" +
                          std::to_string(rni);
        auto it = uniqueVerts.find(key);
        if(it != uniqueVerts.end()) {
          faceIdx.push_back(it->second);
          continue;
        }

        Vertex v;
        v.position = positions[rvi];
        if(rti >= 0) v.texCoord = texCoords[rti];
        if(rni >= 0) v.normal = normals[rni];
        else missingAnyNormal = true;

        GLuint newIndex = static_cast<GLuint>(verts.size());
        verts.push_back(v);
        uniqueVerts.emplace(key, newIndex);
        faceIdx.push_back(newIndex);
      }

      if(!faceOk) continue; // skip the malformed face, keep loading the rest

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

  // If any vertex is missing a normal (the file had none, or some faces omitted
  // them), synthesize smooth ones: accumulate each triangle's area-weighted face
  // normal onto its three vertices, then normalize. This overwrites any normals
  // the file did provide, trading authored hard edges for a consistent result --
  // acceptable since a partially-normalled mesh would otherwise have black faces.
  if(missingAnyNormal && !verts.empty()) {
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
