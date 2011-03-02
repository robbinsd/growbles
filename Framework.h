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
#ifndef __linux__
#include <assimp/assimp.hpp>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h>
#include <assimp/aiMaterial.h>
#else
#include <assimp.hpp>
#include <aiScene.h>
#include <aiPostProcess.h>
#include <aiMaterial.h>
#endif

#include <memory>
#include <iostream>

#endif
