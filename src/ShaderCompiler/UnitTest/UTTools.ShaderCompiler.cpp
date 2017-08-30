#include <Core/Kaleido3D.h>
#include <Core/Os.h>
#include <Core/Module.h>
#include <Core/LogUtil.h>
#include <Core/Interface/ICrossShaderCompiler.h>
#include <iostream>

#if K3DPLATFORM_OS_WIN
#pragma comment(linker,"/subsystem:console")
#endif

using namespace std;
using namespace k3d;

void TestShaderCompiler()
{
    Os::MemMapFile vShFile;
    vShFile.Open("../../Data/Test/testCompiler.glsl", IOFlag::IORead);
    String vertexSource((const char*)vShFile.FileData());
    vShFile.Close();
        
	auto shMod = StaticPointerCast<k3d::IShModule>(GlobalModuleManager.FindModule("ShaderCompiler"));
	if (shMod)
	{
		// test compile
		SharedPtr<k3d::IShCompiler> vkCompiler = shMod->CreateShaderCompiler(NGFX_RHI_VULKAN);
		if (vkCompiler)
		{
			NGFXShaderDesc desc = { NGFX_SHADER_FORMAT_TEXT, NGFX_SHADER_LANG_GLSL, NGFX_SHADER_PROFILE_MODERN, NGFX_SHADER_TYPE_VERTEX, "main" };
			NGFXShaderBundle bundle;
			auto ret = vkCompiler->Compile(vertexSource, desc, bundle);
			K3D_ASSERT(ret == NGFX_SHADER_COMPILE_OK);

			// test shader serialize
			Os::File shBundle;
			shBundle.Open(KT("shaderbundle.sh"), IOWrite);
			Archive ar;
			ar.SetIODevice(&shBundle);
			ar << bundle;
			shBundle.Close();

			// write spirv to file
			Os::File spirvFile;
			spirvFile.Open(KT("triangle.spv"), IOWrite);
			spirvFile.Write(bundle.RawData.Data(), bundle.RawData.Length());
			spirvFile.Close();

			// test shader deserialize;
			Os::File shBundleRead;
			NGFXShaderBundle bundleRead;
			shBundleRead.Open(KT("shaderbundle.sh"), IORead);
			Archive readar;
			readar.SetIODevice(&shBundleRead);
			readar >> bundleRead;
			shBundleRead.Close();

			// test hlsl compile
			NGFXShaderBundle hlslBundle;
			Os::MemMapFile hlslVertFile;
			hlslVertFile.Open("../../Data/Test/TestMaterial.hlsl", IOFlag::IORead);
			NGFXShaderDesc hlsldesc = { NGFX_SHADER_FORMAT_TEXT, NGFX_SHADER_LANG_HLSL, NGFX_SHADER_PROFILE_MODERN, NGFX_SHADER_TYPE_VERTEX, "main" };
			auto hlslret = vkCompiler->Compile(String((const char*)hlslVertFile.FileData()), hlsldesc, hlslBundle);
			K3D_ASSERT(hlslret == NGFX_SHADER_COMPILE_OK);

			// test spirv reflect
			Os::MemMapFile spirvFileRead;
			NGFXShaderBundle spirvBundle;
			spirvFileRead.Open(KT("triangle.spv"), IORead);
			NGFXShaderDesc spirvDesc = { NGFX_SHADER_FORMAT_BYTE_CODE, NGFX_SHADER_LANG_GLSL, NGFX_SHADER_PROFILE_MODERN, NGFX_SHADER_TYPE_VERTEX, "main" };
			auto spirvRet = vkCompiler->Compile(String(spirvFileRead.FileData(), spirvFileRead.GetSize()), spirvDesc, spirvBundle);
			K3D_ASSERT(spirvRet == NGFX_SHADER_COMPILE_OK);
		}
            
        SharedPtr<k3d::IShCompiler> mtlCompiler = shMod->CreateShaderCompiler(NGFX_RHI_METAL);
        if(mtlCompiler)
        {
            NGFXShaderDesc desc = { NGFX_SHADER_FORMAT_TEXT, NGFX_SHADER_LANG_GLSL, NGFX_SHADER_PROFILE_MODERN, NGFX_SHADER_TYPE_VERTEX, "main" };
            NGFXShaderBundle bundle;
            auto ret = mtlCompiler->Compile(vertexSource, desc, bundle);
            K3D_ASSERT(ret == NGFX_SHADER_COMPILE_OK);
                
            // test shader serialize
            Os::File shBundle;
            shBundle.Open(KT("metalshaderbundle.sh"), IOWrite);
            Archive ar;
            ar.SetIODevice(&shBundle);
            ar << bundle;
            shBundle.Close();
        }
	}
}

int main(int argc, char**argv)
{
	TestShaderCompiler();
	return 0;
}