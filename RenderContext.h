#ifndef RENDERCONTEXT_H
#define RENDERCONTEXT_H

#include "Framework.h"
#include "Matrix.h"
#include "DepthRenderTarget.h"
#include "SceneGraph.h"

#define SHADOW_TEXTURE_WIDTH 3000
#define SHADOW_TEXTURE_HEIGHT 3000

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
    ~RenderContext()
    {
        mContext = NULL;
    }

    /*
     * Initialize the scene.
     */
    void Init(Context& context);

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

    /*
     * Instance variables.
     */
    Context* mContext;

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
};

#endif /* RENDERCONTEXT_H */
