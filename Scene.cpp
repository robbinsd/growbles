#include "Scene.h"

#define ARMADILLO_PATH "scene/armadillo.3ds"
#define CATHEDRAL_PATH "scene/cathedral.3ds"
#define SPHERE_PATH "scene/sphere.3ds"

Scene::Scene() : mContext(NULL)
               , mShadowTarget(SHADOW_TEXTURE_WIDTH, SHADOW_TEXTURE_HEIGHT)
               , mDoingShadowPass(false)
               , mShadowsDirty(true)
{
    /*
     * Lighting Defaults.
     */

    // w = 0 signifies directional light
    mLights[SCENELIGHT_DIRECTIONAL].position.Set(1.0, -0.8, 0.0, 0.0);
    mLights[SCENELIGHT_DIRECTIONAL].ambient.Set(0.2, 0.2, 0.2, 1.0);
    mLights[SCENELIGHT_DIRECTIONAL].diffuse.Set(1.0, 1.0, 1.0, 1.0);
    mLights[SCENELIGHT_DIRECTIONAL].specular.Set(1.0, 1.0, 1.0, 1.0);

    // Point light
    mLights[SCENELIGHT_POINT].position.Set(0.0, 11.0, 0.0, 1.0);
    mLights[SCENELIGHT_POINT].ambient.Set(0.2, 0.2, 0.2, 1.0);
    mLights[SCENELIGHT_POINT].diffuse.Set(0.4, 0.4, 0.4, 1.0);
    mLights[SCENELIGHT_POINT].specular.Set(0.4, 0.4, 0.4, 1.0);

    /*
     * Camera defaults.
     */
    mCameraPos.y = 10.0f;
    mCameraPos.x = -17.0f;
    mPitch = -30.0;
    mYaw = 90.0;
}

void
Scene::Init(Context& context)
{
    // Save the context
    mContext = &context;

    // Initialize the scene graph
    mSceneGraph.Init(context);

    // Add the models to the scene graph
    mSceneGraph.LoadScene(CATHEDRAL_PATH, "Cathedral", &mSceneGraph.rootNode);
    mSceneGraph.LoadScene(ARMADILLO_PATH, "Armadillo", &mSceneGraph.rootNode);

    // Make sure the shadow pass starts disabled
    SetShadowPassEnabled(false);

    // Bootstrap our light situation
    LightingChanged();

    // Environment-map the armadillo
    Vector emapPos(0.0, 3.0, 0.0, 1.0);
    mSceneGraph.FindMesh("Armadillo_0")->EnvironmentMap(*this, emapPos);

    // Apply the camera
    SetViewToCamera();
}

void
Scene::Render()
{
    // If our shadow buffer is dirty, do a shadow pass
    if (mShadowsDirty)
        ShadowPass();

    // Clear the buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind the shadow texture
    GL_CHECK(glActiveTexture(SHADOW_TEXTURE_UNIT));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, mShadowTarget.textureID()));

    // Render our scenegraph
    mSceneGraph.Render();

    // Unbind the shadow texture
    GL_CHECK(glActiveTexture(SHADOW_TEXTURE_UNIT));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    // Flush
    glFlush();
}

void
Scene::ShadowPass()
{
    // Bind the framebuffer
    mShadowTarget.bind();

    // Clear the depth buffer
    GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));

    // Set the appropriate shader state
    SetShadowPassEnabled(true);

    // The viewport is set to the size of the target texture.
    GL_CHECK(glViewport(0, 0, SHADOW_TEXTURE_WIDTH, SHADOW_TEXTURE_HEIGHT));

    // Render the models
    mSceneGraph.Render();

    // Reset the viewport (and, incidentally, the projection matrix)
    mContext->SetupView();

    // Disable the shadow pass
    SetShadowPassEnabled(false);

    // Unbind the render target
    mShadowTarget.unbind();

    // Our shadows are now valid
    mShadowsDirty = false;
}

void
Scene::MoveCamera(float forward, float right)
{
    // Looking down the -z axis, right is +x and forward is -z
    Vector direction(right, 0.0, -forward, 1.0);

    // This vector is a vector in view space. Put it in world space and
    // apply it to our world space camera coordinates.
    direction = GeneratePanMatrix().Inverse().MVProduct(direction);
    mCameraPos = mCameraPos + direction;

    // Re-send the camera info
    SetViewToCamera();
}

static void ClampDegrees(float& input)
{
    while (input > 180.0f)
        input -= 360.0f;
    while (input < -180.0f)
        input += 360.0f;
}

void
Scene::PanCamera(float plusPitch, float plusYaw)
{
    mPitch += plusPitch;
    mYaw += plusYaw;

    // Make sure we're in the -180,180 range
    ClampDegrees(mPitch);
    ClampDegrees(mYaw);

    SetViewToCamera();
}

void
Scene::MoveLight(float x, float z)
{
    mLights[SCENELIGHT_DIRECTIONAL].position.x += x;
    mLights[SCENELIGHT_DIRECTIONAL].position.z += z;

    LightingChanged();
}

void
Scene::LightingChanged()
{
    // Apply the lighting to OpenGL
    SetLighting();

    // Generate the lighting matrix for the shaders
    RegenerateLightMatrix();

    // Flag that our shadow texture is invalid
    mShadowsDirty = true;
}

void
Scene::SetShadowPassEnabled(bool enabled)
{
    mDoingShadowPass = enabled;
    SET_UNIFORM(mContext, 1i, "shadowPass", enabled ? 1 : 0);
}

void
Scene::SetLighting()
{
    // First light (directional)
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION,
              (GLfloat*) &mLights[SCENELIGHT_DIRECTIONAL].position);
    glLightfv(GL_LIGHT0, GL_AMBIENT,
              (GLfloat*) &mLights[SCENELIGHT_DIRECTIONAL].ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,
              (GLfloat*) &mLights[SCENELIGHT_DIRECTIONAL].diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR,
              (GLfloat*) &mLights[SCENELIGHT_DIRECTIONAL].specular);

    // Second light (point)
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION,
              (GLfloat*) &mLights[SCENELIGHT_POINT].position);
    glLightfv(GL_LIGHT1, GL_AMBIENT,
              (GLfloat*) &mLights[SCENELIGHT_POINT].ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,
              (GLfloat*) &mLights[SCENELIGHT_POINT].diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR,
              (GLfloat*) &mLights[SCENELIGHT_POINT].specular);
}

void
Scene::SetView(Matrix& view)
{
    // Read the matrix out in OpenGL format
    GLfloat viewArray[16];
    view.Get(viewArray);

    // Apply
    GL_CHECK(glMatrixMode(GL_MODELVIEW));
    GL_CHECK(glLoadIdentity());
    GL_CHECK(glMultMatrixf(viewArray));

    // Generate the inverse upper-3x3 view matrix for the shader.
    GLfloat invView[9];
    view.Inverse().Get3x3(invView);
    SET_UNIFORMMATV(mContext, 3fv, "inverseViewMatrix", invView);

    // Reset the lighting using the new view matrix
    SetLighting();
}

void
Scene::SetViewToCamera()
{
    // Set the view matrix
    Matrix cameraMatrix = GenerateCameraMatrix();
    SetView(cameraMatrix);
}

Matrix
Scene::GenerateCameraMatrix()
{
    // Start with the pan matrix
    Matrix rv = GeneratePanMatrix();

    // Translate to our desired position
    rv.Translate(-mCameraPos.x, -mCameraPos.y, -mCameraPos.z);

    // All done
    return rv;
}

Vector
Scene::GetCameraDirection()
{
    Vector direction;
    double factor = M_PI / 180.0;
    direction.x = sin(factor * mYaw) * cos(factor * mPitch);
    direction.y = sin(factor * mPitch);
    direction.z = -cos(factor * mYaw) * cos(factor * mPitch);
    return direction;
}

Matrix
Scene::GeneratePanMatrix()
{
    // Start with the identity
    Matrix rv;

    // Rotate to our desired pitch and yaw
    //
    // Note - Our convention is that positive pitch looks up,
    // and positive yaw looks right. Since rotations follow the
    // right-hand rule, we want to rotate around the positive-X
    // and negative-Y axes. We negate the rotation because the
    // view matrix is the opposite of the transform we would apply
    // to the camera (since we're really moving the world around
    // the camera).
    rv.Rotate(-mPitch, 1.0, 0.0, 0.0);
    rv.Rotate(-mYaw, 0.0, -1.0, 0.0);


    return rv;
}

void
Scene::RegenerateLightMatrix()
{
    // Start with the identity
    Matrix lightMat;

    // Add an orthographic projection
    lightMat.Ortho(-25.0f, 25.0f, -25.0f, 25.0f, -27.0f, 27.0f);

    // Look the appropriate angle
    Vector eye(0.0f, 0.0f, 0.0f, 1.0f);
    Vector center = mLights[SCENELIGHT_DIRECTIONAL].position;
    Vector up(0.0f, 1.0f, 0.0f, 1.0f);
    lightMat.LookAt(eye, center, up);

    // Store the light-space matrix to the shader
    GLfloat lightMatArray[16];
    lightMat.Get(lightMatArray);
    SET_UNIFORMMATV(mContext, 4fv, "lightMatrix", lightMatArray);
}

void
Scene::RenderShadowQuad()
{
    // Sanitize our matricies
    GL_CHECK(glMatrixMode(GL_MODELVIEW));
    GL_CHECK(glPushMatrix());
    GL_CHECK(glLoadIdentity());
    GL_CHECK(glMatrixMode(GL_PROJECTION));
    GL_CHECK(glPushMatrix());
    GL_CHECK(glLoadIdentity());
    GL_CHECK(glOrtho(-0.1, 1.1, -0.1, 1.1, -1.0, 1.0));

    // Disable the shader (use the fixed-function pipeline)
    GL_CHECK(glUseProgram(0));

    // Enable texturing
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glEnable(GL_TEXTURE_2D));
    GL_CHECK(glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, mShadowTarget.textureID()));

    // Draw our quad
    glBegin(GL_QUADS);
    glTexCoord2d(0.0,0.0); glVertex2d(0.0,0.0);
    glTexCoord2d(1.0,0.0); glVertex2d(1.0,0.0);
    glTexCoord2d(1.0,1.0); glVertex2d(1.0,1.0);
    glTexCoord2d(0.0,1.0); glVertex2d(0.0,1.0);
    glEnd();

    // Flush
    GL_CHECK(glFlush());

    // Disable fixed-function texturing
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
    GL_CHECK(glDisable(GL_TEXTURE_2D));

    // Reenable the shader
    GL_CHECK(glUseProgram(mContext->shader.programID()));

    // Restore our old matrices
    GL_CHECK(glMatrixMode(GL_MODELVIEW));
    GL_CHECK(glPopMatrix());
    GL_CHECK(glMatrixMode(GL_PROJECTION));
    GL_CHECK(glPopMatrix());
}
