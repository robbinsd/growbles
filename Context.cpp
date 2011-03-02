#include "Context.h"

Context::Context() : windowSettings(24, 8, 2)
                   , window(sf::VideoMode(800, 600), "CS248 - Assignment 3",
                            sf::Style::Close, windowSettings)
                   , shader(SHADER_PATH)
{

    // Initialize GLEW on Windows, to make sure that OpenGL 2.0 is loaded
#ifdef FRAMEWORK_USE_GLEW
    GLint error = glewInit();
    if (GLEW_OK != error) {
        std::cerr << glewGetErrorString(error) << std::endl;
        exit(-1);
    }
    if (!GLEW_VERSION_2_0) {
        std::cerr << "This program requires OpenGL 2.0" << std::endl;
        exit(-1);
    }
#endif

    // Common defaults
    GL_CHECK(glClearDepth(1.0f));
    GL_CHECK(glClearColor(0.6f, 0.56f, 1.0f, 1.0f));
    GL_CHECK(glEnable(GL_DEPTH_TEST));

    // Initialize and use our shaders
    shader.Init();
    GL_CHECK(glUseProgram(shader.programID()));

    // Setup the view system
    SetupView();

    // Set up the shader
    SET_UNIFORM(this, 1i, "spriteMap", SPRITE_TEXTURE_SAMPLER);
    SET_UNIFORM(this, 1i, "diffuseMap", DIFFUSE_TEXTURE_SAMPLER);
    SET_UNIFORM(this, 1i, "specularMap", SPECULAR_TEXTURE_SAMPLER);
    SET_UNIFORM(this, 1i, "normalMap", NORMAL_TEXTURE_SAMPLER);
    SET_UNIFORM(this, 1i, "shadowMap", SHADOW_TEXTURE_SAMPLER);
    SET_UNIFORM(this, 1i, "envMap", ENV_TEXTURE_SAMPLER);
}

void
Context::SetupView()
{
    GL_CHECK(glViewport(0, 0, window.GetWidth(), window.GetHeight()));
    GL_CHECK(glMatrixMode(GL_PROJECTION));
    GL_CHECK(glLoadIdentity());
    GL_CHECK(gluPerspective(90.0,
                            ((GLfloat)window.GetWidth()) /
                            ((GLfloat)window.GetHeight()),
                            CAMERA_NEAR, CAMERA_FAR));

    // Make sure to pass the viewport size to the shader
    SET_UNIFORM(this, 1f, "viewportWidth", window.GetWidth());
}
