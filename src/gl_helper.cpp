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
void PushSceneChangeEvent(const SceneChangeData &h_scene_change_data) {
    auto *scene_change_data = new SceneChangeData(h_scene_change_data);

    SDL_Event event;
    SDL_zero(event);

    event.type = SCENE_CHANGE_EVENT;
    event.user.data1 = static_cast<void*>(scene_change_data);
    if (SDL_PushEvent(&event) != 1) {
        printf("Could not push scene change event\n");
    }
}

unsigned int LoadShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) {
    std::ifstream vertexShaderFile(vertexShaderPath), 
                  fragmentShaderFile(fragmentShaderPath);
    if (vertexShaderFile.bad() || !vertexShaderFile.is_open()) {
        if (fragmentShaderFile.bad() || !fragmentShaderFile.is_open()) {
            throw std::invalid_argument("Could not find files at path " + vertexShaderPath + " or " + fragmentShaderPath);
        } else {
            throw std::invalid_argument("Could not find file at path " + vertexShaderPath);
        }
    }
    if (fragmentShaderFile.bad() || !fragmentShaderFile.is_open()) {
        throw std::invalid_argument("Cound not find file at path " + fragmentShaderPath);
    }

    // Reads the entire file to string
    std::string vertexShaderStr = std::string(std::istreambuf_iterator<char>(vertexShaderFile), std::istreambuf_iterator<char>());
    std::string fragmentShaderStr = std::string(std::istreambuf_iterator<char>(fragmentShaderFile), std::istreambuf_iterator<char>());

    const char *vertexShaderSource = vertexShaderStr.c_str();
    const char *fragmentShaderSource = fragmentShaderStr.c_str();

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int  success;
    std::string infoLog;
    infoLog.resize(512);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog.data());
        std::cout << "[VERTEX::COMPILATION_FAILED]" << infoLog << std::endl;
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog.data());
        std::cout << "[FRAGMENT::COMPILATION_FAILED]" << infoLog << std::endl;;
    }

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog.data());
        
        std::cout << "[PROGRAM::COMPILATION_FAILED]" << infoLog << std::endl;;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

uint32_t LoadImage(const std::string &imagePath, uint32_t *width, uint32_t *height) {
    SDL_Surface *original = IMG_Load(imagePath.c_str());
    SDL_Surface *modified = SDL_CreateRGBSurfaceWithFormat(0, original->w, original->h, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_BlitSurface(original, nullptr, modified, nullptr);
    SDL_FreeSurface(original);


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