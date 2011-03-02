#ifndef CONTEXT_H
#define CONTEXT_H

#include "Framework.h"
#include "Shader.h"
#include "Matrix.h"
#include <assert.h>

/*
 * General definitions.
 */

#define SHADER_PATH "shaders/phong"

#define CAMERA_NEAR 0.1
#define CAMERA_FAR 80.0

#define GL_CHECK(f) {\
    (f); \
    GLenum error = glGetError(); \
    if (GL_NO_ERROR != error) {\
        printf("GL Error - %s:%d - %s\n", __FILE__, __LINE__,  gluErrorString(error)); \
        exit(-1); \
    } \
}

#ifndef MAX
#define MAX(a,b) (a > b ? a : b)
#endif
#ifndef MIN
#define MIN(a,b) (a > b ? b : a)
#endif

#ifndef MAX3
#define MAX3(a,b,c) MAX(MAX(a,b),c)
#endif
#ifndef MIN3
#define MIN3(a,b,c) MIN(MIN(a,b),c)
#endif

#define IN_RANGE(val, low, high) ((low <= val) && (val <= high))

#define SET_UNIFORM(context, suffix, name, val) {\
    GLint location; \
    GL_CHECK(location = glGetUniformLocation(context->shader.programID(), name)); \
    assert(location >= 0); \
    GL_CHECK(glUniform##suffix(location, val)); \
}

#define SET_UNIFORMV(context, suffix, name, val) {\
    GLint location; \
    GL_CHECK(location = glGetUniformLocation(context->shader.programID(), name)); \
    assert(location >= 0); \
    GL_CHECK(glUniform##suffix(location, 1, val)); \
}

#define SET_UNIFORMMATV(context, suffix, name, val) {\
    GLint location; \
    GL_CHECK(location = glGetUniformLocation(context->shader.programID(), name)); \
    assert(location >= 0); \
    GL_CHECK(glUniformMatrix##suffix(location, 1, GL_FALSE, val)); \
}

// We use fixed texture samplers for the various samplers we use
// in our fragment shader
#define SPRITE_TEXTURE_SAMPLER 0
#define SPRITE_TEXTURE_UNIT GL_TEXTURE0
#define DIFFUSE_TEXTURE_SAMPLER 1
#define DIFFUSE_TEXTURE_UNIT GL_TEXTURE1
#define SPECULAR_TEXTURE_SAMPLER 2
#define SPECULAR_TEXTURE_UNIT GL_TEXTURE2
#define NORMAL_TEXTURE_SAMPLER 3
#define NORMAL_TEXTURE_UNIT GL_TEXTURE3
#define SHADOW_TEXTURE_SAMPLER 4
#define SHADOW_TEXTURE_UNIT GL_TEXTURE4
#define ENV_TEXTURE_SAMPLER 5
#define ENV_TEXTURE_UNIT GL_TEXTURE5

/*
 * Context container.
 */
struct Context {

    // Window state
    sf::WindowSettings windowSettings;
    sf::RenderWindow window;

    // Animation state
    sf::Clock clock;

    // Shader
    Shader shader;

    /*
     * Constructor. Initializes all our state.
     */
    Context();

    /*
     * Sets up the viewport and projection.
     */
    void SetupView();
};

#endif /* CONTEXT_H */
