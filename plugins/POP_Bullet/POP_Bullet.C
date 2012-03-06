/*
 * Copyright (c) 2010
 *    Side Effects Software Inc.  All rights reserved.
 *
 * Redistribution and use of Houdini Development Kit samples in source and
 * binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. The name of Side Effects Software may not be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE `AS IS' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *----------------------------------------------------------------------------
 */

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Color.h>
#include <GEO/GEO_PrimPart.h>
#include <GEO/GEO_Point.h>
#include <GU/GU_Detail.h>
#include <PRM/PRM_Include.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_ChoiceList.h>

//----------------------------------------------------------------------------- 

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include <BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h>
#include <BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

//-----------------------------------------------------------------------------

#include "POP_Bullet.h"

#include <map>
#include <vector>
#include <string>


using namespace HDK_Sample;
/*
static PRM_Name        names[] =
{
    PRM_Name("center",        "Center"),
    PRM_Name(0)
};*/

/*
static PRM_Name        names[] ={
        PRM_Name("granules",    "Granule Types"),
};*/

//static PRM_Name    theGeoRep(NAME_GEO_REP,     "Granule Type");
/*
static PRM_Name        theGeoRepNames[] = {

        PRM_Name("0",        "Sphere"),
        PRM_Name("1",        "Cylinder"),
        //PRM_Name(GEO_REP_NINE,            "9-Spheres"),
        //PRM_Name(GEO_REP_TWO_BOXES,        "TwoBoxes"),
        PRM_Name(0),
};


static PRM_ChoiceList   theGeoRepNamesMenu((PRM_ChoiceListType)(PRM_CHOICELIST_REPLACE | PRM_CHOICELIST_EXCLUSIVE ), &(theGeoRepNames[0]) );
*/


PRM_Template
POP_Bullet::myTemplateList[] =
{
    PRM_Template(PRM_FLT_J,     1,  &POPactivateName,    PRMoneDefaults,    0,     &PRMunitRange),  // Activation Param UI
    PRM_Template(PRM_STRING,    1,  &POPsourceName,      0,     &POP_Node::pointGroupMenu),         // Source Group Param UI
    //PRM_Template(PRM_STRING,    1,    &theGeoRep, &defGeoRep, &theGeoRepNamesMenu),        // Define which shape to use
    //PRM_Template(PRM_XYZ_J,     3,  &names[0]),                                                      
    PRM_Template()
};

OP_TemplatePair
POP_Bullet::myTemplatePair (myTemplateList, &POP_LocalVar::myTemplatePair);

OP_VariablePair
POP_Bullet::myVariablePair (0, &POP_LocalVar::myVariablePair);

//-----------------------------------------------------------------------------

void
newPopOperator (OP_OperatorTable* table)
{
    table->addOperator(
        new OP_Operator("POP_Bullet",                        // Name
                        "Impact",                           // English
                        POP_Bullet::myConstructor,          // "Constructor"
                        &POP_Bullet::myTemplatePair,        // simple parms
                        1,                                     // MinSources
                        1,                                    // MaxSources
            &POP_Bullet::myVariablePair));                  // variables 
}

//-----------------------------------------------------------------------------

OP_Node*
POP_Bullet::myConstructor (     OP_Network* net, const char* name, 
                                OP_Operator* entry)
{
    return new POP_Bullet(net, name, entry);

    
}

int *POP_Bullet::myIndirect = 0;


POP_Bullet::POP_Bullet (    OP_Network* net, 
                            const char* name, OP_Operator* entry)
                            :POP_LocalVar (net, name, entry) 
{
    if (!myIndirect)
    {
        myIndirect = allocIndirect(sizeof(myTemplateList)/sizeof(PRM_Template));
    }

    //testVar = new btVector3(1,1,1);

    //cout<<"Just starting bullet world..."<<endl;

    //dynamicsWorld = NULL;

    //btCollisionShape* fallShape;

    //fallShape = NULL;
 
    broadphase = new btDbvtBroadphase(); // Look at trying to get the CUDA broadphase working...
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver;
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
    //cout<<"Finished creating dynamicsWorld..."<<endl;

    // This causes a huge freeze, no error messages, Houdini just goes sub-zero. Why?
    rigidBodies = new std::map<int, bulletbody>();

    //cout<<"Just finished bullet world..."<<endl;

}

POP_Bullet::~POP_Bullet (void) 
{
    cleanSystem();
}

OP_ERROR
POP_Bullet::cookPop (OP_Context& context)
{
    POP_ContextData*        data = (POP_ContextData*) context.getData();
    float                   t = context.getTime();
    GEO_PrimParticle*       part;
    UT_String               sourceName;
    const GA_PointGroup*    sourceGroup = NULL;
    float                   step = data->myTimeInc;
    GEO_AttributeHandle     sourceval_gah;
    UT_Interrupt*           boss = UTgetInterrupt();

    //cout<<"Starting to cook, time at: "<<t<<endl;

    //btCollisionObjectArray totalObjects;

    //cout<<"Done creating variables..."<<endl;
    
    //
    // Bullet Initialization    
    //
/*
    if (dynamicsWorld == NULL)
    {

        cout<<"Just starting bullet world..."<<endl;
        
        btBroadphaseInterface* broadphase = new btDbvtBroadphase(); // Look at trying to get the CUDA broadphase working...

        btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
        btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

        cout<<"Halfway there..."<<endl;
        
        btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
        
        cout<<"Finished solver..."<<endl;

        cout<<""<<endl;
        
        dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
        cout<<"Finished creating dynamicsWorld..."<<endl;

        // This causes a huge freeze, no error messages, Houdini just goes sub-zero. Why?
        rigidBodies = new std::map<int, bulletbody>();

        cout<<"Just finished bullet world..."<<endl;
    }*/

    //
    // End Bullet Initialization
    //

/*
    cout<<"TestVector: "<<testVar->getX()<<endl;

    testVar->setX( testVar->getX()+1 );
    
    cout<<"TestVector (added): "<<testVar->getX()<<endl;
*/
    if (lockInputs(context) >= UT_ERROR_ABORT)
        return(error());

    setupDynamicVars(data);

    // Build the particle list that this POP needs to process.
    if (buildParticleList(context) >= UT_ERROR_ABORT)
        goto done;

    // Check if the cook is being requested just because we need an
    // update of the guide geometry. If so, don't need to do anything else.
    if (data->isGuideOnly())
        goto done;

    // check activation event
    if (!checkActivation(data, (POP_FParam) &POP_Bullet::ACTIVATE))
        goto done;

    // Point group to act on.
    SOURCE(sourceName);
    if (sourceName.isstring())
    {
        sourceGroup = parsePointGroups((const char*) sourceName,
                                       data->getDetail());
        if (!sourceGroup)
        {
            addError(POP_BAD_GROUP, sourceName);
            goto done;
        }
    }

    // In order for the local variables to work, we must initialize them.
    setupVars(data, sourceGroup);

    // For efficiency, the parameter values are cached if they do not
    // change per particle (e.g. constants or expressions not dependent
    // on particle variables). Use these macros to cache the values.
    // This is done using POP_FCACHE()
    
    if (error() >= UT_ERROR_ABORT)
        goto done;

    // Need to set this for the local variables. It represents the
    // $ITER variable. Also need to keep track of myCurrPt which
    // tells the local variables which point to retrieve values from.
    myCurrIter = 0;
    

    //
    // If there are no particles, destroy the bullet world completely, perhaps?
    //

    // Set bullet gravity to zero, so only POP forces affect the objects 
    dynamicsWorld->setGravity(btVector3(0,0,0));
    
    // Add and eval the 'bullet_type' attribute to the points
    //data->getDetail()->addPointAttrib("bullet_type", 1 * sizeof(int), GB_ATTRIB_INT, GB_ATTRIB_INFO_NONE, 0);
    
    sourceval_gah = data->getDetail()->getPointAttribute("granule");
    
    // THOUGHT: Perhaps look at GEOPRIM_Particle's iternateNext() as a faster way to do this?


    //cout<<"Getting ready to iterate through each particle..."<<endl;
    //cout<<"myCurrIter: "<<myCurrIter<<endl<<endl<<endl;
    //sleep(3);


    //If on first frame, clean the system. TODO: make it look at the start frame/time of the scene

    if (t == 0)
    {
        if (dynamicsWorld != NULL)
            cleanSystem();

        broadphase = new btDbvtBroadphase(); // Look at trying to get the CUDA broadphase working...
        collisionConfiguration = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConfiguration);
        solver = new btSequentialImpulseConstraintSolver;
        dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
        //cout<<"Finished creating dynamicsWorld..."<<endl;

        // This causes a huge freeze, no error messages, Houdini just goes sub-zero. Why?
        rigidBodies = new std::map<int, bulletbody>();
        //cout<<"cleaned the house, built world :) "<<endl;
    }
/*
    else
    {
        cleanSystem();
    }
*/

    

    
    // If a source group is specified, then process only the points in
    // that group.
    if (sourceGroup)
    {
        GEO_Point *ppt;
        GA_FOR_ALL_GROUP_POINTS(data->getDetail(), sourceGroup, ppt)
        {
            getVeloc(ppt, data, t, sourceval_gah);
            //cout<<"myCurrIter: "<<myCurrIter<<endl;
            //sleep(0.5);
            myCurrIter++;
        }
    }
    else
    {
    // Process each particle primitive fed into this POP and then
    // each particle within that primitive.
        for (part = myParticleList.iterateInit() ; part ; part = myParticleList.iterateNext())
        {
            if (boss->opInterrupt())
                goto done;
            
            GEO_Point *ppt;
            //cout<<"First level of per particle loop: "<<myCurrIter<<endl;
            //cout<<"part= "<<part<<endl;
            for (POP_ParticleIterator it(part); !it.atEnd(); ++it)
            {
                ppt = part->getParent()->getGEOPoint(*it);
                myCurrPtOff = ppt->getMapOffset();
                //cout<<"Inside loop, going into getVeloc on iter: "<<myCurrIter<<endl;
                getVeloc(ppt, data, t, sourceval_gah);
                //cout<<"myCurrIter: "<<myCurrIter<<" myCurrPt: "<<myCurrPt<<endl;
                //cout<<""<<endl<<endl<<endl<<endl;
                //sleep(0.5);
                myCurrIter++;
            }
        }
    }

    //cout<<"About to step through world..."<<endl;
    //sleep(3);

    // Step through the bullet simulation once
    
    dynamicsWorld->stepSimulation( step , 10);             // This could read the oversampling parameter on the POP Network 

    myCurrIter = 0;
    //totalObjects = dynamicsWorld->getCollisionObjectArray();
        //fallRigidBody->getMotionState()->getWorldTransform(trans);
    /*    
    for (int size=0; size < totalObjects.size(); size++)
    {
        cout<<"Number of objects in bullet world: "<<size+1<<endl;
        cout<<"Veloc of Bullet Object: "<<
    }*/
    //cout<<"--------------------------------"<<endl<<endl;

    //cout<<"Finished stepping through world..."<<endl;

    // Now take the velocity from the bulletBodies and apply it back onto the particles

    //cout<<"About to start setVelocity() function..."<<endl;
    
    if (sourceGroup)
    {
        GEO_Point *ppt;
        GA_FOR_ALL_GROUP_POINTS(data->getDetail(), sourceGroup, ppt)
        {
            setVeloc(ppt, data, t);
            //cout<<"myCurrIter: "<<myCurrIter<<endl;
            //sleep(0.5);
            myCurrIter++;
        }
    }
    else
    {
    // Process each particle primitive fed into this POP and then
    // each particle within that primitive.
        for (part = myParticleList.iterateInit() ; part ; part = myParticleList.iterateNext())
        {
            if (boss->opInterrupt())
                goto done;
            
            GEO_Point *ppt;
            //cout<<"First level of per particle loop: "<<myCurrIter<<endl;
            //cout<<"part= "<<part<<endl;
            for (POP_ParticleIterator it(part); !it.atEnd(); ++it)
            {
                ppt = part->getParent()->getGEOPoint(*it);
                myCurrPtOff = ppt->getMapOffset();
                //cout<<"Inside loop, going into getVeloc on iter: "<<myCurrIter<<endl;
                setVeloc(ppt, data, t);
                //cout<<"myCurrIter: "<<myCurrIter<<" myCurrPt: "<<myCurrPt<<endl;
                //cout<<""<<endl<<endl<<endl<<endl;
                //sleep(0.5);
                myCurrIter++;
            }
        }
    }
    


/*
    dynamicsWorld->removeRigidBody(fallRigidBody);
    delete fallRigidBody->getMotionState();
    delete fallRigidBody;

    delete fallShape;

    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
*/
    //cout<<"Done with cook, with time at:"<<t<<endl<<endl<<"|--------------------------------------|"<<endl<<endl<<endl<<endl<<endl;

done:

    cleanupDynamicVars();

    unlockInputs();

    // Reset this so something is defined for UI dialog updates.
    //myCurrPt = NULL;

    return error();
}


//
// This function applies the velocity from the particle to the corresponding bulletBody. A new bulletBody is created if the particle doesn't have one in the map.
//

void
POP_Bullet::getVeloc ( GEO_Point* ppt, POP_ContextData* data, float t, GEO_AttributeHandle &sourceval_gah)
{
    UT_Vector3            center;
    UT_Vector3            color;
    UT_Vector3          d;
    UT_Vector3          p;
    UT_Color            HSVtoRGB(UT_HSV);
    UT_Color            RGBtoHSV(UT_RGB);

    
    //cout<<"In getVeloc..."<<endl;

    //
    // Get variables to send data to bullet engine
    //

    //GU_Detail* gdp = ppt;
    UT_Vector3          vel;                        
    int                 id;
    bulletbody          currBody;       // This is created to help keep track of the particle id and bulletbody id
    float               pscale;            // Used for uniform spheres, box primitives, and granules
    UT_Vector3            scale;            // For non-uniform spheres, capsules, cones, and cylinders
    int                    btShape;        
    float               mass;
    //GB_Attribute*       btShape;        // Attribute created to define each particle's shape

    //std::map<int, bulletbody>::iterator rigidBodiesIt;
    
    // Added above, in cook()
    //data->getDetail()->addPointAttrib("bullet_type", 1 * sizeof(int), GB_ATTRIB_INT, GB_ATTRIB_INFO_NONE, 0);

    p = ppt->getPos();
    vel = ppt->getValue<UT_Vector3>( data->getVelocityOffset());
    id = ppt->getValue<int>( data->getIDOffset());
    mass = ppt->getValue<float>( data->getMassOffset() );
    pscale = ppt->getValue<float>( data->getScaleOffset() );
    scale = ppt->getValue<float>( data->getScale3Offset() );
    
    
    btShape = sourceval_gah.getI();
        
    
    //cout<<" id: "<<id<<" Mass: "<<mass<<" pscale: "<<pscale<<endl;

    //
    // Add a loop here to see if the id matches the dead list; if it does, remove the bulletbody
    //
    
    rigidBodiesIt = rigidBodies->find(id);

    //cout<<"rigidBodiesIt (id): "<<(*rigidBodiesIt).first<<endl;

    // If there is no bulletbody mapped to this id, create the sphere at this position
    if ( rigidBodiesIt == rigidBodies->end() )               
    {
        //cout<<"Nothing in rigidBodies map..."<<endl;

        btCollisionShape* fallShape = NULL;   // TODO put pscale attr here


        // This is where you can reassing granule shapes via their point attribute

        if (btShape == 0)
            fallShape = new btSphereShape(pscale);
            
        //else if (btShape == 1)
            //fallShape = new btCapsuleShape( scale.x(), scale.y() );
        
        
        
        btRigidBody* fallRigidBody;
        
        btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3( p.x(),p.y(),p.z() )));
        btScalar btMass = mass;                                      // TODO look at the mass attr
        
        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(btMass,fallMotionState,fallShape);

        fallRigidBody = new btRigidBody(fallRigidBodyCI);
        dynamicsWorld->addRigidBody(fallRigidBody);

        //cout<<"fallRigidBody: "<<fallRigidBody<<endl;

        //Add the bulletbody to the rigidBodies map 
        // EXAMPLE: state->m_bulletBodies->insert(std::make_pair( currObject->getObjectId(), currBody ));

        currBody.popId = id;
        currBody.bodyId = fallRigidBody;

        //cout<<"currBody: "<<currBody.bodyId<<endl<<endl;
        
        rigidBodies->insert( std::make_pair(id,currBody) );
        
        rigidBodiesIt = rigidBodies->find(id);

        
        //cout<<"rigidBody added..."<<endl<<endl;

        //cout<<"rigidBodiesIt: "<<(*rigidBodiesIt).first<<" "<<(*rigidBodiesIt).second.bodyId<<endl<<endl;

    }
    
    // Get the velocity from the particle, and give it to the bulletBodystr

    if (rigidBodiesIt != rigidBodies->end() )
    {
        //cout<<"... Ready to do PtVeloc to btVeloc ..."<<endl;
        rigidBodiesIt = rigidBodies->find(id);
    
        (rigidBodiesIt->second.bodyId)->setLinearVelocity( btVector3(vel[0],vel[1],vel[2]) );
        //cout<<"Part -> Bullet veloc..."<<endl<<endl;
    }
    

    //std::map< int, bulletBody >::iterator bodyIt;
    

    // Check to see if state has been created
    
    //state = new POP_BulletBulletState();

    //This looks to see if this particle has ever been in the bullet world 
    //bodyIt = state->m_bulletBodies->find( myCurrPt );
    
    //bodyIt = state->m_bulletBodies->end();
    //cout<<"End of getVelocity function!"<<endl<<endl;
    
}

//
// This function goes through each particle, and grabs the new velocity from its corresponding bulletBody
//


// For some reason, this function isn't getting the right rigidBodies and rigidBodiesIt values

void
POP_Bullet::setVeloc ( GEO_Point* ppt, POP_ContextData* data, float t )
{
    btVector3       btVel;
    //std::map<int, bulletbody>::iterator rigidBodiesIt;

    UT_Vector3          vel;                        
    UT_Vector3          p;
    int                 id;
    //bulletbody          currBody;       // This is created to help keep track of the particle id and bulletbody id
    float               mass;

    p = ppt->getPos();
    vel = ppt->getValue<UT_Vector3>(data->getVelocityOffset());
    id = ppt->getValue<int>(data->getIDOffset());
    mass = ppt->getValue<float>(data->getMassOffset() );

    rigidBodiesIt = rigidBodies->find(id);
    btVel = (*rigidBodiesIt).second.bodyId->getLinearVelocity();

    

    if (mass <= 0)
    {   
        btTransform btrans;
        btVel = btVector3(0,0,0);
           
        btrans.setOrigin( btVector3(p[0],p[1],p[2]) );
        
        (*rigidBodiesIt).second.bodyId->getMotionState()->setWorldTransform(btrans);
    }
        
    //cout<<"Inside of setVeloc function (still).."<<endl;
    //cout<<"TimeStep: "<<t<<endl
    //cout << "rigidBodiesIt: " << &rigidBodiesIt << endl;
    //cout << "rigidBodiesIt.second: " << &((*rigidBodiesIt).second) << endl;
    //cout << "rigidBodiesIt.second.bodyId: " << (*rigidBodiesIt).second.bodyId<<endl;
    //cout<<"rigidBodiesIt.second: "<<endl;//(*rigidBodiesIt).second.bodyId<<endl;
    
    //btVel = (*rigidBodiesIt).second.bodyId->getLinearVelocity();

    //cout<<"Bullet Veloc: "<<UT_Vector3(btVel[0],btVel[1],btVel[2] )<<endl;
    
    
    // TODO position and rotation probably need to be used, if we're using these for the granules
    

    ppt->setValue<UT_Vector3>(data->getVelocityOffset(), UT_Vector3(btVel[0],btVel[1],btVel[2] )  );
    
}

void
POP_Bullet::addAttrib (void* userdata)
{
    //POP_ContextData*            data = (POP_ContextData*) userdata;

    // Make sure that this detail has the color attribute.
    //addDiffuseAttrib(data);
}

void
POP_Bullet::cleanSystem()
{

    if (dynamicsWorld != NULL)
    {
        //cout<<"Starting to destroy the (bullet) world..."<<endl;

        for( int i =  dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; --i ) 
        {
            btCollisionObject* obj =  dynamicsWorld->getCollisionObjectArray()[i];

            //rigidBodiesIt = rigidBodies->find(i);
            //delete (*rigidBodiesIt).second;

            //delete body->getMotionState();      
            dynamicsWorld->removeCollisionObject( obj );
            delete obj;
        }

        delete broadphase;
        delete collisionConfiguration;
        delete dispatcher;
        delete solver;
        delete dynamicsWorld;

        broadphase = NULL;
        collisionConfiguration = NULL;
        dispatcher = NULL;
        solver = NULL;
        dynamicsWorld = NULL;

        rigidBodies->clear();
        delete rigidBodies;
        
        //cout<<"... finished destroying the world. Muahahahaha!"<<endl;
    }
}