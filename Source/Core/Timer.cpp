#include "Timer.h"

#include "LogUtil.h"

#ifdef K3DPLATFORM_OS_WIN
#include <Windows.h>
#include <intrin.h>
#elif defined(K3DPLATFORM_OS_LINUX)
#include <time.h>
///
/// \brief rdtsc for x64 machine
/// \return
///
static __inline__ unsigned long long __rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

static int64 frequency = 0;
static int64 QueryFrequency()
{
    timespec sp;
    frequency = ::clock_getres(CLOCK_MONOTONIC,&sp);
}

#endif


#ifdef K3DPLATFORM_OS_WIN 
static int64 frequency = 0;

static int64 GetPerformanceFrequency()
{
  static int64 counterFrequency = 0;
  LARGE_INTEGER frequency;
  if ( !QueryPerformanceFrequency( &frequency ) ) {
    counterFrequency = 0;
  }
  else {
    counterFrequency = frequency.QuadPart;
  }
  return counterFrequency;
}

static inline int64 GetPerformanceCounter()
{
  if ( frequency > 0 ) {
    LARGE_INTEGER counter;

    if ( QueryPerformanceCounter( &counter ) ) {
      return counter.QuadPart;
    }
    else {
      k3d::kDebug( "QueryPerformanceCounter failed, although QueryPerformanceFrequency succeeded." );
      return 0;
    }
  }
  else
    return 0;
}
#endif

static inline int64 ticksToNanoseconds( int64 ticks )
{
  if ( frequency > 0 ) {
    // QueryPerformanceCounter uses an arbitrary frequency
    int64 seconds = ticks / frequency;
    int64 nanoSeconds = (ticks - seconds * frequency) * 1000000000 / frequency;
    return seconds * 1000000000 + nanoSeconds;
  }
  else {
    // GetTickCount(64) return milliseconds
    return ticks * 1000000;
  }
}

namespace k3d {

	Timer::Timer(Precision precision)
		: m_Precision(precision)
		, m_BaseTime(0)
		, m_CurrentTime(0)
		, m_FrameRate(0)
		, m_Enabled(false)
	{
#ifdef K3DPLATFORM_OS_WIN
		frequency = GetPerformanceFrequency();
#elif defined(K3DPLATFORM_OS_LINUX)
		frequency = QueryFrequency();
#endif
	}


	Timer::~Timer()
	{
	}

	void Timer::ResetTimer()
	{
#ifdef K3DPLATFORM_OS_WIN
		m_CurrentTime = GetPerformanceCounter();
#endif
	}

	int64 Timer::MicrosecElapsed()
	{
#ifdef K3DPLATFORM_OS_WIN
		m_LastTime = m_CurrentTime;
		m_CurrentTime = GetPerformanceCounter();
#endif
		return ticksToNanoseconds(m_CurrentTime - m_LastTime) / 1000000;
	}

	void Timer::BeginTimer()
	{
		m_BaseTime = __rdtsc();
	}

	int64 Timer::EndTimer()
	{
		m_OffSetTime = __rdtsc() - m_BaseTime;
		return m_OffSetTime;
	}

	void Timer::Update()
	{
		m_FrameRate = 1.0f*m_OffSetTime / frequency;
		//SetTimer( 0, 0, 0, TimerFunction );
	}

	Timer* Timer::CreateNewTimer()
	{
		return new Timer;
	}

	void Timer::EnableTimer(bool enable)
	{
		m_Enabled = enable;
	}

}