/*
 * Copyright (c) 2010
 *	Side Effects Software Inc.  All rights reserved.
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

#ifndef __POP_Impact_h__
#define __POP_Impact_h__

// If the POP is allowed to access the local variables defined for 
// particles, need to use the POP_LocalVar class.
#include <POP/POP_LocalVar.h>

#include <map>
#include <vector>
#include <string>

//#define NAME_GEO_REP                "granule_type"
#define	ATT_GRANULE_TYPE		"granType", sizeof(int), GB_ATTRIB_INT
#define ATT_PSCALE      		"pscale", sizeof(float), GB_ATTRIB_FLOAT
#define ATT_MASS        		"mass", sizeof(float), GB_ATTRIB_FLOAT

namespace HDK_Sample {

enum POP_ImpactIndex
{
    ISL_ACTIVATE,
    ISL_SOURCE,
    ISL_CENTER
};

//Adding other things like 'type' and 'mass' will be likely; see end of addBulletBody in snow solver for reference

typedef struct bulletBodystr {
    int popId;
    btRigidBody* bodyId;
} bulletbody;


class POP_Impact : public POP_LocalVar
{
public:

    static OP_Node*	myConstructor 	(   OP_Network* net, const char* name, 
				                        OP_Operator* entry);

    static PRM_Template		            myTemplateList[];
    static OP_TemplatePair	            myTemplatePair;
    static OP_VariablePair	            myVariablePair;
    
    //GETSET_DATA_FUNCS_I(NAME_GEO_REP, GeoRep);


    btDiscreteDynamicsWorld* 			dynamicsWorld;
	std::map<int, bulletbody> *rigidBodies;

	std::map<int, bulletbody>::iterator rigidBodiesIt;
	
	btBroadphaseInterface* 				broadphase;
    btDefaultCollisionConfiguration* 	collisionConfiguration;
    btCollisionDispatcher* 				dispatcher;
    btSequentialImpulseConstraintSolver* solver;

	//btVector3*                          testVar;

    //POP_ImpactBulletState               *state;

    //std::map<int, bulletBody> *m_bulletBodies;          // Added by Chris

protected:

	POP_Impact (   OP_Network* net, const char* name, 
			       OP_Operator* entry);
			       
    virtual 			~POP_Impact (void);

    virtual OP_ERROR 	cookPop		    (OP_Context &context);
    virtual void        addAttrib       (void* userdata);
    virtual void        cleanSystem     ();

private:

    float  				ACTIVATE (float t)       { FLOAT_PARM("activate", ISL_ACTIVATE, 0, t) };
    void   				SOURCE  (UT_String &s)   { STR_PARM("source", ISL_SOURCE, 0, 0) };

    static int*			myIndirect;

    void        		getVeloc (  GEO_Point* ppt, POP_ContextData* data, float t, GEO_AttributeHandle &sourceval_gah);
	void        		setVeloc (  GEO_Point* ppt, POP_ContextData* data, float t);
};


}	// End HDK_Sample namespace


#undef FLOAT_PARM
#undef INT_PARM
#undef STR_PARM
#undef STR_PARM_NE

#endif
