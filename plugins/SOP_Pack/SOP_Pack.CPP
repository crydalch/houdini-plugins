#include <UT/UT_DSOVersion.h>
#include <UT/UT_Math.h>
#include <UT/UT_Matrix3.h>
#include <UT/UT_Matrix4.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimPoly.h>
#include <PRM/PRM_Include.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <SOP/SOP_Guide.h>

#include "SOP_Pack.h"

void newSopOperator(OP_OperatorTable *table)
{
    table->addOperator(new OP_Operator( "pack",
                                        "Pack",
                                        SOP_Pack::myConstructor,
                                        SOP_Pack::myTemplateList,
                                        1,
                                        1,
                                        0));
}

static PRM_Name names[] = {
    PRM_Name("iters", "Iterations"),
};

PRM_Template SOP_Pack::myTemplateList[] = {
    PRM_Template(PRM_INT, 1, &names[0], PRMoneDefaults),
    PRM_Template(),
};

OP_Node *
SOP_Pack::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_Pack(net, name, op);
}

SOP_Pack::SOP_Pack(OP_Network *net, const char *name, OP_Operator *op)
    : SOP_Node(net, name, op), myGroup(0)
{
}

SOP_Pack::~SOP_Pack() 
{
}

OP_ERROR SOP_Pack::cookInputGroups(OP_Context &context, int alone)
{
    // The SOP_Node::cookInputPointGroups() provides a good default
    // implementation for just handling a point selection.
    return cookInputPointGroups(context, myGroup, myDetailGroupPair, alone);
}

OP_ERROR
SOP_Pack::cookMySop(OP_Context &context)
{


    gdp
    unlockInputs();
    return error();
}