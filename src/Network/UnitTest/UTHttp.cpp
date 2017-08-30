#include <Network.h>
#include <HttpModule.h>

#if K3DPLATFORM_OS_WIN
#pragma comment(linker,"/subsystem:console")
#endif

using namespace k3d;
using namespace Net;

struct IOStream : public IHttpStream
{
    void Write(const void* Data, uint64 SzData) override;
    const void* Data() const override;
    uint64 Size() const override;

    IOStream();
};

int main(int argc, const char* argv[])
{
    auto HttpModule = StaticPointerCast<IHttpModule>( ACQUIRE_PLUGIN(Network) );
    HttpModule->Start();
    Http::PtrClient HttpClient = HttpModule->GetHttpClient();
    Http::PtrResp Response;
    if (HttpClient)
    {
        auto HttpConnection = HttpClient->MakeConnection("ci.appveyor.com");
        auto HttpRequest = HttpConnection->MakeRequest(HttpMethod::Get, "/api/buildjobs/k993nt84qj16ykhl/artifacts/third_party_windows.zip");
        auto HttpResponse = HttpRequest->GetResponse(true);
        HttpResponse->GetData();
    }

    return 0;
}