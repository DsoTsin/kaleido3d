#pragma once
#include <Math/kMath.hpp>
#include <Math/kGeometry.hpp>
#include <KTL/Factory.hpp>
#include <KTL/UniqueIdentifier.hpp>

class k3dRenderNode : public Factory<k3dRenderNode>, public UniqueIdentifier<k3dRenderNode> {
public:
  enum {

    Node_Player,
    Node_Vehicle,
    Node_Camera,
    Node_Object,
    Node_StaticObject,
    Node_DynamicObject,

    Node_RenderableMax,

    Node_Light,
    Node_LightOmni,
    Node_LightSpot,
    Node_LightDirectional,
    Node_LightProbe,

    Node_Terrain,

    Node_DebugDrawable,

    Node_TypeNum

  }; // NodeType Info

  k3dRenderNode(int type);
  virtual ~k3dRenderNode();

  // uuid
  int                     GetID() const;

  static void             ClearID();
  static k3dRenderNode *  GetNode(int id);
  static int              IsNode(k3dRenderNode *node);

  // node factory
  static k3dRenderNode *  CreateNode(int type);
  static k3dRenderNode *  CreateNode(const char *name);

  inline int              GetType() const { return m_NodeType; }
  const char *            GetTypeName() const;

  void                    SetEnabled(int enable);
  int                     IsEnabled() const;

  void                    SetVisible(int visible);
  int                     IsVisible() const;

  void                    SetShadow(int shadow);
  int                     IsShadow() const;

  void                    SetName(const char *name);
  const char *            GetName() const;

  void                    SetParent(k3dRenderNode *node);
//  void                    SetWorldParent(k3dRenderNode *node);
  k3dRenderNode *         GetParent() const;

  void                    AddChild(k3dRenderNode *node);
  void                    RemoveChild(k3dRenderNode *node);
  int                     IsChild(const k3dRenderNode *node) const;
  int                     GetNumChilds() const;
  int                     FindChild(const char *name) const;
  k3dRenderNode *         GetChild(int id) const;
  int                     NeedUpdate() const;
  virtual void            Update(float deltaTime);

  static void             UpdateNodes(float deltaTime);

//  int needFlush() const;
//  virtual void flush(float ifps);
//  static void flushNodes(float ifps);

//  void setProperty(const char *name);
//  Property *getProperty() const;
//  const char *getPropertyName() const;

//  static void clearNodeProperty(Property *property);

  // world position
//  inline WorldPosition *getWorldPosition() const { return position; }

  // transformation
//  void SetTransform(const mat4 &transform);
//  inline const mat4 &GetTransform() const { return *transform; }

  void                        SetWorldTransform(const kMath::Mat4f & transform);

  inline const kMath::Mat4f & SetWorldTransform() const { return *m_WorldTransform; }
  inline const kMath::Mat4f & GetIWorldTransform() const { return *m_IWorldTransform; }

//  virtual int isWorldBounds() const;
  virtual const kMath::AABB & GetBoundingBox() const;
  virtual const kMath::BoundingSphere &getBoundSphere() const;

  // visible distances
  virtual float GetMinDistance() const;
  virtual float GetMaxDistance() const;

  virtual k3dRenderNode *Clone() const;

protected:

  // update world position
//  void update_world_position();

  virtual void UpdateEnable();
  virtual void UpdatePosition();
  virtual void UpdateTransform();

  virtual k3dRenderNode *Copy(k3dRenderNode *node) const;

protected:
  char                                m_NodeType;

private:

  unsigned char                       m_Enabled : 1;
  unsigned char                       m_IsMovable : 1;

  unsigned char                       m_Visiblity : 1;
  unsigned char                       m_ShadowFlag : 1;

  k3dString *                         m_Name;
  k3dRenderNode *                     m_Parent;
  std::vector<k3dRenderNode*>         m_Children;

  kMath::Vec3f *                      m_Position;
  // local transform
  kMath::Mat4f *                      m_Transform;
  kMath::Mat4f *                      m_WorldTransform;
  kMath::Mat4f *                      m_IWorldTransform;
};
