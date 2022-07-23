#include <stack>
#include <functional>
#include <fstream>

#include "portable-file-dialogs.h"

#include "shader.hpp"
#include "camera.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "hello_cube.hpp"
#include "gl_helper.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include "imgui_stdlib.h"

namespace ImGui {
    void InputString(const char *, std::string &s);
}

struct KeyFrame {
    float time;
    uint32_t cube_id;
};

struct Animation {
    std::vector<KeyFrame> key_frames;
    float length;
};


int main() {
    SDL_Window *window = Init_SDL_and_GL("Animation Editor", 1280, 720);


    ASSERT_ON_GL_ERROR();

    ModelRenderer renderer;

    ASSERT_ON_GL_ERROR();

    int model_id = NULL_ID;
    std::unordered_map<uint32_t, MCModelPart> model_groups;

    std::optional<std::string> path;
    std::optional<Texture> texture;

    std::unordered_map<std::string, Animation> animations;

    assert(window);

    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    glViewport(0, 0, width, height);

    ASSERT_ON_GL_ERROR();

    uint32_t globals_3d_ubo = generate_ubo(renderer.shader, renderer.camera.view_matrix());
    
    ASSERT_ON_GL_ERROR();

    uint32_t ticks = SDL_GetTicks();
    bool is_running = true;
    std::vector<SDL_Event> events;
    uint32_t delta_ticks = 1000 / FPS;
    while (is_running) {
        events.clear();
        for (SDL_Event event = {}; SDL_PollEvent(&event);) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type) {
            case SDL_QUIT: 
                is_running = false;
                break;
            default:
                if (event.type == SCENE_CHANGE_EVENT) {
                    SceneChangeData *scd = static_cast<SceneChangeData*>(event.user.data1);
                    assert(scd);
                    delete scd;
                }
            break;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_F1) {
                SDL_SetRelativeMouseMode(SDL_GetRelativeMouseMode() == SDL_TRUE ? SDL_FALSE : SDL_TRUE);
            }
            if (event.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode() == SDL_TRUE) {
                renderer.camera.rotate_right(0.1f * 0.25f * M_PI * event.motion.xrel / 30.f);
                renderer.camera.rotate_upwards(0.1f * -0.25f * M_PI * event.motion.yrel  / 30.f);
            }
            events.push_back(event);
        }
        {
            auto keystate = SDL_GetKeyboardState(nullptr);
            vec2 delta_move = { 0, 0 };
            if (keystate[SDL_SCANCODE_LEFT]) {
                delta_move.x -= 1.f;
            }
            if (keystate[SDL_SCANCODE_RIGHT]) {
                delta_move.x += 1.f;
            }
            if (keystate[SDL_SCANCODE_DOWN]) {
                delta_move.y -= 1.f;
            }
            if (keystate[SDL_SCANCODE_UP]) {
                delta_move.y += 1.f;
            }
            if (glm::length(delta_move) > 0.001f) {
                delta_move = delta_move / glm::length(delta_move);
            }
            delta_move = delta_move / 30.f;
            renderer.camera.pos(renderer.camera.pos() + delta_move.x * renderer.camera.right() + delta_move.y * renderer.camera.forward());
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Open", "Ctrl+O")) {
                        auto res = pfd::open_file("Open mcmodel", "", { ".mcmodel" });
                        auto results = res.result();
                        if (results.size() > 0) {
                            EditableMCModel full(results[0]);
                            model_groups = full.models;
                            for (const auto &[id, model] : model_groups) {
                                model_id = std::max(model_id, static_cast<int>(id));
                            }
                        }
                    }
                    if (ImGui::MenuItem("Save", "Ctrl+S")) {
                        if (path == std::nullopt) {
                            auto res = pfd::save_file("Save mcmodel", "", { ".mcmodel" });
                            path = res.result();
                            if (path->size() == 0) {
                                path = std::nullopt;
                            }
                        }
                        if (path != std::nullopt) {
                            std::ofstream fostream(*path, std::ios_base::trunc | std::ios_base::binary);
                            fostream.exceptions(~std::ios_base::goodbit);
                            for (const auto &[id, model] : model_groups) {
                                model.serialize(fostream);
                            }
                        }
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            ImGui::Begin("Scene");

            static bool wireframe = false;
            if (ImGui::Button("Toggle wireframe")) {
                wireframe = !wireframe;
                if (wireframe) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                } else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
            }

            if (ImGui::Button("Add Cube")) {
                ++model_id;
                model_groups.emplace(model_id, MCModelPart(model_id, 0));
            }
            static uint32_t selected_cube = NULL_ID;

            if (ImGui::Button("Delete") && model_groups.find(selected_cube) != model_groups.end()) {
                model_groups.erase(selected_cube);
            }
            if (ImGui::Button("Duplicate") && model_groups.find(selected_cube) != model_groups.end()) {
                ++model_id;
                MCModelPart dup = model_groups[selected_cube];
                dup.id = model_id;
                model_groups.emplace(dup.id, dup);
            }


            if (model_groups.find(selected_cube) == model_groups.end()) {
                selected_cube = NULL_ID;
            }

            if (selected_cube != NULL_ID) {
                assert(model_groups.find(selected_cube) != model_groups.end());

                auto &group = model_groups[selected_cube];

                char label[128];
                sprintf(label, "MyObject %d", selected_cube);

                ImGui::InputInt("parent_id", reinterpret_cast<int*>(&group.parent_id));
                if (group.parent_id == group.id || model_groups.find(group.parent_id) == model_groups.end()) {
                    group.parent_id = NULL_ID;
                } else {
                    group.assign_parent(model_groups[group.parent_id]);
                }

                //ImGui::BeginChild(label);
                ImGui::DragFloat3("size", &group.size.x);
                //ImGui::EndChild();

                ImGui::DragFloat3("pos", &group.pos.x);
            }
            {
                ImGui::BeginChild("ModelGroups");
                for (const auto &[id, model_group] : model_groups) {
                    char label[128];
                    sprintf(label, "MyObject %d", id);
                    if (ImGui::Selectable(label, selected_cube == id)) {
                        selected_cube = id;
                    }
                }

                ImGui::EndChild();
            }
            
            ImGui::End();

            ImGui::Begin("UV Editor");
            if (ImGui::Button("Open Image")) {
                auto results = pfd::open_file("Open Image").result();
                if (results.size() > 0) {
                    texture.emplace(Texture(results[0]));
                }
            }
            static int selected_face = 0;
            if (model_groups.find(selected_cube) != model_groups.end()) {
                auto &group = model_groups[selected_cube];

                for (int i = 0; i < 6; ++i) {
                    const char *labels[6] = {
                        "Top",
                        "Bottom",
                        "Front",
                        "Back",
                        "Right",
                        "Left",
                    };
                    if (ImGui::Selectable(labels[i], selected_face == i)) {
                        selected_face = i;
                    }
                }
                float iter_amount_x = 0.1f;
                float iter_amount_y = 0.1f;
                if (texture != std::nullopt) {
                    iter_amount_x = 1.f / texture->width();
                    iter_amount_y = 1.f / texture->height();
                }
                ImGui::DragFloat("face rect: x", &group.uvs[selected_face].x, iter_amount_x, 0.f, 1.f);
                ImGui::DragFloat("face rect: y", &group.uvs[selected_face].y, iter_amount_y, 0.f, 1.f);
                ImGui::DragFloat("face rect: w", &group.uvs[selected_face].z, iter_amount_x, 0.f, 1.f);
                ImGui::DragFloat("face rect: h", &group.uvs[selected_face].w, iter_amount_y, 0.f, 1.f);
            }
            auto draw_list = ImGui::GetWindowDrawList();
            if (texture.has_value()) {
                ImVec2 pos = ImGui::GetCursorScreenPos();
                float scale = std::min(400.f / texture->width(), 400.f / texture->height());
                auto pos1 = ImVec2(pos.x + texture->width() * scale, pos.y + texture->height() * scale);
                draw_list->AddImage(reinterpret_cast<ImTextureID>(texture->get_id()), pos, pos1);
                if (model_groups.find(selected_cube) != model_groups.end()) {
                    auto &group = model_groups[selected_cube];
                    auto w_h = ImVec2(pos1.x - pos.x, -pos1.y + pos.y);
                    auto p0 = ImVec2(pos.x + w_h.x * group.uvs[selected_face].x, pos.y - w_h.y * group.uvs[selected_face].y);
                    auto p1 = ImVec2(p0.x + w_h.x * group.uvs[selected_face].z, p0.y - w_h.y * group.uvs[selected_face].w);
                    draw_list->AddRect(p0, p1, 0xffffffff);
                }
                ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + texture->height() * scale));
            }

            ImGui::End();

            static std::string selected_animation = "";
            ImGui::Begin("Animation Editor");
            auto tl = ImGui::GetCursorScreenPos();

            auto win_size = ImGui::GetWindowSize();
            ImGui::BeginChild("Animations", ImVec2(std::min<int>(200, win_size.x / 5), 0));
            if (ImGui::Button("Add Animation")) {
                animations[std::to_string(animations.size())] = Animation{};
            }
            if (animations.find(selected_animation) != animations.end()) {
                auto old_selected_animation = selected_animation;
                ImGui::InputText("name", &selected_animation);
                if (old_selected_animation != selected_animation) {
                    Animation anim = animations[old_selected_animation];
                    animations.erase(old_selected_animation);
                    animations[selected_animation] = anim;
                }
            }
            for (const auto &[name, animation] : animations) {
                if (ImGui::Selectable(name.c_str(), name == selected_animation)) {
                    selected_animation = name;
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();

            static float time_cursor = 0.f;
            if (animations.find(selected_animation) != animations.end()) {
                auto &anim = animations[selected_animation];
                tl = ImGui::GetCursorScreenPos();
                
                ImGui::BeginChild("Animation Info", ImVec2(std::min<int>(200, win_size.x / 5), 0));
                ImGui::DragFloat("length", &anim.length);

                ImGui::EndChild();
                ImGui::SameLine();
            }
            
            auto br = ImVec2(tl.x + win_size.x - 20, tl.y + win_size.y - 30);
            tl = ImGui::GetCursorScreenPos();
            draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRect(tl, br, 0xffffffff);

            ImGui::End();
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        Texture *tex = nullptr;
        if (texture != std::nullopt) {
            tex = &*texture;
        }
        update_ubo_matrices(globals_3d_ubo, renderer.camera.view_matrix());

        renderer.draw(model_groups, tex);

        ASSERT_ON_GL_ERROR();

        ImGui::Render();
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);

        delta_ticks = SDL_GetTicks() - ticks;
        if (delta_ticks  < 1000 / FPS) {
            SDL_Delay((1000 / FPS) - delta_ticks);
        }
        delta_ticks = SDL_GetTicks() - ticks;
        ticks = SDL_GetTicks();
    }

    SDL_DestroyWindow(window);

    Quit_SDL_and_GL();
    return 0;
}


