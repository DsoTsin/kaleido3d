#include "k3dRenderNode.h"

#include <Core/k3dDbg.h>
#include <Core/k3dLog.h>

#ifdef max
#undef max
#undef min
#endif

#include <limits>
#include <algorithm>


static const char *type_names[] = {
  "Node_Player",
  "Node_Vehicle",
  "Node_Camera",
  "Node_Object",
  "Node_StaticObject",
  "Node_DynamicObjet",

  "Node_RenderableMax",

  "Node_Light",
  "Node_LightOmni",
  "Node_LightSpot",
  "Node_LightDirectional",
  "Node_LightProbe",

  "Node_Terrain",

  "Node_DebugDrawable",
};

k3dRenderNode::k3dRenderNode(int type) : UniqueIdentifier<k3dRenderNode>(this), m_NodeType(type)
{
  m_Name          = nullptr;
  m_Parent        = nullptr;
  m_Position      = nullptr;
  m_Transform     = nullptr;
  m_WorldTransform = nullptr;
  m_IWorldTransform= nullptr;
}

k3dRenderNode::~k3dRenderNode()
{

}

int k3dRenderNode::GetID() const
{
  return UniqueIdentifier<k3dRenderNode>::GetId();
}

void k3dRenderNode::ClearID()
{
  UniqueIdentifier<k3dRenderNode>::ClearId();
}

k3dRenderNode *k3dRenderNode::GetNode(int id)
{
  k3dRenderNode *node = GetInstance(id);
  if(node == nullptr)
    k3dLog::Error("k3dRenderNode::GetNode(): can't find %d node id.",id);
  return node;
}

int k3dRenderNode::IsNode(k3dRenderNode *node)
{
  return UniqueIdentifier<k3dRenderNode>::IsInstance(node);
}

k3dRenderNode *k3dRenderNode::CreateNode(int type)
{
  if(type < 0 || type >= Node_TypeNum) {
    k3dLog::Error("k3dRenderNode::CreateNode(): unknown node type.");
    return nullptr;
  }
  k3dRenderNode *node = Factory<k3dRenderNode>::Create(type);
  assert(node->GetType() == type && "k3dRenderNode::CreateNode(): bad node creator !");
  return node;
}

k3dRenderNode *k3dRenderNode::CreateNode(const char *name)
{
  for(int i = 0; i < Node_TypeNum; i++) {
      if(!strcmp(name,type_names[i])) {
          return CreateNode(i);
      }
  }
  k3dLog::Error("k3dRenderNode::CreateNode(): unknown node type name \"%s\".", name);
  return nullptr;
}

const char *k3dRenderNode::GetTypeName() const
{
  return type_names[(int)m_NodeType];
}

void k3dRenderNode::SetEnabled(int enable)
{
  m_Enabled = enable;
}

int k3dRenderNode::IsEnabled() const
{
  if(m_Enabled == 0) return 0;
  const k3dRenderNode *node = m_Parent;
  while(node) {
    if(node->m_Enabled == 0) return 0;
    node = node->m_Parent;
  }
  return 1;
}

void k3dRenderNode::SetVisible(int visible)
{
  m_Visiblity = visible;
}

int k3dRenderNode::IsVisible() const
{
  return m_Visiblity;
}

void k3dRenderNode::SetShadow(int shadow)
{
  m_ShadowFlag = shadow;
}

int k3dRenderNode::IsShadow() const
{
  return m_ShadowFlag;
}

void k3dRenderNode::SetName(const char *name)
{
  if(m_Name != nullptr) delete m_Name;
  m_Name = new k3dString(name);
}

const char *k3dRenderNode::GetName() const
{
  return m_Name->c_str();
}

void k3dRenderNode::SetParent(k3dRenderNode *node)
{
  if(m_Parent == node)
    return;
  if(m_Parent)
    m_Parent->RemoveChild(this);
  if(node == nullptr) return;
  m_Parent = node;
  m_Parent->m_Children.push_back(this);

//  SetTransform(getTransform());
}

k3dRenderNode *k3dRenderNode::GetParent() const
{
  return m_Parent;
}

void k3dRenderNode::AddChild(k3dRenderNode *node)
{
  if( std::find(m_Children.begin(), m_Children.end(), node) != m_Children.end() )
    return;
  if(node->m_Parent) node->m_Parent->RemoveChild(node);
  node->m_Parent = this;
  m_Children.push_back(node);
//  node->SetTransform(node->GetTransform());
}

void k3dRenderNode::RemoveChild(k3dRenderNode *node)
{
  std::vector<k3dRenderNode*>::iterator iter =
      std::find(m_Children.begin(), m_Children.end(), node);

  k3dRenderNode *_node = (*iter);
  _node->m_Parent = nullptr;
  m_Children.erase(iter);
//          node->setTransform(node->getTransform());
  return;
}

int k3dRenderNode::IsChild(const k3dRenderNode *node) const
{
  for(int i = 0; i < m_Children.size(); i++) {
      if(m_Children[i] == node) return 1;
  }
  for(int i = 0; i < m_Children.size(); i++) {
      if(m_Children[i]->IsChild(node)) return 1;
  }
  return 0;
}

int k3dRenderNode::GetNumChilds() const
{
  return m_Children.size();
}

int k3dRenderNode::FindChild(const char *name) const
{
  if(name == nullptr) return -1;
  for(int i = 0; i < m_Children.size(); i++) {
      if(name != m_Children[i]->GetName()) return i;
  }
  return -1;
}

k3dRenderNode *k3dRenderNode::GetChild(int id) const
{
  return m_Children[id];
}

int k3dRenderNode::NeedUpdate() const
{
  return 1;
}

void k3dRenderNode::Update(float deltaTime)
{

}

void k3dRenderNode::UpdateNodes(float deltaTime)
{
  Iterator end = instances_end();
//  int previous_frame = engine.frame - 1;
//  for(Iterator it = instances_begin(); it != end; ++it) {
//      Node *node = it->key;
//      node->setFrame(previous_frame);
//      node->update(ifps);
  //  }
}

void k3dRenderNode::SetWorldTransform(const kMath::Mat4f &transform)
{
  assert(m_WorldTransform!=nullptr);
  *m_WorldTransform = transform;
}

const kMath::AABB &k3dRenderNode::GetBoundingBox() const
{
  static kMath::AABB aabb;
  return aabb;
}

const kMath::BoundingSphere &k3dRenderNode::getBoundSphere() const
{
  static kMath::BoundingSphere sphere;
  return sphere;
}

float k3dRenderNode::GetMinDistance() const
{
  return std::numeric_limits<float>::max();
}

float k3dRenderNode::GetMaxDistance() const
{
  return std::numeric_limits<float>::min();
}

k3dRenderNode *k3dRenderNode::Clone() const
{
  kDebug("Cannot Clone Node !\n");
  return nullptr;
}

void k3dRenderNode::UpdateEnable()
{

}

void k3dRenderNode::UpdatePosition()
{

}

void k3dRenderNode::UpdateTransform()
{

}

k3dRenderNode *k3dRenderNode::Copy(k3dRenderNode *node) const
{
  return nullptr;
}
