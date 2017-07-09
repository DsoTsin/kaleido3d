#include "HairNode.h"

namespace k3d
{
  MTypeId HairNode::id(0x2017);

  MObject HairNode::input;
  MObject HairNode::output;

  HairNode::HairNode()
  {
  }

  HairNode::~HairNode()
  {
  }
  
  MStatus
  HairNode::compute(const MPlug& plug, MDataBlock& data)
  {
    return MStatus::kSuccess;
  }

  MStatus
  HairNode::connectionMade(const MPlug& plug, const MPlug& otherPlug, bool asSrc)
  {
    return MStatus::kSuccess;
  }

  MStatus
  HairNode::connectionBroken(const MPlug& plug, const MPlug& otherPlug, bool asSrc)
  {
    return MStatus::kSuccess;
  }

  void* HairNode::creator()
  {
    return new HairNode;
  }

  MStatus
  HairNode::initialize()
  {
    MFnNumericAttribute nAttr;
    output = nAttr.create("output", "out",
      MFnNumericData::kFloat, 0.0);
    nAttr.setWritable(false);
    nAttr.setStorable(false);
    
    input = nAttr.create("input", "in",
      MFnNumericData::kFloat, 0.0);
    nAttr.setStorable(true);

    addAttribute(input);
    attributeAffects(input, output);
    addAttribute(output);

    return MStatus::kSuccess;
  }
}