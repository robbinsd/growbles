#include "SceneGraph.h"
#include "Scene.h"

using std::list;
using std::vector;
using std::string;

SceneMesh::SceneMesh(SceneGraph* scene, const char* name,
                     unsigned material) : mSceneGraph(scene)
                                        , mMaterial(material)
                                        , mName(name)
                                        , mCubeTextureID(0)
                                        , mDoingEnvMap(false)
{
}

// Faces of a texture cube
GLenum sFaceNames[] = {GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                       GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                       GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
float sFaceDirections[][3] = { {1.0, 0.0, 0.0}, {-1.0, 0.0, 0.0},
                               {0.0, 1.0, 0.0}, {0.0, -1.0, 0.0},
                               {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0} };


float sUpDirections[][3] = { {0.0, -1.0, 0.0}, {0.0, -1.0, 0.0},
                             {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0},
                             {0.0, -1.0, 0.0}, {0.0, -1.0, 0.0} };

void
SceneMesh::Render()
{
    // If we're environment mapping this node and its descendants, we don't
    // want to render them.
    if (mDoingEnvMap)
        return;

    // If we have an environment map, enable environment mapping
    if (mCubeTextureID != 0) {

        // Bind the cube texture
        GL_CHECK(glActiveTexture(ENV_TEXTURE_UNIT));
        GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, mCubeTextureID));

        // Set the flag
        SET_UNIFORM(mSceneGraph->context, 1i, "mapEnvironment", 1);
    }


    // Enable the mesh material
    assert(mMaterial < mSceneGraph->materials.size());
    mSceneGraph->materials[mMaterial].SetEnabled(true);

    // Grab the positions of our attributes
    GLint positionPos, texcoordPos, normalPos, tangentPos, bitangentPos;
    GLint shaderID = mSceneGraph->context->shader.programID();

    GL_CHECK(positionPos = glGetAttribLocation(shaderID, "positionIn"));
    GL_CHECK(texcoordPos = glGetAttribLocation(shaderID, "texcoordIn"));
    GL_CHECK(normalPos = glGetAttribLocation(shaderID, "normalIn"));
    GL_CHECK(tangentPos = glGetAttribLocation(shaderID, "tangentIn"));
    GL_CHECK(bitangentPos = glGetAttribLocation(shaderID, "bitangentIn"));

    // Enable the appropriate attribute arrays
    GL_CHECK(glEnableVertexAttribArray(positionPos));
    GL_CHECK(glEnableVertexAttribArray(texcoordPos));
    GL_CHECK(glEnableVertexAttribArray(normalPos));
    GL_CHECK(glEnableVertexAttribArray(tangentPos));
    GL_CHECK(glEnableVertexAttribArray(bitangentPos));

    // Send our attributes down the pipeline
    GL_CHECK(glVertexAttribPointer(positionPos, 3, GL_FLOAT, GL_FALSE,
                                   0, &mPositions[0]));
    GL_CHECK(glVertexAttribPointer(texcoordPos, 2, GL_FLOAT, GL_FALSE,
                                   0, &mTexcoords[0]));
    GL_CHECK(glVertexAttribPointer(normalPos, 3, GL_FLOAT, GL_FALSE,
                                   0, &mNormals[0]));
    GL_CHECK(glVertexAttribPointer(tangentPos, 3, GL_FLOAT, GL_FALSE,
                                   0, &mTangents[0]));
    GL_CHECK(glVertexAttribPointer(bitangentPos, 3, GL_FLOAT, GL_FALSE,
                                   0, &mBitangents[0]));

    // We should have 3 vertices per triangle, and each vertex has 3 coordinates
    assert(mPositions.size() % 3 == 0);
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, mPositions.size() / 3));

    // Disable the appropriate attribute arrays
    GL_CHECK(glDisableVertexAttribArray(positionPos));
    GL_CHECK(glDisableVertexAttribArray(texcoordPos));
    GL_CHECK(glDisableVertexAttribArray(normalPos));
    GL_CHECK(glDisableVertexAttribArray(tangentPos));
    GL_CHECK(glDisableVertexAttribArray(bitangentPos));

    // Disable the material
    mSceneGraph->materials[mMaterial].SetEnabled(false);

    // Disable any environment mapping
    SET_UNIFORM(mSceneGraph->context, 1i, "mapEnvironment", 0);
}

void
SceneMesh::AddTriangle(SceneVertex& v1, SceneVertex& v2, SceneVertex& v3)
{
    // Kick into the helper.
    AddVertex(v1);
    AddVertex(v2);
    AddVertex(v3);
}

void
SceneMesh::InitWithMesh(const aiMesh* mesh)
{
    // If the mesh doesn't contain triangles, we ignore it
    if (!(mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE))
        return;

    // We don't support meshes with mixed primitives
    assert(mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE);

    // Iterate over our faces. Each should be a triangle.
    for (unsigned i = 0; i < mesh->mNumFaces; ++i) {

        // Get a handle on the face
        const aiFace* face = mesh->mFaces + i;
        assert(face->mNumIndices == 3);

        // Declare 3 vertices to make up that triangle.
        SceneVertex triangle[3];

        // Iterate over the verticies in the face
        for (unsigned j = 0; j < face->mNumIndices; ++j) {

            unsigned vertexIndex = face->mIndices[j];

            // Initialize the vertex structure
            triangle[j].position.Set(mesh->mVertices[vertexIndex]);
            triangle[j].normal.Set(mesh->mNormals[vertexIndex]);
            triangle[j].tangent.Set(mesh->mTangents[vertexIndex]);
            triangle[j].bitangent.Set(mesh->mBitangents[vertexIndex]);
            triangle[j].texcoord.Set(mesh->mTextureCoords[0][vertexIndex]);
        }

        // Add the triangle
        AddTriangle(triangle[0], triangle[1], triangle[2]);
    }
}

void
SceneMesh::AddVertex(SceneVertex& v)
{
    // Position
    mPositions.push_back(v.position.x);
    mPositions.push_back(v.position.y);
    mPositions.push_back(v.position.z);

    // Normal
    mNormals.push_back(v.normal.x);
    mNormals.push_back(v.normal.y);
    mNormals.push_back(v.normal.z);

    // Tangent
    mTangents.push_back(v.tangent.x);
    mTangents.push_back(v.tangent.y);
    mTangents.push_back(v.tangent.z);

    // Bitangent
    mBitangents.push_back(v.bitangent.x);
    mBitangents.push_back(v.bitangent.y);
    mBitangents.push_back(v.bitangent.z);

    // Texture coordinates
    mTexcoords.push_back(v.texcoord.x);
    mTexcoords.push_back(v.texcoord.y);
}

void
SceneMesh::EnvironmentMap(Scene& scene, Vector& eyePos)
{
    // Flag that we're in the process of texture generation. This
    // disables rendering of this model, which is what we want when
    // we're trying to render from the perspective of the model.
    mDoingEnvMap = true;

    // Set up the cube texture if we don't already have it.
    if (mCubeTextureID == 0) {
        GL_CHECK(glGenTextures(1, &mCubeTextureID));
        assert(mCubeTextureID > 0);
        GL_CHECK(glActiveTexture(ENV_TEXTURE_UNIT));
        GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, mCubeTextureID));
        GL_CHECK(glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                                 GL_LINEAR));
        GL_CHECK(glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER,
                                 GL_LINEAR));
        GL_CHECK(glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,
                                 GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,
                                 GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,
                                 GL_CLAMP_TO_EDGE));

        // Allocate texture memory
        for (unsigned i = 0; i < 6; i++)
            GL_CHECK(glTexImage2D(sFaceNames[i], 0, GL_RGBA, CUBEMAP_SIDE_SIZE,
                                  CUBEMAP_SIDE_SIZE, 0, GL_RGBA, GL_FLOAT, NULL));
    }

    // Set the projection matrix and viewport
    GL_CHECK(glMatrixMode(GL_PROJECTION));
    GL_CHECK(glLoadIdentity());
    GL_CHECK(gluPerspective(90.0, 1.0, CAMERA_NEAR, CAMERA_FAR));
    GL_CHECK(glViewport(0, 0, CUBEMAP_SIDE_SIZE, CUBEMAP_SIDE_SIZE));


    // Render each face of the cube
    for (unsigned i = 0; i < 6; ++i) {

        // Our view direction is a unit vector. Use it to compute the 'center'
        // point for the camera.
        Vector viewDirection(sFaceDirections[i][0], sFaceDirections[i][1],
                             sFaceDirections[i][2], 0.0);
        Vector center = eyePos + viewDirection;
        assert(center.w == 1.0f);

        // Up direction
        Vector up(sUpDirections[i][0], sUpDirections[i][1], sUpDirections[i][2], 0.0);

        // Make our view matrix and apply it to the scene
        Matrix cubeView;
        cubeView.LookAt(eyePos, center, up);
        scene.SetView(cubeView);

        // Render the scene to the back buffer
        scene.Render();

        // Bind the cube texture
        GL_CHECK(glActiveTexture(ENV_TEXTURE_UNIT));
        GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, mCubeTextureID));

        // Make sure we're reading from the back buffer
        GL_CHECK(glReadBuffer(GL_BACK));

        // Write the contents of the back buffer to the texture
        glCopyTexSubImage2D(sFaceNames[i], 0, 0, 0, 0, 0,
                            CUBEMAP_SIDE_SIZE, CUBEMAP_SIDE_SIZE);

        /*
         * Debugging code. Writes the cube faces to a file.
         * Note that this appears to write the images upside-down.
         * Calling window->Display() renders the buffer right-side up. Shrug.
         */
        /*
        sf::Image image;
        char num[4];
        sprintf(num, "%d\n", i);
        string filename = string("cubeface_") + string(num) + string(".jpg");
        sf::IntRect rect(0,0, CUBEMAP_SIDE_SIZE, CUBEMAP_SIDE_SIZE);
        image.CopyScreen(mContext->window, rect);
        image.SaveToFile(filename.c_str());
        */
    }

    // Reapply the camera to the scene
    scene.SetViewToCamera();

    // Reset the projection matrix and viewport
    mSceneGraph->context->SetupView();

    // All done
    mDoingEnvMap = false;

    // Generate our own material
    mSceneGraph->materials.push_back(Material(*mSceneGraph->context));
    mMaterial = mSceneGraph->materials.size() - 1;
    mSceneGraph->materials[mMaterial].mDiffuse.Set(1.0, 1.0, 0.6, 1.0);
    mSceneGraph->materials[mMaterial].mSpecular.Set(1.0, 1.0, 0.6, 1.0);
    mSceneGraph->materials[mMaterial].mAmbient.Set(1.0, 1.0, 1.0, 1.0);
    mSceneGraph->materials[mMaterial].mShininess = 500.0;
}

void
SceneMesh::StoreGeometry(CollisionDetector& detector, Matrix transform)
{
    // For each triangle
    assert(mPositions.size() % 9 == 0);
    for (unsigned i = 0; i < mPositions.size(); i += 9) {

        // For each vertex
        Vector vertices[3];
        for (unsigned j = 0; j < 3; ++j)
            vertices[j].Set(mPositions[i + 3*j + 0],
                            mPositions[i + 3*j + 1],
                            mPositions[i + 3*j + 2],
                            1.0);

        // Add the transformed triangle
        detector.AddTriangle(transform.MVProduct(vertices[0]),
                             transform.MVProduct(vertices[1]),
                             transform.MVProduct(vertices[2]));
    }
}

SceneNode::SceneNode(SceneGraph* scene, Matrix transform,
                     const char* name) : mSceneGraph(scene)
                                       , mTransform(transform)
                                       , mName(name)
{
}

SceneNode::~SceneNode()
{
    // Null out our scenegraph pointer
    mSceneGraph = NULL;

    // Recursively delete our children
    for (list<SceneNode*>::iterator it = mChildren.begin();
         it != mChildren.end(); ++it)
        delete *it;

    // Clear the list of children
    mChildren.clear();

    // Clear the list of meshes
    mMeshes.clear();
}

void
SceneNode::AddChild(SceneNode* child)
{
    mChildren.push_back(child);
}

void
SceneNode::AddMesh(unsigned mesh)
{
    mMeshes.push_back(mesh);
}

SceneNode*
SceneNode::FindNode(const string& name)
{
    // Is it us?
    if (name == mName)
        return this;

    // Try our descendants
    SceneNode* rv = NULL;
    for (list<SceneNode*>::iterator it = mChildren.begin();
         it != mChildren.end(); ++it) {
        rv = (*it)->FindNode(name);
        if (rv)
            return rv;
    }

    // No results. Return null.
    return NULL;
}

void
SceneNode::Render(Matrix base)
{
    // Generate our transformation matrix
    Matrix trans = base.MMProduct(mTransform);
    GLfloat modelMat[16];
    trans.Get(modelMat);

    // Apply it to the modelview matrix
    GL_CHECK(glMatrixMode(GL_MODELVIEW));
    GL_CHECK(glPushMatrix());
    GL_CHECK(glMultMatrixf(modelMat));

    // Write it separately to the shader as well (for shadow mapping)
    SET_UNIFORMMATV(mSceneGraph->context, 4fv, "modelMatrix", modelMat);

    // Draw the meshes at this node
    for (list<unsigned>::iterator it = mMeshes.begin();
         it != mMeshes.end(); ++it)
        mSceneGraph->meshes[*it].Render();

    // Get rid of the model matrix, leaving GL with just the view matrix
    GL_CHECK(glMatrixMode(GL_MODELVIEW));
    GL_CHECK(glPopMatrix());

    // Draw child nodes
    for (list<SceneNode*>::iterator it = mChildren.begin();
         it != mChildren.end(); ++it)
        (*it)->Render(trans);
}

void
SceneNode::StoreGeometry(CollisionDetector& detector, Matrix modelMat)
{
    // Generate the transform for this node
    Matrix trans = modelMat.MMProduct(mTransform);

    // Store all the meshes
    for (list<unsigned>::iterator it = mMeshes.begin();
         it != mMeshes.end(); ++it)
        mSceneGraph->meshes[*it].StoreGeometry(detector, trans);

    // Store child nodes
    for (list<SceneNode*>::iterator it = mChildren.begin();
         it != mChildren.end(); ++it)
        (*it)->StoreGeometry(detector, trans);
}

SceneGraph::SceneGraph() : rootNode(this, Matrix(), "248_SCENEGRAPH_ROOT")
{
}

SceneGraph::~SceneGraph()
{
    // Destroy the materials
    for (vector<Material>::iterator it = materials.begin();
         it != materials.end(); ++it)
        it->Destroy();
}

void
SceneGraph::Init(Context& c)
{
    context = &c;
}

void
SceneGraph::Render()
{
    rootNode.Render(Matrix());
}

SceneMesh*
SceneGraph::FindMesh(const string& name)
{
    for (unsigned i = 0; i < meshes.size(); ++i)
        if (meshes[i].GetName() == name)
            return &meshes[i];
    return NULL;
}

void
SceneGraph::StoreGeometry(CollisionDetector& detector)
{
    rootNode.StoreGeometry(detector, Matrix());
}

void
SceneGraph::LoadScene(const char* path, const char* sceneName, SceneNode* parent)
{
    // Import the scene
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcessPreset_TargetRealtime_Quality);
    if (!scene || scene->mNumMeshes <= 0) {
        std::cerr << importer.GetErrorString() << std::endl;
        exit(-1);
    }

    // Within an aiScene, there are many references to material and
    // mesh indices. Since we can load multiple aiScenes, we need to determine
    // the offset relative to which the aiScene indices are valid.
    unsigned meshOffset = meshes.size();
    unsigned materialOffset = materials.size();

    // Load the materials
    for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
        materials.push_back(Material(*context));
        materials.back().InitWithMaterial(scene->mMaterials[i]);
    }

    // Load the meshes
    for (unsigned i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[i];

        // 3DS files tend to have duplicate mesh names, so we generate
        // uniqueness rather than enforcing it
        string meshName = string(sceneName) + string("_") + string(mesh->mName.data);
        while (FindMesh(meshName) != NULL)
            meshName += string("_");

        // Add the mesh to the list and initialize it
        meshes.push_back(SceneMesh(this, meshName.c_str(),
                                   materialOffset + mesh->mMaterialIndex));
        meshes.back().InitWithMesh(mesh);
    }

    // Make the nodes
    LoadNode(parent, scene->mRootNode, sceneName, meshOffset);
}

void
SceneGraph::LoadNode(SceneNode* parent, aiNode* node, const char* sceneName,
                     unsigned meshOffset)
{
    // Determine the transform at this node
    Matrix transform;
    transform.Set(node->mTransformation);

    // Make the node name, and make sure it's unique
    string nodeName = string(sceneName) + string("_") + string(node->mName.data);
    assert(rootNode.FindNode(nodeName) == NULL);

    // Generate the SceneNode
    SceneNode* sceneNode = new SceneNode(this, transform, nodeName.c_str());

    // Add the meshes
    for (unsigned i = 0; i < node->mNumMeshes; ++i)
        sceneNode->AddMesh(node->mMeshes[i] + meshOffset);

    // Attach this node to its parent
    parent->AddChild(sceneNode);

    // Add the children
    for (unsigned i = 0; i < node->mNumChildren; ++i)
        LoadNode(sceneNode, node->mChildren[i], sceneName, meshOffset);
}
