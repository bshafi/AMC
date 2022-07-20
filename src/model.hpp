#pragma once

#include "standard.hpp"
#include "mesh.hpp"
#include "camera.hpp"

#define NULL_ID 0
struct MCModelPart {
    glm::vec3 size;
    glm::vec3 pos;
    glm::vec3 rotation;

    std::array<glm::vec4, 6> uvs;

    uint32_t id;
    uint32_t parent_id;

    glm::mat4x4 model_matrix(const glm::mat4x4 &parent_model = glm::identity<glm::mat4x4>()) const;

    Mesh generate();

    void assign_parent(const MCModelPart  &parent);

    MCModelPart();
    MCModelPart(uint32_t id, uint32_t pid = 0, glm::vec3 size = { 1.f, 1.f, 1.f }, glm::vec3 pos = { 0.f, 0.f, 0.f }, glm::vec3 rot = { 0.f, 0.f, 0.f});

    static std::optional<MCModelPart> deserialize(std::istream &is);
    void serialize(std::ostream &os) const;
};

struct FullMCModel {
    std::unordered_map<uint32_t, MCModelPart> models;
};


struct MCMModelGraph;

struct ModelRenderer {
    ModelRenderer();
    void draw(const std::unordered_map<uint32_t, MCModelPart> &models, Texture *texture = nullptr);
    void draw(const MCMModelGraph &, glm::mat4 parent);

    Shader shader;
    uint32_t globals_3d_ubo;
    Camera camera;
    MeshBuffer unit_cube_mesh;
};

