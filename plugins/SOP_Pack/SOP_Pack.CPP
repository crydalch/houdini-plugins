#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
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
    // btVector3 bbMin(-100,-100,-100);
    // btVector3 bbMax(100,100,100);
    // broadphase = new btAxisSweep3(bbMin,bbMax,12000);
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
    btConvexHullShape*      fallConvexShape = NULL;
    btCollisionShape*       fallShape = NULL;
    btRigidBody*            fallRigidBody = NULL;
    btDefaultMotionState*   fallMotionState = NULL;
    btScalar                btMass = 1;
    btQuaternion            bt_rot;

    UT_AutoInterrupt progress("Pack SOP: getting points");

    const btVector3 jellyBeanVerts[] =
        {
            btVector3(0.013135,-0.027011,0.013135),
            btVector3(-0.013135,-0.027011,-0.013135),
            btVector3(0.012784,-0.002048,0.012784),
            btVector3(-0.012784,-0.002048,-0.012784),
            btVector3(-0.013135,-0.028933,-0.013135),
            btVector3(0.013135,-0.028933,0.013135),
            btVector3(-0.012784,-0.004230,-0.012784),
            btVector3(0.012784,-0.004230,0.012784),
            btVector3(-0.023708,0.019194,-0.023708),
            btVector3(0.023708,0.019194,0.023708),
            btVector3(-0.023708,0.020342,-0.023708),
            btVector3(0.023708,0.020342,0.023708),
            btVector3(0.024098,-0.027244,0.024098),
            btVector3(-0.024098,-0.027244,-0.024098),
            btVector3(0.024098,-0.026337,0.024098),
            btVector3(-0.024098,-0.026337,-0.024098),
            btVector3(0.000000,-0.018784,0.000000),
            btVector3(0.000000,-0.021075,0.000000),
            btVector3(0.000000,0.013102,0.000000),
            btVector3(0.000000,0.033759,0.000000),
            btVector3(0.000000,0.015059,0.000000),
            btVector3(0.000000,-0.037049,0.000000),
            btVector3(0.000000,-0.033759,0.000000),
            btVector3(0.000000,-0.035526,0.000000),
            btVector3(0.026071,-0.011407,0.026071),
            btVector3(0.034469,0.000000,0.034469),
            btVector3(0.026071,-0.009663,0.026071),
            btVector3(-0.026071,-0.009663,-0.026071),
            btVector3(-0.034469,0.000000,-0.034469),
            btVector3(-0.026071,-0.011407,-0.026071),
            btVector3(0.016407,-0.016271,0.016407),
            btVector3(0.000000,-0.032276,0.000000),
            btVector3(-0.016407,-0.016271,-0.016407),
            btVector3(0.000000,-0.000765,0.000000),
            btVector3(-0.016407,-0.018434,-0.016407),
            btVector3(0.000000,-0.034262,0.000000),
            btVector3(0.016407,-0.018434,0.016407),
            btVector3(0.000000,-0.003081,0.000000),
            btVector3(-0.019232,0.007080,-0.019232),
            btVector3(0.019232,0.007080,0.019232),
            btVector3(0.000000,0.026791,0.000000),
            btVector3(-0.025852,0.025319,-0.025852),
            btVector3(0.025852,0.025319,0.025852),
            btVector3(0.000000,0.027978,0.000000),
            btVector3(-0.019232,0.008987,-0.019232),
            btVector3(0.019232,0.008987,0.019232),
            btVector3(0.019914,-0.030523,0.019914),
            btVector3(-0.019914,-0.030523,-0.019914),
            btVector3(0.000000,-0.035115,0.000000),
            btVector3(0.025852,-0.025319,0.025852),
            btVector3(-0.025852,-0.025319,-0.025852),
            btVector3(0.000000,-0.034249,0.000000),
            btVector3(0.019914,-0.028944,0.019914),
            btVector3(-0.019914,-0.028944,-0.019914),
            btVector3(0.031880,-0.003848,0.031880),
            btVector3(0.031880,-0.002820,0.031880),
            btVector3(-0.031880,-0.002820,-0.031880),
            btVector3(-0.031880,-0.003848,-0.031880),
        };

    fallConvexShape = new btConvexHullShape();

    for (int s=0;s<58;s++)
        fallConvexShape->addPoint(jellyBeanVerts[2]);

    //cout<<"getPos()"<<endl;
    for (int i = 0; i < numPoints; i++)
    {

        if (progress.wasInterrupted())
        {
            emptyWorldOfBodies();
            break;
        }

        posHandle.setElement(gdp->points()(i));
        p = posHandle.getV4();
        rotHandle.setElement(gdp->points()(i));
        r = rotHandle.getV4();

        // Didn't realize this was an issue, might be why the jelly bean shapes weren't rotating earlier! I'm
        //      so glad this fact was noted in the SIM_SolverBullet code!
        bt_rot.setEulerZYX( r.z(), r.y(), r.x() );

        //cout<<"\tPtNo: "<< i <<" p: "<< p <<" r: "<<endl;// r <<endl;
        //temporarily assign fallShape to a sphere, then we'll we get our convex shape working
        fallShape = new btBoxShape(btVector3(1,1,1));

        fallMotionState = new btDefaultMotionState(btTransform( bt_rot,btVector3(p.x(),p.y(),p.z()) ));

        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(btMass,fallMotionState,fallConvexShape);
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

    UT_AutoInterrupt progress("Pack SOP: packing points");

    for (int i = 0; i < numPoints; i++)
    {
        if (progress.wasInterrupted())
        {
            emptyWorldOfBodies();
            break;
        }

        rigidBodiesIt = rigidBodies->find(i);
        btPos = (*rigidBodiesIt).second.bodyId->getCenterOfMassPosition();
        btRot = (*rigidBodiesIt).second.bodyId->getOrientation();

        //cout<<"\tPtNo: "<< i <<" btPos: "<< btPos <<" btRot: "<< btRot <<endl;

        posHandle.setElement(gdp->points()(i));
        rotHandle.setElement(gdp->points()(i));

        p = posHandle.getV4();
        r = rotHandle.getV4();

        posHandle.setV4(UT_Vector4(btPos[0],btPos[1],btPos[2],1));
        rotHandle.setV4(UT_Vector4(btRot[2],btRot[1],btRot[0],btRot[3]));
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