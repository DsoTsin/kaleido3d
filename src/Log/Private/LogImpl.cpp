#include <Core/Kaleido3D.h>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <condition_variable>

#include <Public/ILogModule.h>

#include <Core/App.h>
#include <Core/Os.h>
#include <Core/WebSocket.h>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#if K3DPLATFORM_OS_MAC
#import <Foundation/NSObjCRuntime.h>
#endif

using namespace std;

namespace k3d
{
	static inline const char* GetLocalTime() {
		time_t t = time(NULL);
		struct tm *tm = localtime(&t);
		static char time_info[128] = { 0 };
		::snprintf(time_info, 128, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
		return time_info;
	}

	class FileLogger : public ILogger
	{
	public:
		FileLogger()
		{
			String name = Os::Path::Join(GetEnv()->GetLogDir(), GetEnv()->GetInstanceName() + ".log");
			m_LogFile.Open(name.CStr(), IOWrite);
			m_Thread = new Os::Thread([this]()->void {
				while (true)
				{
					if (m_Logs.empty())
					{
						unique_lock<std::mutex> uSignal(m_Signal);
						m_CV.wait(uSignal);
					}
					else
					{
						lock_guard<mutex> scopeLock(m_LogMutex);
						while (!m_Logs.empty())
						{
							std::string log = m_Logs.front();
							m_LogFile.Write(log.data(), log.size());
							m_Logs.pop();
						}
					}
				}
			}, "FileLogger");
			m_Thread->Start();
		}

		~FileLogger() override
		{
			m_LogFile.Close();
		}

		void Log(ELogLevel const & logLv, const char * tag, const char * msg) override
		{
			lock_guard<mutex> scopeLock(m_LogMutex);
			static char sCurBuffer[4096] = { 0 }; // 4K buffer less than websocket buffer size
			snprintf(sCurBuffer, 4096, "[%s]@[%s]:%s\n", GetLocalTime(), Os::Thread::GetCurrentThreadName().CStr(), msg);
			m_Logs.push(sCurBuffer);
			m_CV.notify_one();
		}

	private:
		Os::File				m_LogFile;
		std::queue<std::string> m_Logs;
		Os::Thread*				m_Thread;
		mutex					m_LogMutex;
		mutex					m_Signal;
		condition_variable		m_CV;
	};


	class WebSocketLogger : public ILogger, public Net::WebSocket
	{
	public:
		static const uint32 BUF_LEN = 8192;

		WebSocketLogger() : Net::WebSocket()
		{
			m_Thread = new Os::Thread([this]()->void {
				this->BindAndListen();
				Os::IPv4Address unnamedClient("");
				Os::SocketHandle client;
				while (true)
				{
					client = this->Accept(unnamedClient);
					if (client == -1)
					{
						Os::Sleep(1000);
						continue;
					}

					char buffer[BUF_LEN + 1] = { 0 };
					uint64 recvLen = this->Receive(client, buffer, BUF_LEN);
					if (recvLen > 0 && recvLen < BUF_LEN)
					{
						//OutputDebugStringA(buffer);
					}

					while (true)
					{
						bool canQuit = false;
						if (m_Logs.empty())
						{
							unique_lock<std::mutex> uSignal(m_Signal);
							m_CV.wait(uSignal);
						}
						else
						{
							lock_guard<mutex> scopeLock(m_LogMutex);
							while (!m_Logs.empty())
							{
								LogItem log = m_Logs.front();
								string output = log.JsonStr();
								uint64 sent = Send(client, output.data(), (uint32)output.size());
								m_Logs.pop();
								if (sent <= 0)
									canQuit = true;
							}
						}
						if (canQuit)
						{
							break;
						}
					}
				}
			}, "WebsocketLogger");
			m_Thread->Start();
		}

		~WebSocketLogger() override
		{
		}

		void Log(ELogLevel const & lv, const char * tag, const char * logLine) override
		{
#if 0
			{
				lock_guard<mutex> scopeLock(m_LogMutex);
				static char sCurBuffer[4096] = { 0 }; // 4K buffer less than websocket buffer size
				snprintf(sCurBuffer, 4096, "[%s]@[%s]:%s", GetLocalTime(), Os::Thread::GetCurrentThreadName().CStr(), logLine);
				m_Logs.push({ sCurBuffer, tag, lv });
			}
			m_CV.notify_one();
#endif
		}

	protected:
		struct LogItem
		{
			LogItem(string const &log, string const &tag, ELogLevel const& lv)
				: LogLine(log), Tag(tag), LogLv(lv)
			{}
			string		JsonStr()
			{
				rapidjson::StringBuffer s;
				rapidjson::Writer<rapidjson::StringBuffer> writer(s);
				writer.StartObject();
				writer.Key("LogLevel");
				writer.Int(int(LogLv));
				writer.Key("Tag");
				writer.String(Tag.c_str());
				writer.Key("Log");
				writer.String(LogLine.c_str());
				writer.EndObject();
				return s.GetString();
			}
			string		LogLine;
			string		Tag;
			ELogLevel	LogLv;
		};

		void BindAndListen()
		{
			Create();
			Os::IPv4Address addr(":7000");
			this->SetBlocking(true);
			Bind(addr);
			Listen(10);
		}

	private:
		queue<LogItem>			m_Logs;
		Os::Thread*				m_Thread;
		mutex					m_LogMutex;
		mutex					m_Signal;
		condition_variable		m_CV;
	};

	class ConsoleLogger : public ILogger
	{
	public:
		ConsoleLogger()
		{
		}

		~ConsoleLogger()
		{
		}

		void Log(ELogLevel const &Lv, const char * Tag, const char * Msg) override
		{
#if K3DPLATFORM_OS_WIN
			static thread_local char logBuffer[4096] = { 0 };
			sprintf(logBuffer, "[%s]-[%d]\t%s\t%s\n", GetLocalTime(), Lv, Tag, Msg);
			OutputDebugStringA(logBuffer);
#elif K3DPLATFORM_OS_MAC
            NSLog(@"%s::%s", Tag, Msg);
#elif K3DPLATFORM_OS_ANDROID
            
#else
            
#endif
		}
	};

	class KawaLogModule : public ILogModule
	{
	public:

		KawaLogModule()
		{
		}
		~KawaLogModule()
		{
			m_pLoggers.clear();
		}

		void Start() override
		{}
		
		void Shutdown() override
		{}
		
		const char * Name() override
		{
			return "KawaLog";
		}

		ILogger* GetLogger(ELoggerType const& type) override {
			if (m_pLoggers.find(type) == m_pLoggers.end()) {
				if (!m_pLoggers[type]) {
					lock_guard<mutex> scopeLock(m_CreateMutex);
					if (!m_pLoggers[type]) {
						switch (type) {
							case ELoggerType::EConsole:
								m_pLoggers[type] = new ConsoleLogger;
								break;
							case ELoggerType::EFile:
								m_pLoggers[type] = new FileLogger;
								break;
							case ELoggerType::EWebsocket:
								m_pLoggers[type] = new WebSocketLogger;
								break;
						}
					}
				}
			}
			return m_pLoggers[type];
		}

	private:
		struct Hash
		{
			int operator ()(ELoggerType type) const
			{
				return (int)type;
			}
		};
		unordered_map<ELoggerType,ILogger*,Hash>m_pLoggers;
		mutex									m_CreateMutex;
	};
}

MODULE_IMPLEMENT(KawaLog, k3d::KawaLogModule)
