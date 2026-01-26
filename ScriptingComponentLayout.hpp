#pragma once
#include <string>
#include <sstream>
#include <mono/jit/jit.h>

class ScriptingComponentLayout
{
public:
	ScriptingComponentLayout(MonoClass* mono_class, std::string& class_namespace, std::string& class_name, MonoMethod* start_method = nullptr, MonoMethod* update_method = nullptr) 
		: mono_class(mono_class)
		, class_namespace(class_namespace)
		, class_name(class_name)
		, start_method(start_method)
		, update_method(update_method)
	{
	}

	void Start(MonoObject* classInstance)
	{
		mono_runtime_invoke(start_method, classInstance, NULL, NULL);
	}

	void Update(MonoObject* classInstance, float deltaTime)
	{
		void* args[1];
		args[0] = &deltaTime;

		mono_runtime_invoke(update_method, classInstance, args, NULL);
	}

	std::string GetDescription() const
	{
		std::ostringstream ss;
		ss << "Gathered component class (namespace=" << class_namespace
			<< "), (name=" << class_name
			<< "), (startExist=" << (start_method != nullptr)
			<< "), (updateExist=" << (update_method != nullptr) << ")";
		return ss.str();
	}

	MonoClass* mono_class;

	MonoMethod* start_method;
	MonoMethod* update_method;

	std::string class_name;
	std::string class_namespace;
};