#include "gl_helper.hpp"

#include "model.hpp"


glm::mat4x4 MCModel::model(const glm::mat4x4 &parent_model) const {
    glm::mat4x4 a = glm::identity<glm::mat4x4>();
    a = glm::translate(a, pos);
    a = glm::scale(a, size);
    a = parent_model * a;

    return a;
}
void MCModel::assign_parent(const MCModel  &parent) {
    this->parent_id = parent.id;
    glm::vec3 old_pos = pos;

    glm::vec3 new_pos = parent.pos - old_pos;
}

MCModel::MCModel(uint32_t id, uint32_t pid, glm::vec3 size, glm::vec3 pos, glm::vec3 rot) 
    : size(size), pos(pos), rotation(rot), id(id), parent_id(pid) {
    assert(id != parent_id);
}
Mesh MCModel::generate() {
    return CreateCubeMesh(size, pos);
}
std::optional<MCModel> MCModel::deserialize(std::istream &is) {
    try {
        uint32_t id = read_binary<uint32_t>(is);
        uint32_t parent_id = read_binary<uint32_t>(is);

        glm::vec3 size = {
            read_binary<float>(is),
            read_binary<float>(is),
            read_binary<float>(is),
        };
        glm::vec3 pos = {
            read_binary<float>(is),
            read_binary<float>(is),
            read_binary<float>(is),
        };
        glm::vec3 rot = {
            read_binary<float>(is),
            read_binary<float>(is),
            read_binary<float>(is),
        };

        MCModel model(id, parent_id, size, pos, rot);

        for (auto &uv : model.uvs) {
            uv = glm::vec4(
                read_binary<float>(is),
                read_binary<float>(is),
                read_binary<float>(is),
                read_binary<float>(is)
            );
        }

        auto anim_count = read_binary<uint32_t>(is);
        assert(anim_count == 0);

        return model;
    } catch (std::ios_base::failure &error) {
        return std::nullopt;
    }
}


MCModel::MCModel()
    : size(1, 1, 1), pos(0, 0, 0), rotation(0, 0, 0) {
    
}

void MCModel::serialize(std::ostream &os) const {
    write_binary<uint32_t>(os, id);
    write_binary<uint32_t>(os, parent_id);

    write_binary<float>(os, size.x);
    write_binary<float>(os, size.y);
    write_binary<float>(os, size.z);

    write_binary<float>(os, pos.x);
    write_binary<float>(os, pos.y);
    write_binary<float>(os, pos.z);
    
    write_binary<float>(os, rotation.x);
    write_binary<float>(os, rotation.y);
    write_binary<float>(os, rotation.z);

    for (const auto &uv : uvs) {
        write_binary<float>(os, uv.x);
        write_binary<float>(os, uv.y);
        write_binary<float>(os, uv.z);
        write_binary<float>(os, uv.w);
    }

    write_binary<uint32_t>(os, 0);
}

struct MCMModelGraph {
    const MCModel *model;
    std::vector<MCMModelGraph> children;

    MCMModelGraph(const std::unordered_map<uint32_t, MCModel> &models)
        : model(nullptr) {
        if (models.size() == 0) {
            return;
        }

        std::unordered_multimap<uint32_t, const MCModel&> parent_to_children;
        for (const auto &[id, model] : models) {
            assert(model.parent_id == NULL_ID || models.find(model.parent_id) != models.cend());
            parent_to_children.insert({ model.parent_id, model });
        }

        auto root_iter = parent_to_children.equal_range(NULL_ID);
        for (auto root_child = root_iter.first; root_child != root_iter.second; ++root_child) {
            MCMModelGraph child;
            child.model = &root_child->second;
            child.generate_children(parent_to_children, root_child->second.id);
            children.emplace_back(std::move(child));
        }
    }
    MCMModelGraph() {

    }
    void generate_children(const std::unordered_multimap<uint32_t, const MCModel&> &parent_to_children, uint32_t parent_id) {
        assert(this->model != nullptr);

        auto root_iter = parent_to_children.equal_range(parent_id);
        for (auto root_child = root_iter.first; root_child != root_iter.second; ++root_child) {
            MCMModelGraph child;
            child.model = &root_child->second;
            child.generate_children(parent_to_children, root_child->second.id);
            children.emplace_back(std::move(child));
        }
    }

};



ModelRenderer::ModelRenderer() 
    : shader("shaders/model.vert", "shaders/model.frag"), camera({ 0, 0, 0 }), unit_cube_mesh(CreateCubeMesh({ 1.f, 1.f, 1.f }, { 0.f, 0.f, 0.f })) {
     
    camera.pos(camera.pos() - camera.forward() * 10.f);

    ASSERT_ON_GL_ERROR();

    glGenBuffers(1, &globals_3d_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);ASSERT_ON_GL_ERROR();
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3, nullptr, GL_STATIC_DRAW);ASSERT_ON_GL_ERROR();

    glm::mat4 view = camera.view_matrix();ASSERT_ON_GL_ERROR();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));ASSERT_ON_GL_ERROR();

    glm::uvec2 win_size = GetTrueWindowSize();
    glm::mat4 projection = glm::perspective(static_cast<float>(M_PI / 4), static_cast<float>(win_size.x) / static_cast<float>(win_size.y), 0.1f, 100.f);ASSERT_ON_GL_ERROR();
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));ASSERT_ON_GL_ERROR();

    glm::mat4 model = glm::transpose(glm::identity<glm::mat4>());
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::mat4), glm::value_ptr(model));


    ASSERT_ON_GL_ERROR();

    this->shader.bind_UBO("globals_3d", 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, globals_3d_ubo);

    ASSERT_ON_GL_ERROR();
}

void ModelRenderer::draw(const std::unordered_map<uint32_t, MCModel> &models, Texture *texture) {
    ASSERT_ON_GL_ERROR();
     
    glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
    glm::mat4 view = camera.view_matrix();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));


    glm::uvec2 win_size = GetTrueWindowSize();
    glm::mat4 projection = glm::perspective(static_cast<float>(M_PI / 4), static_cast<float>(win_size.x) / static_cast<float>(win_size.y), 0.1f, 100.f);ASSERT_ON_GL_ERROR();
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));ASSERT_ON_GL_ERROR();

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    ASSERT_ON_GL_ERROR();


    MCMModelGraph graph(models);

    if (texture != nullptr) {
        shader.bind_texture_to_sampler_2D({
            { "textures[0]", *texture }
        });
        shader.use();
    }

    for (size_t i = graph.children.size(); i > 0;) {
        --i;

        draw(graph.children[i], glm::identity<glm::mat4>());
    }
 
    shader.use();
}


void ModelRenderer::draw(const MCMModelGraph &graph, glm::mat4 parent) {
    ASSERT_ON_GL_ERROR();
     
    glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
    glm::mat4 model_mat = graph.model->model(parent);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model_mat));

    glm::mat4 view = camera.view_matrix();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));


    glm::uvec2 win_size = GetTrueWindowSize();
    glm::mat4 projection = glm::perspective(static_cast<float>(M_PI / 4), static_cast<float>(win_size.x) / static_cast<float>(win_size.y), 0.1f, 100.f);ASSERT_ON_GL_ERROR();
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));ASSERT_ON_GL_ERROR();

    unit_cube_mesh.rebuild(CreateCubeMesh({ 1.f, 1.f, 1.f }, { 0.f, 0.f, 0.f }, graph.model->uvs));

    unit_cube_mesh.draw();

    for (size_t i = graph.children.size(); i > 0;) {
        --i;

        draw(graph.children[i], parent);
    }

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    ASSERT_ON_GL_ERROR();

}
