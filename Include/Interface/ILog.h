#pragma once
#ifndef __ILog_h__
#define __ILog_h__

namespace k3d
{
	enum class ELogLevel
	{
		Default,
		Debug,
		Info, /** [Message] */
		Warn, /** [Warning] */
		Error,/** [Error]*/
		Fatal, /** [Fatal]*/
		Profile
	};

	class ILogger
	{
	public:
		virtual ~ILogger() {}
		virtual void Log(ELogLevel const &, const char * tag, const char *) = 0;
	};
}

#endif // !__ILog_h__
