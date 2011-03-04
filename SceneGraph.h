#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include "Framework.h"
#include <vector>
#include <list>
#include <string>
#include "Material.h"
#include "Matrix.h"

#define CUBEMAP_SIDE_SIZE 500

class RenderContext;
class SceneGraph;

struct SceneVertex {

    Vector position;
    Vector normal;
    Vector tangent;
    Vector bitangent;
    Vector texcoord;
};

class SceneMesh {

    public:

    /*
     * Simple constructor.
     */
    SceneMesh(SceneGraph* scene, const char* name, unsigned material);

    /*
     * Name getter.
     */
    const std::string& GetName() { return mName; }

    /*
     * Renders a Mesh.
     */
    void Render(RenderContext& renderContext);

    /*
     * Add a triangle to the mesh.
     */
    void AddTriangle(SceneVertex& v1, SceneVertex& v2, SceneVertex& v3);

    /*
     * Helper routine to initialize us with an aiMesh.
     */
    void InitWithMesh(const aiMesh* mesh);

    /*
     * Environment maps this mesh.
     */
    void EnvironmentMap(RenderContext& scene, Vector& eyePos);

    /*
     * Store the geometry in worldspace.
     */
    //void StoreGeometry(CollisionDetector& detector, Matrix transform);

    protected:

    /*
     * Helper to add a vetex. We keep this private to make sure
     * that we only have triangles.
     */
    void AddVertex(SceneVertex& v);

    // Pointer to our scene graph
    SceneGraph* mSceneGraph;

    // Our material index
    unsigned mMaterial;

    // Position array (3 per vertex)
    std::vector<GLfloat> mPositions;

    // Normal, tangent, and bitangent arrays (3 per vertex)
    std::vector<GLfloat> mNormals;
    std::vector<GLfloat> mTangents;
    std::vector<GLfloat> mBitangents;

    // Texture coordinate array (2 per vertex)
    std::vector<GLfloat> mTexcoords;

    // The name of this mesh
    std::string mName;

    // For environment mapping
    GLuint mCubeTextureID;
    bool mDoingEnvMap;
};

class SceneNode {

    public:

    /*
     * Constructor.
     */
    SceneNode(SceneGraph* scene, Matrix transform, const char* name);

    /*
     * Destructor. destroys all child nodes.
     */
    ~SceneNode();

    void AddChild(SceneNode* child);
    void AddMesh(unsigned mesh);

    /*
     * Recursive DFS to find a node with a given name.
     */
    SceneNode* FindNode(const std::string& name);

    /*
     * Renders the node, starting from the coordinates
     * induced by the base matrix.
     */
    void Render(RenderContext& renderContext, Matrix base);

    /*
     * Stores the geometry of this node in worldspace.
     */
    //void StoreGeometry(CollisionDetector& detector, Matrix modelMat);

    protected:

    // Pointer to the scene graph
    SceneGraph* mSceneGraph;

    // The transformation applied at this node
    Matrix mTransform;

    // The name of this node
    std::string mName;

    // Child nodes
    std::list<SceneNode*> mChildren;

    // meshes at this node
    std::list<unsigned> mMeshes;
};

struct SceneGraph {

    /*
     * Publicly accessible members.
     */

    // The root node in the scene graph
    SceneNode rootNode;

    // A vector of our meshes
    std::vector<SceneMesh> meshes;

    /*
     * Constructor.
     */
    SceneGraph();

    /*
     * Destructor.
     */
    ~SceneGraph();

    /*
     * Adds a node to the scenegraph.
     *
     * Returns a pointer to the added node.
     */
    SceneNode* AddNode(SceneNode* parent, Matrix transform, const char* name);

    /*
     * Adds an aiScene, descending from the given node.
     */
    void LoadScene(RenderContext& renderContext,
                   const char* filename, const char* sceneName,
                   SceneNode* parent);

    /*
     * Renders the scene graph.
     */
    void Render(RenderContext& renderContext);

    /*
     * Finds a mesh with the given name. NULL if not found.
     */
    SceneMesh* FindMesh(const std::string& name);

    protected:

    /*
     * Helper method to load a node.
     */
    void LoadNode(SceneNode* parent, aiNode* node, const char* sceneName,
                  unsigned meshOffset);
};


#endif /* SCENEGRAPH_H */
