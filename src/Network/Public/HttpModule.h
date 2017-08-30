#pragma once

namespace Net
{
    using String = k3d::String;

    struct IHttpHeader
    {
        virtual IHttpHeader& AddCookie(String const& Cookie) = 0;
        virtual IHttpHeader& AddHeader(String const& Key, String const& Value) = 0;
        virtual ~IHttpHeader() {}
    };
    struct IHttpResponse;
    struct IHttpRequest;
    struct IHttpConnection;
    struct IHttpClient;

    namespace Http
    {
        using PtrHeader = k3d::SharedPtr<IHttpHeader>;
        using PtrResp = k3d::SharedPtr<IHttpResponse>;
        using PtrReq = k3d::SharedPtr<IHttpRequest>;
        using PtrConn = k3d::SharedPtr<IHttpConnection>;
        using PtrClient = k3d::SharedPtr<IHttpClient>;
    }

    enum class HttpMethod : uint32
    {
        Post,
        Get,
        Head,
        Put,
        Option
    };

    enum HttpResult : uint32
    {
        /* Successful */
        Ok                  = 200, // 200
        Created             = 201, // 201
        Accepted            = 202, // 202
        ResetContent        = 205, // 205
        PartialContent      = 206, // 206

        /* Redirections */
        MultipleChoices     = 300, // 300
        MovedPermanently    = 301, // 301
        Found               = 302, // 302

        /* Client Error */
        BadRequest          = 400, // 400
        UnAuthorized        = 401, // 401
        Forbidden           = 403, // 403
        NotFound            = 404, // 404

        /* Server Error */
        InternalSrvErr      = 500, // 500
        BadGateway          = 502, // 502
        ServiceUnAvailable  = 503, // 503
        GatewayTimeout      = 504, // 504
    };

    struct IHttpStream
    {
        virtual ~IHttpStream() {}
        virtual void Write(const void* Data, uint64 SzData) = 0;
        virtual const void* Data() const = 0;
        virtual uint64 Size() const = 0;
    };

    struct IHttpResponse
    {
        virtual ~IHttpResponse() {}
        virtual void SetStream(IHttpStream* pHttpStream) {}
        virtual HttpResult GetResult() const = 0;
        virtual String const& GetData() const = 0;
    };

    struct IHttpRequest
    {
        virtual ~IHttpRequest() {}

        virtual const String&   GetHost() const = 0;
        virtual HttpMethod      GetMethod() const = 0;

        /* Response */
        virtual Http::PtrResp   GetResponse(bool bOnlyGetResponseHeader = false) = 0;
    };
    // User interface
    struct INetSpeedNotifier
    {
        virtual void UpdateCurrentSpeed(float KBPersecond) = 0;
    };

    struct IHttpConnection
    {
        virtual ~IHttpConnection() {}
        virtual void            InstallSpeedNotifier(INetSpeedNotifier* Notifier) = 0;
        virtual void            SetUserAgent(String const& UA) {}
        virtual Http::PtrReq    MakeRequest(HttpMethod Method, String const& UriPath) = 0;
        virtual Http::PtrHeader MakeHeader() = 0;
    };

    struct IHttpClient
    {
        virtual ~IHttpClient() {}

        virtual Http::PtrConn   MakeConnection(String const& Host, bool WithSSL = false) = 0;
    };

    class IHttpModule : public k3d::IModule
    {
    public:
        virtual Http::PtrClient GetHttpClient() = 0;
    };
}