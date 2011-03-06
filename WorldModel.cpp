#include "WorldModel.h"

void
WorldModel::Init(SceneGraph& sceneGraph)
{
    // Save parameters
    mSceneGraph = &sceneGraph;
}

void
WorldModel::Step()
{
    // BOF step physics
    dynamicsWorld->stepSimulation(1/60.f,10);
    
    btTransform trans;
    fallRigidBody->getMotionState()->getWorldTransform(trans);
    
    player->moveTo(player->getX(), trans.getOrigin().getY(), player->getZ());
    
    //std::cout << "sphere height: " << trans.getOrigin().getY() << std::endl;
    // EOF step physics
}

void GetState(WorldState& stateOut)
{
}

void SetState(WorldState& stateIn)
{
}

void
WorldModel::GrowPlayer(unsigned playerID)
{
}

void
WorldModel::ShrinkPlayer(unsigned playerID)
{
}

void
WorldModel::SetPlayer(Player* p)
{
    player = p;
}

void
WorldModel::MovePlayer(unsigned playerID, int direction)
{
    switch (direction) {
        case USERINPUT_MASK_UP:
            player->move(0.1, 0.0, 0.0);
            break;
        case USERINPUT_MASK_DOWN:
            player->move(-0.1, 0.0, 0.0);
            break;
        case USERINPUT_MASK_LEFT:
            player->move(0.0, 0.0, -0.1);
            break;
        case USERINPUT_MASK_RIGHT:
            player->move(0.0, 0.0, 0.1);
            break;
        default:
            break;
    }
}

void
WorldModel::SetupPhysicsSimulation()
{
    broadphase = new btDbvtBroadphase();
    
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    
    solver = new btSequentialImpulseConstraintSolver;
    
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
    
    dynamicsWorld->setGravity(btVector3(0,-10,0));
    
    groundShape = new btStaticPlaneShape(btVector3(0,2,0),1);
    
    fallShape = new btSphereShape(1);
    
    btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)));
    
    btRigidBody::btRigidBodyConstructionInfo
    groundRigidBodyCI(0,groundMotionState,groundShape,btVector3(0,0,0));
    groundRigidBody = new btRigidBody(groundRigidBodyCI);
    
    dynamicsWorld->addRigidBody(groundRigidBody);
    
    btDefaultMotionState* fallMotionState =
    new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,50,0)));
    
    btScalar mass = 1;
    btVector3 fallInertia(0,0,0);
    fallShape->calculateLocalInertia(mass,fallInertia);
    
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,fallMotionState,fallShape,fallInertia);
    fallRigidBody = new btRigidBody(fallRigidBodyCI);
    dynamicsWorld->addRigidBody(fallRigidBody);
}

void
WorldModel::DestroyPhysicsSimulation()
{
    dynamicsWorld->removeRigidBody(fallRigidBody);
    delete fallRigidBody->getMotionState();
    delete fallRigidBody;
    
    dynamicsWorld->removeRigidBody(groundRigidBody);
    delete groundRigidBody->getMotionState();
    delete groundRigidBody;
    
    delete fallShape;
    
    delete groundShape;
    
    delete dynamicsWorld;
    delete solver;
    delete collisionConfiguration;
    delete dispatcher;
    delete broadphase;
}

