#include "CoreMinimal.h"

#include <errno.h>
#define WIN32_NO_STATUS
#include <windows.h>
#include <WbemCli.h>
#include <comutil.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "comsuppw.lib")

static IWbemServices    *s_service;
static IWbemLocator     *s_locator;
static UINT64           *s_old_time_stamp;
static UINT64           *s_old_pproc_usage;
static UINT             s_cores_count;
static float            *s_cores_load_data;

static float *__GetCpuUsage(void)
{
	HRESULT hr = NULL;
	ULONG retVal;
	UINT i;

	IWbemClassObject *pclassObj;
	IEnumWbemClassObject *pEnumerator;

	hr = s_service->ExecQuery(bstr_t("WQL"), bstr_t("SELECT TimeStamp_Sys100NS, PercentProcessorTime, Frequency_PerfTime FROM Win32_PerfRawData_PerfOS_Processor"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
	for (i = 0; i < s_cores_count; i++) {
		hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclassObj, &retVal);
		if (!retVal) {
			break;
		}

		VARIANT vtPropTime;
		VARIANT vtPropClock;
		VariantInit(&vtPropTime);
		VariantInit(&vtPropClock);

		hr = pclassObj->Get(L"TimeStamp_Sys100NS", 0, &vtPropTime, 0, 0);
		UINT64 newTimeStamp = _wtoi64(vtPropTime.bstrVal);

		hr = pclassObj->Get(L"PercentProcessorTime", 0, &vtPropClock, 0, 0);
		UINT64 newPProcUsage = _wtoi64(vtPropClock.bstrVal);

		s_cores_load_data[i] = (float)(1.0 - (((double)newPProcUsage - (double)s_old_pproc_usage[i]) / ((double)newTimeStamp - (double)s_old_time_stamp[i]))) * 100.0f;

		if (s_cores_load_data[i] < 0) s_cores_load_data[i] = 0.0;
		else if (s_cores_load_data[i] > 100.0) s_cores_load_data[i] = 100.0;

		s_old_pproc_usage[i] = newPProcUsage;
		s_old_time_stamp[i] = newTimeStamp;

		VariantClear(&vtPropTime);
		VariantClear(&vtPropClock);

		pclassObj->Release();
	}

	pEnumerator->Release();
	return &s_cores_load_data[0];
}

int __Init(void)
{
	IWbemClassObject *pclassObj;
	IEnumWbemClassObject *pEnumerator;
	HRESULT hr;
	ULONG retVal;

	s_service = NULL;
	s_cores_count = 0;

	s_old_time_stamp = NULL;
	s_old_pproc_usage = NULL;
	s_locator = NULL;

	s_cores_load_data = NULL;

	CoInitializeEx(0, COINIT_MULTITHREADED);
	CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

	hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&s_locator);
	if (FAILED(hr)) {
		return 0;
	}
	hr = s_locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &s_service);
	if (FAILED(hr)) {
		return 0;
	}

	CoSetProxyBlanket(s_service, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

	hr = s_service->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_Processor"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
	hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclassObj, &retVal);
	if (retVal) {
		VARIANT vtProp;
		VariantInit(&vtProp);
		hr = pclassObj->Get(L"NumberOfLogicalProcessors", 0, &vtProp, 0, 0);
		s_cores_count = vtProp.uintVal;

		if (s_cores_count) {
			s_old_time_stamp = new UINT64[s_cores_count];
			s_old_pproc_usage = new UINT64[s_cores_count];
			s_cores_load_data = new float[s_cores_count];
			float zeroFloat = 0.0;
			memset(s_cores_load_data, *(int*)&zeroFloat, sizeof(float)*s_cores_count);
		}

		VariantClear(&vtProp);
	}

	pclassObj->Release();
	pEnumerator->Release();

	// warm-up query engine
	for (int i = 0; i < 16; ++i) {
		__GetCpuUsage();
	}
	return 1;
}

namespace GPUPerf
{
  namespace NV
  {
#define NVAPI_MAX_PHYSICAL_GPUS   64
#define NVAPI_MAX_USAGES_PER_GPU  34
    bool inited = false;
    HMODULE nvapi = NULL;
    typedef int *(*NvAPI_QueryInterface_t)(unsigned int offset);
    typedef int(*NvAPI_Initialize_t)();
    typedef int(*NvAPI_EnumPhysicalGPUs_t)(int **handles, int *count);
    typedef int(*NvAPI_GPU_GetUsages_t)(int *handle, unsigned int *usages);

    NvAPI_QueryInterface_t      NvAPI_QueryInterface = NULL;
    NvAPI_Initialize_t          NvAPI_Initialize = NULL;
    NvAPI_EnumPhysicalGPUs_t    NvAPI_EnumPhysicalGPUs = NULL;
    NvAPI_GPU_GetUsages_t       NvAPI_GPU_GetUsages = NULL;

    int                         NvGpuCount = 0;
    int*                        NvGpuHandles[NVAPI_MAX_PHYSICAL_GPUS] = { NULL };
    unsigned int                NvGpuUsages[NVAPI_MAX_USAGES_PER_GPU] = { 0 };
    float                       NvGpuUsagesPercents[NVAPI_MAX_USAGES_PER_GPU] = { 0 };

    void Init()
    {
      nvapi = LoadLibrary("nvapi64.dll");
      if (nvapi == NULL)
      {
        return;
      }
      NvAPI_QueryInterface = (NvAPI_QueryInterface_t)GetProcAddress(nvapi, "nvapi_QueryInterface");
      // some useful internal functions that aren't exported by nvapi.dll
      NvAPI_Initialize = (NvAPI_Initialize_t)(*NvAPI_QueryInterface)(0x0150E828);
      NvAPI_EnumPhysicalGPUs = (NvAPI_EnumPhysicalGPUs_t)(*NvAPI_QueryInterface)(0xE5AC921F);
      NvAPI_GPU_GetUsages = (NvAPI_GPU_GetUsages_t)(*NvAPI_QueryInterface)(0x189A1FDF);
      if (NvAPI_Initialize == NULL || NvAPI_EnumPhysicalGPUs == NULL ||
        NvAPI_EnumPhysicalGPUs == NULL || NvAPI_GPU_GetUsages == NULL)
      {
        return;
      }
      (*NvAPI_Initialize)();

      // gpuUsages[0] must be this value, otherwise NvAPI_GPU_GetUsages won't work
      NvGpuUsages[0] = (NVAPI_MAX_USAGES_PER_GPU * 4) | 0x10000;

      (*NvAPI_EnumPhysicalGPUs)(NvGpuHandles, &NvGpuCount);
      inited = true;
    }

    int GetGpuCount()
    {
        if (!inited)
        {
            Init();
        }
        return NvGpuCount;
    }

    float GetGpuUsage(int Id)
    {
      if(!inited)
      {
        Init();
      }
      if(nvapi)
      {
        (*NvAPI_GPU_GetUsages)(NvGpuHandles[Id], NvGpuUsages);
        NvGpuUsagesPercents[0] = 1.0f * NvGpuUsages[3] / 100;
      }
      return NvGpuUsagesPercents[0];
    }

  }
}


namespace k3d
{
namespace os
{
float* GetCpuUsage()
{
	if (!s_service)
	{
		__Init();
	}
	return __GetCpuUsage();
}

U32
GetGpuCount()
{
  return GPUPerf::NV::GetGpuCount();
}

float GetGpuUsage(int Id)
{
  return GPUPerf::NV::GetGpuUsage(Id);
}
}
}