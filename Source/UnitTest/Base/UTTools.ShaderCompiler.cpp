#include "Common.h"
#include <Core/Module.h>
#include <iostream>

using namespace std;
using namespace k3d;

threadptr TestShaderCompiler()
{
	auto ret = std::make_shared<std::thread>([]() {
        
        Os::MemMapFile vShFile;
        vShFile.Open("../../Data/Test/testCompiler.glsl", IOFlag::IORead);
        String vertexSource((const char*)vShFile.FileData());
        vShFile.Close();
        
		rhi::IShModule* shMod = (rhi::IShModule*)GlobalModuleManager.FindModule("ShaderCompiler");
		if (shMod)
		{
			// test compile
			SharedPtr<rhi::IShCompiler> vkCompiler = shMod->CreateShaderCompiler(rhi::ERHI_Vulkan);
			if (vkCompiler)
			{
				rhi::ShaderDesc desc = { rhi::EShFmt_Text, rhi::EShLang_GLSL, rhi::EShProfile_Modern, rhi::ES_Vertex, "main" };
				rhi::ShaderBundle bundle;
				auto ret = vkCompiler->Compile(vertexSource, desc, bundle);
				K3D_ASSERT(ret == rhi::shc::E_Ok);

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
				rhi::ShaderBundle bundleRead;
				shBundleRead.Open(KT("shaderbundle.sh"), IORead);
				Archive readar;
				readar.SetIODevice(&shBundleRead);
				readar >> bundleRead;
				shBundleRead.Close();

				// test hlsl compile
				rhi::ShaderBundle hlslBundle;
				Os::MemMapFile hlslVertFile;
				hlslVertFile.Open("../../Data/Test/TestMaterial.hlsl", IOFlag::IORead);
				rhi::ShaderDesc hlsldesc = { rhi::EShFmt_Text, rhi::EShLang_HLSL, rhi::EShProfile_Modern, rhi::ES_Vertex, "main" };
				auto hlslret = vkCompiler->Compile(String((const char*)hlslVertFile.FileData()), hlsldesc, hlslBundle);
				K3D_ASSERT(hlslret == rhi::shc::E_Ok);

				// test spirv reflect
				Os::MemMapFile spirvFileRead;
				rhi::ShaderBundle spirvBundle;
				spirvFileRead.Open(KT("triangle.spv"), IORead);
				rhi::ShaderDesc spirvDesc = { rhi::EShFmt_ByteCode, rhi::EShLang_GLSL, rhi::EShProfile_Modern, rhi::ES_Vertex, "main" };
				auto spirvRet = vkCompiler->Compile(String(spirvFileRead.FileData(), spirvFileRead.GetSize()), spirvDesc, spirvBundle);
				K3D_ASSERT(spirvRet == rhi::shc::E_Ok);
			}
            
            SharedPtr<rhi::IShCompiler> mtlCompiler = shMod->CreateShaderCompiler(rhi::ERHI_Metal);
            if(mtlCompiler)
            {
                rhi::ShaderDesc desc = { rhi::EShFmt_Text, rhi::EShLang_GLSL, rhi::EShProfile_Modern, rhi::ES_Vertex, "main" };
                rhi::ShaderBundle bundle;
                auto ret = mtlCompiler->Compile(vertexSource, desc, bundle);
                K3D_ASSERT(ret == rhi::shc::E_Ok);
                
                // test shader serialize
                Os::File shBundle;
                shBundle.Open(KT("metalshaderbundle.sh"), IOWrite);
                Archive ar;
                ar.SetIODevice(&shBundle);
                ar << bundle;
                shBundle.Close();
            }
		}
	});
	return ret;
}
