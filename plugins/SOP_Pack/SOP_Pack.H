#ifndef __SOP_Pack_h__
#define __SOP_Pack_h__

#include <SOP/SOP_Node.h>

class SOP_Pack : public SOP_Node
{
public:
    SOP_Pack(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~SOP_Pack();

    static PRM_Template         myTemplateList[];
    static OP_Node              *myConstructor(OP_Network*, const char *, OP_Operator *);
    virtual OP_ERROR            cookInputGroups(OP_Context &context, int alone = 0);

protected:
    virtual OP_ERROR         cookMySop(OP_Context &context);

private:
    fpreal  AMP(fpreal t)       { return evalFloat("amp", 0, t); }

    GU_DetailGroupPair   myDetailGroupPair;

    const GA_PointGroup *myGroup;
};

#endif