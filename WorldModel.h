#ifndef WORLDMODEL_H
#define WORLDMODEL_H

class SceneGraph;

class WorldModel {

    public:

    /*
     * Initializes the world model.
     */
    void Init(SceneGraph& sceneGraph);

    /*
     * Steps the model forward in time.
     */
    void Step();

    protected:

    // The scenegraph associated with this world
    SceneGraph* mSceneGraph;
};

#endif /* WORLDMODEL_H */
