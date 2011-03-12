#ifndef RENDERCONTEXT_H
#define RENDERCONTEXT_H

#include "Framework.h"
#include "Matrix.h"
#include "DepthRenderTarget.h"
#include "SceneGraph.h"
#include "Shader.h"
#include <vector>
#include "WorldModel.h"

/*
 * General parameters.
 */

#define SHADER_PATH "shaders/phong"

#define CAMERA_NEAR 0.1
#define CAMERA_FAR 80.0

#define SHADOW_TEXTURE_WIDTH 3000
#define SHADOW_TEXTURE_HEIGHT 3000

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

struct LightInfo {

    Vector position;
    Vector ambient;
    Vector diffuse;
    Vector specular;
};

typedef enum {
    SCENELIGHT_DIRECTIONAL = 0,
    SCENELIGHT_POINT,
    SCENELIGHT_COUNT
} SceneLightType;

class RenderContext {

public:

    /*
     * Constructor.
     */
    RenderContext();

    /*
     * Destructor.
     */
    ~RenderContext();

    /*
     * Initialize the rendering context.
     */
    void Init();

    /*
     * Render the scene.
     */
    void Render(SceneGraph& sceneGraph);

    /*
     * Camera Movement.
     */
    void MoveCamera(float forward, float right);
    void PanCamera(float plusPitch, float plusYaw);

    /*
     * Light Movement.
     */
    void MoveLight(float x, float z);

    /*
     * Sets a new view matrix.
     *
     * At the level of Scene, we assume that the modelview has
     * only a view matrix, and that we can clobber it at will
     * with something new.
     */
    void SetView(Matrix& view);

    /*
     * Sets the view matrix to the camera view.
     */
    void SetViewToCamera();

    /*
     * Sets up the viewport and projection.
     */
    void SetViewportAndProjection();

    /*
     * Gets the shader program ID.
     */
    GLint GetShaderID() { return mShader.programID(); };

    /*
     * Gets the window.
     */
    sf::RenderWindow* GetWindow() { return &mWindow; };

    // Publicly accessible vector of the materials loaded for this rendering
    // context.
    std::vector<Material> materials;
    
    /*
     * Renders the platform for debugging
     */
    void RenderPlatform(WorldModel& world);

protected:

    /*
     * Called to set up our entire lighting scheme.
     */
    void LightingChanged();

    /*
     * Flips the appropriate flags to enable or disable the
     * shadow pass.
     */
    void SetShadowPassEnabled(bool enabled);

    /*
     * Renders the shadow pass.
     */
    void ShadowPass(SceneGraph& sceneGraph);

    /*
     * Applies the current lighting scheme to OpenGL.
     */
    void SetLighting();

    /*
     * Generates the camera view matrix.
     */
    Matrix GenerateCameraMatrix();

    /*
     * Generates just the panning part of the matrix.
     */
    Matrix GeneratePanMatrix();

    /*
     * Gets the camera direction.
     */
    Vector GetCameraDirection();

    /*
     * Regenerates the a projection + view matrix
     * that transforms objects from world space into
     * projected light space. Stores the result in
     * the shader uniform.
     */
    void RegenerateLightMatrix();

    /*
     * Renders the shadow buffer to a quad.
     *
     * Useful for debugging.
     */
    void RenderShadowQuad();

    // Lighting
    LightInfo mLights[SCENELIGHT_COUNT];

    // Camera Info
    float mPitch, mYaw;
    Vector mCameraPos;

    // Shadow texture
    DepthRenderTarget mShadowTarget;

    // Doing a shadow pass?
    bool mDoingShadowPass;

    // Need a shadow pass before re-rendering?
    bool mShadowsDirty;

    // Window state
    sf::WindowSettings mWindowSettings;
    sf::RenderWindow mWindow;

    // Shader
    Shader mShader;
};

/*
 * Macros to set uniforms.
 */

#define SET_UNIFORM(context, suffix, name, val) {\
    GLint location; \
    GL_CHECK(location = glGetUniformLocation((context)->GetShaderID(), name)); \
    assert(location >= 0); \
    GL_CHECK(glUniform##suffix(location, val)); \
}

#define SET_UNIFORMV(context, suffix, name, val) {\
    GLint location; \
    GL_CHECK(location = glGetUniformLocation((context)->GetShaderID(), name)); \
    assert(location >= 0); \
    GL_CHECK(glUniform##suffix(location, 1, val)); \
}

#define SET_UNIFORMMATV(context, suffix, name, val) {\
    GLint location; \
    GL_CHECK(location = glGetUniformLocation((context)->GetShaderID(), name)); \
    assert(location >= 0); \
    GL_CHECK(glUniformMatrix##suffix(location, 1, GL_FALSE, val)); \
}


#endif /* RENDERCONTEXT_H */
