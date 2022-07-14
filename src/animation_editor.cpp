

#include "gl_helper.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

int main() {
    SDL_Window *window = Init_SDL_and_GL("Animation Editor", 1280, 720);

    assert(window);

    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    glViewport(0, 0, width, height);

    ASSERT_ON_GL_ERROR();


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
            events.push_back(event);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
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