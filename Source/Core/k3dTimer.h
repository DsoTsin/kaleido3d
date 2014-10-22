#ifndef __k3dTimer_h__
#define __k3dTimer_h__
#pragma once

#include <Config/Prerequisities.h>

class k3dTimer
{
public:

  enum Precision
  {
    NanoSecond,
    MicroSecond
  };

  explicit k3dTimer(Precision precision = MicroSecond);
  ~k3dTimer();

  void      ResetTimer();
  int64     MicrosecElapsed();

  void      BeginTimer();
  int64      EndTimer();
  float     GetFrameRate() { return m_FrameRate; }

  void      Update();

  void      EnableTimer( bool enable );

  k3dTimer* CreateNewTimer();

private:
  static const int CMAX_PREFRAMES = 60;

  Precision m_Precision;
  int       m_TimerId;
  std::function<void()> TimerFunction;

  uint64    m_BaseTime;
  uint64    m_LastTime;
  uint64    m_OffSetTime;
  uint64    m_CurrentTime;

  uint64    m_PreFrameTime[ CMAX_PREFRAMES ];

  float     m_FrameRate;
  bool      m_Enabled;
};

#endif
