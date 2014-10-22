#pragma once

enum {
  Mesh_PrimLine,
  Mesh_PrimTriangle,
  Mesh_PrimTriangleStrip,
  Mesh_PrimQuad
};

class k3dRenderMesh {
public:
  k3dRenderMesh();
  virtual ~k3dRenderMesh();

  int     GetPrimitiveType() const;

  virtual void Draw();

protected:

  int m_NumVertices;
  int m_PrimType;
};
