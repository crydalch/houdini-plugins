#include <UT/UT_DSOVersion.h>
#include <SYS/SYS_Math.h>
#include <GU/GU_Detail.h>
#include <GEO/GEO_AttributeHandle.h>
#include <PRM/PRM_Include.h>
#include <OP/OP_Director.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include <BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h>
#include <BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

#include "SOP_Pack.h"

void newSopOperator(OP_OperatorTable *table)
{
     table->addOperator(new OP_Operator("pack_sop",
                    "Pack SOP",
                     SOP_Pack::myConstructor,
                     SOP_Pack::myTemplateList,
                     1,
                     1,
                     0));
}

PRM_Template SOP_Pack::myTemplateList[] = {
    PRM_Template(),
};


OP_Node *SOP_Pack::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_Pack(net, name, op);
}

SOP_Pack::SOP_Pack(OP_Network *net, const char *name, OP_Operator *op)
    : SOP_Node(net, name, op)
{
    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver;
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
    rigidBodies = new std::map<int, bulletbody>();
}

SOP_Pack::~SOP_Pack()
{
    cleanSystem();
}

OP_ERROR SOP_Pack::cookMySop(OP_Context &context)
{
    fpreal       frame;
    int          npts;

    if (lockInputs(context) >= UT_ERROR_ABORT)
        return error();

    // Duplicate input geometry
    duplicateSource(0, context);

    npts = gdp->points().entries();

    GEO_AttributeHandle     Phandle, Rhandle;
    UT_Vector4              Pvalue, Rvalue;

    Phandle = gdp->getAttribute(GEO_POINT_DICT, "P");
    Rhandle = gdp->getAttribute(GEO_POINT_DICT, "rot");

    // Get Points
    getPos(Phandle, Rhandle, npts);

    //Step Bullet World Here
    dynamicsWorld->stepSimulation(0.12,240);

    setPos(Phandle, Rhandle, npts);

    //cleanSystem();
    emptyWorldOfBodies();

    unlockInputs();
    return error();
}

void SOP_Pack::getPos(GEO_AttributeHandle& posHandle, GEO_AttributeHandle& rotHandle, int& numPoints)
{
    UT_Vector4              p, r;
    bulletbody              currBody;
    btCollisionShape*       fallShape = NULL;
    btRigidBody*            fallRigidBody = NULL;
    btDefaultMotionState*   fallMotionState = NULL;
    btScalar                btMass = 10;
    //cout<<"getPos()"<<endl;
    for (int i = 0; i < numPoints; i++)
    {
        posHandle.setElement(gdp->points()(i));
        p = posHandle.getV4();
        rotHandle.setElement(gdp->points()(i));
        r = rotHandle.getV4();
        //cout<<"\tPtNo: "<< i <<" p: "<< p <<" r: "<<endl;// r <<endl;
        //temporarily assign fallShape to a sphere, then we'll we get our convex shape working
        fallShape = new btSphereShape(0.5);

        fallMotionState = new btDefaultMotionState(btTransform( btQuaternion(0,0,0,1),btVector3(p.x(),p.y(),p.z()) ));

        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(btMass,fallMotionState,fallShape);
        fallRigidBody = new btRigidBody(fallRigidBodyCI);
        dynamicsWorld->addRigidBody(fallRigidBody);

        currBody.ptNum = i;
        currBody.bodyId = fallRigidBody;

        rigidBodies->insert(std::make_pair(i,currBody));
    }
}

void SOP_Pack::setPos(GEO_AttributeHandle& posHandle, GEO_AttributeHandle& rotHandle, int& numPoints)
{
    UT_Vector4              p, r;
    btVector3               btPos;
    btQuaternion            btRot;
    //cout<<"setPos()"<<endl;
    for (int i = 0; i < numPoints; i++)
    {
        rigidBodiesIt = rigidBodies->find(i);
        btPos = (*rigidBodiesIt).second.bodyId->getCenterOfMassPosition();
        btRot = (*rigidBodiesIt).second.bodyId->getOrientation();

        //cout<<"\tPtNo: "<< i <<" btPos: "<< btPos <<" btRot: "<< btRot <<endl;

        posHandle.setElement(gdp->points()(i));
        rotHandle.setElement(gdp->points()(i));

        p = posHandle.getV4();
        r = rotHandle.getV4();

        posHandle.setV4(UT_Vector4(btPos[0],btPos[1],btPos[2],1));
        rotHandle.setV4(UT_Vector4(btRot[0],btRot[1],btRot[2],btRot[3]));
    }
}

void SOP_Pack::emptyWorldOfBodies()
{
    if (dynamicsWorld != NULL)
    {
        for( int i =  dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; --i ) 
        {
            btCollisionObject* obj =  dynamicsWorld->getCollisionObjectArray()[i];  
            dynamicsWorld->removeCollisionObject( obj );
            delete obj;
        }
        rigidBodies->clear();
    }
}

void SOP_Pack::cleanSystem()
{

    if (dynamicsWorld != NULL)
    {
        for( int i =  dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; --i ) 
        {
            btCollisionObject* obj =  dynamicsWorld->getCollisionObjectArray()[i];  
            dynamicsWorld->removeCollisionObject( obj );
            delete obj;
        }

        broadphase = NULL;
        collisionConfiguration = NULL;
        dispatcher = NULL;
        solver = NULL;
        dynamicsWorld = NULL;

        delete broadphase;
        delete collisionConfiguration;
        delete dispatcher;
        delete solver;
        delete dynamicsWorld;

        rigidBodies->clear();
        delete rigidBodies;
    }
}