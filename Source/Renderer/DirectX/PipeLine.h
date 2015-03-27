#pragma once
#include <KTL/NonCopyable.hpp>
#include "DXCommon.h"
#include "Renderer.h"
#include "Device.h"

namespace k3d
{
  namespace d3d12 
  { 
	  class PipeLine : public NonCopyable
	  {
	  public:
		  explicit PipeLine(d3d12::Device & device);
		  ~PipeLine();
	  private:
	  };
  }
}