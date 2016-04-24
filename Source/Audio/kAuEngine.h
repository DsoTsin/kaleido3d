#pragma once
#include <KTL/Singleton.h>

class kAuEngine : public Singleton<kAuEngine> {
public:
  kAuEngine();
  ~kAuEngine();
};
