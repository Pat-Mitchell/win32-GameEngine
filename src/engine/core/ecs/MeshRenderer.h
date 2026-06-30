/// @file MeshRenderer.h
/// @brief Component: the GPU mesh + shader an entity is drawn with.

#pragma once

class Mesh;
class Shader;

// Non-owning handles: the Mesh/Shader are created and owned elsewhere (the app),
// and must outlive any entity that references them. An entity needs both this and
// a Transform to be drawn by the RenderSystem.
struct MeshRenderer {
  Mesh* mesh = nullptr;
  Shader* shader = nullptr;
};
