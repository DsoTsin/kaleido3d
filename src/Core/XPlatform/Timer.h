#ifndef __k3dTimer_h__
#define __k3dTimer_h__
#pragma once
#if 0
namespace k3d
{
	class K3D_CORE_API Timer
  {
	public:

		enum Precision
		{
			NanoSecond,
			MicroSecond
		};

		explicit Timer(Precision precision = MicroSecond);
		~Timer();

		void      ResetTimer();
		int64     MicrosecElapsed();

		void      BeginTimer();
		int64     EndTimer();
		float     GetFrameRate() { return m_FrameRate; }

		void      Update();

		void      EnableTimer(bool enable);

		Timer* CreateNewTimer();

	private:
		static const int CMAX_PREFRAMES = 60;

		Precision m_Precision;
		int       m_TimerId;
	//	std::function<void()> TimerFunction;

		uint64    m_BaseTime;
		uint64    m_LastTime;
		uint64    m_OffSetTime;
		uint64    m_CurrentTime;

		uint64    m_PreFrameTime[CMAX_PREFRAMES];

		float     m_FrameRate;
		bool      m_Enabled;
	};
}
#endif
#endif
