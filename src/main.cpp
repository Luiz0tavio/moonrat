#include <functional>
#include <memory>
#include <SDL2/SDL_image.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "renderer/Window.hpp"
#include "renderer/BulkText.hpp"
#include "events/Input.hpp"
#include "renderer/Object3D.hpp"
#include "renderer/BulkObject3D.hpp"
#include "renderer/Player.hpp"
#include "renderer/NPC.hpp"
#include "renderer/Projectiles.hpp"

void update() {

}

int main(int argc, char *argv[]) {
    Memory::Provider::initPools();

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Could not initialize sdl2: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) == 0) {
        std::cerr << "Could not initialize IMG's flags" << std::endl;
        return EXIT_FAILURE;
    }

    {
        SDL_DisplayMode DM;
        SDL_GetCurrentDisplayMode(0, &DM);
        std::array<int, 2> window_default_size = {DM.w, DM.h};

        auto window = std::make_unique<Renderer::Window>("Moonrat", window_default_size);

        auto mainContext = SDL_GL_CreateContext(window->getWindow());

        GLenum err = glewInit();
        if (err != GLEW_OK) {
            std::cerr << "Glew is not ok =(" << std::endl;
            return EXIT_FAILURE;
        }

        if (!GLEW_VERSION_2_0) {
            std::cerr << "Error: your graphic card does not support OpenGL 2.0" << std::endl;
            return EXIT_FAILURE;
        }

        GLfloat white_color[4]{1.f, 1.f, 1.f, 1.f};
        auto text_test = new Renderer::Text(-1.f, -1.f, 48, white_color);
        text_test->setText("Test Bottom Left");
        Renderer::BulkText::getInstance().push_back(text_test);

        auto ground = new Renderer::Object3D(glm::vec3(0.f));
        ground->loadTexture("./data/environment/ground.jpg", GL_RGB);

        float groundScale = 100.f;

        auto groundMesh = new Mesh();
        groundMesh->texture_index = 0;
        groundMesh->vertices = {};
        groundMesh->vertices.push_back({ 1.0f * groundScale,  1.0f * groundScale, .0f, 100.0f,   0.0f});
        groundMesh->vertices.push_back({-1.0f * groundScale, -1.0f * groundScale, .0f,   0.0f, 100.0f});
        groundMesh->vertices.push_back({-1.0f * groundScale,  1.0f * groundScale, .0f,   0.0f,   0.0f});

        groundMesh->vertices.push_back({ 1.0f * groundScale,  1.0f * groundScale, .0f, 100.0f,   0.0f});
        groundMesh->vertices.push_back({-1.0f * groundScale, -1.0f * groundScale, .0f,   0.0f, 100.0f});
        groundMesh->vertices.push_back({ 1.0f * groundScale, -1.0f * groundScale, .0f, 100.0f, 100.0f});

        ground->addMesh(groundMesh);

        Renderer::BulkObject3D::getInstance().push_back(ground);

        auto player = new Renderer::Player(glm::vec3(0.f));
        player->importFromFile("./data/players/", "robot_1.fbx", {GL_RGB});
        player->transformModel(glm::scale(glm::mat4(1.0f), glm::vec3(0.025f)));
        player->transformVertices(glm::rotate(glm::radians(90.f), glm::vec3(1.f, .0f, .0f)));
        player->transformVertices(glm::rotate(glm::radians(180.f), glm::vec3(.0f, .0f, 1.f)));

        Renderer::BulkObject3D::getInstance().push_back(player);

        auto camera = new Renderer::Camera(
            Renderer::BulkObject3D::getInstance().GetShaderProgram(),
            window_default_size,
            player->getPosition()
        );

        auto npc = new Renderer::NPC(glm::vec3(20.f, 30.f, 0.f));
        npc->importFromFile("./data/mobs/spider/", "with_texture.dae", {GL_RGBA, GL_RGB});
        npc->transformModel(glm::scale(glm::mat4(1.0f), glm::vec3(0.025f)));
        Renderer::BulkObject3D::getInstance().push_back(npc);

        glAlphaFunc(GL_GREATER, 0.5);
        glEnable(GL_ALPHA_TEST);

        glDepthFunc(GL_LESS);
        glEnable(GL_DEPTH_TEST);


        auto loop = [&]() -> bool {
            auto start = SDL_GetTicks();

            // Set screen to black
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            update();

            Renderer::BulkText::getInstance().draw(camera);
            Renderer::BulkObject3D::getInstance().draw(camera);

            Renderer::Projectiles::getInstance().timeTick(player, npc);
            npc->timeTick(player);

            auto quit = Events::Input::getInstance().HandleEvent(camera, player);
            if (quit) return false;

            // Swap Window
            SDL_GL_SwapWindow(window->getWindow());

            // Adjust FPS
            if (1000 / 60 > (SDL_GetTicks() - start)) {
                SDL_Delay(1000 / 60 - (SDL_GetTicks() - start));
            }

            return true;
        };

        while (loop());

        SDL_GL_DeleteContext(mainContext);
    }

    Memory::Provider::destroyPools();
    SDL_Quit();
    glEnd();

    return EXIT_SUCCESS;
}
