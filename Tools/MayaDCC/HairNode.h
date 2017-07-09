#pragma once
#include "MayaCommon.h"

#define HAIR_NODE "K3DHairNode"

class MDataBlock;

namespace k3d
{
  class HairNode : public MPxNode
  {
  public:

    HairNode();
    ~HairNode() override;

    virtual MStatus	compute(const MPlug& plug, MDataBlock& data) override;
    virtual MStatus connectionMade(const MPlug& plug, const MPlug& otherPlug, bool asSrc);
    virtual MStatus connectionBroken(const MPlug& plug, const MPlug& otherPlug, bool asSrc);

  private:
    static MObject input;
    static MObject output;

  public:
    static MTypeId  id;
    static  void*		creator();
    static  MStatus	initialize();
  };
}
