#pragma once
#include <Core/Module.h>
#include <unordered_map>
#include "v8.h"
#include "libplatform/libplatform.h"

namespace v8
{
	class BaseObjectWrap
	{
	private:
		using ExternalMap = std::unordered_map<int, Persistent<External>*>;
		ExternalMap		externals_;
		Isolate *		isolate_;

	public:
		BaseObjectWrap()
		{}

		~BaseObjectWrap()
		{}

		template <class T>
		T * Unwrap(const FunctionCallbackInfo<Value>& args, int fieldId)
		{
			auto self = args.Holder();
			auto i = Local<External>::Cast(self->GetInternalField(fieldId));
			T* ptr = reinterpret_cast<T*>(i->Value());
			return ptr;
		}

		template <class T>
		void Wrap(const FunctionCallbackInfo<Value>& args, int fieldId, T * ptr) {
			Local<Object> object = args.This();
			auto ext = External::New(isolate_, ptr);
			externals_[fieldId] = new Persistent<External>;
			externals_[fieldId]->Reset(isolate_, ext);
			//external_.SetWeak(point, )
			object->SetInternalField(fieldId, ext);
		}
	};

	class ScriptEngine
	{
	private:
		Isolate*                isolate_;
		Global<Context>         context_;
		Local<ObjectTemplate>   global_template_;
		bool                    initialized_;

	public:
		ScriptEngine();
		~ScriptEngine();

		Local<Value> ExecuteScript(const char * scripts);

		Isolate* getIsoloate() const { return isolate_; }

	private:
		void initializeWithExtData();

		void initialize();
		void destroy();

		Context::Scope *        context_scope_;
		Persistent<Context> *   primary_context_;
	};

	class V8ModuleFacade : public ::k3d::IModule
	{
	public:
		virtual void ExecuteScript(const char * script) = 0;
	};

	class V8Module : public V8ModuleFacade
	{
	public:
		V8Module();
		~V8Module() override;
		void Start() override;
		void Shutdown() override;
		virtual void ExecuteScript(const char * script);
		const char * Name() override;
	private:
		ScriptEngine * m_Inst;
	};
}
