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

void update()
{

}

int main(int argc, char* argv[])
{
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

        GLfloat white_color[4] {1.f, 1.f, 1.f, 1.f};
        auto text_test = new Renderer::Text(-1.f, -1.f, 48, white_color);
        text_test->setText("Test Bottom Left");
        Renderer::BulkText::getInstance().push_back(text_test);

        glAlphaFunc(GL_GREATER, 0.5);
        glEnable(GL_ALPHA_TEST);

        auto wolf = new Renderer::Object3D("./data/mobs/wolf/Wolf_dae.dae");
        Renderer::BulkObject3D::getInstance().push_back(wolf);

        auto loop = [&]() -> bool {

            auto start = SDL_GetTicks();

            // Set screen to black
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            update();

            Renderer::BulkText::getInstance().draw(window_default_size);
            Renderer::BulkObject3D::getInstance().draw();

            auto quit = Events::Input::getInstance().HandleEvent();
            if (quit) return false;

            // Swap Window
            SDL_GL_SwapWindow(window->getWindow());

            // Adjust FPS
            if (1000/60 > (SDL_GetTicks() - start)) {
                SDL_Delay(1000/60 - (SDL_GetTicks() - start));
            }

            return true;
        };

        while(loop());

        SDL_GL_DeleteContext(mainContext);
    }

    Memory::Provider::destroyPools();
    SDL_Quit();
    glEnd();

    return EXIT_SUCCESS;
}