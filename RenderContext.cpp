#include "RenderContext.h"
#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <fstream>

using std::vector;

RenderContext::RenderContext() : mDoingShadowPass(false)
                               , mShadowsDirty(true)
                               , mWindowSettings(24, 8, 2)
                               , mWindow(sf::VideoMode(800, 600), "Growbles",
                                         sf::Style::Close, mWindowSettings)
                               , mShader(SHADER_PATH)
{
    mWindow.PreserveOpenGLStates(true);
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

RenderContext::~RenderContext()
{
    // Destroy the materials
    for (vector<Material>::iterator it = materials.begin();
         it != materials.end(); ++it)
        it->Destroy();
}

void
RenderContext::Init()
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
    mShader.Init();
    GL_CHECK(glUseProgram(mShader.programID()));

    // Initialize the shadow buffer
    mShadowTarget.Init(SHADOW_TEXTURE_WIDTH, SHADOW_TEXTURE_HEIGHT);

    // Setup the view system
    SetViewportAndProjection();

    // Set up the shader
    SET_UNIFORM(this, 1i, "spriteMap", SPRITE_TEXTURE_SAMPLER);
    SET_UNIFORM(this, 1i, "diffuseMap", DIFFUSE_TEXTURE_SAMPLER);
    SET_UNIFORM(this, 1i, "specularMap", SPECULAR_TEXTURE_SAMPLER);
    SET_UNIFORM(this, 1i, "normalMap", NORMAL_TEXTURE_SAMPLER);
    SET_UNIFORM(this, 1i, "shadowMap", SHADOW_TEXTURE_SAMPLER);
    SET_UNIFORM(this, 1i, "envMap", ENV_TEXTURE_SAMPLER);

    // Make sure the shadow pass starts disabled
    SetShadowPassEnabled(false);

    // Initialize our falcon device
    falcon.Init();

    // Bootstrap our light situation
    LightingChanged();

    // Apply the camera
    SetViewToCamera();
    
    // BOF load skybox textures
    std::string fullPath0 = "scenefiles/space_back.jpg";
    // If the file exists, initialize the appropriate texture
    std::ifstream file0(fullPath0.c_str(), std::ifstream::in);
    if (file0)
        skyboxTextures[0].Init(fullPath0);
    
    std::string fullPath1 = "scenefiles/space_right.jpg";
    // If the file exists, initialize the appropriate texture
    std::ifstream file1(fullPath1.c_str(), std::ifstream::in);
    if (file1)
        skyboxTextures[1].Init(fullPath1);
    
    std::string fullPath2 = "scenefiles/space_front.jpg";
    // If the file exists, initialize the appropriate texture
    std::ifstream file2(fullPath2.c_str(), std::ifstream::in);
    if (file2)
        skyboxTextures[2].Init(fullPath2);
    
    std::string fullPath3 = "scenefiles/space_left.jpg";
    // If the file exists, initialize the appropriate texture
    std::ifstream file3(fullPath3.c_str(), std::ifstream::in);
    if (file3)
        skyboxTextures[3].Init(fullPath3);
    
    std::string fullPath4 = "scenefiles/space_bottom.jpg";
    // If the file exists, initialize the appropriate texture
    std::ifstream file4(fullPath4.c_str(), std::ifstream::in);
    if (file4)
        skyboxTextures[4].Init(fullPath4);
    
    std::string fullPath5 = "scenefiles/space_top.jpg";
    // If the file exists, initialize the appropriate texture
    std::ifstream file5(fullPath5.c_str(), std::ifstream::in);
    if (file5)
        skyboxTextures[5].Init(fullPath5);
    // EOF load skybox textures
}

void
RenderContext::Render(SceneGraph& sceneGraph)
{
    // If our shadow buffer is dirty, do a shadow pass
    if (mShadowsDirty)
        ShadowPass(sceneGraph);

    // Clear the buffers
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind the shadow texture
    GL_CHECK(glActiveTexture(SHADOW_TEXTURE_UNIT));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, mShadowTarget.textureID()));

    // Render our scenegraph
    sceneGraph.Render();

    // Unbind the shadow texture
    GL_CHECK(glActiveTexture(SHADOW_TEXTURE_UNIT));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    // Flush
    glFlush();
}

void
RenderContext::RenderSkybox()
{	
    // Clear the buffers here because the skybox is always the first thing drawn
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use fixed pipeline
    GL_CHECK(glUseProgram(0));
    
    // Store the current matrix
    glPushMatrix();

    // Reset and transform the matrix.
    glLoadIdentity();
    gluLookAt(0, 0, 0, mCameraPos.x, mCameraPos.y, mCameraPos.z, 0, 1, 0);

    // Enable/Disable features
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    // Just in case we set all vertices to white.
    glColor4f(1,1,1,1);
    
    // Render the front quad
    //glBindTexture(GL_TEXTURE_2D, _skybox[0]);
    skyboxTextures[0].SetEnabled(true, GL_TEXTURE0);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(  0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 0); glVertex3f( -0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 1); glVertex3f( -0.5f,  0.5f, -0.5f );
        glTexCoord2f(0, 1); glVertex3f(  0.5f,  0.5f, -0.5f );
    glEnd();
    skyboxTextures[0].SetEnabled(false, GL_TEXTURE0);
    
    // Render the left quad
    //glBindTexture(GL_TEXTURE_2D, _skybox[1]);
    skyboxTextures[1].SetEnabled(true, GL_TEXTURE0);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(  0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f,  0.5f, -0.5f );
        glTexCoord2f(0, 1); glVertex3f(  0.5f,  0.5f,  0.5f );
    glEnd();
    skyboxTextures[1].SetEnabled(false, GL_TEXTURE0);
    
    // Render the back quad
    //glBindTexture(GL_TEXTURE_2D, _skybox[2]);
    skyboxTextures[2].SetEnabled(true, GL_TEXTURE0);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f( -0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f,  0.5f,  0.5f );
        glTexCoord2f(0, 1); glVertex3f( -0.5f,  0.5f,  0.5f );
    glEnd();
    skyboxTextures[2].SetEnabled(false, GL_TEXTURE0);
    
    // Render the right quad
    //glBindTexture(GL_TEXTURE_2D, _skybox[3]);
    skyboxTextures[3].SetEnabled(true, GL_TEXTURE0);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f( -0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 0); glVertex3f( -0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f( -0.5f,  0.5f,  0.5f );
        glTexCoord2f(0, 1); glVertex3f( -0.5f,  0.5f, -0.5f );
    glEnd();
    skyboxTextures[3].SetEnabled(false, GL_TEXTURE0);
    
    // Render the top quad
    //glBindTexture(GL_TEXTURE_2D, _skybox[4]);
    skyboxTextures[4].SetEnabled(true, GL_TEXTURE0);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex3f( -0.5f,  0.5f, -0.5f );
        glTexCoord2f(0, 0); glVertex3f( -0.5f,  0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f,  0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f,  0.5f, -0.5f );
    glEnd();
    skyboxTextures[4].SetEnabled(false, GL_TEXTURE0);
    
    // Render the bottom quad
    //glBindTexture(GL_TEXTURE_2D, _skybox[5]);
    skyboxTextures[5].SetEnabled(true, GL_TEXTURE0);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f( -0.5f, -0.5f, -0.5f );
        glTexCoord2f(0, 1); glVertex3f( -0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f, -0.5f, -0.5f );
    glEnd();
    skyboxTextures[5].SetEnabled(false, GL_TEXTURE0);
    
    // Restore enable bits and matrix
    glPopAttrib();
    glPopMatrix();
    
    // Renable the shader
    GL_CHECK(glUseProgram(GetShaderID()));
}

void
RenderContext::RenderPlatform(WorldModel& world)
{
    // Disable the shader so we can draw the platform using the fixed pipeline
    GL_CHECK(glUseProgram(0));
    
    // Draw platform
    world.GetPlatform()->render();
    
    // Draw debug wireframes
    world.GetDynamicsWorld()->debugDrawWorld();

    // Flush
    GL_CHECK(glFlush());
    
    // Reenable the shader
    GL_CHECK(glUseProgram(GetShaderID()));
}

void
RenderContext::RenderAllElse()
{
    // Use fixed pipeline
    GL_CHECK(glUseProgram(0));
    
    // Draw all the strings in myTexts
    DrawString();
    
    // Flush
    GL_CHECK(glFlush());
    
    // Renable the shader
    GL_CHECK(glUseProgram(GetShaderID()));
}

void
RenderContext::ShadowPass(SceneGraph& sceneGraph)
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
    sceneGraph.Render();

    // Reset the viewport (and, incidentally, the projection matrix)
    SetViewportAndProjection();

    // Disable the shadow pass
    SetShadowPassEnabled(false);

    // Unbind the render target
    mShadowTarget.unbind();

    // Our shadows are now valid
    mShadowsDirty = false;
}

void
RenderContext::MoveCamera(float forward, float right)
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

void
RenderContext::MoveCameraAbsolute(float x, float z)
{
    // Move the camera by absolute coordinates
    mCameraPos.x += x;
    mCameraPos.z += z;

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
RenderContext::PanCamera(float plusPitch, float plusYaw)
{
    mPitch += plusPitch;
    mYaw += plusYaw;

    // Make sure we're in the -180,180 range
    ClampDegrees(mPitch);
    ClampDegrees(mYaw);

    SetViewToCamera();
}

void
RenderContext::MoveLight(float x, float z)
{
    mLights[SCENELIGHT_DIRECTIONAL].position.x += x;
    mLights[SCENELIGHT_DIRECTIONAL].position.z += z;

    LightingChanged();
}

void
RenderContext::SetViewportAndProjection()
{
    GL_CHECK(glViewport(0, 0, mWindow.GetWidth(), mWindow.GetHeight()));
    GL_CHECK(glMatrixMode(GL_PROJECTION));
    GL_CHECK(glLoadIdentity());
    GL_CHECK(gluPerspective(60.0,
                            ((GLfloat)mWindow.GetWidth()) /
                            ((GLfloat)mWindow.GetHeight()),
                            CAMERA_NEAR, CAMERA_FAR));

    // Make sure to pass the viewport size to the shader
    SET_UNIFORM(this, 1f, "viewportWidth", mWindow.GetWidth());
}

void
RenderContext::LightingChanged()
{
    // Apply the lighting to OpenGL
    SetLighting();

    // Generate the lighting matrix for the shaders
    RegenerateLightMatrix();

    // Flag that our shadow texture is invalid
    mShadowsDirty = true;
}

void
RenderContext::SetShadowPassEnabled(bool enabled)
{
    mDoingShadowPass = enabled;
    SET_UNIFORM(this, 1i, "shadowPass", enabled ? 1 : 0);
}

void
RenderContext::SetLighting()
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
RenderContext::SetView(Matrix& view)
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
    SET_UNIFORMMATV(this, 3fv, "inverseViewMatrix", invView);

    // Reset the lighting using the new view matrix
    SetLighting();
}

void
RenderContext::SetViewToCamera()
{
    // Set the view matrix
    Matrix cameraMatrix = GenerateCameraMatrix();
    SetView(cameraMatrix);
}

Matrix
RenderContext::GenerateCameraMatrix()
{
    // Start with the pan matrix
    Matrix rv = GeneratePanMatrix();

    // Translate to our desired position
    rv.Translate(-mCameraPos.x, -mCameraPos.y, -mCameraPos.z);

    // All done
    return rv;
}

Vector
RenderContext::GetCameraDirection()
{
    Vector direction;
    double factor = M_PI / 180.0;
    direction.x = sin(factor * mYaw) * cos(factor * mPitch);
    direction.y = sin(factor * mPitch);
    direction.z = -cos(factor * mYaw) * cos(factor * mPitch);
    return direction;
}

Matrix
RenderContext::GeneratePanMatrix()
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
RenderContext::RegenerateLightMatrix()
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
    SET_UNIFORMMATV(this, 4fv, "lightMatrix", lightMatArray);
}

void
RenderContext::RenderShadowQuad()
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
    GL_CHECK(glUseProgram(GetShaderID()));

    // Restore our old matrices
    GL_CHECK(glMatrixMode(GL_MODELVIEW));
    GL_CHECK(glPopMatrix());
    GL_CHECK(glMatrixMode(GL_PROJECTION));
    GL_CHECK(glPopMatrix());
}

void
RenderContext::RenderString(std::string str, unsigned duration, unsigned size, float x, float y, unsigned r, unsigned g, unsigned b)
{
    /*
     // Load the font from a file, if we want other fonts
     sf::Font MyFont;
     if (!MyFont.LoadFromFile("arial.ttf", 50))
     {
     // Error...
     }
     */
    myText text;
    text.str.SetText(str);
    text.str.SetFont(sf::Font::GetDefaultFont());
    text.str.SetSize(size);
    text.str.SetColor(sf::Color(r, g, b));
    //myText.SetRotation(90.f);
    //myText.SetScale(2.f, 2.f);
    text.str.SetPosition(x, y);
    text.duration = duration;
    myTexts.push_back(text);
}

void
RenderContext::DrawString()
{
    /*
    GL_CHECK(glUseProgram(0));
    glActiveTexture(GL_TEXTURE0);
    mWindow.Draw(myText);
    GL_CHECK(glUseProgram(GetShaderID()));
     */
    glActiveTexture(GL_TEXTURE0);
    for (unsigned i=0; i<myTexts.size(); i++) {
        if (myTexts[i].duration <= 0) {
            myTexts.erase(myTexts.begin()+i);
            continue;
        }
        mWindow.Draw(myTexts[i].str);
        myTexts[i].duration -= 32;
    }
}
