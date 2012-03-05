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
 
/*
 *	POP_Repulse modifications done by Chris Rydalch - add anything important
 */

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Color.h>
#include <GEO/GEO_PrimPart.h>
#include <GEO/GEO_Point.h>
#include <GU/GU_Detail.h>
#include <PRM/PRM_Include.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>

#include "POP_Repulse.h"

//-----------------------------------------------------------------------------

using namespace HDK_Sample;

PRM_Template
POP_Repulse::myTemplateList[] =
{
    PRM_Template(PRM_FLT_J,     1, &POPactivateName, PRMoneDefaults, 0, &PRMunitRange),
    //PRM_Template(PRM_STRING,    1, &POPsourceName, 0, &POP_Node::pointGroupMenu),
    PRM_Template()
};

OP_TemplatePair
POP_Repulse::myTemplatePair (myTemplateList, &POP_LocalVar::myTemplatePair);

OP_VariablePair
POP_Repulse::myVariablePair (0, &POP_LocalVar::myVariablePair);

//-----------------------------------------------------------------------------

void
newPopOperator (OP_OperatorTable* table)
{
    table->addOperator(
        new OP_Operator("repulse",		 					// Name
                        "Repulse",	                   	 	// English
                        POP_Repulse::myConstructor,  	 	// "Constructor"
                        &POP_Repulse::myTemplatePair,  		// simple parms
                        1,                             	 	// MinSources
                        1,				 					// MaxSources
			&POP_Repulse::myVariablePair));					// variables 
}

//-----------------------------------------------------------------------------

OP_Node*
POP_Repulse::myConstructor (OP_Network* net, const char* name, 
			      OP_Operator* entry)
{
    return new POP_Repulse(net, name, entry);
}

int *POP_Repulse::myIndirect = 0;

POP_Repulse::POP_Repulse (OP_Network* net, const char* name, 
			      OP_Operator* entry)
	      :POP_LocalVar (net, name, entry) 
{
    if (!myIndirect)
	myIndirect = allocIndirect(sizeof(myTemplateList)/sizeof(PRM_Template));
}

POP_Repulse::~POP_Repulse (void) 
{
}

OP_ERROR
POP_Repulse::cookPop (OP_Context& context)
{
    POP_ContextData*    			data = (POP_ContextData*) context.getData();
    //float							t = context.getTime();
    //int							thread = context.getThread();
    GEO_PrimParticle*   			part;
    //GEO_PrimParticle*   			part2;
    GEO_ParticleVertex* 			pvtx;
    GEO_ParticleVertex* 			pvtx2;
    UT_String           			sourceName;
    const GB_PointGroup 			*sourceGroup = NULL;

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
    if (!checkActivation(data, (POP_FParam) &POP_Repulse::ACTIVATE))
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

    if (error() >= UT_ERROR_ABORT)
        goto done;

    // Need to set this for the local variables. It represents the
    // $ITER variable. Also need to keep track of myCurrPt which
    // tells the local variables which point to retrieve values from.
    myCurrIter = 0;

    // If a source group is specified, then process only the points in
    // that group.
    if (sourceGroup)
    {
        FOR_ALL_GROUP_POINTS(data->getDetail(), sourceGroup, myCurrPt)		
		{
            changePoint(myCurrPt, myCurrPt, data); // TODO adapt algorithm to group
	    	myCurrIter++;
		}
    }
    else
    {
	// Process each particle primitive fed into this POP and then
	// each particle within that primitive.
        for (part = myParticleList.iterateInit() ; part ; part = myParticleList.iterateNext())
        {
			//cout<<"Entered 'part' for-loop...."<<endl;
			for (pvtx = part->iterateInit() ; pvtx ; pvtx = pvtx->next)
			{
				//pvtx2 = pvtx->next;
				myCurrPt = pvtx->getPt();
				pvtx2 = pvtx->next;
				for (pvtx2 ; pvtx2 ; pvtx2 = pvtx2->next)
				{
					//myCurrPt = pvtx->next;
					nextPt = pvtx2->getPt();
					changePoint(myCurrPt, nextPt, data);
					//cout<<"myCurrIter= "<<myCurrIter<<"-----------------------------------"<<endl<<endl;
					myCurrIter++;
				}

			}	    	
        }
    }

done:

    cleanupDynamicVars();

    unlockInputs();

    // Reset this so something is defined for UI dialog updates.
    myCurrPt = NULL;

    return error();
}

void
POP_Repulse::changePoint (GEO_Point* ppt, GEO_Point* ppt2, POP_ContextData* data)
{
	UT_Vector3		a1, a2, p1, p2, dir;//, v1, v2;
	int				id1, id2; // For debugging
	float			radius1, radius2, m1, m2, distSqrd;
	
	// TODO Enable per-particle mass/pscale attributes - mass causes NaN, pscale uses first particle's value, I think
		
	/*cout<<"id1: "<<id1<<"  id2: "<<id2<<endl<<endl;
 	id1 = ppt->getValue<int>( data->getIDOffset());
	id2 = ppt2->getValue<int>( data->getIDOffset());*/
 	
	p1 = ppt->getPos();
	p2 = ppt2->getPos();
	
	a1 = ppt->getValue<UT_Vector3>( data->getAccelOffset());
	a2 = ppt2->getValue<UT_Vector3>( data->getAccelOffset());
	/*
	v1 = ppt->getValue<UT_Vector3>( data->getVelocityOffset());
	v2 = ppt2->getValue<UT_Vector3>( data->getVelocityOffset());
	*/
	m1 = ppt->getValue<float>( data->getMassOffset() );
	m2 = ppt2->getValue<float>( data->getMassOffset() );
	
	radius1 = ppt->getValue<float>( data->getScaleOffset() );
	radius2 = ppt2->getValue<float>( data->getScaleOffset() );

	dir = p1 - p2;
	distSqrd = dir.length2();
	
	if (distSqrd > 0.0f)
	{
		dir.normalize();
		
		float F = 1.0f/distSqrd;
		
		a1 += dir * ( F / m1 );
		a2 -= dir * ( F / m2 );
		
		ppt->setValue<UT_Vector3>(data->getAccelOffset(), a1  );
		ppt2->setValue<UT_Vector3>(data->getAccelOffset(), a2  );
		
	}
	
	//cout<<"Done with changePoint()"<<endl<<endl;

}


