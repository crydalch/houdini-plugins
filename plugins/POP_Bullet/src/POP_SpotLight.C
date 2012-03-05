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

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Color.h>
#include <GEO/GEO_PrimPart.h>
#include <GEO/GEO_Point.h>
#include <GU/GU_Detail.h>
#include <PRM/PRM_Include.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include "POP_SpotLight.h"

//-----------------------------------------------------------------------------

using namespace HDK_Sample;

static PRM_Name		names[] =
{
    PRM_Name("center",		"Center"),
    PRM_Name(0)
};

PRM_Template
POP_SpotLight::myTemplateList[] =
{
    PRM_Template(PRM_FLT_J,     1, &POPactivateName, PRMoneDefaults, 0,
                 &PRMunitRange),
    PRM_Template(PRM_STRING,    1, &POPsourceName, 0,
                 &POP_Node::pointGroupMenu),
    PRM_Template(PRM_XYZ_J,     3, &names[0]),
    PRM_Template()
};

OP_TemplatePair
POP_SpotLight::myTemplatePair (myTemplateList, &POP_LocalVar::myTemplatePair);

OP_VariablePair
POP_SpotLight::myVariablePair (0, &POP_LocalVar::myVariablePair);

//-----------------------------------------------------------------------------

void
newPopOperator (OP_OperatorTable* table)
{
    table->addOperator(
        new OP_Operator("hdk_spotlight",		 // Name
                        "SpotLight",                   	 // English
                        POP_SpotLight::myConstructor,  	 // "Constructor"
                        &POP_SpotLight::myTemplatePair,  // simple parms
                        1,                             	 // MinSources
                        1,				 // MaxSources
			&POP_SpotLight::myVariablePair));// variables 
}

//-----------------------------------------------------------------------------

OP_Node*
POP_SpotLight::myConstructor (OP_Network* net, const char* name, 
			      OP_Operator* entry)
{
    return new POP_SpotLight(net, name, entry);
}

int *POP_SpotLight::myIndirect = 0;

POP_SpotLight::POP_SpotLight (OP_Network* net, const char* name, 
			      OP_Operator* entry)
	      :POP_LocalVar (net, name, entry) 
{
    if (!myIndirect)
	myIndirect = allocIndirect(sizeof(myTemplateList)/sizeof(PRM_Template));
}

POP_SpotLight::~POP_SpotLight (void) 
{
}

OP_ERROR
POP_SpotLight::cookPop (OP_Context& context)
{
    POP_ContextData*    data = (POP_ContextData*) context.getData();
    float		t = context.getTime();
    int			thread = context.getThread();
    GEO_PrimParticle*   part;
    GEO_ParticleVertex* pvtx;
    POP_FParam		centerx;
    POP_FParam		centery;
    POP_FParam		centerz;
    UT_String           sourceName;
    const GB_PointGroup *sourceGroup = NULL;


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
    if (!checkActivation(data, (POP_FParam) &POP_SpotLight::ACTIVATE))
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
    POP_FCACHE(centerx, CENTERX, getCenterX, myCenterX, POP_SpotLight);
    POP_FCACHE(centery, CENTERY, getCenterY, myCenterY, POP_SpotLight);
    POP_FCACHE(centerz, CENTERZ, getCenterZ, myCenterZ, POP_SpotLight);

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
            changePoint(myCurrPt, data, t, centerx, centery, centerz);
	    myCurrIter++;
	}
    }
    else
    {
	// Process each particle primitive fed into this POP and then
	// each particle within that primitive.
        for (part = myParticleList.iterateInit() ;
             part ; part = myParticleList.iterateNext())
        {
	    for (pvtx = part->iterateInit() ; pvtx ; pvtx = pvtx->next)
	    {
		myCurrPt = pvtx->getPt();
	        changePoint(myCurrPt, data, t, centerx, centery, centerz);
		myCurrIter++;
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
POP_SpotLight::changePoint (GEO_Point* ppt, POP_ContextData* data, float t,
			    POP_FParam centerx, POP_FParam centery, 
			    POP_FParam centerz)
{
    UT_Vector3		center;
    UT_Vector3		color;
    UT_Vector3          d;
    UT_Vector3          p;
    UT_Color		HSVtoRGB(UT_HSV);
    UT_Color		RGBtoHSV(UT_RGB);
    float		r, g, b;
    float		h, s, v;
    float		d2;


    // Use the POP_PEVAL maxro to evaluate the parameter. If the value
    // is cached, it retrieves the value from cache otherwise, it evaluates
    // the parameter for this particle.
    center.assign(POP_PEVAL(centerx), POP_PEVAL(centery), POP_PEVAL(centerz));

    // Find out the distance from the point to the specified center.
    p = ppt->getPos();
    d = p - center;

    // Get the color attribute for this point.
    color = ppt->getValue<UT_Vector3>(data->getDiffuseOffset());

    // Convert from RGB to HSV space. 
    RGBtoHSV.setValue(color.x(), color.y(), color.z());
    RGBtoHSV.getHSV(&h, &s, &v);

    // Scale the intensity (V) by the inverse square of the distance length.
    d2 = d.length2();
    v = UTequalZero(d2) ? 1.0f : 1.0f / d.length2();

    // The intensity should never exceed 1.0.
    if (v > 1.0f)
	v = 1.0f;
    HSVtoRGB.setValue(h, s, v);
    HSVtoRGB.getRGB(&r, &g, &b);

    color.assign(r, g, b);
    ppt->setValue<UT_Vector3>(data->getDiffuseOffset(), color);
}

void
POP_SpotLight::addAttrib (void* userdata)
{
    POP_ContextData*            data = (POP_ContextData*) userdata;

    // Make sure that this detail has the color attribute.
    addDiffuseAttrib(data);
}
