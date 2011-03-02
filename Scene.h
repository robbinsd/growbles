#ifndef SCENE_H
#define SCENE_H

#include "Framework.h"
#include "Matrix.h"
#include "DepthRenderTarget.h"
#include "SceneGraph.h"
#include "CollisionDetector.h"
#include "ParticleEmitter.h"

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

class Scene {

public:

    /*
     * Constructor.
     */
    Scene();

    /*
     * Destructor.
     */
    ~Scene()
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
    void Render();


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
     * Bullet bounciness.
     */
    void IncreaseBulletBounciness();
    void DecreaseBulletBounciness();

    /*
     * Bullet speed.
     */
    void IncreaseBulletSpeed();
    void DecreaseBulletSpeed();

    /*
     * Bullet spread.
     */
    void IncreaseBulletSpread();
    void DecreaseBulletSpread();

    /*
     * Gravity.
     */
    void IncreaseDownwardGravity();
    void DecreaseDownwardGravity();

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
     * Gun.
     */
    void SetGunFiring(bool firing) { mParticleGun.SetInjectionEnabled(firing); };

protected:

    /*
     * Loads a scene.
     */
    const aiScene* LoadScene(Assimp::Importer* importer, const char* path);

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
    void ShadowPass();

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
     * Render our particle emitters.
     */
    void RenderParticleEmitters();

    /*
     * Renders the shadow buffer to a quad.
     *
     * Useful for debugging.
     */
    void RenderShadowQuad();


    /*
     * Draws (in red) the nearest triangle we would collide with
     * if we moved red.
     *
     * Useful for debugging collision detection.
     */
    void DrawCollidingTriangle();

    /*
     * Instance variables.
     */
    Context* mContext;
    SceneGraph mSceneGraph;
    CollisionDetector mCollisionDetector;

    // Lighting
    LightInfo mLights[SCENELIGHT_COUNT];

    // Camera Info
    float mPitch, mYaw;
    Vector mCameraPos;

    // Shadow texture
    DepthRenderTarget mShadowTarget;

    // Particle emitters
    FireEmitter mFireEmitters[6];
    SmokeEmitter mSmokeEmitters[6];
    GunEmitter mParticleGun;

    // Step clock
    sf::Clock mStepClock;

    // Doing a shadow pass?
    bool mDoingShadowPass;
};

#endif /* SCENE_H */
