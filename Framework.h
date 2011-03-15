#ifndef FRAMEWORK_H
#define FRAMEWORK_H

// GLEW must be included first, if we need it.
#ifdef _WIN32
#define GLEW_STATIC
#define FRAMEWORK_USE_GLEW
#include <GL/glew.h>
#endif
#ifdef __linux__
#define FRAMEWORK_USE_GLEW
#include <GL/glew.h>
#endif

// SFML automatically includes SDL headers
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>

// Open Asset Import Library
#if defined __linux__ || defined _WIN32
#include <assimp.hpp>
#include <aiScene.h>
#include <aiPostProcess.h>
#include <aiMaterial.h>
#else
#include <assimp/assimp.hpp>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h>
#include <assimp/aiMaterial.h>
#endif

// Define whether or not we want networking to be built
//#define NETWORKING

// Falcon Haptics Library
#define FALCON
#ifdef FALCON
#include "chai3d.h"
#endif
// Bullet
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

#include <memory>
#include <iostream>
#include <assert.h>

/*
 * Useful macros.
 */

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

#ifndef PRINTD
#define PRINTD(a) printf("value = %d\n", a);
#endif

#endif
