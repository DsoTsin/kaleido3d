#include "Kaleido3D.h"
#include <Core/App.h>

#include <Windows.ApplicationModel.h>
#include <Windows.ApplicationModel.Core.h>
#include <Windows.Graphics.Display.h>

#include <wrl/client.h>
#include <wrl/event.h>
#include <wrl/wrappers/corewrappers.h>

#include <ppl.h>
#include <ppltasks.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::ApplicationModel;
using namespace ABI::Windows::ApplicationModel::Activation;
using namespace ABI::Windows::ApplicationModel::Core;
using namespace ABI::Windows::Graphics::Display;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::UI::Input;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace concurrency;

namespace k3d
{
    /*
    class UWPAppView : public RuntimeClass<ABI::Windows::ApplicationModel::Core::IFrameworkView>
    {
    public:
        InspectableClass(L"k3d.UWPAppView", BaseTrust);

        HRESULT RuntimeClassInitialize(App* app);
        IFACEMETHOD(Initialize)(ABI::Windows::ApplicationModel::Core::ICoreApplicationView* appView);
        IFACEMETHOD(SetWindow)(ABI::Windows::UI::Core::ICoreWindow* window);
        IFACEMETHOD(Load)(HSTRING entryPoint);
        IFACEMETHOD(Run)();
        IFACEMETHOD(Uninitialize)();
    };
    class UWPAppSource : public RuntimeClass<ABI::Windows::ApplicationModel::Core::IFrameworkViewSource, FtmBase>
    {
        friend class App;
    public:
        InspectableClass(L"k3d.UWPAppSource", BaseTrust);
        HRESULT RuntimeClassInitialize(App* app)
        {
            m_App = app;
            return S_OK;
        }
        IFACEMETHOD(CreateView)(ABI::Windows::ApplicationModel::Core::IFrameworkView** frameworkView)
        {
            return MakeAndInitialize<UWPAppView>(frameworkView, m_App);
        }
        App* m_App;
    };*/
}