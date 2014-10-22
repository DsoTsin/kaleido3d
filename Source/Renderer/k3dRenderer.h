#pragma once
//Low level API
class k3dRenderer
{
public:
  k3dRenderer();
  virtual ~k3dRenderer();

  virtual void Init();
  virtual void Destroy();

  virtual void FlushCommand();
  virtual int GetNumDrawCalls();


protected:

};
