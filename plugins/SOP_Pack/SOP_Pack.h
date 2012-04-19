#ifndef __SOP_Pack_h__
#define __SOP_Pack_h__

#include <SOP/SOP_Node.h>

typedef struct bulletBodystr {
    int ptNum;
    btRigidBody* bodyId;
} bulletbody;

class SOP_Pack : public SOP_Node
{
    public:
        
        SOP_Pack(OP_Network *net, const char *name, OP_Operator *op);
        
        virtual ~SOP_Pack();

        static PRM_Template                     myTemplateList[];
        static OP_Node                          *myConstructor(OP_Network*, const char *,
                                                OP_Operator *);

        btDiscreteDynamicsWorld*                dynamicsWorld;

        std::map<int, bulletbody>               *rigidBodies;
        std::map<int, bulletbody>::iterator     rigidBodiesIt;
        
        btBroadphaseInterface*                  broadphase;
        btDefaultCollisionConfiguration*        collisionConfiguration;
        btCollisionDispatcher*                  dispatcher;
        btSequentialImpulseConstraintSolver*    solver;

    protected:
        virtual OP_ERROR            cookMySop(OP_Context &context);
        virtual void                cleanSystem();
        virtual void                emptyWorldOfBodies();

    private:
        void                        getPos(GEO_AttributeHandle& posHandle, GEO_AttributeHandle& rotHandle, int& numPoints);
        void                        setPos(GEO_AttributeHandle& posHandle, GEO_AttributeHandle& rotHandle, int& numPoints);
};

#endif