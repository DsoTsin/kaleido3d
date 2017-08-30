#include "Network.h"
#include "HttpModule.h"
#include <Core/Os.h>
#if WITH_OPENSSL
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif

#include <unordered_map>

using namespace k3d;

namespace Net
{
    namespace Http
    {
        class Connection;
        class Client;
        using SpConn = SharedPtr<Connection>;
        using SpClient = SharedPtr<Client>;

        bool SplitUri(String const& Uri, String& Protocol, String& Host, int& Port, String& File, bool& UseSSL)
        {
            const char *Ptr = *Uri;
            const char *sl = strstr(Ptr, "//");
            unsigned int offs = 0;
            Port = -1;
            bool SSL = false;
            if (sl)
            {
                size_t colon = Uri.FindFirstOf(':');
                size_t firstslash = Uri.FindFirstOf('/');
                if (colon < firstslash)
                    Protocol = Uri.SubStr(0, colon);
                if (strncmp(Ptr, "http://", 7) == 0)
                {
                    offs = 7;
                    Port = 80;
                }
                else if (strncmp(Ptr, "https://", 8) == 0)
                {
                    offs = 8;
                    Port = 443;
                    SSL = true;
                }
                else
                    return false;

                Ptr = sl + 2;
            }

            sl = strchr(Ptr, '/');
            if (!sl)
            {
                Host = Ptr;
                File = "/";
            }
            else
            {
                Host = Uri.SubStr(offs, sl - Ptr);
                File = sl;
            }

            size_t colon = Host.FindFirstOf(':');
            if (colon != String::npos)
            {
                Port = atoi(*Host + colon + 1);
            }
            UseSSL = SSL;

            return true;
        }

        struct Header : public IHttpHeader
        {
            String RawHeader;
            std::unordered_map<String, String> HeaderFields;

            IHttpHeader& AddCookie(String const& Cookie)
            {
                return *this;
            }
            IHttpHeader& AddHeader(String const& Key, String const& Value)
            {
                HeaderFields[Key] = Value;
                return *this;
            }
        };

        struct Response : public IHttpResponse
        {
            Response() : ChunkedTransfer(false), ContentLength(0), Stream(nullptr) {}

            HttpResult GetResult() const { return Result; }
            String const& GetData() const { return Data;  }

            void SetStream(IHttpStream* pHttpStream) override
            {
                Stream = pHttpStream;
            }

            HttpResult  Result;
            Header      Header;
            bool        ChunkedTransfer;
            uint64      ContentLength;
            String      Data;
            IHttpStream*Stream;
        };

        struct Request : public IHttpRequest
        {
            Request(HttpMethod M, String Uri, SpConn Conn);

            const String&   GetHost() const override;
            HttpMethod      GetMethod() const override { return Method; }
            Http::PtrResp   GetResponse(bool bOnlyGetResponseHeader) override;

            SpConn      OwningConn;
            HttpMethod  Method;
            int         Port;
            bool        UseSSL;
        };

        class Connection : public Os::Socket, public IHttpConnection,
            public k3d::EnableSharedFromThis<Connection>
        {
        public:

            enum ParseError
            {
                BadHttpHeader = -10001,
                InCompleteHeader = -10002,
            };

            Connection(String TargetHost, SpClient _Client, bool UseHttps = false)
                : Socket(Os::SockType::TCP)
                , Host(TargetHost)
                , V4SrvAddr(NULL)
                , OwningClient(_Client)
                , HeaderParsed(false)
                , SpeedNotifier(nullptr)
            {
                auto Pos = TargetHost.FindLastOf(':');
                int Port = UseHttps? 443 : 80;
                if (Pos != String::npos)
                {
                    String PortStr = TargetHost.SubStr(Pos + 1, TargetHost.Length() - Pos - 1);
                    Port = atoi(PortStr.CStr());
                }
                hostent* RemoteHost = ::gethostbyname(TargetHost.CStr());
                switch (RemoteHost->h_addrtype)
                {
                case AF_INET:
                {
                    V4SrvAddr = new Os::IPv4Address();
                    struct in_addr addr;
                    int i = 0;
                    while (RemoteHost->h_addr_list[i] != 0)
                    {
                        addr.s_addr = *(u_long *)RemoteHost->h_addr_list[i++];
                        V4SrvAddr->SetAddr(addr);
                    }
                    V4SrvAddr->SetIpPort(Port);
                    break;
                }
                case AF_INET6:
                    break;
                }
                Create();
                SetBlocking(true);
                Connect(*V4SrvAddr);
                Port++;
            }

            virtual ~Connection() override
            {
                if (V4SrvAddr)
                {
                    //disconnect()
                    delete V4SrvAddr;
                }
            }

            // --- Module Interface ----
            void InstallSpeedNotifier(INetSpeedNotifier* Notifier) override { SpeedNotifier = Notifier; }
            Http::PtrHeader MakeHeader() override { return k3d::MakeShared<Header>(); }
            virtual Http::PtrReq MakeRequest(HttpMethod Method, String const& UriPath) override
            {
                return k3d::MakeShared<Request>(Method, UriPath, SharedFromThis());
            }


            virtual uint64 SendData(String const& Data)
            {
                return Send(GetHandle(), Data);
            }

            void ProcessDataStream(Response& Resp, uint64 RecvLen, const char* Buffer, int64 SzBuf)
            {
                if (!HeaderParsed)
                {
                    int64 HeaderEndOffset = ParseHeader(Buffer, RecvLen, Resp);
                    if (HeaderEndOffset > 0) // Complete Header Parsed
                    {
                        int64 SzContentPiece = RecvLen - HeaderEndOffset - 4;
                        if (SzContentPiece >= 0)
                        {
                            HeaderParsed = true;
                            if (SzContentPiece > 0 && Resp.ContentLength > 0)
                            {
                                if (Resp.Stream)
                                {
                                    Resp.Stream->Write(Buffer + HeaderEndOffset + 4, SzContentPiece);
                                }
                            }
                            return;
                        }
                    }
                }
                else
                {
                    Resp.Stream->Write(Buffer, SzBuf);
                }
            }

            virtual uint64 GetData(Response& Resp)
            {
                char Buffer[4096] = { 0 };
                uint64 RecvLen = 0;
                {
                    // Insert Speed Notifier
                    RecvLen = Receive(GetHandle(), Buffer, 4096);
                }
                ProcessDataStream(Resp, RecvLen, Buffer, 4096);
                return RecvLen;
            }

            int64 ParseHeader(const char* Data, int SzBuffer, Response& RespHeader)
            {
                const char* HeaderPtr = Data;
                int64 HeaderEndOffSet = 0;
                if ((SzBuffer >= 5) && memcmp("HTTP/", HeaderPtr, 5)) // Check Http
                {
                    return BadHttpHeader;
                }
                const char *HeaderEnd = strstr(HeaderPtr, "\r\n\r\n"); // Find Header End
                if (!HeaderEnd)
                {
                    return InCompleteHeader;
                }
                HeaderEndOffSet = HeaderEnd - Data;
                HeaderPtr = strchr(HeaderPtr + 5, ' ');
                if (!HeaderPtr)
                    return BadHttpHeader;
                ++HeaderPtr;
                RespHeader.Result = (HttpResult)atoi(HeaderPtr);
                RespHeader.ChunkedTransfer = false;
                RespHeader.ContentLength = 0;
                HeaderPtr = strstr(HeaderPtr, "\r\n");
                const char *CurPtr = HeaderPtr;
                const char * const MaxPtr = Data + SzBuffer;
                while (CurPtr < MaxPtr)
                {
                    while (isspace(*CurPtr))
                    {
                        ++CurPtr;
                        if (CurPtr >= MaxPtr)
                        {
                            return HeaderEndOffSet > 0 ? HeaderEndOffSet : InCompleteHeader;
                        }
                    }
                    const char * const Colon = strchr(CurPtr, ':');
                    if (!Colon)
                        return BadHttpHeader;
                    const char *ValEnd = strchr(Colon, '\n');
                    if (!ValEnd)
                        return BadHttpHeader;
                    while (ValEnd[-1] == '\n' || ValEnd[-1] == '\r')
                        --ValEnd;
                    const char *Val = Colon + 1;
                    while (isspace(*Val) && Val < ValEnd)
                        ++Val;
                    String Key(CurPtr, Colon - CurPtr);
                    String Value(Val, ValEnd - Val);
                    RespHeader.Header.HeaderFields[Key] = Value;
                    if (strcmp("Content-Length", *Key) == 0)
                    {
                        RespHeader.ContentLength = atoi(*Value);
                    }
                    CurPtr = ValEnd;
                }
                return HeaderEndOffSet;
            }

            virtual void OnHandleError(int Code) override
            {
            }

            const String& GetHost() const { return Host; }

            SpClient GetClient() const { return OwningClient; }

        protected:
            INetSpeedNotifier*  SpeedNotifier;
            SpClient           OwningClient;
            bool                HeaderParsed;

        private:
            String              Host;
            Os::IPv4Address*    V4SrvAddr;
        };

        class SSLConnection : public Connection
        {
        public:
            SSLConnection(String TargetHost, SpClient _Client);
            ~SSLConnection() override;

            uint64 SendData(String const& Data) override;
            uint64 GetData(Response& Resp) override;

        private:
#if WITH_OPENSSL
            SSL*                SSL_;
            SSL_CTX*            SSL_Context;
#endif
        };

        SSLConnection::SSLConnection(String TargetHost, SpClient _Client) : Connection(TargetHost, _Client, true)
        {
#if WITH_OPENSSL
            SSL_library_init();
            SSL_load_error_strings();
            SSL_Context = SSL_CTX_new(SSLv23_client_method());
            SSL_ = SSL_new(SSL_Context);
            int ret = SSL_set_fd(SSL_, GetHandle());
            if (ret == 0)
            {

            }
            ret = SSL_connect(SSL_);
#endif
        }

        SSLConnection::~SSLConnection()
        {
#if WITH_OPENSSL
            int ret = SSL_shutdown(SSL_);
            if (ret != 1)
            {
            }
            SSL_free(SSL_);
            SSL_CTX_free(SSL_Context);
            ERR_free_strings();
#endif
        }

        uint64 SSLConnection::SendData(String const& Data)
        {
            uint64 Sent = SSL_write(SSL_, *Data, Data.Length());
            if (Sent == -1)
            {
            }
            return Sent;
        }

        uint64 SSLConnection::GetData(Response& Resp)
        {
            char Buffer[4096] = { 0 };
            uint64 RecvLen = 0;
            RecvLen = SSL_read(SSL_, Buffer, 4096);
            ProcessDataStream(Resp, RecvLen, Buffer, 4096);
            return RecvLen;
        }

        Request::Request(HttpMethod M, String Uri, SpConn Conn) : Method(M), OwningConn(Conn)
        {
            if (OwningConn)
            {
                switch (Method)
                {
                case HttpMethod::Get:
                {
                    String ReqStr = "GET";
                    ReqStr.AppendSprintf(" %s HTTP/1.1\r\n", *Uri);
                    ReqStr.AppendSprintf("Host: %s\r\n", *GetHost());
                    ReqStr.AppendSprintf("User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:57.0) Gecko/20100101 Firefox/57.0\r\n");
                    ReqStr.AppendSprintf("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n");
                    ReqStr.AppendSprintf("Accept-Language: en-US,en;q=0.5\r\n");
                    ReqStr.AppendSprintf("Accept-Encoding: gzip, deflate, br\r\n");
                    ReqStr.AppendSprintf("Connection: keep-alive\r\n");
                    ReqStr.AppendSprintf("Upgrade-Insecure-Requests: 1\r\n\r\n"); // double \r\n ends
                    uint64 SentLen = OwningConn->SendData(ReqStr);
                    SentLen++;
                    break;
                }
                case HttpMethod::Head:
                {
                    String ReqStr = "HEAD";
                    ReqStr.AppendSprintf(" %s HTTP/1.1\r\n", *Uri);
                    ReqStr.AppendSprintf("Host: %s\r\n", *GetHost());
                    uint64 SentLen = OwningConn->SendData(ReqStr);
                    break;
                }
                case HttpMethod::Post:
                {
                    break;
                }
                }
            }
        }

        const String&  Request::GetHost() const { return OwningConn->GetHost(); }

        class Client : public IHttpClient, public EnableSharedFromThis<Client>
        {
        public:
            Client() {}
            ~Client() {}

            Http::PtrConn   MakeConnection(String const& Host, bool WithSSL) override
            {
                if (WithSSL)
                {
                    return k3d::MakeShared<SSLConnection>(Host, SharedFromThis());
                }
                else
                {
                    return k3d::MakeShared<Connection>(Host, SharedFromThis());
                }
            }
        };

        Http::PtrResp Request::GetResponse(bool bOnlyGetResponseHeader)
        {
            Http::PtrResp FinalResp;
            auto Resp = k3d::MakeShared<Response>();
            FinalResp = Resp;
            if (OwningConn)
            {
                uint64 SzData = OwningConn->GetData(*Resp);
                
                switch (Resp->GetResult())
                {
                case HttpResult::Ok: // 200
                {
                    if (!bOnlyGetResponseHeader)
                    {
                        while (SzData = OwningConn->GetData(*Resp));
                    }
                    // Finished
                    break;
                }
                case HttpResult::Found: // Redirection
                {
                    String TargetUri = Resp->Header.HeaderFields["Location"];
                    String Protocol;
                    String Host;
                    int Port;
                    String File;
                    bool UseSSL;
                    SplitUri(TargetUri, Protocol, Host, Port, File, UseSSL);
                    SpClient HttpClient = OwningConn->GetClient();
                    auto NewConn = HttpClient->MakeConnection(Host, true);
                    auto HttpsReq = NewConn->MakeRequest(HttpMethod::Get, File);

                    FinalResp = HttpsReq->GetResponse(bOnlyGetResponseHeader);
                    break;
                }
                }
            }
            return FinalResp;
        }
    }

    class FHttpModuleImpl : public IHttpModule
    {
    public:
        FHttpModuleImpl() {}
        ~FHttpModuleImpl() {}

        Http::PtrClient     GetHttpClient() override { return GClient; }
        void                Start() override;
        void                Shutdown() override {}
        const char *        Name() override { return "Network"; }

    private:
        Http::PtrClient    GClient;
    };

    void FHttpModuleImpl::Start()
    {
        if (!GClient)
        {
            GClient = k3d::MakeShared<Http::Client>();
        }
    }
}

MODULE_IMPLEMENT(Network, Net::FHttpModuleImpl)