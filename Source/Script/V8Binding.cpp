#include "Kaleido3D.h"
#include "V8Binding.h"
#include <Core/LogUtil.h>
#include <Core/AssetManager.h>
#include <RHI/Vulkan/VkCommon.h>
#include <RHI/Vulkan/Private/VkRHI.h>

using namespace k3d;

namespace v8
{
	ScriptEngine::ScriptEngine()
	{
		initializeWithExtData();
		initialize();
		KLOG(Info,"V8Script", "---------------- loaded....");
	}

	ScriptEngine::~ScriptEngine()
	{
		destroy();
	}

	Local<Value> ScriptEngine::ExecuteScript(const char * scripts)
	{
		Isolate::Scope isolate_scope(getIsoloate());
		HandleScope handle_scope(getIsoloate());
		Local<Context> context = Local<Context>::New(getIsoloate(), context_);
		Context::Scope context_scope(context);
		Local<Value> result;
		TryCatch try_catch(getIsoloate());
		Local<Script> compiled_script;
		if (!Script::Compile(context, String::NewFromUtf8(getIsoloate(), scripts)).ToLocal(&compiled_script)) {
			String::Utf8Value error(try_catch.Exception());
			return result;
		}
		// Run the script!
		auto val = compiled_script->Run(context);
		if (!val.ToLocal(&result)) {
			String::Utf8Value error(try_catch.Exception());
			return result;
		}
		String::Utf8Value ret(result);
		KLOG(Info, "V8Script", "---- ExecuteScript %s", *ret);
		return result;
	}

	void ScriptEngine::initializeWithExtData() {
		IAsset * natives_blob = AssetManager::Open("asset://Test/natives_blob.bin");
		uint64 natives_blobs_length = natives_blob->GetLength();
		StartupData ndata = { (const char*)natives_blob->GetBuffer(), (int)natives_blobs_length };
		V8::SetNativesDataBlob(&ndata);

		IAsset * snapshot_blob = AssetManager::Open("asset://Test/snapshot_blob.bin");
		uint64 snapshot_blob_length = snapshot_blob->GetLength();
		StartupData sdata = { (const char*)snapshot_blob->GetBuffer(), (int)snapshot_blob_length };
		V8::SetSnapshotDataBlob(&sdata);

		auto platform = platform::CreateDefaultPlatform();
		V8::InitializePlatform(platform);
		V8::Initialize();
	}


	static void GetCurrentDeviceInfo(const FunctionCallbackInfo<Value>& args) 
	{
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(),"do you know ?", NewStringType::kNormal).ToLocalChecked());
	}

	void ScriptEngine::initialize() {
		Isolate::CreateParams create_params;
		create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
		auto isolate = Isolate::New(create_params);
		isolate_ = isolate;
		isolate_->Enter();
		HandleScope handleScope(isolate);
		global_template_ = ObjectTemplate::New(isolate);
		global_template_->Set(String::NewFromUtf8(isolate, "GetVkCurrentDevice"), FunctionTemplate::New(isolate, GetCurrentDeviceInfo));
		Local<Context> context = Context::New(isolate, nullptr, global_template_);
		context_.Reset(isolate, context);
	}

	void ScriptEngine::destroy() {
		//isolate_->Exit();
		//isolate_->Dispose();
		V8::Dispose();
		V8::ShutdownPlatform();
	}

	V8Module::V8Module() : m_Inst(nullptr)
	{

	}

	V8Module::~V8Module()
	{
		
	}

	void V8Module::Start()
	{
		if (m_Inst == nullptr)
		{
			m_Inst = new ScriptEngine;
		}
	}

	void V8Module::Shutdown()
	{
		if (m_Inst) 
		{
			delete m_Inst;
			m_Inst = nullptr;
		}
	}

	void V8Module::ExecuteScript(const char * script)
	{
		auto ret = m_Inst->ExecuteScript(script);
	}

	const char * V8Module::Name()
	{
		return "V8Script";
	}

}

MODULE_IMPLEMENT(V8Script, v8::V8Module)
