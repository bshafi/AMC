#include <array>
#include <fstream>
#include <iostream>
#include <mutex>

#include "gl_helper.hpp"
#include "gui.hpp"

#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

constexpr uint32_t DEFAULT_SDL_INIT_FLAGS = SDL_INIT_EVERYTHING;
constexpr uint32_t DEFAULT_IMG_INIT_FLAGS = IMG_INIT_PNG;
constexpr uint32_t DEFAULT_FONT_SIZE = 24;
constexpr SDL_Color WHITE = { 255, 255, 255, 255 };

TTF_Font *default_font = nullptr;

uint32_t SCENE_CHANGE_EVENT;

struct FilterData {
    std::mutex filter_mutex;
    glm::uvec2 fake_window_bounds = glm::uvec2(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
    glm::uvec2 true_window_bounds = glm::uvec2(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
    SDL_GLContext context;
} filter_data;

SDL_Window *Init_SDL_and_GL() {
    assert(SDL_Init(DEFAULT_SDL_INIT_FLAGS) == 0);
    assert(IMG_Init(DEFAULT_IMG_INIT_FLAGS) == DEFAULT_IMG_INIT_FLAGS);
    assert(TTF_Init() == 0);

    default_font = TTF_OpenFont("resources/cmuntt.ttf", DEFAULT_FONT_SIZE);
    assert(default_font);

    assert(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) == 0);
    assert(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2) == 0);
    assert(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) == 0);
    assert(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG) == 0);

    assert(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) == 0);
    assert(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24) == 0);
    assert(SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8) == 0);

    //SDL_SetHint(SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG, "1");
    assert(SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1") == SDL_TRUE);

    SCENE_CHANGE_EVENT = SDL_RegisterEvents(1);
    assert(SCENE_CHANGE_EVENT != (uint32_t)-1);


    SDL_Window *window = SDL_CreateWindow(
        "Another Minecraft Clone", 
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 
        INITIAL_WINDOW_WIDTH,
        INITIAL_WINDOW_HEIGHT, 
        DEFAULT_SDL_WINDOW_FLAGS
    );

    filter_data.filter_mutex.lock();
    {
        int w, h;
        SDL_GL_GetDrawableSize(window, &w, &h);
        filter_data.true_window_bounds = glm::uvec2(static_cast<uint32_t>(w), static_cast<uint32_t>(h));

        filter_data.context = SDL_GL_CreateContext(window);
        assert(filter_data.context);

        assert(glewInit() == GLEW_OK);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(135 / 255.f, 206 / 255.f,235 / 255.f, 1.0f);

    }
    filter_data.filter_mutex.unlock();

    // Using SDL_SetEventFilter instead of SDL_AddEventWatch
    // Even thought SDL_AddEventWatch allows smooth resizing behavior the
    // screen would have to be redrawn forcing global data access overcomplicating the project
    
    SDL_SetEventFilter(
        [](void *userdata, SDL_Event *event){
            FilterData *filter_data = static_cast<FilterData*>(userdata);
            filter_data->filter_mutex.lock();

            *event = filter_events(filter_data->fake_window_bounds, filter_data->true_window_bounds, *event);

            filter_data->filter_mutex.unlock();
            return 1;
        },
        &filter_data
    );

    filter_data.filter_mutex.lock();
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, filter_data.context);
    ImGui_ImplOpenGL3_Init("#version 150");

    filter_data.filter_mutex.unlock();

    Renderer::Initialize();

    return window;
}
void Quit_SDL_and_GL() {

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    Renderer::Terminate();

    filter_data.filter_mutex.lock();
    SDL_GL_DeleteContext(filter_data.context);
    filter_data.filter_mutex.unlock();

    TTF_CloseFont(default_font);

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

SDL_Event filter_events(glm::uvec2 &fake_window_bounds, glm::uvec2 &window_bounds, const SDL_Event &event) {
    SDL_Event true_event = event;
    switch (event.type) {
    case SDL_MOUSEMOTION: {
        const glm::vec2 fake_rel = glm::vec2(event.motion.xrel, event.motion.yrel);
        const glm::vec2 fake_pos = glm::vec2(event.motion.x, event.motion.y);
        const glm::vec2 true_pos = (fake_pos * glm::vec2(window_bounds)) / glm::vec2(fake_window_bounds);
        const glm::vec2 true_rel = true_pos - ((fake_pos - fake_rel) * glm::vec2(window_bounds)) / glm::vec2(fake_window_bounds);

        true_event.motion.x = static_cast<int>(true_pos.x);
        true_event.motion.y = static_cast<int>(true_pos.y);

        true_event.motion.xrel = static_cast<int>(true_rel.x);
        true_event.motion.yrel = static_cast<int>(true_rel.y);
    }
        break;
    case SDL_MOUSEBUTTONDOWN: // continue;
    case SDL_MOUSEBUTTONUP: {
        const glm::vec2 fake_pos = glm::vec2(event.button.x, event.button.y);
        const glm::vec2 true_pos = (fake_pos * glm::vec2(window_bounds)) / glm::vec2(fake_window_bounds);
        true_event.button.x = static_cast<int>(true_pos.x);
        true_event.button.y = static_cast<int>(true_pos.y);
    }
        break;
    case SDL_WINDOWEVENT: {
        switch (event.window.event) {
        case SDL_WINDOWEVENT_RESIZED: {
            SDL_Window *window = SDL_GetWindowFromID(event.window.windowID);
            assert(window);

            fake_window_bounds.x = event.window.data1;
            fake_window_bounds.y = event.window.data2;
            int w, h;
            SDL_GL_GetDrawableSize(window, &w, &h);
            glViewport(0, 0, w, h);
            window_bounds = glm::uvec2(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
            true_event.window.data1 = static_cast<int32_t>(window_bounds.x);
            true_event.window.data2 = static_cast<int32_t>(window_bounds.y);
        }
            break;
        default:
            break;
        }
    }
        break;
    default:
        break;
    }
    return true_event;
}
void PushSceneChangeEvent(const SceneChangeData &_scene_change_data) {
    auto *scene_change_data = new SceneChangeData(_scene_change_data);

    SDL_Event event;
    SDL_zero(event);

    event.type = SCENE_CHANGE_EVENT;
    event.user.data1 = static_cast<void*>(scene_change_data);
    if (SDL_PushEvent(&event) != 1) {
        printf("Could not push scene change event\n");
    }
}

std::optional<uint32_t> LoadShader(const std::string &shader_path, const uint32_t &type) {
    assert(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER || type == GL_GEOMETRY_SHADER);

    std::ifstream shader_file(shader_path);
    if (shader_file.bad() || !shader_file.is_open()) {
        std::cout << "File: " << shader_path << " could not be loaded" << std::endl;
        return std::nullopt;
    }

    std::string shader_str = std::string(std::istreambuf_iterator<char>(shader_file), std::istreambuf_iterator<char>());
    const char *shader_source = shader_str.c_str();

    uint32_t shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    const uint32_t BUFFER_SIZE = 512;
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        std::string info_log;
        info_log.resize(BUFFER_SIZE);
        glGetShaderInfoLog(shader, BUFFER_SIZE, NULL, info_log.data());
        std::cout << "[";
        switch (type) {
        case GL_VERTEX_SHADER:
            std::cout << "VERTEX";
            break;
        case GL_FRAGMENT_SHADER:
            std::cout << "FRAGMENT";
            break;
        case GL_GEOMETRY_SHADER:
            std::cout << "GEOMETRY";
            break;
        default:
            assert(false);
        }
        std::cout << "::COMPILATION_FAILED]" << info_log << std::endl;
        return std::nullopt;
    }

    return shader;
}

unsigned int LoadShaderProgram(const std::string &vertex_shader_path, const std::string &fragment_shader_path, const std::string &geometry_shader_path) {
    const bool using_geometry_shader = !geometry_shader_path.empty();
    
    std::optional<uint32_t> vertex_shader = LoadShader(vertex_shader_path, GL_VERTEX_SHADER);
    std::optional<uint32_t> fragment_shader = LoadShader(fragment_shader_path, GL_FRAGMENT_SHADER);
    std::optional<uint32_t> geometry_shader = std::nullopt;
    if (using_geometry_shader) {
        geometry_shader = LoadShader(geometry_shader_path, GL_GEOMETRY_SHADER);
    }

    std::optional<uint32_t> shader_program = std::nullopt;
    if (
        vertex_shader.has_value() &&
        fragment_shader.has_value() &&
        !(using_geometry_shader && !geometry_shader.has_value())
    ) {
        uint32_t program = glCreateProgram();
        glAttachShader(program, vertex_shader.value());
        glAttachShader(program, fragment_shader.value());
        if (using_geometry_shader) {
            glAttachShader(program, geometry_shader.value());
        }
        glLinkProgram(program);
        int success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if(!success) {
            std::string info_log;
            info_log.resize(512);
            glGetProgramInfoLog(program, 512, NULL, info_log.data());
            
            std::cout << "[PROGRAM::COMPILATION_FAILED]" << info_log << std::endl;
            glDeleteProgram(program);
        } else {
            shader_program = program;
        }
    }

    if (vertex_shader.has_value()) {
        glDeleteShader(vertex_shader.value());
    }
    if (fragment_shader.has_value()) {
        glDeleteShader(fragment_shader.value());
    }
    if (geometry_shader.has_value()) {
        glDeleteShader(geometry_shader.value());
    }

    if (!shader_program.has_value()) {
        throw "Invalid Shader program";
    }

    return shader_program.value();
}

SDL_Surface *IMG_LoadRGBA32(const std::string &path) {
    SDL_Surface *original = IMG_Load(path.c_str());
    assert(original);

    SDL_Surface *modified = SDL_CreateRGBSurfaceWithFormat(0, original->w, original->h, 32, SDL_PIXELFORMAT_RGBA32);
    assert(modified);
    SDL_BlitSurface(original, nullptr, modified, nullptr);
    SDL_FreeSurface(original);

    return modified;
}

uint32_t LoadImage(const std::string &imagePath, uint32_t *width, uint32_t *height) {
    SDL_Surface *modified = IMG_LoadRGBA32(imagePath);

    if (width != nullptr) {
        *width = modified->w;
    }
    if (height != nullptr) {
        *height = modified->h;
    }

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, modified->w, modified->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, modified->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(modified);

    return texture;

}

uint32_t LoadCubeMap(const std::string &right, const std::string &left, const std::string &top, const std::string &bottom, const std::string &back, const std::string &front) {
    const std::array<const std::string, 6> faces = { right, left, top, bottom, back, front };

    ASSERT_ON_GL_ERROR();

    uint32_t tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);
    for (uint32_t i = 0; i < faces.size(); ++i) {
        SDL_Surface *surface = IMG_LoadRGBA32(faces[i]);
        assert(surface);

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
            0, GL_RGB, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels
        );
        ASSERT_ON_GL_ERROR();

        SDL_FreeSurface(surface);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    ASSERT_ON_GL_ERROR();

    return tex_id;
}

uint32_t RasterizeText(const std::string &text) {
    assert(default_font);

    SDL_Surface *modified = TTF_RenderText_Blended(default_font, text.c_str(), WHITE);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, modified->w, modified->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, modified->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(modified);

    return texture;
}

bool glBreakOnError() {
    GLenum error_code = glGetError();
    if (error_code == GL_NO_ERROR) {
        return false;
    }

    // TODO: Make this more comprehesive and clean
    switch (error_code) {
    case GL_INVALID_ENUM: std::cout << "GL_INVALID_ENUM" << std::endl; break;
    case GL_INVALID_OPERATION: std::cout << "GL_INVALID_OPERATION" << std::endl; break;
    case GL_INVALID_VALUE: std::cout << "GL_INVALID_VALUE" << std::endl; break;
    default:
        std::cout << "GL unkown error " << reinterpret_cast<void*>(error_code) << std::endl;
        break;
    }

    return true;
}
glm::uvec2 GetTrueWindowSize() {
    filter_data.filter_mutex.lock();
    auto window_size = filter_data.true_window_bounds;
    filter_data.filter_mutex.unlock();

    return window_size;
}

// Assume that the gl types are the same as the cpp types
static_assert(std::is_same<uint32_t, GLuint>::value);
static_assert(std::is_same<int32_t, GLint>::value);
static_assert(std::is_same<float, GLfloat>::value);
static_assert(std::is_same<int, GLsizei>::value);
static_assert(std::is_same<GLenum, unsigned>::value);

// Assume the glm types are tightly packed
static_assert(sizeof(glm::vec3) == sizeof(std::array<float, 3>));
static_assert(sizeof(glm::vec2) == sizeof(std::array<float, 2>));
static_assert(sizeof(glm::ivec2) == sizeof(std::array<int32_t, 2>));
static_assert(sizeof(glm::ivec3) == sizeof(std::array<int32_t, 3>));
static_assert(sizeof(glm::uvec2) == sizeof(std::array<uint32_t, 2>));
static_assert(sizeof(glm::uvec3) == sizeof(std::array<uint32_t, 3>));